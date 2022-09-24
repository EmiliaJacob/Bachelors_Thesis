/*
 Mosquitto-Plugin fuer YottaDB
 

 dabei ist str ein laengencodierter String - siehe Funktion convert in M oder resp_2_send hier
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"

#include <libyottadb.h>

void resp_2_send();

#define UNUSED(A) (void)(A)

static mosquitto_plugin_id_t * mosq_pid = NULL;
static char * resp;

static int callback_basic_auth(int event, void *event_data, void *userdata) {
	struct mosquitto_evt_basic_auth * ed = event_data;
	static ci_name_descriptor ci_auth = {{sizeof("AUTH")-1, "AUTH"},NULL};
	int rc, rc2;
	rc = ydb_cip(&ci_auth, &rc2, mosquitto_client_id(ed->client), ed->username ? ed->username : "", ed->password ? ed->password : "");
	mosquitto_log_printf(MOSQ_LOG_INFO, "MOSQ_EVT_BASIC_AUTH %s / %s", ed->username, ed->password);
	mosquitto_log_printf(MOSQ_LOG_INFO, "auth: %d '%d'", rc, rc2);
	return !rc2 ? MOSQ_ERR_SUCCESS : MOSQ_ERR_AUTH;
}

static int callback_acl_check(int event, void *event_data, void *userdata) {
	struct mosquitto_evt_acl_check * ed = event_data;
	static ci_name_descriptor ci = {{sizeof("ACL")-1, "ACL"},NULL};
	int rc, rc2 = 1;
	mosquitto_log_printf(MOSQ_LOG_INFO, "MOSQ_EVT_ACL_CHECK topic %s acc %d msg %s\n", ed->topic, ed->access, ed->payload);
	rc = ydb_cip(&ci, &rc2,
		mosquitto_client_id(ed->client),
		mosquitto_client_username(ed->client) ? mosquitto_client_username(ed->client) : "",
		ed->access,
		ed->topic,
		ed->payloadlen,
		ed->payload ? ed->payload : "",
		resp
	);
	mosquitto_log_printf(MOSQ_LOG_INFO, "rc=%d rc2=%d   '%d'\n", rc, rc2, *resp);
  if (!rc && !rc2){
    mosquitto_log_printf(MOSQ_LOG_INFO, "HELLO");
		resp_2_send();
  }
  else{
    mosquitto_log_printf(MOSQ_LOG_INFO, "NO");
		resp_2_send();
  }
	return 0;
}

static int callback_disconnect(int event, void *event_data, void *userdata) {
	struct mosquitto_evt_disconnect * ed = event_data;
	mosquitto_log_printf(MOSQ_LOG_INFO, "MOSQ_EVT_DISCONNECT %s\n", mosquitto_client_id(ed->client));
	return MOSQ_ERR_SUCCESS;
}

void resp_2_send() {
	char *i_p = resp+1, *t_p, *m_p;
	int i_l, t_l, m_l, n= resp[0];
	
	mosquitto_log_printf(MOSQ_LOG_INFO, "Sende %d Nachrichten", n);
	for (int i = 0; i < n; i++) {
		mosquitto_log_printf(MOSQ_LOG_INFO, "Nachricht %d:",i);
		i_l = 256 * *i_p + *(i_p + 1);
		mosquitto_log_printf(MOSQ_LOG_INFO, " %d '%s'", i_l, i_p + 2);
		t_p = i_p + i_l + 3;
		t_l = 256 * *t_p + *(t_p + 1);
		mosquitto_log_printf(MOSQ_LOG_INFO, " %d '%s'", t_l, t_p + 2);
		m_p = t_p + t_l + 3;
		m_l = 256 * *m_p + *(m_p + 1);
		mosquitto_log_printf(MOSQ_LOG_INFO, " %d '%s'", m_l, m_p + 2);
		mosquitto_broker_publish_copy(i_l ? i_p + 2 : NULL, t_p + 2, m_l, m_p + 2, 0, 0, NULL);
		i_p = m_p + 256 * *m_p + *(m_p + 1) + 3;
	}
	resp[0] = '\0';
}
static int callback_message(int event, void *event_data, void *userdata)
{
  struct mosquitto_evt_message * ed = event_data;

  //check for right topic
  char topicArray[strlen(ed->topic)+1];
  strcpy(topicArray, ed->topic);

  //char *articleId = NULL;
  char *delim = "/";

  int number_of_subtopics = 1;
  char *subtopic = strtok(topicArray, delim);

  ydb_buffer_t articleId;

  while(subtopic != NULL) {

    mosquitto_log_printf(MOSQ_LOG_INFO, "%d SUBTOPIC FOUND: %s\n", number_of_subtopics, subtopic);

    switch(number_of_subtopics){
      case 1:
        if(strcmp(subtopic,"aabay") != 0){
          return MOSQ_ERR_SUCCESS;
        }
        break;
      case 2:
        if(strcmp(subtopic,"bids") != 0){
          return MOSQ_ERR_SUCCESS;
        }
        break;
      case 3:
        //articleId = subtopic;
        YDB_LITERAL_TO_BUFFER(subtopic, &articleId);
        break;
      case 4:
        return MOSQ_ERR_SUCCESS;
        break;
    }

    number_of_subtopics += 1;

    subtopic = strtok(NULL, delim);
  }

  //ydb.set("^hello",0,NULL,"world");
  
  ydb_buffer_t g_var; // TODO: move somewhere else
  YDB_LITERAL_TO_BUFFER("^latestBid", &g_var);
  int ydb_set_s_result = ydb_set_s(&g_var, 0, NULL, &articleId); // TODO: check for return
  if(ydb_set_s_result == YDB_OK){
    mosquitto_log_printf(MOSQ_LOG_INFO, "SETTING THE GLOBAL VAR WERKED");
  }
	mosquitto_log_printf(MOSQ_LOG_INFO, "topic: %s, payload: %s",ed->topic, ed->payload);
	return MOSQ_ERR_SUCCESS;
}

static int callback_tick(int event, void *event_data, void *userdata) {
	int rc;
	static ci_name_descriptor ci = {{sizeof("TICK")-1, "TICK"},NULL};
	rc = ydb_cip(&ci, resp);
	if (resp[0]) {
		mosquitto_log_printf(MOSQ_LOG_INFO, "MOSQ_EVT_TICK");
		resp_2_send();
	}
	return rc;
}

int mosquitto_plugin_version(int supported_version_count, const int *supported_versions)
{
	mosquitto_log_printf(MOSQ_LOG_INFO, "mosquitto_plugin_version\n");
	for(int i=0; i<supported_version_count; i++)
		if(supported_versions[i] == 5)
			return 5;
	return -1;
}

char ci_fn[64];
int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	int rc;
	char * rou;
	FILE * ci_fp;
	mosq_pid = identifier;
	
	if (!(resp = malloc(1000000)))
		return MOSQ_ERR_NOMEM;
	
	// Optionen auswerten
	mosquitto_log_printf(MOSQ_LOG_INFO, "init %d\n", opt_count);
	for (int i = 0; i < opt_count; i++) {
		mosquitto_log_printf(MOSQ_LOG_INFO, "\t%d %s %s\n", i, opts[i].key + 4, opts[i].value);
		if (!strncmp(opts[i].key, "env-", 4))
			setenv(opts[i].key + 4, opts[i].value, 1);
		else if (!strcmp(opts[i].key, "rou"))
			rou = opts[i].value, mosquitto_log_printf(MOSQ_LOG_INFO, "Routine '%s'\n", rou);
	}

	sprintf(ci_fn, "/tmp/mosquitto-ydb-%d.ci", getpid());
	printf("%s\n", ci_fn);
	ci_fp = fopen(ci_fn, "w");
	fprintf(ci_fp,
		"AUTH: ydb_int_t * AUTH^%s(I:ydb_char_t*, I:ydb_char_t*, I:ydb_char_t*)\n"
		"ACL:  ydb_int_t* ACL^%s(I:ydb_char_t*, I:ydb_char_t*, I:ydb_int_t, I:ydb_char_t*, I:ydb_int_t, I:ydb_char_t*, O:ydb_char_t *)\n"
		"TICK: ydb_char_t * TICK^MOSQUITTO()\n"
		, rou, rou
	);
	fclose(ci_fp);
	setenv("ydb_ci", ci_fn, 1);
	rc = ydb_init();
	mosquitto_log_printf(MOSQ_LOG_INFO,"ydb_init: %d\n", rc);
	
	//user_data beschreiben - derzeit nicht genutzt, nur zum Lernen!
	char userdata_text[] = "123";
	*user_data = mosquitto_malloc(sizeof(userdata_text));
	memcpy(*user_data, userdata_text, sizeof(userdata_text));

	// Callback-Fkt registrieren
	return
	mosquitto_callback_register(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_basic_auth, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_ACL_CHECK, callback_acl_check, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_DISCONNECT, callback_disconnect, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, *user_data);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	remove(ci_fn);

	// mosq_pid aus mosquitto_plugin_init!!!
	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_message, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_ACL_CHECK, callback_acl_check, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_DISCONNECT, callback_disconnect, NULL);
	// ||mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL)
	

}
