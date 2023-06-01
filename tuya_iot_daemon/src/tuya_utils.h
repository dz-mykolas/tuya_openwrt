#ifndef TUYA_UTILS_H
#define TUYA_UTILS_H

#include <stdlib.h>

#include "cJSON.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "tuyalink_core.h"
#include "daemon_utils.h"
#include "lua_utils.h"

void on_connected(tuya_mqtt_context_t *context, void *user_data);
void on_disconnect(tuya_mqtt_context_t *context, void *user_data);
void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg);
void ram_report_free(tuya_mqtt_context_t *context, char *device_id);

void tuya_action_switch(tuya_mqtt_context_t *context, const tuyalink_message_t *msg, struct LM_module_list *modules_auto);
void tuya_action_write_file(char *data, char buffer[100]);
void tuya_action_ram(char buffer[100]);

void tuya_ready_lua_modules(struct LM_module_list *modules, struct LM_module_list *modules_auto, struct LM_module_list *modules_action);
int tuya_init(tuya_mqtt_context_t **client, char **argv, struct LM_module_list *modules_action);
int tuya_deinit(tuya_mqtt_context_t **client);

int write_file(char *data);

#endif