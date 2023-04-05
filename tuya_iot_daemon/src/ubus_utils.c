#include "ubus_utils.h"

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY]	  = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY]	  = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
	[SHARED_MEMORY]	  = { .name = "shared", .type = BLOBMSG_TYPE_INT64 },
	[BUFFERED_MEMORY] = { .name = "buffered", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
};

static const struct blobmsg_policy devices_policy[] = {
    [MEMORY_DATA] = { .name = "usb_devices", .type = BLOBMSG_TYPE_ARRAY },
};

static const struct blobmsg_policy device_toggle_policy[__ATTR_MAX] = {
    [ATTR_RESPONSE] = { .name = "response", .type = BLOBMSG_TYPE_INT32 },
    [ATTR_MSG] = { .name = "msg", .type = BLOBMSG_TYPE_STRING },
};

static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct MemData *memory_data = (struct MemData *)req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[MEMORY_DATA]) {
		return;
	}

	blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY_DATA]),
		      blobmsg_data_len(tb[MEMORY_DATA]));

	memory_data->total    = blobmsg_get_u64(memory[TOTAL_MEMORY]);
	memory_data->free     = blobmsg_get_u64(memory[FREE_MEMORY]);
	memory_data->shared   = blobmsg_get_u64(memory[SHARED_MEMORY]);
	memory_data->buffered = blobmsg_get_u64(memory[BUFFERED_MEMORY]);
}

static void devices_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct DevicesData *device_data= (struct DeviceData *)req->priv;
    struct blob_attr *tb[__INFO_MAX];

    blobmsg_parse(devices_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[MEMORY_DATA]) {
        return;
    }

    struct blob_attr *device;
    int rem;
    cJSON *devices_arr = cJSON_CreateArray();

    blobmsg_for_each_attr(device, tb[MEMORY_DATA], rem) {
        char *device_str = blobmsg_format_json(device, false);
        cJSON *device_json = cJSON_CreateString(device_str);
        cJSON_AddItemToArray(devices_arr, device_json);
        free(device_str);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "usb_devices", devices_arr);

    device_data->devices = cJSON_Print(root);
    cJSON_Delete(root);
}

static void response_handler(struct ubus_request *req, int type, struct blob_attr *msg)
{   
    struct ResponseData *response_data= (struct ResponseData *)req->priv;
    struct blob_attr *tb[__ATTR_MAX];

    blobmsg_parse(device_toggle_policy, __ATTR_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));

    if (!tb[ATTR_RESPONSE] || !tb[ATTR_MSG])
        return;

    response_data->response = blobmsg_get_u32(tb[ATTR_RESPONSE]);
    response_data->msg = blobmsg_get_string(tb[ATTR_MSG]);
}

int ubus_connect_helper(struct ubus_context **ctx, char *object, uint32_t *id)
{
    int ret = 1;
    if (!(*ctx = ubus_connect(NULL)))
        log_event(LOGS_ERROR, "Error while connecting to ubus");
    else if (ubus_lookup_id(*ctx, object, id) == 1) {
        ubus_free(*ctx);
        log_event(LOGS_ERROR, "Failed to find system object on ubus");
    }
	else
        ret = 0;
    return ret;
}

int ubus_get_memory(int *free_memory)
{
    struct ubus_context *ctx;
	uint32_t id;
	struct MemData memory = { 0 };
    int ret = 1;
    if (ubus_connect_helper(&ctx, "system", &id) == 1)
        return ret;
    if (ubus_invoke(ctx, id, "info", NULL, board_cb, &memory, 1000) == 1)
		log_event(LOGS_ERROR, "Failed to request system info");
    else {
		*free_memory = memory.free;
		ret = 0;
	}
    ubus_free(ctx);
	return ret;
}

int ubus_esp_get_devices(char **buffer)
{
    struct DevicesData devices_data = { 0 };
    struct ubus_context *ctx;
    uint32_t id;
    int ret = 1;
    if (ubus_connect_helper(&ctx, "usb_controller", &id) == 1)
        return ret;
    else if (ubus_invoke(ctx, id, "get", NULL, devices_cb, &devices_data, 1000) == 1)
		log_event(LOGS_ERROR, "Failed to request device info");
    else {
		snprintf(buffer, 300, "%s", devices_data.devices);
        ret = 0;
	}
    ubus_free(ctx);
    return ret;
}

int ubus_esp_toggle(char *func, char *device, int pin, int *response, char **msg)
{
    cJSON *args = cJSON_CreateObject();
    cJSON_AddStringToObject(args, "name", device);
    cJSON_AddNumberToObject(args, "pin", pin);
    struct blob_buf b = {};
    blob_buf_init(&b, 0);
    char *args_json = cJSON_PrintUnformatted(args);
    blobmsg_add_json_from_string(&b, args_json);

    struct ResponseData response_data = { 0 };
    struct ubus_context *ctx;
    uint32_t id;
    int ret = 1;
    if (ubus_connect_helper(&ctx, "usb_controller", &id) == 1) {
        return ret;
    } else if (ubus_invoke(ctx, id, func, b.head, response_handler, &response_data, 2100) == 1) {
        log_event(LOGS_ERROR, "Failed to invoke ubus toggle");
    } else if (response_data.msg == NULL && response_data.response == 0) {
        snprintf(msg, 100, "%s", "Device does not exist");
        *response = 1;
    } else {
        snprintf(msg, 100, "%s", response_data.msg);
        *response = response_data.response;
    }
    cJSON_Delete(args);
    free(args_json);
    blob_buf_free(&b);
    ubus_free(ctx);
}
