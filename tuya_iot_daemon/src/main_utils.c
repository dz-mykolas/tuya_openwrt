#include "main_utils.h"

#define LOGS_ERROR   3
#define LOGS_WARNING 4

struct MemData {
	int total;
	int free;
	int shared;
	int buffered;
};

enum {
	TOTAL_MEMORY,
	FREE_MEMORY,
	SHARED_MEMORY,
	BUFFERED_MEMORY,
	__MEMORY_MAX,
};

enum {
	MEMORY_DATA,
	__INFO_MAX,
};

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY]	  = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY]	  = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
	[SHARED_MEMORY]	  = { .name = "shared", .type = BLOBMSG_TYPE_INT64 },
	[BUFFERED_MEMORY] = { .name = "buffered", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
};

volatile sig_atomic_t running = 1;
void tuya_loop(tuya_mqtt_context_t **client, struct ubus_context **ctx)
{
	int ubus_fail	 = 1;
	int free_memory	 = 0;
	int retry_count	 = 0;
	int report_count = 0;
	while (running) {
		/* RETRIES UBUS 5 TIMES */
		if (ubus_fail == 1 && retry_count < 5) {
			free_memory = -1;
			ubus_fail   = ubus_get_memory(ctx, &free_memory);
			retry_count++;
		}
		/* REPORTS FREE MEMORY EVERY 10 SUCCESSFUL LOOPS */
		if (report_count % 10 == 0 && ubus_fail == 0) {
			char buffer[100];
			snprintf(buffer, sizeof(buffer), "{\"ram_free\":{\"value\":\"%dMB\"}}",
				 free_memory / (1024 * 1024));
			tuyalink_thing_property_report(*client, (*client)->config.device_id, buffer);
			report_count = 0;
			retry_count  = 0;
			ubus_fail    = 1;
		}
		tuya_mqtt_loop(*client);
		report_count++;
	}
}

int program_deinit(tuya_mqtt_context_t **client, struct ubus_context **ctx)
{
	int exit = 0;
	if (tuya_mqtt_disconnect(*client) || tuya_mqtt_deinit(*client)) {
		log_event(LOGS_ERROR, "Error while disconnecting from tuya");
		exit = EXIT_FAILURE;
	}
	ubus_free(*ctx);
	log_event(LOGS_WARNING, "Device properly disconnected");
	return exit;
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

static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct MemData *memoryData = (struct MemData *)req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[MEMORY_DATA]) {
		return;
	}

	blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY_DATA]),
		      blobmsg_data_len(tb[MEMORY_DATA]));

	memoryData->total    = blobmsg_get_u64(memory[TOTAL_MEMORY]);
	memoryData->free     = blobmsg_get_u64(memory[FREE_MEMORY]);
	memoryData->shared   = blobmsg_get_u64(memory[SHARED_MEMORY]);
	memoryData->buffered = blobmsg_get_u64(memory[BUFFERED_MEMORY]);
}

int ubus_get_memory(struct ubus_context **ctx, int *free_memory)
{
	uint32_t id;
	struct MemData memory = { 0 };

	if (*ctx == NULL) {
		if (!(*ctx = ubus_connect(NULL)))
			log_event(LOGS_ERROR, "Error while connecting to ubus");
	} else if (ubus_lookup_id(*ctx, "system", &id) == 1)
		log_event(LOGS_ERROR, "Failed to find system object on ubus");
	else if (ubus_invoke(*ctx, id, "info", NULL, board_cb, &memory, 1000) == 1)
		log_event(LOGS_ERROR, "Failed to request system info");
	else {
		*free_memory = memory.free;
		return 0;
	}
	return 1;
}