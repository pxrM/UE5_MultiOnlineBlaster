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
        MaxCachedInstances = 8,
    },
}

local ok, errors = UIRegistry.Replace(self, Definitions)
if not ok then
    for _, message in ipairs(errors) do
        print(message)
    end
end