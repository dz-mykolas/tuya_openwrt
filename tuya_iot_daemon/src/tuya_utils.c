#include "tuya_utils.h"
#include "tuya_cacert.h"

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
	switch (msg->type) {
	case THING_TYPE_ACTION_EXECUTE:
        tuya_action_switch(context, msg);
		break;
	default:
		break;
	}
	printf("\r\n");
}

void tuya_action_switch(tuya_mqtt_context_t *context, const tuyalink_message_t *msg)
{
    cJSON *json = cJSON_Parse(msg->data_string);
    char *type  = json->child->next->valuestring;
    char buffer[300];
    if (strcmp(type, "file_write") == 0) {
        tuya_action_write_file(json->child->child->valuestring, buffer);
        tuyalink_thing_property_report(context, msg->device_id, buffer);
    } else if (strcmp(type, "get_ram") == 0) {
        tuya_action_ram(buffer);
        tuyalink_thing_property_report(context, msg->device_id, buffer);
    } else {
        snprintf(buffer, 25, "%s", "Function does not exist");
        log_event(LOGS_ERROR, buffer);
    }
    cJSON_Delete(json);
}

void tuya_action_write_file(char *data, char buffer[])
{
    if (write_file(data) == 1) {
        snprintf(buffer, 100, "{\"file_write_error\":{\"value\":true}}");
        log_event(LOGS_ERROR, "Failed to write to file");
    } else {
        snprintf(buffer, 100, "{\"file_write_error\":{\"value\":false}}");
        log_event(LOGS_NOTICE, "Wrote to file");
    }
}

void tuya_action_ram(char buffer[])
{
    int free_memory	 = 0;
    if (lua_ubus_get_memory(&free_memory) == EXIT_SUCCESS) {
        snprintf(buffer, 100, "{\"ram_free\":{\"value\":\"%dMB\"}}",
            free_memory / (1024 * 1024));
        return;
    }
    snprintf(buffer, 100, "{\"ram_free\":{\"value\":\"error\"}}");
}

int tuya_init(tuya_mqtt_context_t **client, char **argv)
{
	const tuya_mqtt_config_t config = { .host	   = "m1.tuyacn.com",
					    .port	   = 8883,
					    .cacert	   = tuya_cacert_pem,
					    .cacert_len	   = sizeof(tuya_cacert_pem),
					    .device_id	   = argv[2],
					    .device_secret = argv[3],
					    .keepalive	   = 60,
					    .timeout_ms	   = 2000,
					    .on_messages   = on_messages };

	int ret;
	ret = tuya_mqtt_init(*client, &config);
	if (ret != OPRT_OK) {
		log_event(LOGS_ERROR, "Error while initializing!");
		tuya_mqtt_deinit(*client);
		return EXIT_FAILURE;
	}
	ret = tuya_mqtt_connect(*client);
	if (ret != OPRT_OK) {
		log_event(LOGS_ERROR, "Error while connecting!");
		tuya_mqtt_deinit(*client);
		return EXIT_FAILURE;
	}
	log_event(LOGS_NOTICE, "Device Connected");
	return ret;
}

int write_file(char *data)
{
	FILE *fptr;
	fptr = fopen("/tmp/example.txt", "w");
	if (fptr == NULL) {
		log_event(LOGS_WARNING, "Error writing!");
		return 1;
	}

	fprintf(fptr, "%.*s\n", 300, data);
	fclose(fptr);
	return 0;
}

int tuya_deinit(tuya_mqtt_context_t **client)
{
	int exit = 0;
	if (tuya_mqtt_disconnect(*client) || tuya_mqtt_deinit(*client)) {
		log_event(LOGS_ERROR, "Error while disconnecting from tuya");
		exit = EXIT_FAILURE;
	}
	log_event(LOGS_WARNING, "Device properly disconnected");
    
	return exit;
}