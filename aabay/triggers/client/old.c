#include <mosquitto.h>
#include <libyottadb.h>
#include <stdio.h>
#include <string.h>

#define BROKER_PORT 1883
#define BROKER_HOSTNAME "localhost"

void createClientAndPublish(int count, ydb_char_t *topic, ydb_char_t *payload)
{

  int initResult = mosquitto_lib_init();
  struct mosquitto *client;

  if(initResult != MOSQ_ERR_SUCCESS) 
    return;

  client = mosquitto_new(NULL, true, NULL);

  if(client == NULL) 
    return;
  
  int conn_result = mosquitto_connect(client, BROKER_HOSTNAME, BROKER_PORT, 0);

  if(conn_result != MOSQ_ERR_SUCCESS)
    return;

  int pub_result = mosquitto_publish(
    client,
    NULL,
    topic,
    strlen(payload),
    payload,
    0,
    true
  );

  if(pub_result != MOSQ_ERR_SUCCESS)
    return;

  // clean up
  mosquitto_disconnect(client);
  mosquitto_destroy(client);
}

