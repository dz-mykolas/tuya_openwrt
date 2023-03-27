#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <argp.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>

#include "daemon_utils.h"
#include "tuya_utils.h"

struct arguments {
  char *args[3];
};

error_t parse_opt(int key, char *arg, struct argp_state *state);
void sig_handler(int sig);
uint64_t memory_cb(struct ubus_request *req, int type, struct blob_attr *msg);
int ubus_get_memory(struct ubus_context **ctx, int *free_memory);