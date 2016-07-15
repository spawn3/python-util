cmod = require("counter")

c = cmod(0)

c:add(4)
c:decrement()
print("val=" .. c:getval())
c:subtract(-2)
c:increment()
print(c)
