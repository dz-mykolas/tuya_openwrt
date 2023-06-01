#include "tuya_utils.h"
#include "tuya_cacert.h"

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
    struct LM_module_list *modules_auto = (struct LM_module_list *)context->config.user_data;

	switch (msg->type) {
	case THING_TYPE_ACTION_EXECUTE:
        tuya_action_switch(context, msg, modules_auto);
		break;
	default:
		break;
	}
}

void tuya_action_switch(tuya_mqtt_context_t *context, const tuyalink_message_t *msg, struct LM_module_list *modules_auto)
{
    cJSON *json = cJSON_Parse(msg->data_string);
    char *action_name  = json->child->next->valuestring;
    char buffer[MAX_BUFFER_SIZE];

    bool exists = false;
    for (int i = 0; i < modules_auto->module_count; i++) {
        if (strcmp(action_name, modules_auto->module[i].cfg.action_name) == 0 && modules_auto->module[i].cfg.type == MODULE_ACTION) {
            exists = true;
            lua_run_module(&(modules_auto->module[i]), buffer);
            log_event(LOGS_ERROR, "Result from buffer: %s", buffer);
            tuyalink_thing_property_report(context, msg->device_id, buffer);
        }
    }
    if (exists == false)
        log_event(LOGS_ERROR, "Action does not exist");
    
    cJSON_Delete(json);
}

void tuya_ready_lua_modules(struct LM_module_list *modules, struct LM_module_list *modules_auto, struct LM_module_list *modules_action)
{
    modules_auto->module_count = 0;
    modules_action->module_count = 0;

    if (lua_open_modules(modules) != EXIT_SUCCESS)
        return EXIT_FAILURE;
    for (int i = 0; i < modules->module_count; ++i) {
        if (lua_load_module(&(modules->module[i])) != EXIT_SUCCESS)
            continue;
        char buffer[MAX_BUFFER_SIZE];
        if (modules->module[i].cfg.type == MODULE_AUTO) {
            modules_auto->module[modules_auto->module_count] = modules->module[i];
            modules_auto->module_count++;
        } else {
            modules_action->module[modules_action->module_count] = modules->module[i];
            modules_action->module_count++;
        }
    }
    
    if (modules_auto->module_count > 0)
        log_event(LOGS_ERROR, "Auto modules: ");
    for (int i = 0; i < modules_auto->module_count; ++i) {
        log_event(LOGS_ERROR, "File: %s", modules_auto->module[i].filename);
        log_event(LOGS_ERROR, "Type: %lu", modules_auto->module[i].cfg.type);
        log_event(LOGS_ERROR, "Interval: %lu", modules_auto->module[i].cfg.interval);
        lua_init_module(&(modules_auto->module[i]));
    }
    if (modules_action->module_count > 0)
        log_event(LOGS_ERROR, "Action modules: ");    
    for (int i = 0; i < modules_action->module_count; ++i) {
        log_event(LOGS_ERROR, "File: %s", modules_action->module[i].filename);
        log_event(LOGS_ERROR, "Type: %lu", modules_action->module[i].cfg.type);
        log_event(LOGS_ERROR, "Interval: %lu", modules_action->module[i].cfg.interval);
        lua_init_module(&(modules_action->module[i]));
    }
}

int tuya_init(tuya_mqtt_context_t **client, char **argv, struct LM_module_list *modules_action)
{
	tuya_mqtt_config_t config = { .host	   = "m1.tuyacn.com",
					    .port	   = 8883,
					    .cacert	   = tuya_cacert_pem,
					    .cacert_len	   = sizeof(tuya_cacert_pem),
					    .device_id	   = argv[2],
					    .device_secret = argv[3],
					    .keepalive	   = 60,
					    .timeout_ms	   = 1000,
					    .on_messages   = on_messages,
                        .user_data = modules_action
                        };

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