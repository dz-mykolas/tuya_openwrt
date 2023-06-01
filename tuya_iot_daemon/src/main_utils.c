#include "main_utils.h"

volatile sig_atomic_t running = 1;

int run_on_interval(struct LM_module *module, int interval, char buffer[MAX_BUFFER_SIZE])
{
    if (module->loaded != true)
        return EXIT_FAILURE;
    else if ((interval % module->cfg.interval) == 0) {
        if (lua_run_module(module, buffer) != EXIT_SUCCESS) {
            lua_deinit_module(module);
            return EXIT_FAILURE;
        } else {
            return EXIT_SUCCESS;
        }
    }
}

void program_loop(tuya_mqtt_context_t **client, struct LM_module_list *modules_auto)
{
    char buffer[MAX_BUFFER_SIZE];
    int interval = 1;
	while (running) {
        if (interval > MAX_LUA_INTERVAL)
            interval = 1;
        for (int i = 0; i < modules_auto->module_count; i++) {
            if (run_on_interval(&(modules_auto->module[i]), interval, buffer) == EXIT_SUCCESS) {
                log_event(LOGS_ERROR, "Result from buffer: %s", buffer);
                tuyalink_thing_property_report(*client, (*client)->config.device_id, buffer);
            }
        }
		tuya_mqtt_loop(*client);
        interval++;
	}
}

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	switch (key) {
	case ARGP_KEY_ARG:
		if (state->arg_num > 2) {
			printf("Too many arguments!\n");
			argp_usage(state);
		}
		arguments->args[state->arg_num] = arg;
		break;
	case ARGP_KEY_END:
		if (state->arg_num <= 2) {
			printf("Not enough arguments!\n");
			argp_usage(state);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

void sig_handler(int sig)
{
	switch (sig) {
	case SIGINT:
		printf("\nCTRL+C. Exiting\n");
		running = 0;
		break;
	case SIGTERM:
		printf("\nKILL Signal. Exiting\n");
		running = 0;
		break;
	}
}