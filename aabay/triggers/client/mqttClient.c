#include <mosquitto.h>
#include <libyottadb.h>
#include <stdio.h>
#include <string.h>

#define BROKER_PORT 1883
#define BROKER_HOSTNAME "localhost"

void createClientAndPublish(int count, ydb_char_t *topic, ydb_char_t *payload) {

  static int initResult = -1;
  static int connResult = -1; // passiert init nur einmal? wahrscheinlich

  static struct mosquitto *client = NULL;
  printf("JJJ %d\n", connResult);

  if(connResult != MOSQ_ERR_SUCCESS) {
    printf("HELLO");

    if((initResult = mosquitto_lib_init()) != MOSQ_ERR_SUCCESS) {
      return;
    }
    if((client = mosquitto_new(NULL, true, NULL)) == NULL) {
      return;
    }
    if((connResult = mosquitto_connect_async(client, BROKER_HOSTNAME, BROKER_PORT, 10)) != MOSQ_ERR_SUCCESS) {
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
  
  // clean up
  //mosquitto_disconnect(client);
  //mosquitto_destroy(client);
}

int main() {
  createClientAndPublish(1, "hello", "world");
  createClientAndPublish(2, "hello", "world");
  return 0;
}
