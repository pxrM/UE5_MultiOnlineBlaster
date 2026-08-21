---Template for a screen bound through a registry ScriptModule field.
---
---Copy to Content/Script/UI/<Name>.lua and point a registry entry at it:
---
---    ["UI.Test"] = {
---        WidgetClass  = "/Game/Blueprint/UI/WBP_TestScreen.WBP_TestScreen_C",
---        ScriptModule = "UI.TestScreen",
---    }
---
---The widget blueprint needs no reparenting and no LuaModuleName; it only has to
---derive from a UIFramework screen base that does NOT implement IUnLuaInterface
---(UUIActivatableScreenBase is the usual choice). A class that carries its own
---static module through UUIUnLuaScreenBase ignores ScriptModule.
local M = UnLua.Class()

---Called by UnLua right after binding. The widget tree exists here, so BindWidget
---members are already reachable, but NativeConstruct has not run yet.
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
    -- Do not call Release here. A closed screen may still be retained by the
    -- UIFramework widget cache and activated again later.
end

function M:OnUIRestoredFromCache(Context)
end

return M
