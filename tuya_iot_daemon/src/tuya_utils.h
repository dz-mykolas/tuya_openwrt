#include <stdlib.h>

#include "cJSON.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "tuyalink_core.h"
#include "daemon_utils.h"
#include "ubus_utils.h"

void on_connected(tuya_mqtt_context_t *context, void *user_data);
void on_disconnect(tuya_mqtt_context_t *context, void *user_data);
void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg);
void ram_report_free(tuya_mqtt_context_t *context, char *device_id);
int write_file(char *data);
int tuya_init(tuya_mqtt_context_t **client, char **argv);

void tuya_action_switch(tuya_mqtt_context_t *context, const tuyalink_message_t *msg);
void tuya_action_write_file(char *data, char **buffer);
void tuya_action_ram(char **buffer);
void tuya_action_devices(char **buffer);
void tuya_action_toggle(char **response, const tuyalink_message_t *msg);
