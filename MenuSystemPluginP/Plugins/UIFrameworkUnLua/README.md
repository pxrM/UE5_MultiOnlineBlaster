# UIFramework UnLua Adapter

This optional plugin keeps UnLua out of `UIFrameworkCore` and
`UIFrameworkWidgets`. Disable or replace it without changing the UI manager,
layer stack, cache, or lifecycle contracts.

## Create a Lua-backed screen

1. Create a Widget Blueprint derived from `UIUnLuaScreenBase`.
2. Set **Lua Module Name** in class defaults, for example `UI.MainMenu`.
3. Copy `Content/Script/UIFramework/ScreenTemplate.lua` to
   `Content/Script/UI/MainMenu.lua` and implement the screen behavior.
4. Register the Widget Blueprint in the UIFramework widget registry as usual.

```lua
local M = UnLua.Class()

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

return M
```

Call global UI operations through `UUIScriptBridgeSubsystem`. This preserves
UIFramework lifecycle, caching, input routing, and scene-transition behavior.
The bridge and its dynamic delegates are reflected and can be used directly by
UnLua; no Lua-specific calls are required in the core framework.

Do not call UnLua `Release()` from `OnUIClosed`: UIFramework may retain the
screen in its closed-instance cache. Release script-owned references from the
normal widget destruction path, or disable caching for that registry entry.

## Lua table registry

For fast iteration, a game-instance-local registry can be defined entirely in
Lua. GameplayTag names must still exist in `DefaultGameplayTags.ini`, but no
Registry DataAsset editing is required:

```lua
local UIRegistry = require "UIFramework.Registry"

local Definitions = {
    ["UI.Menu"] = {
        WidgetClass = "/Game/Blueprint/UI/WBP_MainMenu.WBP_MainMenu_C",
        Layer = "FullWindow",
        CachePolicy = "KeepPersistent",
    },
    ["UI.Test"] = {
        WidgetClass = "/Game/Blueprint/UI/WBP_Toast.WBP_Toast_C",
        Layer = "Notification",
        CachePolicy = "KeepUntilIdle",
        IdleTimeoutSeconds = 10.0,
        AllowMultiple = true,
        BlocksInput = false,
        HandlesBack = false,
        MaxOpenInstances = 5,
    },
}

local ok, errors = UIRegistry.Replace(self, Definitions)
if not ok then
    for _, message in ipairs(errors) do
        print(message)
    end
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
