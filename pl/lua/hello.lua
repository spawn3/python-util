#!/usr/bin/env lua

-- defines a factorial function
function fact(n)
	if n == 0 then
		return 1
	else
		return n * fact(n - 1)
	end
end

print("enter a number:")
a = io.read("*n")        -- read a number
print(fact(a))


num = 42

s = 'walternate'
t = "double-quotes are also fine"
u = [[Double brackets start and end multi-line strings.]]

t = nil

while num < 50 do
	num = num + 1
end

if num > 40 then
	print('over 40')
elseif s ~= 'walternate' then
	io.write('not over 40\n')
else
	thisIsGlobal = 5

	local line = io.read()
	print('winter is coming, ' .. line)
end
