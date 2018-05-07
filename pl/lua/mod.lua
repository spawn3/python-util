-- Suppose the file mod.lua looks like this:
local M = {}

local function sayMyName()
	print('Hrunkner')
end

function M.sayHello()
	print('Why hello there')
	sayMyName()
end

return M
