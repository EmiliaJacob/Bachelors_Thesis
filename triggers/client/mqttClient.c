#include <mosquitto.h>
#include <libyottadb.h>
#include <stdio.h>
#include <string.h>

#define BROKER_PORT 1883
#define BROKER_HOSTNAME "localhost"

void publishMqttMessage(int count, ydb_char_t *topic, ydb_char_t *payload) {
  if(count != 2)
    return;
  
  static int conn_result = -1; 

  static struct mosquitto *client = NULL;

  if(conn_result != MOSQ_ERR_SUCCESS) { 
    if(mosquitto_lib_init() != MOSQ_ERR_SUCCESS) {
      return;
    }
    if((client = mosquitto_new(NULL, true, NULL)) == NULL) {
      return;
    }
    if(mosquitto_loop_start(client) != MOSQ_ERR_SUCCESS) {
      return;
    }
    if((conn_result = mosquitto_connect(client, BROKER_HOSTNAME, BROKER_PORT, 10)) != MOSQ_ERR_SUCCESS) {
      return;
    }
  }

  int pub_result = mosquitto_publish (
    client,
    NULL,
    topic,
    strlen(payload),
    payload,
    0,
    true
  );

  if(pub_result != MOSQ_ERR_SUCCESS) {
    return;
  }
}