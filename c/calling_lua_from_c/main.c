#include <stdio.h>

#include <lua.h>
// #include <luaxlib.h>
#include <lualib.h>

int main(int argc, char **argv) {
    lua_State *L;
    L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_dofile(L, "counter_test.lua")) {
        printf("Could not load file: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 0;
    }

    lua_close(L);
    return 0;
}
