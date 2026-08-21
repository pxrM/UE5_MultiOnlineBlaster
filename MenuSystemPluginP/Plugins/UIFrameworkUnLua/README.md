# UIFramework UnLua Adapter

This optional plugin keeps UnLua out of `UIFrameworkCore` and
`UIFrameworkWidgets`. Disable or replace it without changing the UI manager,
layer stack, cache, or lifecycle contracts.

## Create a Lua-backed screen

Two mechanisms, one per widget class. Pick one — a class that declares a static
module ignores the registry field.

### A. Registry-driven binding (no reparenting)

1. Leave the Widget Blueprint on any UIFramework screen base that does **not**
   implement `IUnLuaInterface` — `UUIActivatableScreenBase` is the usual choice.
2. Copy `Content/Script/UIFramework/ScreenTemplate.lua` to
   `Content/Script/UI/MainMenu.lua` and implement the screen behavior.
3. Set `ScriptModule` on the registry entry:

```lua
["UI.Menu"] = {
    WidgetClass  = "/Game/Blueprint/UI/WBP_MainMenu.WBP_MainMenu_C",
    ScriptModule = "UI.MainMenu",
    Layer        = "FullWindow",
    CachePolicy  = "KeepPersistent",
}
```

`ScriptModule` also exists on the Registry DataAsset, so this works without the
Lua table registry.

The manager binds the module right after the widget is constructed and before it
is pushed onto a layer, so the Lua `Initialize` sees populated `BindWidget`
members but still runs ahead of `NativeConstruct`.

Because UnLua binds a module to a `UClass`, **two registry keys must not map the
same `WidgetClass` to different `ScriptModule`s**. Both the Lua validator and
`RegisterRuntimeEntry` reject that. Two keys sharing one class and one module is
fine.

Cached instances (`KeepUntilIdle` and friends) are bound once at creation and
keep the binding across reuse. Re-registering a key drops its cache, so a changed
`ScriptModule` takes effect on the next open.

### B. Static binding on the class

1. Create a Widget Blueprint derived from `UIUnLuaScreenBase`.
2. Set **Lua Module Name** in class defaults, for example `UI.MainMenu`.
3. Copy `Content/Script/UIFramework/ScreenTemplate.lua` to
   `Content/Script/UI/MainMenu.lua` and implement the screen behavior.
4. Register the Widget Blueprint in the UIFramework widget registry as usual.
   Leave `ScriptModule` empty.

### The Lua module

```lua
local M = UnLua.Class()

function M:Initialize(Initializer)
end

function M:OnUIOpening(Context)
    local payload = Context.Payload
end

function M:OnUIOpened(Context)
end

function M:OnUIActivated()
end

function M:OnUIDeactivated()
end

function M:OnUIClosing(Context)
end

function M:OnUIClosed(Context)
end

function M:OnUIRestoredFromCache(Context)
end

return M
```

Call global UI operations through `UUIScriptBridgeSubsystem`. This preserves
UIFramework lifecycle, caching, input routing, and scene-transition behavior.
The bridge and its dynamic delegates are reflected and can be used directly by
UnLua; no Lua-specific calls are required in the core framework.

Binding gives the module lifecycle callbacks and widget events. Driving MVVM data
bindings from Lua needs the view-model helper below.

Do not call UnLua `Release()` from `OnUIClosed`: UIFramework may retain the
screen in its closed-instance cache. Release script-owned references from the
normal widget destruction path, or disable caching for that registry entry.

## Driving MVVM bindings from Lua

UnLua writes `UPROPERTY`s by copying memory. It never calls the setter, so MVVM
never learns the field changed and bound widgets keep showing the old value — with
no error. A raw `vm.Score = 5` therefore does nothing visible.

`UUIViewModelBase` exposes a script-facing write path that broadcasts:

```lua
local vm = self:GetViewModel()
vm:SetFieldInt("Score", 100)
vm:SetFieldText("Label", NSLOCTEXT("Game", "Ready", "Ready"))
```

`SetFieldString` / `SetFieldName` / `SetFieldInt` / `SetFieldInt64` /
`SetFieldFloat` / `SetFieldBool` / `SetFieldObject` / `SetFieldText` all return
true only when the value actually changed. They refuse a name that is not a
`FieldNotify` field, and refuse a type mismatch instead of reinterpreting memory.
`SetFieldFloat` accepts both `float` and `double` properties.

