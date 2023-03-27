#include "tuya_utils.h"
#include "tuya_cacert.h"

#define LOGS_ERROR   3
#define LOGS_WARNING 4
#define LOGS_NOTICE  5

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
	cJSON *json = cJSON_Parse(msg->data_string);
	char *data  = json->child->child->valuestring;
	char buffer[100];
	switch (msg->type) {
	case THING_TYPE_ACTION_EXECUTE:
		if (tuya_write_file(data, (char *)user_data) == 1) {
			log_event(LOGS_ERROR, user_data);
			snprintf(buffer, 100, "{\"file_write_error\":{\"value\":true}}");
			log_event(LOGS_ERROR, "Failed to write to file");
		} else {
			snprintf(buffer, 100, "{\"file_write_error\":{\"value\":false}}");
			log_event(LOGS_NOTICE, "Wrote to file");
		}
		tuyalink_thing_property_report(context, msg->device_id, buffer);
		break;
	default:
		break;
	}
	cJSON_Delete(json);
	printf("\r\n");
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

int tuya_write_file(char *data, char *dir)
{
	FILE *fptr;
	fptr = fopen("/tmp/example.txt", "w");
	if (fptr == NULL) {
		log_event(LOGS_WARNING, "Error writing!");
		return 1;
	}

	fprintf(fptr, "%.*s", 300, data);
	fclose(fptr);
	return 0;
}