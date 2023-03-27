#include "main_utils.h"
#include "tuya_cacert.h"

#define LOGS_ERROR 3
#define LOGS_WARNING 4
#define LOGS_NOTICE 5

/* ARGP */
const char *argp_program_version = 
    "simple_daemon 1.0";
const char *argp_program_bug_address = 
    "<none>";
static char doc[] = 
    "Simple daemon that connects to Tuya Cloud using it's C SDK";
static char args_doc[] = 
    "product_id device_id device_secret";
static struct argp_option options[] = {
    { 0 }
};
static struct argp argp = { options, parse_opt, args_doc, "Simple daemon"};

/* UBUS */
static struct ubus_context *ctx;

/* TUYA */
tuya_mqtt_context_t client_instance;

/* INFINITE LOOP */
volatile sig_atomic_t running = 1;

int main(int argc, char **argv)
{
    /* SIGNAL HANDLING */
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handler;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    
    /* ARGUMENT PARSING */
    struct arguments arguments = { 0 };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    /* TUYA */
    const tuya_mqtt_config_t config= {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = arguments.args[1],
        .device_secret = arguments.args[2],
        .keepalive = 60,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages
    };
    int ret;
    tuya_mqtt_context_t *client = &client_instance;
    ret = tuya_mqtt_init(client, &config);
    if (ret != OPRT_OK) {
        log_event(LOGS_ERROR, "Error while initializing!");
        tuya_mqtt_deinit(client);
        return EXIT_FAILURE;
    }
    ret = tuya_mqtt_connect(client);
    if (ret != OPRT_OK) {
        log_event(LOGS_ERROR, "Error while connecting!");
        tuya_mqtt_deinit(client);
        return EXIT_FAILURE;
    }
    log_event(LOGS_NOTICE, "Device Connected");
    
    /* INFINITE LOOP */
    int ubus_fail = 1;
    int free_memory = 0;
    for(int i = 0, j = 0; running != 0; i++)
    {
        /* RETRIES UBUS 5 TIMES */
        if (ubus_fail == 1 && j < 5) {
            free_memory = -1;
            ubus_fail = ubus_get_memory(&ctx, &free_memory);
            j++;
        }
        if (i % 10 == 0 && ubus_fail == 0) {
            char buffer[100];
            snprintf(buffer, 100, "{\"ram_free\":{\"value\":\"%dMB\"}}", free_memory / (1024 * 1024));
            tuyalink_thing_property_report(client, arguments.args[1], buffer);
            i = 0;
            j = 0;
            ubus_fail = 1;
        }
        tuya_mqtt_loop(client);
    }

    /* DISCONNECT */
    ret = tuya_mqtt_disconnect(client);
    if (ret != OPRT_OK) {
        log_event(LOGS_ERROR, "Error while disconnecting!");

    } else {
        log_event(LOGS_WARNING, "Device Disconnected");
    }
    ret = tuya_mqtt_deinit(client);
    ubus_free(ctx);
    return ret;
}
