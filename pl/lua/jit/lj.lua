local ffi = require('ffi')
local bit = require('bit')

ffi.cdef[[
int printf(const char *fmt, ...);
int add(int x, int y);  /* don't forget to declare */
]]

ffi.C.printf("Hello %s!\n", "world")

ffi.load('uuid')

local mylib = ffi.load('myffi', true)

local res = ffi.C.add(1, 2);
print (res)
