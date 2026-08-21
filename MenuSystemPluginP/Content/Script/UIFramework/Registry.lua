local Registry = {}

local Layers = {
    PersistentSystem = UE.EUILayer.PersistentSystem,
    Loading = UE.EUILayer.Loading,
    Background = UE.EUILayer.Background,
    Dock = UE.EUILayer.Dock,
    FullWindow = UE.EUILayer.FullWindow,
    PopupWindow = UE.EUILayer.PopupWindow,
    Guide = UE.EUILayer.Guide,
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

---Mirrors the field initializers of FUIWidgetEntry (UIWidgetRegistry.h).
---RegisterRuntimeWidget takes every field positionally, so omitted Lua fields
---have to be filled in here. Keep in sync when the struct defaults change.
local Defaults = {
    Layer = "FullWindow",
    CachePolicy = "Transient",
    AllowMultiple = false,
    BlocksInput = true,
    HandlesBack = true,
    MaxOpenInstances = 0,
    MaxCachedInstances = 8,
    IdleTimeoutSeconds = 60.0,
    ScriptModule = "",
}

local function value(config, name)
    local result = config[name]
    if result == nil then
        return Defaults[name]
    end
    return result
end

local function resolve_enum(map, enumName, fieldName, key)
    if type(enumName) ~= "string" then
        return enumName
    end
    local result = map[enumName]
    if result == nil then
        return nil, string.format("UI registry '%s' has invalid %s '%s'", key, fieldName, enumName)
    end
    return result
end

---Pure-Lua validation. Returns a positional argument list ready for the bridge,
---or nil plus an error message. Never touches the runtime registry.
local function validate_one(key, config)
    if type(key) ~= "string" or key == "" then
        return nil, "UI registry keys must be non-empty GameplayTag strings"
    end
    if type(config) ~= "table" then
        return nil, string.format("UI registry '%s' must be a table", key)
    end
    if type(config.WidgetClass) ~= "string" or config.WidgetClass == "" then
        return nil, string.format("UI registry '%s' requires WidgetClass", key)
    end

    local layer, layerError = resolve_enum(Layers, value(config, "Layer"), "Layer", key)
    if layer == nil then
        return nil, layerError
    end

    local cachePolicy, cacheError = resolve_enum(
        CachePolicies, value(config, "CachePolicy"), "CachePolicy", key)
    if cachePolicy == nil then
        return nil, cacheError
    end

    local scriptModule = value(config, "ScriptModule")
    if type(scriptModule) ~= "string" then
        return nil, string.format("UI registry '%s' ScriptModule must be a string", key)
    end
    if scriptModule:match("%.lua$") then
        return nil, string.format(
            "UI registry '%s' ScriptModule '%s' must not include the .lua suffix", key, scriptModule)
    end
    if scriptModule:find("[/\\]") then
        return nil, string.format(
            "UI registry '%s' ScriptModule '%s' must use dots, not path separators", key, scriptModule)
    end

    local allowMultiple = value(config, "AllowMultiple")
    local maxOpen = value(config, "MaxOpenInstances")
    if not allowMultiple and maxOpen > 0 then
        return nil, string.format(
            "UI registry '%s' sets MaxOpenInstances but disallows multiple instances", key)
    end
    if maxOpen < 0 or value(config, "MaxCachedInstances") < 0 then
        return nil, string.format("UI registry '%s' has a negative instance limit", key)
    end

    -- A zero TTL means the reaper frees the instance on its next pass, so the entry
    -- pays for instance caching and gets none of the benefit.
    local idleTimeout = value(config, "IdleTimeoutSeconds")
    if cachePolicy == CachePolicies.KeepUntilIdle and idleTimeout <= 0 then
        return nil, string.format(
            "UI registry '%s' uses KeepUntilIdle with IdleTimeoutSeconds %s; a non-positive "
            .. "timeout discards the instance immediately. Use Transient instead.",
            key, tostring(idleTimeout))
    end

    return {
        key,
        config.WidgetClass,
        layer,
        cachePolicy,
        allowMultiple,
        value(config, "BlocksInput"),
        value(config, "HandlesBack"),
        maxOpen,
        value(config, "MaxCachedInstances"),
        idleTimeout,
        scriptModule,
    }
end

---Validate every definition before any of them is committed, so a malformed table
---cannot leave the runtime overlay half-written.
---
---Script binding is per widget class, so two keys must not map the same WidgetClass
---to different ScriptModules. The C++ side rejects it too, but catching it here names
---both offending keys instead of only the losing one.
---@return table|nil pending
---@return table errors
local function validate_all(definitions)
    local pending, errors = {}, {}
    local claimedByClass = {}
    for key, config in pairs(definitions) do
        local args, err = validate_one(key, config)
        if args then
            local widgetClass, scriptModule = args[2], args[11]
            if scriptModule ~= "" then
                local claim = claimedByClass[widgetClass]
                if claim and claim.module ~= scriptModule then
                    errors[#errors + 1] = string.format(
                        "UI registry '%s' and '%s' share WidgetClass '%s' but request different "
                        .. "ScriptModules ('%s' vs '%s')",
                        claim.key, key, widgetClass, claim.module, scriptModule)
                elseif not claim then
                    claimedByClass[widgetClass] = { key = key, module = scriptModule }
                end
            end
            pending[#pending + 1] = args
        else
            errors[#errors + 1] = err
        end
    end
    if #errors > 0 then
        return nil, errors
    end
    return pending, errors
end

---Validate a definitions table without touching the runtime registry.
---
---Pure Lua: needs no world context and no bridge, so it works during startup, in
---tests, and as a dry run before committing. Apply and Replace both go through this
---first, which is what keeps a malformed table from half-writing the overlay.
---
---Script binding is per widget class, so two keys must not map the same WidgetClass
---to different ScriptModules. The C++ side rejects it too, but catching it here names
---both offending keys instead of only the losing one.
---
---@return boolean success
---@return table errors  empty when success
function Registry.Validate(definitions)
    assert(type(definitions) == "table", "UI registry definitions must be a table")
    local pending, errors = validate_all(definitions)
    return pending ~= nil, errors
end

local function commit(bridge, pending, replaceExisting)
    local errors = {}
    for _, args in ipairs(pending) do
        local ok, err = bridge:RegisterRuntimeWidget(
            args[1], args[2], args[3], args[4], args[5],
            args[6], args[7], args[8], args[9], args[10],
            args[11], replaceExisting)
        if not ok then
            errors[#errors + 1] = err
        end
    end
    return #errors == 0, errors
end

local function get_bridge(worldContext)
    local bridge = UE.UUIScriptBridgeSubsystem.Get(worldContext)
    assert(bridge ~= nil, "UUIScriptBridgeSubsystem is unavailable")
    return bridge
end

---Apply a table of runtime registry entries to the current game instance.
---Nothing is registered unless every definition validates.
---@return boolean success
---@return table errors
function Registry.Apply(worldContext, definitions, replaceExisting)
    assert(type(definitions) == "table", "UI registry definitions must be a table")
    if replaceExisting == nil then
        replaceExisting = true
    end

    local pending, errors = validate_all(definitions)
    if not pending then
        return false, errors
    end
    return commit(get_bridge(worldContext), pending, replaceExisting)
end

---Replace the complete runtime overlay. Keys omitted from definitions are removed.
---Validation runs before Clear, so a malformed table leaves the overlay intact.
---@return boolean success
---@return table errors
function Registry.Replace(worldContext, definitions)
    assert(type(definitions) == "table", "UI registry definitions must be a table")

    local pending, errors = validate_all(definitions)
    if not pending then
        return false, errors
    end

    local bridge = get_bridge(worldContext)
    bridge:ClearRuntimeWidgets()
    return commit(bridge, pending, true)
end

function Registry.Remove(worldContext, key)
    return get_bridge(worldContext):UnregisterRuntimeWidget(key)
end

function Registry.Clear(worldContext)
    get_bridge(worldContext):ClearRuntimeWidgets()
end

Registry.Layers = Layers
Registry.CachePolicies = CachePolicies
Registry.Defaults = Defaults

return Registry
