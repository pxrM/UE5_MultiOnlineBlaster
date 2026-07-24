local Registry = {}

local Layers = {
    Background = UE.EUILayer.Background,
    Dock = UE.EUILayer.Dock,
    FullWindow = UE.EUILayer.FullWindow,
    PopupWindow = UE.EUILayer.PopupWindow,
    Notification = UE.EUILayer.Notification,
    Tips = UE.EUILayer.Tips,
}

local CachePolicies = {
    Transient = UE.EUICachePolicy.Transient,
    CacheClass = UE.EUICachePolicy.CacheClass,
    KeepUntilIdle = UE.EUICachePolicy.KeepUntilIdle,
    KeepUntilSceneChange = UE.EUICachePolicy.KeepUntilSceneChange,
    KeepPersistent = UE.EUICachePolicy.KeepPersistent,
}

local function value(config, name, default)
    local result = config[name]
    if result == nil then
        return default
    end
    return result
end

local function resolve_enum(map, enumName, fieldName, key)
    if type(enumName) ~= "string" then
        return enumName
    end
    local result = map[enumName]
    assert(result ~= nil, string.format("UI registry '%s' has invalid %s '%s'", key, fieldName, enumName))
    return result
end

local function register_one(bridge, key, config, replaceExisting)
    assert(type(key) == "string" and key ~= "", "UI registry keys must be non-empty GameplayTag strings")
    assert(type(config) == "table", string.format("UI registry '%s' must be a table", key))
    assert(type(config.WidgetClass) == "string" and config.WidgetClass ~= "",
        string.format("UI registry '%s' requires WidgetClass", key))

    local layer = resolve_enum(Layers, value(config, "Layer", "FullWindow"), "Layer", key)
    local cachePolicy = resolve_enum(
        CachePolicies,
        value(config, "CachePolicy", "Transient"),
        "CachePolicy",
        key)

    return bridge:RegisterRuntimeWidget(
        key,
        config.WidgetClass,
        layer,
        cachePolicy,
        value(config, "AllowMultiple", false),
        value(config, "BlocksInput", true),
        value(config, "HandlesBack", true),
        value(config, "MaxOpenInstances", 0),
        value(config, "MaxCachedInstances", 8),
        value(config, "IdleTimeoutSeconds", 60.0),
        replaceExisting)
end

---Apply a table of runtime registry entries to the current game instance.
---@return boolean success
---@return table errors
function Registry.Apply(worldContext, definitions, replaceExisting)
    assert(type(definitions) == "table", "UI registry definitions must be a table")
    if replaceExisting == nil then
        replaceExisting = true
    end

    local bridge = UE.UUIScriptBridgeSubsystem.Get(worldContext)
    assert(bridge ~= nil, "UUIScriptBridgeSubsystem is unavailable")

    local errors = {}
    for key, config in pairs(definitions) do
        local ok, err = register_one(bridge, key, config, replaceExisting)
        if not ok then
            errors[#errors + 1] = err
        end
    end
    return #errors == 0, errors
end

---Replace the complete runtime overlay. Keys omitted from definitions are removed.
---@return boolean success
---@return table errors
function Registry.Replace(worldContext, definitions)
    Registry.Clear(worldContext)
    return Registry.Apply(worldContext, definitions, true)
end

function Registry.Remove(worldContext, key)
    local bridge = UE.UUIScriptBridgeSubsystem.Get(worldContext)
    assert(bridge ~= nil, "UUIScriptBridgeSubsystem is unavailable")
    return bridge:UnregisterRuntimeWidget(key)
end

function Registry.Clear(worldContext)
    local bridge = UE.UUIScriptBridgeSubsystem.Get(worldContext)
    assert(bridge ~= nil, "UUIScriptBridgeSubsystem is unavailable")
    bridge:ClearRuntimeWidgets()
end

Registry.Layers = Layers
Registry.CachePolicies = CachePolicies

return Registry
