#ifndef LUA_UTILS_H
#define LUA_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

#include "daemon_utils.h"

#define MAX_LUA_MODULES 30

enum module_type {
    MODULE_AUTO,
    MODULE_ACTION,
};

struct Lua_config {
    __uint8_t type;
    __uint32_t interval;
};

struct Lua_module {
    lua_State *L;
    char filename[NAME_MAX];
    struct Lua_config cfg;
    bool loaded;
};

struct Lua_modules {
    struct Lua_module[MAX_LUA_MODULES];
    __uint8_t module_count;
};

int lua_ubus_get_memory(int *free_memory);
int lua_read_filenames(struct Lua_files *lfiles);
int lua_read_filenames(struct Lua_files *lfiles);
int lua_open_modules(lua_State **L_states[MAX_LUA_MODULES]);

#endif