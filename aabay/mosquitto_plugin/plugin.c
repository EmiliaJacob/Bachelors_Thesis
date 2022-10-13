
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
// c++ includes
#include <iostream>
#include <string>

#include "ydb-global.h"

void convert_and_send_spooled_messages();

#define UNUSED(A) (void)(A)

static mosquitto_plugin_id_t * mosq_pid = NULL;
static char *spooled_messages;

static int callback_basic_auth(int event, void *event_data, void *userdata) 
 {
	struct mosquitto_evt_basic_auth * basic_auth_event_data = (mosquitto_evt_basic_auth*)event_data;
 	
 	mosquitto_log_printf(MOSQ_LOG_INFO, "basic_auth callback received: %s / %s", basic_auth_event_data->username, basic_auth_event_data->password);
		
	// if(!basic_auth_event_data->username && !basic_auth_event_data->password) // TODO: durch strcmp ersetzen
	return MOSQ_ERR_SUCCESS;
	
	// if(basic_auth_event_data->username == "felix" && basic_auth_event_data->password == "jacob")
		// return MOSQ_ERR_SUCCESS;
	
	// return MOSQ_ERR_AUTH;
}

static int callback_acl_check(int event, void *event_data, void *userdata) 
 {
 	// TODO: ACL file definieren
 	// TODO: return value von callins checken
 	
 	int status_code = MOSQ_ERR_ACL_DENIED;
	struct mosquitto_evt_acl_check * acl_event_data = (mosquitto_evt_acl_check*)event_data;
	
	mosquitto_log_printf(MOSQ_LOG_INFO, "acl_check callback received topic %s acc %d msg %s", acl_event_data->topic, acl_event_data->access, acl_event_data->payload);
	
//	if(acl_event_data->access == MOSQ_ACL_SUBSCRIBE) {
//		if(acl_event_data->topic == "chat") {
//			// do spool^MOSQUITTO("m","","chat",clid_" has joined") ; als ziel fuer das spooling wird die lokale variable m gewaehlt. sie ist wahrscheinlich im shared memory zusammen mit den statischen variabeln des c triggers
//			status_code = MOSQ_ERR_SUCCESS;
//		}
//	}
//	
//	if(acl_event_data->access == MOSQ_ACL_WRITE) {
//		
//		// . do spool^MOSQUITTO("m",clid,topic,"ok") ; mqttfetch-response wird der lokale var m hinzugefuegt
//		// . do spool^MOSQUITTO("m","","chat",payload) ; chatnachricht des clients wird an alle anderen chats subscriber releast
//		status_code = MOSQ_ERR_SUCCESS;
//	}
//	
//	if(acl_event_data->access == MOSQ_ACL_READ) {
//		status_code = MOSQ_ERR_SUCCESS;
//	}
	
	// set resp=$$convert^MOSQUITTO(.m) ; response wird auf alle gespoolten nachrichten in stringform gesetzt
	
	// if (ci_success == YDB_OK && ci_return_val == YDB_OK)
	// 	resp_2_send();
	
	status_code = MOSQ_ERR_SUCCESS;
	return status_code;
}

static int callback_disconnect(int event, void *event_data, void *userdata) 
{
	struct mosquitto_evt_disconnect * disconnect_event_data = (mosquitto_evt_disconnect*)event_data;

	mosquitto_log_printf(MOSQ_LOG_INFO, "MOSQ_EVT_DISCONNECT %s\n", mosquitto_client_id(disconnect_event_data->client));
	
	return MOSQ_ERR_SUCCESS;
}

void convert_and_send_spooled_messages() 
{
	char *i_p= spooled_messages+1, *t_p, *m_p;
	int i_l, t_l, m_l, n= spooled_messages[0];
	
	mosquitto_log_printf(MOSQ_LOG_INFO, "------------RESP_2_SEND------------");
	
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
	
	mosquitto_log_printf(MOSQ_LOG_INFO, "------------RESP_2_SEND------------\n");
	
	spooled_messages[0] = '\0';
}

 static int callback_message(int event, void *event_data, void *userdata)
{
	/*
	 unused - nicht verwendet!!!
	 */
//	struct mosquitto_evt_message * ed = event_data;
//	static ci_name_descriptor ci = {{sizeof("MSG")-1, "MSG"},NULL};
//	int rc;
	return MOSQ_ERR_SUCCESS;
/*	mosquitto_log_printf(MOSQ_LOG_INFO, "MOSQ_EVT_MESSAGE client %s user %s", mosquitto_client_id(ed->client), mosquitto_client_username(ed->client));
	mosquitto_log_printf(MOSQ_LOG_INFO, "%d %lu", ed->payloadlen, (long unsigned) ed->payload);
	printf("MOSQ_EVT_MESSAGE client %s user %s\n", mosquitto_client_id(ed->client), mosquitto_client_username(ed->client));
	printf("%d %lu\n", ed->payloadlen, (long unsigned) ed->payload);
	rc = ydb_cip(&ci, response, mosquitto_client_id(ed->client), mosquitto_client_username(ed->client) ? mosquitto_client_username(ed->client) : "", ed->topic, ed->payload? ed->payload : "");
	mosquitto_log_printf(MOSQ_LOG_INFO,"Result: %d", rc);
	resp_2_send();
	return MOSQ_ERR_SUCCESS;*/
}

