#include <libubus.h>
#include <libubox/blobmsg_json.h>

#include "cJSON.h"

#define LOGS_ERROR   3
#define LOGS_WARNING 4

struct MemData {
	int total;
	int free;
	int shared;
	int buffered;
};

struct DevicesData {
	char *devices;
};

struct ResponseData {
	int response;
    char *msg;
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

enum {
    ATTR_RESPONSE,
    ATTR_MSG,
    __ATTR_MAX
};


static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg);
static void devices_cb(struct ubus_request *req, int type, struct blob_attr *msg);

int ubus_connect_helper(struct ubus_context **ctx, char *object, uint32_t *id);
int ubus_get_memory(int *free_memory);
int ubus_esp_get_devices(char **buffer);
int ubus_esp_toggle(char *func, char *device, int pin, int *response, char **msg);
