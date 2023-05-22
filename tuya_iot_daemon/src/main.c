#include "main_utils.h"

#define LOGS_ERROR   3
#define LOGS_WARNING 4
#define LOGS_NOTICE  5

/* ARGP */
const char *argp_program_version     = "simple_daemon 1.0";
const char *argp_program_bug_address = "<none>";
static char doc[]		     = "Simple daemon that connects to Tuya Cloud using it's C SDK";
static char args_doc[]		     = "product_id device_id device_secret";
static struct argp_option options[]  = { { 0 } };
static struct argp argp		     = { options, parse_opt, args_doc, "Simple daemon" };

/* TUYA */
tuya_mqtt_context_t client_instance;

int main(int argc, char **argv)
{
	/* SIGNAL HANDLING */
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sig_handler;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	/* ARGUMENT PARSING */
	struct arguments arguments = { 0 };
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // TODO 1:
    // Some modules are with type auto.
    // Split them by type to different
    // structs.
    // TODO 2:
    // Run auto with tuya_loop
    // TODO 3:
    // Send action ones to user_date
    // by config.
    /* MODULES INIT */
    struct LM_module_list modules;
    struct LM_module_list modules_auto;
    modules_auto.module_count = 0;
    struct LM_module_list modules_action;
    modules_action.module_count = 0;
    lua_State *L_states[MAX_LUA_MODULES];
    if (lua_open_modules(&modules) != EXIT_SUCCESS)
        return EXIT_FAILURE;
    for (int i = 0; i < modules.module_count; ++i) {
        if (lua_load_module(&(modules.module[i])) != EXIT_SUCCESS)
            continue;
        char buffer[MAX_BUFFER_SIZE];
        if (modules.module[i].cfg.type == MODULE_AUTO) {
            modules_auto.module[modules_auto.module_count] = modules.module[i];
            modules_auto.module_count++;
        } else {
            modules_action.module[modules_action.module_count] = modules.module[i];
            modules_action.module_count++;
        }
    }
    
    log_event(LOGS_ERROR, "Auto modules: ");
    for (int i = 0; i < modules_auto.module_count; ++i) {
        log_event(LOGS_ERROR, "File: %s", modules_auto.module[i].filename);
        log_event(LOGS_ERROR, "Type: %lu", modules_auto.module[i].cfg.type);
        log_event(LOGS_ERROR, "Interval: %lu", modules_auto.module[i].cfg.interval);
        lua_init_module(&(modules_auto.module[i]));
    }
    log_event(LOGS_ERROR, "Action modules: ");
    for (int i = 0; i < modules_action.module_count; ++i) {
        log_event(LOGS_ERROR, "File: %s", modules_action.module[i].filename);
        log_event(LOGS_ERROR, "Type: %lu", modules_action.module[i].cfg.type);
        log_event(LOGS_ERROR, "Interval: %lu", modules_action.module[i].cfg.interval);
        char buffer[MAX_BUFFER_SIZE];
        lua_init_module(&(modules_action.module[i]));
        if (lua_run_module(&(modules_action.module[i]), buffer) == EXIT_SUCCESS)
            log_event(LOGS_ERROR, "Result from buffer: %s", buffer);
    }

	/* TUYA INIT */
	tuya_mqtt_context_t *client = &client_instance;
	int exit = 0;
	if (tuya_init(&client, argv, &modules))
		exit = 1;
	/* INFINITE LOOP */
	if (exit == 0)
		program_loop(&client, modules_auto);
	/* DISCONNECT */
	if (tuya_deinit(&client))
		return EXIT_FAILURE;

    for (int i = 0; i < modules_auto.module_count; ++i) {
        if (modules_auto.module[i].loaded == true)
            lua_deinit_module(&(modules_auto.module[i]));
    }
    for (int i = 0; i < modules_action.module_count; ++i) {
        if (modules_action.module[i].loaded == true)
            lua_deinit_module(&(modules_action.module[i]));
    }

	return EXIT_SUCCESS;
}
