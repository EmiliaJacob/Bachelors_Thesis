#include "eventCallbacks.h"
#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "eventCallbacks.h"

static mosquitto_plugin_id_t *pluginId = NULL;
static void **userData = NULL;

int mosquitto_plugin_version(int supported_version_count, const int *supported_versions){
   return 5; // TODO: make constant
}

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count) {
    pluginId = identifier; 
    userData = user_data;
   mosquitto_log_printf(MOSQ_LOG_INFO, "This plugin was sucessfully loaded");
    mosquitto_callback_register(pluginId, MOSQ_EVT_TICK, onTick, NULL, *user_data);
    mosquitto_callback_register(pluginId, MOSQ_EVT_MESSAGE, onMessage, NULL, *user_data);
    mosquitto_callback_register(pluginId, MOSQ_EVT_BASIC_AUTH, onBasicAuth, NULL, *user_data);
    return 0; //TODO: return >0 when error occurs
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count){
    return 0; //TODO: make return dependant
}
