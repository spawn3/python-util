function fibonacci()
	local m = 1
	local n = 1

	while true do
		coroutine.yield(m)
		m, n = n, m + n
	end
end


gen = coroutine.create(fibonacci)
print (coroutine.resume(gen))
print (coroutine.resume(gen))
print (coroutine.resume(gen))
print (coroutine.resume(gen))
print (coroutine.resume(gen))
