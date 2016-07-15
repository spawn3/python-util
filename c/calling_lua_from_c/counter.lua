local M = {}
local M_mt = { __metatable = {}, __index = M }

function M:new(start)
    if self ~= M then
        return nil, "First argument must be self"
    end

    local o = setmetatable({}, M_mt)
    o._count = start
    return o
end
setmetatable(M, { __call = M.new })

function M:add(amount)
    self._count = self._count + amount
end

function M:subtract(amount)
    self._count = self._count - amount
end

function M:increment()
    self:add(1)
end

function M:decrement()
    self:subtract(1)
end

function M:getval()
    return self._count
end

function M_mt:__tostring()
    return string.format("%d", self._count)
end

return M
