---Runtime UI registry overlay.
---
---Entries here override matching DA_UIRegistry entries for the current game
---instance and affect subsequent opens. Requires the GameplayTag to already
---exist in DefaultGameplayTags.ini.
---
---Call from a UnLua class that has a world context, for example a GameInstance
---script's Initialize:
---
---    local UIConfig = require "UIFramework.UIConfig"
---    UIConfig.Install(self)
---
local UIRegistry = require "UIFramework.Registry"

local UIConfig = {}

local Definitions = {
    ["UI.Test"] = {
        WidgetClass = "/Game/Blueprint/UI/WBP_TestScreen.WBP_TestScreen_C",
        Layer = "FullWindow",
        CachePolicy = "KeepUntilIdle",
        IdleTimeoutSeconds = 10.0,

        -- Binds Content/Script/UI/TestScreen.lua to the instance at open time. No
        -- reparenting and no LuaModuleName needed on the widget blueprint. Copy
        -- UIFramework/ScreenTemplate.lua to that path before enabling.
        -- ScriptModule = "UI.TestScreen",
    },

    -- Template for a transient, non-blocking, stackable notification. Point
    -- WidgetClass at a real toast WBP before enabling.
    -- ["UI.Toast"] = {
    --     WidgetClass = "/Game/Blueprint/UI/WBP_Toast.WBP_Toast_C",
    --     ScriptModule = "UI.Toast",
    --     Layer = "Notification",
    --     CachePolicy = "KeepUntilIdle",
    --     IdleTimeoutSeconds = 10.0,
    --     AllowMultiple = true,
    --     BlocksInput = false,
    --     HandlesBack = false,
    --     MaxOpenInstances = 5,
    --     MaxCachedInstances = 8,
    -- },
}

UIConfig.Definitions = Definitions

---Replace the whole runtime overlay with Definitions. Keys absent from the
---table are removed from the overlay, revealing their DataAsset entry again.
---@param worldContext UObject any object with a world, e.g. the calling UObject
---@return boolean success
---@return table errors
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
