#include "lua_utils.h"

int lua_ubus_get_memory(int *free_memory)
{
    int status, result;
	lua_State *L;
	L = luaL_newstate();
	luaL_openlibs(L);
    
	status = luaL_loadfile(L, "/usr/local/lib/ubus_memory.lua");
	if (status) {
		log_event(LOGS_ERROR, "Couldn't load file: %s\n", lua_tostring(L, -1));
		return EXIT_FAILURE;
	}
	if (lua_pcall(L, 0, 0, 0)) {
		log_event(LOGS_ERROR, "Something went wrong 1\n");
        return EXIT_FAILURE;
	}
    lua_getglobal(L, "ubus_get_memory");
	if (lua_pcall(L, 0, 1, 1)) {
		log_event(LOGS_ERROR, "Something went wrong inside Lua script\n");
        return EXIT_FAILURE;
	}
    if (!lua_isnumber(L, -1)) {
        log_event(LOGS_ERROR, "Lua script did not return a number");
        return EXIT_FAILURE;
    }
    *free_memory = lua_tonumber(L, -1);
	lua_close(L);

    return EXIT_SUCCESS;
}