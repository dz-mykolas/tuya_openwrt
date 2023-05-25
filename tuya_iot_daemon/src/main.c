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

    /* MODULES INIT */
    struct LM_module_list modules;
    struct LM_module_list modules_auto;
    struct LM_module_list modules_action;
    tuya_ready_lua_modules(&modules, &modules_auto, &modules_action);

	/* TUYA INIT */
	tuya_mqtt_context_t *client = &client_instance;
	int exit = 0;
	if (tuya_init(&client, argv, &modules_action))
		exit = 1;
	/* INFINITE LOOP */
	if (exit == 0)
		program_loop(&client, &modules_auto);
	/* DISCONNECT */
	if (tuya_deinit(&client))
		return EXIT_FAILURE;

    for (int i = 0; i < modules.module_count; ++i) {
        if (modules.module[i].loaded == true)
            lua_deinit_module(&(modules.module[i]));
    }

    log_event(LOGS_WARNING, "Program finished");
	return EXIT_SUCCESS;
}
