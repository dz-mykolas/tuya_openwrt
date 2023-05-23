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
#include <stdint.h>

#include "daemon_utils.h"

#define MAX_LUA_INTERVAL 86400
#define MAX_LUA_MODULES 30
#define MAX_BUFFER_SIZE 4096

enum LM_module_type {
    MODULE_AUTO,
    MODULE_ACTION,
};

struct LM_config {
    uint8_t type;
    uint32_t interval;
    char action_name[NAME_MAX];
};

struct LM_module {
    lua_State *L;
    char filename[NAME_MAX];
    struct LM_config cfg;
    bool loaded;
};

struct LM_module_list {
    struct LM_module module[MAX_LUA_MODULES];
    uint8_t module_count;
};

void lua_init_module(struct LM_module *module);
void lua_deinit_module(struct LM_module *module);
int lua_run_module(struct LM_module *module, char buffer[MAX_BUFFER_SIZE]);
struct LM_config lua_load_config(struct LM_module *module);
int lua_load_module(struct LM_module *module);
int lua_read_filenames(struct LM_module_list *modules);
int lua_open_modules(struct LM_module_list *modules);

#endif