Already wrote the property some other way, or batched several writes? Broadcast
explicitly with `vm:NotifyFieldChanged("Score")`. `vm:HasNotifyField(name)` and
`vm:GetNotifyFieldNames()` cover introspection.

For table-like syntax, wrap the view model:

```lua
local ViewModel = require "UIFramework.ViewModel"

function M:OnUIOpening(Context)
    self.VM = ViewModel.Wrap(self:GetViewModel())
    self.VM.Score = 100        -- broadcasts
    self.VM.Note = "hello"     -- broadcasts
    print(self.VM.Score)       -- reads through to the UObject
end
```

`ViewModel.SetAll(vm, { Score = 1, Note = "x" })` assigns a batch and returns the
names it could not write.

Fields must be declared `FieldNotify` on a C++ or Blueprint subclass of
`UUIViewModelBase`. Lua cannot add fields — field *values* are dynamic, the field
*set* is not.

A screen can also supply its own view model now: `CreateViewModel` is a
`BlueprintNativeEvent`, so a bound Lua module can override it.

## How binding is wired

`UIFrameworkWidgets` owns a runtime-neutral seam, `IUIScriptClassBinder`
(`Public/Scripting/UIScriptClassBinder.h`). This plugin registers
`FUIUnLuaClassBinder` into it on `StartupModule`. With the plugin disabled the
seam is empty, `ScriptModule` entries log a warning once each, and everything
else behaves exactly as before.

## Lua table registry

For fast iteration, a game-instance-local registry can be defined entirely in
Lua. GameplayTag names must still exist in `DefaultGameplayTags.ini`, but no
Registry DataAsset editing is required:

```lua
-- Content/Script/UIFramework/UIConfig.lua
local UIRegistry = require "UIFramework.Registry"

local UIConfig = {}

local Definitions = {
    ["UI.Menu"] = {
        WidgetClass = "/Game/Blueprint/UI/WBP_MainMenu.WBP_MainMenu_C",
        ScriptModule = "UI.MainMenu",
        Layer = "FullWindow",
        CachePolicy = "KeepPersistent",
    },
    ["UI.Test"] = {
        WidgetClass = "/Game/Blueprint/UI/WBP_Toast.WBP_Toast_C",
        ScriptModule = "UI.Toast",
        Layer = "Notification",
        CachePolicy = "KeepUntilIdle",
        IdleTimeoutSeconds = 10.0,
        AllowMultiple = true,
        BlocksInput = false,
        HandlesBack = false,
        MaxOpenInstances = 5,
    },
}

function UIConfig.Install(worldContext)
    local ok, errors = UIRegistry.Replace(worldContext, Definitions)
    if not ok then
        for _, message in ipairs(errors) do
            UE.UKismetSystemLibrary.PrintString(worldContext, message, true, true)
        end
    end
    return ok, errors
end

return UIConfig
```

`Registry` needs a world context to reach `UUIScriptBridgeSubsystem`, so
`Install` must be called from a UnLua class that has one — typically the
GameInstance script:

```lua
function M:Initialize(...)
    require("UIFramework.UIConfig").Install(self)
end
```

Runtime entries override matching DataAsset entries and affect subsequent opens.
Open widgets keep their original entry snapshot. Calling `Apply` again replaces
entries and invalidates only the affected closed-widget/class caches, which makes
it suitable for UnLua hot reload. `UIRegistry.Remove` reveals the DataAsset
fallback again; `UIRegistry.Clear` removes every runtime override.

Use `Replace` when the Lua table represents the complete runtime registry: keys
removed from the table are removed from the runtime overlay too. Use `Apply` for
incremental additions or overrides.

Both entry points validate every definition before committing any of them, so a
malformed table returns `false` plus the error list and leaves the existing
overlay untouched. `UIRegistry.Validate(definitions)` runs that same check on its
own — pure Lua, no world context, no bridge — which makes it usable as a dry run
or a startup assertion.

Layer and cache-policy names come from `UIRegistry.Layers` and
`UIRegistry.CachePolicies`; omitted fields fall back to `UIRegistry.Defaults`,
which mirrors the `FUIWidgetEntry` struct defaults.
