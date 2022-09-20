#include "eventCallbacks.h"
#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "eventCallbacks.h"
//#include "libyottadb.h"

int onReload(){

}

int onAclCheck(){

}

int onBasicAuth(int event, void *event_data, void *userdata){
	struct mosquitto_evt_basic_auth * ed = event_data;
	mosquitto_log_printf(MOSQ_LOG_INFO, "slkfdjdlksfjldskfjldkf %s / %s / %s", mosquitto_client_id(ed->client), ed->username, ed->password);
    return MOSQ_ERR_AUTH;
}

int onExtAuthStart(){

}

int onControl(){

}

int onMessage(int event, void *event_data, void *userdata){
	struct mosquitto_evt_message * ed = event_data;
    mosquitto_log_printf(MOSQ_LOG_INFO, ed->topic);
    mosquitto_log_printf(MOSQ_LOG_INFO, ed->payload);
    return 0;
}

int onPskKey(){

}


int onTick(){
    //mosquitto_log_printf(MOSQ_LOG_INFO, "TOCK");
    return 0;
}


int onDisconnect(){

}