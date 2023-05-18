# ubus_memory

This module provides a Lua and C implementation for querying memory usage information from the ubus system on OpenWrt platforms. The Lua script communicates with the ubus system to get the memory info, and the C code wraps around the Lua code to provide a C interface for this functionality.
Overview

It is is made up of two parts. The first part is a Lua script, ubus_get_memory(), which connects to ubus and fetches system information including free memory. The second part is a C function, lua_ubus_get_memory(int *free_memory), which initializes a Lua interpreter, loads the Lua script, and then executes the Lua function to get the memory information.
Lua Code

## lua

```lua
function ubus_get_memory()
    ubus = require("ubus")

    local conn = ubus.connect()
    if not conn then
        error("Failed to connect to ubus")
    end
    
    local status = conn:call("system", "info", {})
    local free_mem = -1
    if status and status.memory then
        free_mem = status.memory.free
    end
    conn:close()
    
    return free_mem
end
```

Lua script(`tuya_iot_daemon/src/Lua/ubus_memory.lua`) first imports the ubus library. Then, it creates a connection to the ubus system. If the connection fails, it throws an error.

It then calls conn:call("system", "info", {}) to get the system information, and checks whether the system information contains memory info. If it does, it updates free_mem with the free memory value from the system information. Finally, it closes the connection and returns the free memory.

## c

```c
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
```

The C function (`tuya_iot_daemon/src/lua_utils.c`) creates a new Lua state and opens Lua libraries. It then loads the Lua script from the specified path. If the Lua script fails to load, it logs an error and returns an exit failure. Most of the code in it is error checking and loading libraries. The important parts are here: 1)'lua_getglobal(L, "ubus_get_memory")'. This calls Lua function inside the script file. 2) The returned value from Lua script will be inside 'L' variable which was declared earlier and needs to be converted using '*free_memory = lua_tonumber(L, -1)'. 

The function then calls the Lua function ubus_get_memory() and checks if it returned a number. If it didn't, it logs an error and returns an exit failure. If the Lua function executed successfully, the function updates the 'free_memory' pointer with the free memory fetched from the Lua script and returns an exit success.

# usage

To use this module, simply include the C header file in your C code and call the function lua_ubus_get_memory(int *free_memory). Make sure the Lua script is located at the specified path (/usr/local/lib/ubus_memory.lua).
