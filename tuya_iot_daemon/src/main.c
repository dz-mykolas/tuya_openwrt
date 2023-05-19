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
    lua_State *L_states[MAX_LUA_MODULES];
    if (lua_open_modules(&modules) != EXIT_SUCCESS)
        return EXIT_FAILURE;
    for (int i = 0; i < modules.module_count; ++i) {
        log_event(LOGS_ERROR, "File: %s", modules.module[i].filename);
        lua_load_module(&(modules.module[i]));
        log_event(LOGS_ERROR, "Type: %lu", modules.module[i].cfg.type);
        log_event(LOGS_ERROR, "Interval: %lu", modules.module[i].cfg.interval);
    }

	/* TUYA INIT */
	tuya_mqtt_context_t *client = &client_instance;
	int exit = 0;
	if (tuya_init(&client, argv, &modules))
		exit = 1;
	/* INFINITE LOOP */
	if (exit == 0)
		program_loop(&client);
	/* DISCONNECT */
	if (tuya_deinit(&client))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
