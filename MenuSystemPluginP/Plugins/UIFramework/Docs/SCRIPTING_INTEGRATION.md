# UIFramework Scripting Integration

UIFramework exposes a runtime-neutral scripting facade. UnLua, Puerts, and other
language plugins should bind this facade instead of accessing layer stacks or caches.

## Entry Point

Use `UUIScriptBridgeSubsystem`, one instance per game instance:

```text
Get(WorldContext)
OpenUI(Key, Payload)
OpenUIAsync(Key, Payload) -> UUIScriptAsyncRequest
CloseUI(Key, Result)
CloseWidget(Widget, Result)
ForceCloseAllUI()
HandleBackAction()
ValidateConfiguration(Errors)
```

The bridge forwards all state changes through reflection-friendly dynamic delegates:

```text
OnWidgetOpening
OnWidgetOpened
OnWidgetClosing
OnWidgetClosed
OnWidgetActivationChanged
```

## Payloads

`UUIScriptPayload` is the neutral transport object. A script adapter may subclass it,
store serialized data in `Json`, and retain named Unreal objects in `ObjectReferences`.
Do not pass raw Lua tables or JavaScript objects into the manager because Unreal GC
cannot track their lifetime.

## Asynchronous Opens

`OpenUIAsync` returns a `UUIScriptAsyncRequest`. Keep this object in the script layer
until it completes or is cancelled. Completion is deferred to the game thread so a
listener can be bound immediately after the call even when the UI is already loaded.

Request states are `Pending`, `Succeeded`, `Failed`, and `Cancelled`. Cancelling one
request only suppresses that listener; a shared manager load remains active for other
callers waiting on the same UI key.

## Adapter Boundary

A concrete UnLua or Puerts plugin module should depend on `UIFrameworkWidgets` and its
script runtime, then translate language-native promises/events to the bridge API.
UIFrameworkCore and UIFrameworkWidgets must not depend on UnLua or Puerts.

The project integration for UnLua lives in the separate `UIFrameworkUnLua` plugin.
Its `UUIUnLuaScreenBase` implements `IUnLuaInterface`; derive a Widget Blueprint
from it and set `LuaModuleName` to bind UI lifecycle functions from Lua. See the
adapter README and `Content/Script/UIFramework/ScreenTemplate.lua` for the template.

For development-time registry configuration, require `UIFramework.Registry` and
call `Replace(WorldContext, Definitions)` for a complete registry, or `Apply` for
incremental overrides. Its Lua table is stored as a per-game-
instance runtime overlay; it takes priority over the Registry DataAsset without
mutating packaged assets. Runtime changes affect future opens while live widgets
continue using their immutable open-time entry snapshots.

Script code must not mutate `UUILayerStack`, `UUIWidgetCache`, or manager tracking maps.
All opens and closes go through `UUIScriptBridgeSubsystem` so lifecycle, cache policy,
input activation, scene transitions, and back handling stay consistent.
