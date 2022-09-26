#include <mosquitto.h>
#include <libyottadb.h>
#include <stdio.h>
#include <string.h>

#define BROKER_PORT 1883
#define BROKER_HOSTNAME "localhost"

void createClientAndPublish(ydb_char_t *topic, ydb_char_t *payload) {

  static int initResult = -1;
  static int connResult = -1;

  static struct mosquitto *client = NULL;

  if(connResult != MOSQ_ERR_SUCCESS) {

    if((initresult = mosquitto_lib_init()) != mosq_err_success) {
      return;
    }
    if((client = mosquitto_new(NULL, true, NULL)) == NULL) {
      return;
    }
    if((conn_result = mosquitto_connect_async(client, BROKER_HOSTNAME, BROKER_PORT, 10)) != MOSQ_ERR_SUCCESS) {
      return;
    }

    mosquitto_loop_start(client);
  }

  if(connResult != MOSQ_ERR_SUCCESS) {
    return;
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
  
  printf("hi\n");

  // clean up
  mosquitto_disconnect(client);
  mosquitto_destroy(client);
}

int main() {
  createClientAndPublish("hello", "world");
  return 0;
}
