#include "tuya_utils.h"
#include "tuya_cacert.h"

#define LOGS_ERROR   3
#define LOGS_WARNING 4
#define LOGS_NOTICE  5

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
        tuya_action_write_file(json->child->child->valuestring, &buffer);
        tuyalink_thing_property_report(context, msg->device_id, buffer);
    } else if (strcmp(type, "get_ram") == 0) {
        tuya_action_ram(&buffer);
        tuyalink_thing_property_report(context, msg->device_id, buffer);
    } else if (strcmp(type, "get_devices") == 0) {
        tuya_action_devices(&buffer);
        tuyalink_thing_property_report(context, msg->device_id, buffer);
    } else if (strcmp(type, "device_toggle") == 0) {
        tuya_action_toggle(&buffer, msg);
        tuyalink_thing_action_report(context, msg->device_id, buffer);
    } else {
        snprintf(buffer, 25, "%s", "Function does not exist");
        log_event(LOGS_ERROR, buffer);
    }
    cJSON_Delete(json);
}

void tuya_action_write_file(char *data, char **buffer)
{
    if (write_file(data) == 1) {
        snprintf(buffer, 100, "{\"file_write_error\":{\"value\":true}}");
        log_event(LOGS_ERROR, "Failed to write to file");
    } else {
        snprintf(buffer, 100, "{\"file_write_error\":{\"value\":false}}");
        log_event(LOGS_NOTICE, "Wrote to file");
    }
}

void tuya_action_ram(char **buffer)
{
    int free_memory	 = 0;
    int ubus_fail = ubus_get_memory(&free_memory);
    snprintf(buffer, 100, "{\"ram_free\":{\"value\":\"%dMB\"}}",
            free_memory / (1024 * 1024));
}

void tuya_action_devices(char **buffer)
{
    ubus_esp_get_devices(buffer);
    log_event(LOGS_ERROR, buffer);
}

// TODO: 
// 1) Cleanup(func, device find better way to assign)
// 2) Make cJSON for response
// 3) Check if device exists
void tuya_action_toggle(char **response, const tuyalink_message_t *msg)
{
    cJSON *json = cJSON_Parse(msg->data_string);
    char *func = json->child->child->next->next->valuestring;
    char *device = json->child->child->valuestring;
    int pin = json->child->child->next->valueint;

    int response_int;
    char response_msg[100];
    ubus_esp_toggle(func, device, pin, &response_int, response_msg);
    snprintf(response, 150, 
    "{"
        "\"actionCode\":\"device_toggle\","
        "\"outputParams\":"
            "{"
                "\"response_int\":%d,"
                "\"response_msg\":\"%s\""
            "}"
    "}", response_int, response_msg);
    
    cJSON_Delete(json);
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