int get_and_send_spooled_messages(){

	c_ydb_global _mqttspool("^ms");
	c_ydb_global dummy("dummy");

	string iterator = "";

	if(!_mqttspool.hasChilds())
		return MOSQ_ERR_SUCCESS;

	_mqttspool.lock_inc(1);

	while(iterator=_mqttspool[iterator].nextSibling(), iterator!=""){
		dummy[iterator] = iterator;		
		dummy[iterator]["t"] = (string)_mqttspool[iterator]["t"];
		dummy[iterator]["c"] = (string)_mqttspool[iterator]["c"];
		dummy[iterator]["m"] = (string)_mqttspool[iterator]["m"];
	}

	_mqttspool.kill();
	_mqttspool.lock_dec();

	iterator = "";

	while(iterator=dummy[iterator].nextSibling(), iterator!=""){
		// TODO return ERR Messages everywhere

		int qos = 1; // TODO: Move evtl elsewhere
		bool retain = true;
		mosquitto_property *properties = NULL;

		int result = mosquitto_broker_publish_copy(
			//((string)dummy[iterator]["c"]).c_str(), sendet Message nur an C mit entpsrechender ID
			NULL,
			((string)dummy[iterator]["t"]).c_str(),
			strlen(((string)dummy[iterator]["m"]).c_str()), // \0 fuer message nicht relevant
			((string)dummy[iterator]["m"]).c_str(),
			qos,
			retain,
			properties
		);

		if (result != MOSQ_ERR_SUCCESS) {
			dummy.kill();
			return result;
		}
	}
	
	dummy.kill();
	return MOSQ_ERR_SUCCESS;
}


static int callback_tick(int event, void *event_data, void *userdata) 
{
	return get_and_send_spooled_messages();
}


int mosquitto_plugin_version(int supported_version_count, const int *supported_versions)
{
	mosquitto_log_printf(MOSQ_LOG_INFO, "mosquitto_plugin_version\n");
	
	for(int i=0; i<supported_version_count; i++) {
		if(supported_versions[i] == 5)
			return 5;
	}
	
	return -1;
}

char ci_fn[64];
 
int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	//int rc;
	char * rou;
	FILE * ci_fp;
	mosq_pid = identifier;
	
//	if (!(spooled_messages = malloc(1000000)))
//		return MOSQ_ERR_NOMEM;
	
	// Optionen auswerten
	mosquitto_log_printf(MOSQ_LOG_INFO, "init %d\n", opt_count);
	for (int i = 0; i < opt_count; i++) {
		mosquitto_log_printf(MOSQ_LOG_INFO, "\t%d %s %s\n", i, opts[i].key + 4, opts[i].value);
		if (!strncmp(opts[i].key, "env-", 4))
			setenv(opts[i].key + 4, opts[i].value, 1);
		else if (!strcmp(opts[i].key, "rou"))
			rou = opts[i].value, mosquitto_log_printf(MOSQ_LOG_INFO, "Routine '%s'\n", rou);
	}
	
	//setenv("ydb_dir", "/home/wbantel/.yottadb", 1);
	//setenv("ydb_gbldir", "/home/wbantel/.yottadb/r1.24_x86_64/g/yottadb.gld", 1);
	//setenv("ydb_routines", "/home/wbantel/.yottadb/r1.24_x86_64/o*(/home/wbantel/.yottadb/r1.24_x86_64/r /home/wbantel/.yottadb/r) /usr/local/lib/yottadb/r124/plugin/o/_ydbposix.so /usr/local/lib/yottadb/r124/libyottadbutil.so /usr/local/lib/yottadb/r124/libyottadb.so", 1);
	//setenv("ydb_rel", "r1.24_x86_64", 1);
	
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
//	rc = ydb_init();
//	mosquitto_log_printf(MOSQ_LOG_INFO,"ydb_init: %d\n", rc);
	
	//user_data beschreiben - derzeit nicht genutzt, nur zum Lernen!
	char userdata_text[] = "123";
	*user_data = mosquitto_malloc(sizeof(userdata_text));
	memcpy(*user_data, userdata_text, sizeof(userdata_text));

	// Callback-Fkt registrieren
	return
	mosquitto_callback_register(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_basic_auth, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_DISCONNECT, callback_disconnect, NULL, *user_data);
	//|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, *user_data)
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	remove(ci_fn);

	// mosq_pid aus mosquitto_plugin_init!!!
	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_message, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_DISCONNECT, callback_disconnect, NULL);
	// ||mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL)
}