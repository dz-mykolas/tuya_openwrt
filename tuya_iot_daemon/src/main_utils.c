#include "main_utils.h"

volatile sig_atomic_t running = 1;
void program_loop(tuya_mqtt_context_t **client)
{
	while (running) {
		tuya_mqtt_loop(*client);
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