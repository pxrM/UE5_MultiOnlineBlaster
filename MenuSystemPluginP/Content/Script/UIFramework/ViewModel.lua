---Lua wrapper that makes UIFramework view models behave like plain tables while
---still refreshing UMG bindings.
---
---Why this exists: UnLua writes UPROPERTYs by copying memory directly. It never
---calls the setter, so MVVM never learns the field changed and bound widgets keep
---showing the old value — silently, with no error. Assigning through this wrapper
---routes the write to UUIViewModelBase:SetField*, which broadcasts.
---
---    local ViewModel = require "UIFramework.ViewModel"
---
---    function M:OnUIOpening(Context)
---        self.VM = ViewModel.Wrap(self:GetViewModel())
---        self.VM.PlayerName = "Bob"     -- broadcasts
---        self.VM.Score = 100            -- broadcasts
---        print(self.VM.Score)           -- reads through to the UObject
---    end
---
---Fields must be declared FieldNotify on the C++/Blueprint view model class. Lua
---cannot add new fields; only their values are dynamic.
local ViewModel = {}

---Maps a Lua value to the SetField* overload that can store it. Integers go to
---SetFieldInt because Lua 5.3+ distinguishes them; SetFieldFloat accepts both
---float and double properties on the C++ side.
local function assign(target, name, value)
    local kind = type(value)
    if kind == "string" then
        return target:SetFieldString(name, value)
    elseif kind == "boolean" then
        return target:SetFieldBool(name, value)
    elseif kind == "number" then
        if math.type(value) == "integer" then
            return target:SetFieldInt(name, value)
        end
        return target:SetFieldFloat(name, value)
    elseif kind == "userdata" then
        -- FText and FName arrive as userdata; try the text setter first because it
        -- is the common case for anything displayed, then fall back to objects.
        if target:SetFieldText(name, value) then
            return true
        end
        return target:SetFieldObject(name, value)
    end
    error(string.format("ViewModel: cannot assign a %s to field '%s'", kind, name), 3)
end

local Proxy = {}

Proxy.__index = function(proxy, key)
    local target = rawget(proxy, "__target")
    local value = target[key]
    if type(value) == "function" then
        -- Bind the UObject as self so proxy:Method() still reaches the UObject.
        return function(_, ...)
            return value(target, ...)
        end
    end
    return value
end

Proxy.__newindex = function(proxy, key, value)
    local target = rawget(proxy, "__target")
    assign(target, key, value)
end

---Wrap a UUIViewModelBase so assignments broadcast field changes.
---Wrapping is cheap but not free; cache the wrapper rather than wrapping per write.
---@return table proxy
function ViewModel.Wrap(target)
    assert(target ~= nil, "ViewModel.Wrap requires a view model instance")
    return setmetatable({ __target = target }, Proxy)
end

---The UObject behind a wrapper, or the argument itself if it is not wrapped.
function ViewModel.Unwrap(proxy)
    if type(proxy) == "table" then
        return rawget(proxy, "__target") or proxy
    end
    return proxy
end

---Assign several fields, then report which ones failed.
---Values follow the same type mapping as a single assignment.
---@return boolean success
---@return table failedFieldNames
function ViewModel.SetAll(target, values)
    target = ViewModel.Unwrap(target)
    assert(target ~= nil, "ViewModel.SetAll requires a view model instance")

    local failed = {}
    for name, value in pairs(values) do
        -- A false return also means "value unchanged", so only treat a genuinely
        -- unknown field as a failure.
        if not assign(target, name, value) and not target:HasNotifyField(name) then
            failed[#failed + 1] = name
        end
    end
    return #failed == 0, failed
end

return ViewModel
