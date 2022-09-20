/*
 Mosquitto-Plugin fuer YottaDB
 
 Erwartete Schnittstellen in YottaDB:
 AUTH(clid,user,pass) -> 0 (OK) oder Nicht-0 (NOK)
 MSG(clid,user,topic,msg,ok) -> str
 ACL(clid,user,topic,access,ok) -> str
 TICK() -> str

 dabei ist str ein laenencodierter String - siehe Funktion convert in M oder resp_2_send hier
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
	if (ed->access == 1) // READ geht immer
		return MOSQ_ERR_SUCCESS;
	mosquitto_log_printf(MOSQ_LOG_INFO, "MOSQ_EVT_ACL_CHECK topic %s acc %d user %s\n", ed->topic, ed->access, mosquitto_client_username(ed->client) ? mosquitto_client_username(ed->client) : "");
	rc = ydb_cip(&ci, resp, mosquitto_client_id(ed->client), mosquitto_client_username(ed->client) ? mosquitto_client_username(ed->client) : "", ed->topic, ed->access, &rc2);//, "user", "topic", &(ed->access), &rc2);
	resp_2_send();
	return rc2;
}

static int callback_disconnect(int event, void *event_data, void *userdata) {
	struct mosquitto_evt_disconnect * ed = event_data;
	printf("MOSQ_EVT_DISCONNECT %s\n", mosquitto_client_id(ed->client));
	return MOSQ_ERR_SUCCESS;
}

void resp_2_send() {
	char *i_p = resp+1, *t_p, *m_p;
	int i_l, t_l, m_l, n= resp[0];
	
	mosquitto_log_printf(MOSQ_LOG_INFO,"%d Nachrichten", n);
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

}
static int callback_message(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_message * ed = event_data;
	static ci_name_descriptor ci = {{sizeof("MSG")-1, "MSG"},NULL};
	int rc;
	
	mosquitto_log_printf(MOSQ_LOG_INFO, "MOSQ_EVT_MESSAGE client %s user %s", mosquitto_client_id(ed->client), mosquitto_client_username(ed->client));
	mosquitto_log_printf(MOSQ_LOG_INFO, "%d %lu", ed->payloadlen, (long unsigned) ed->payload);
	printf("MOSQ_EVT_MESSAGE client %s user %s\n", mosquitto_client_id(ed->client), mosquitto_client_username(ed->client));
	printf("%d %lu\n", ed->payloadlen, (long unsigned) ed->payload);
	rc = ydb_cip(&ci, resp, mosquitto_client_id(ed->client), mosquitto_client_username(ed->client) ? mosquitto_client_username(ed->client) : "", ed->topic, ed->payload? ed->payload : "");
	mosquitto_log_printf(MOSQ_LOG_INFO,"Result: %d", rc);
	resp_2_send();
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
	printf("init %d\n", opt_count);
	for (int i = 0; i < opt_count; i++) {
		printf("\t%d %s %s\n", i, opts[i].key + 4, opts[i].value);
		if (!strncmp(opts[i].key, "env-", 4))
			setenv(opts[i].key + 4, opts[i].value, 1);
		else if (!strcmp(opts[i].key, "rou"))
			rou = opts[i].value, printf("Routine '%s'\n", rou);
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
		"MSG:  ydb_char_t * MSG^%s(I:ydb_char_t*, I:ydb_char_t*, I:ydb_char_t*, I:ydb_char_t*)\n"
		"ACL:  ydb_char_t * ACL^%s(I:ydb_char_t*, I:ydb_char_t*, I:ydb_char_t*, I:ydb_int_t, O:ydb_int_t*)\n"
		"TICK: ydb_char_t * TICK^%s()\n"
		, rou, rou, rou, rou
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
	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_basic_auth, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_ACL_CHECK, callback_acl_check, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL, *user_data)
	|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_DISCONNECT, callback_disconnect, NULL, *user_data);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	remove(ci_fn);

	// mosq_pid aus mosquitto_plugin_init!!!
	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_message, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_ACL_CHECK, callback_acl_check, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_DISCONNECT, callback_disconnect, NULL);

}

