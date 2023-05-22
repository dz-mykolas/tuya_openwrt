#ifndef MAIN_UTILS_H
#define MAIN_UTILS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <argp.h>

#include "daemon_utils.h"
#include "tuya_utils.h"

struct arguments {
	char *args[3];
};

error_t parse_opt(int key, char *arg, struct argp_state *state);
void sig_handler(int sig);
void program_loop(tuya_mqtt_context_t **client, struct LM_module_list modules_auto);

#endif