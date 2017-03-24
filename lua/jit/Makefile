# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/gj/huayun/python-util/lua/ffi
#
#

CWD := $(shell pwd)

all:
	gcc -g -o libmyffi.so -Wl,--no-as-needed -fPIC -shared -luuid myffi.c

test:
	LD_LIBRARY_PATH=${CWD} luajit lj.lua
