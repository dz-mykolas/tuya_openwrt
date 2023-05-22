#include "lua_utils.h"

// int lua_open_module(int *free_memory)
// {
//     int status, result;
// 	lua_State *L;
// 	L = luaL_newstate();
// 	luaL_openlibs(L);
    
// 	status = luaL_loadfile(L, "/usr/local/lib/ubus_memory.lua");
// 	if (status) {
// 		log_event(LOGS_ERROR, "Couldn't load file: %s\n", lua_tostring(L, -1));
// 		return EXIT_FAILURE;
// 	}
// 	if (lua_pcall(L, 0, 0, 0)) {
// 		log_event(LOGS_ERROR, "Something went wrong 1\n");
//         return EXIT_FAILURE;
// 	}
//     lua_getglobal(L, "ubus_get_memory");
// 	if (lua_pcall(L, 0, 1, 1)) {
// 		log_event(LOGS_ERROR, "Something went wrong inside Lua script\n");
//         return EXIT_FAILURE;
// 	}
//     if (!lua_isnumber(L, -1)) {
//         log_event(LOGS_ERROR, "Lua script did not return a number");
//         return EXIT_FAILURE;
//     }
//     *free_memory = lua_tonumber(L, -1);
// 	lua_close(L);

//     return EXIT_SUCCESS;
// }

void lua_init_module(struct LM_module *module)
{
    lua_settop(module->L, 0);
    lua_getglobal(module->L, "module_init");
    if (!lua_isfunction(module->L, -1)) {
        log_event(LOGS_WARNING, "Module init: function 'module_init' does not exist");
    }
    else if (lua_pcall(module->L, 0, 1, 0) != 0) {
        log_event(LOGS_WARNING, "Module init: function 'module_init': %s", lua_tostring(module->L, -1));
    }
    module->loaded = true;
}

void lua_deinit_module(struct LM_module *module)
{
    lua_settop(module->L, 0);
    lua_getglobal(module->L, "module_deinit");
    if (!lua_isfunction(module->L, -1))
        log_event(LOGS_WARNING, "Module deinit: function 'module_deinit' does not exist");
    else if (lua_pcall(module->L, 0, 1, 0) != 0)
        log_event(LOGS_WARNING, "Module deinit: function 'module_deinit': %s", lua_tostring(module->L, -1));
    lua_close(module->L);
    module->loaded = false;
}

int lua_run_module(struct LM_module *module, char buffer[MAX_BUFFER_SIZE])
{
    lua_settop(module->L, 0);
    lua_getglobal(module->L, "lua_main");
    if (!lua_isfunction(module->L, -1)) {
        log_event(LOGS_ERROR, "Stopping module execution: function 'lua_main' does not exist");
        module->loaded = false;
        return EXIT_FAILURE;
    }
    if (lua_pcall(module->L, 0, 1, 0) != 0) {
        log_event(LOGS_ERROR, "Stopping module execution: function 'lua_main': %s", lua_tostring(module->L, -1));
        module->loaded = false;
        return EXIT_FAILURE;
    }
    if (!lua_isstring(module->L, -1)) {
        log_event(LOGS_ERROR, "Lua script did not return a string");
        return EXIT_FAILURE;
    }
    snprintf(buffer, MAX_BUFFER_SIZE, "%s", lua_tostring(module->L, -1));

    return EXIT_SUCCESS;
}

struct LM_config lua_load_config(struct LM_module *module)
{
    struct LM_config cfg;
    __int64_t temp_interval = 0;
    char buffer[NAME_MAX];
    if (lua_istable(module->L, -1)) {
        lua_pushstring(module->L, "type");
        lua_gettable(module->L, -2);
        if (!lua_isstring(module->L, -1)) {
            log_event(LOGS_ERROR, "Expected 'type' to be a string");
            cfg.type = MODULE_AUTO;
        } else {
            snprintf(buffer, NAME_MAX, "%s", lua_tostring(module->L, -1));
            if (strcmp(buffer, "auto") == 0)
                cfg.type = MODULE_AUTO;
            else if (strcmp(buffer, "action") == 0)
                cfg.type = MODULE_ACTION;
            else
                cfg.type = MODULE_AUTO;
        }
        lua_pop(module->L, 1);
        lua_pushstring(module->L, "interval");
        lua_gettable(module->L, -2);
        if (!lua_isnumber(module->L, -1)) {
            log_event(LOGS_ERROR, "Expected 'interval' to be a number");
            cfg.interval = 60;
        } else {
            __int64_t temp_interval = lua_tonumber(module->L, -1);
            if (temp_interval > MAX_LUA_INTERVAL) {
                log_event(LOGS_WARNING, "Maximum interval is: %d", MAX_LUA_INTERVAL);
                cfg.interval = MAX_LUA_INTERVAL;
            } else if (temp_interval < 0) {
                log_event(LOGS_WARNING, "Interval can not be lower than 0");
                cfg.interval = 60;
            } else {
                cfg.interval = temp_interval;
            }
        }
        lua_pop(module->L, 1);
    } else {
        cfg.type = MODULE_AUTO;
        cfg.interval = 60;
    }
    return cfg;
}

int lua_load_module(struct LM_module *module)
{
    int status;
    module->L = luaL_newstate();
    luaL_openlibs(module->L);
    char path[PATH_MAX];
    
    snprintf(path, PATH_MAX, "/usr/local/lib/tuya_modules/%s", module->filename);
    status = luaL_loadfile(module->L, path);
    if (status) {
        log_event(LOGS_ERROR, "Stopping module open: couldn't load file: %s", lua_tostring(module->L, -1));
        module->loaded = false;
        return EXIT_FAILURE;
    }
    if (lua_pcall(module->L, 0, 0, 0)) {
        log_event(LOGS_ERROR, "Stopping module open: something went wrong during the file load");
        module->loaded = false;
        return EXIT_FAILURE;
    }
    lua_getglobal(module->L, "tuya_config");
    if (!lua_isfunction(module->L, -1)) {
        log_event(LOGS_ERROR, "Stopping module open: function 'tuya_config' does not exist, setting default config");
        module->loaded = true;
        module->cfg.type = MODULE_AUTO;
        module->cfg.interval = 60;
        return EXIT_SUCCESS;
    }
    if (lua_pcall(module->L, 0, 1, 0) != 0)
        log_event(LOGS_ERROR, "Stopping module open: error running function 'tuya_config': %s, setting default config", lua_tostring(module->L, -1));
    
    module->cfg = lua_load_config(module);
    module->loaded = true;

    return EXIT_SUCCESS;
}

int lua_read_filenames(struct LM_module_list *modules)
{
    DIR *d;
    struct dirent *dir;
    d = opendir("/usr/local/lib/tuya_modules");
    if (!d) {
        log_event(LOGS_ERROR, "Stopping module reading: failed to open directory");
        return EXIT_FAILURE;
    }

    modules->module_count = 0;
    while ((dir = readdir(d)) != NULL) {
        if (modules->module_count >= MAX_LUA_MODULES) {
            log_event(LOGS_WARNING, "Stopping module reading: module limit reached");
            return EXIT_SUCCESS;
        }
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;
        snprintf(modules->module[modules->module_count].filename, NAME_MAX, "%s", dir->d_name);
        modules->module_count++;
    }
    closedir(d);

    return EXIT_SUCCESS;
}

int lua_open_modules(struct LM_module_list *modules)
{
    if (lua_read_filenames(modules) != EXIT_SUCCESS)
        return EXIT_FAILURE;
    if (modules->module_count < 1) {
        log_event(LOGS_WARNING, "Stopping module open: no modules were read");
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
