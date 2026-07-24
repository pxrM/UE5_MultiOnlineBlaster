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
    -- Do not call Release here. A closed screen may still be retained by the
    -- UIFramework widget cache and activated again later.
end

function M:OnUIRestoredFromCache(Context)
end

return M
