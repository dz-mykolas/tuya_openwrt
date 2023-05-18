#ifndef LUA_UTILS_H
#define LUA_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "daemon_utils.h"

int lua_ubus_get_memory(int *free_memory);

#endif