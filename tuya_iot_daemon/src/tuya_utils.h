#include <stdlib.h>

#include "cJSON.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "tuyalink_core.h"
#include "daemon_utils.h"

void on_connected(tuya_mqtt_context_t *context, void *user_data);
void on_disconnect(tuya_mqtt_context_t *context, void *user_data);
void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg);
void ram_report_free(tuya_mqtt_context_t *context, char* device_id);
int tuya_write_file(char *data, char *cwd);