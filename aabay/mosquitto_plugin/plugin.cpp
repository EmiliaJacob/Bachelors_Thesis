
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
// #include "mosquitto_internal.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"

// FIFO 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// MQ
#include <mqueue.h> 
#include <errno.h>

// C++ 
#include <iostream>
#include <string>
#include "ydb-global.h"
#include <regex>
#include "json.h"
#include <sstream>
using std::istringstream;
using std::string;
using std::cout;

// message callback
Json::StreamWriterBuilder builder;
c_ydb_global _articles("^articles");
bool literal_to_json(Json::Value *json, char *literal);
bool publish_response_message(string topic, Json::Value *payload); 
bool publish_response_message(string topic, string payload);

#define UNUSED(A) (void)(A)

static mosquitto_plugin_id_t * mosq_pid = NULL;
//static char *spooled_messages;

static const int QOS_SPOOL = 0;
static const bool RETAIN_SPOOL = false;
static mosquitto_property *PROPERTIES_SPOOL = NULL;

static const int QOS_RESPONSE = 0;
static const bool RETAIN_RESPONSE = false;
static mosquitto_property *PROPERTIES_RESPONSE = NULL;

int get_and_send_spooled_messages();

int receive_mq_messages();

int get_and_send_spooled_messages()
{
	c_ydb_global _mqttspool("^ms"); // TODO: Make static | Where should declaration be in C++?
	c_ydb_global dummy("dummy");

	string iterator = "";

	if(!_mqttspool.hasChilds())
		return MOSQ_ERR_SUCCESS;

	cout << "TEST" << endl;
	int lock_result =_mqttspool.lock_inc(0);

	if(lock_result != YDB_OK) {
		return MOSQ_ERR_SUCCESS;
	}

	while(iterator=_mqttspool[iterator].nextSibling(), iterator!=""){ 
		dummy[iterator] = iterator;		
		dummy[iterator]["topic"] = (string)_mqttspool[iterator]["topic"];
		dummy[iterator]["clientid"] = (string)_mqttspool[iterator]["clientid"];
		dummy[iterator]["message"] = (string)_mqttspool[iterator]["message"];
	}

	_mqttspool.kill();
	_mqttspool.lock_dec();

	iterator = "";

	while(iterator=dummy[iterator].nextSibling(), iterator!=""){
		int result = mosquitto_broker_publish_copy(
			NULL,
			((string)dummy[iterator]["topic"]).c_str(),
			strlen(((string)dummy[iterator]["message"]).c_str()), 
			((string)dummy[iterator]["message"]).c_str(),
			QOS_SPOOL,
			RETAIN_SPOOL,
			PROPERTIES_SPOOL
		);

		if (result != MOSQ_ERR_SUCCESS) {
			dummy.kill();
			return result;
		}
	}
	
	dummy.kill();
	return MOSQ_ERR_SUCCESS;
}

int receive_mq_messages()  // TODO: read a fixed number of messages each call
{
	mqd_t mq_d = mq_open("/mqttspool", O_RDONLY | O_CREAT | O_NONBLOCK, S_IRWXU, NULL); 

	if(mq_d == -1) {
		int errsv = errno;
		cout << "open: " << strerrorname_np(errsv) << "  " << strerror(errsv) << endl;
		return MOSQ_ERR_SUCCESS;
	}

	struct mq_attr attr;

	if(mq_getattr(mq_d, &attr) == -1) {
		int errsv = errno;
		cout << "attr: " << strerrorname_np(errsv) << "  " << strerror(errsv) << endl;
		return MOSQ_ERR_SUCCESS;
	}

	char buffer[attr.mq_msgsize];
	if(mq_receive(mq_d, buffer, attr.mq_msgsize, NULL) == -1) {
		int errsv = errno;
		cout << "rcv: " << strerrorname_np(errsv) << "  " << strerror(errsv) << endl;
		return MOSQ_ERR_SUCCESS;
	}
	else {
		cout << buffer << endl;
	}
	return MOSQ_ERR_SUCCESS;
}

bool literal_to_json(Json::Value *json, char *literal) {
	istringstream literal_stream (literal);
	try {
		literal_stream >> *json;
	} catch(exception& e) {
		return false;
	}
	return true;
}

bool publish_response_message(string topic, string payload) {
	int result = mosquitto_broker_publish_copy( 
		NULL,
		topic.c_str(),
		strlen(payload.c_str()), // TODO: Maybe switch to CPP wrapper of mosquitto
		payload.c_str(),
		QOS_RESPONSE,
		RETAIN_RESPONSE,
		PROPERTIES_RESPONSE 
	);
	return (result == MOSQ_ERR_SUCCESS);
}

bool publish_response_message(string topic, Json::Value &payload) {  
	string serialized_payload = Json::writeString(builder, payload);
	int result = mosquitto_broker_publish_copy( 
		NULL,
		topic.c_str(),
		strlen(serialized_payload.c_str()), 
		serialized_payload.c_str(),
		QOS_RESPONSE,
		RETAIN_RESPONSE,
		PROPERTIES_RESPONSE
	);
	return (result == MOSQ_ERR_SUCCESS);
}
static int callback_message(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_message * ed = (mosquitto_evt_message*)event_data; // TODO: wirklich noetig oder nur fuer kuerzere Aufrufe?

	if(!regex_match(ed->topic, regex("(mqttfetch/aabay/)([^/]+)(/fr/)([0-9]+)"))) {
		return MOSQ_ERR_SUCCESS;
	}

	string response_topic = regex_replace(ed->topic, regex("/fr/"), "/to/");

	const char *client_id = mosquitto_client_id(ed->client);

	Json::Value request_payload;
	Json::Value response_payload;
	response_payload["rc"] = 0;

	if(!literal_to_json(&request_payload, (char*)ed->payload)) {
		response_payload["rc"] = -1;
		publish_response_message(response_topic, response_payload);

		return MOSQ_ERR_SUCCESS; 
	}

	if(request_payload["action"] == "get_articles") {
		string iterator = ""; // TODO: where is the best place to declare variables in c++?
		int json_array_index = 0;
		
		while(iterator=_articles[iterator].nextSibling(), iterator!="") {
			response_payload["articles"][json_array_index]["id"] = iterator;
			response_payload["articles"][json_array_index]["title"] = (string)_articles[iterator]["title"];
			response_payload["articles"][json_array_index]["bid"] = (string)_articles[iterator]["bid"];
			json_array_index += 1;
		}

		publish_response_message(response_topic, response_payload);

		return MOSQ_ERR_SUCCESS;
	} 

	if(request_payload["action"] == "get_article") {
		string requested_article_id = request_payload["id"].asString(); // TODO: move prop out

		if(_articles[requested_article_id].hasChilds()) {
			response_payload["article"]["id"] = requested_article_id;
			response_payload["article"]["title"] = (string)_articles[requested_article_id]["title"];
			response_payload["article"]["bid"] = (string)_articles[requested_article_id]["bid"];
			response_payload["article"]["text"] = (string)_articles[requested_article_id]["text"];
		}
		else {
			response_payload["rc"] = -1;
		}

		publish_response_message(response_topic, response_payload);
	
		return MOSQ_ERR_SUCCESS;
	}

	if(request_payload["action"] == "bid") { // TODO: add check for non-selected article
		string article_id = request_payload["id"].asString();
		string nickname = request_payload["nickname"].asString(); 

		int bid = stoi(request_payload["bid"].asString());// TODO: maybe change everywhere to int
		int maxbid = stoi(_articles[article_id]["maxbid"]); 

		if(!_articles[article_id].hasChilds()) {
			response_payload["rc"] = -3;
			publish_response_message(response_topic, response_payload);

			return MOSQ_ERR_SUCCESS;
		}

		if(nickname == (string)_articles[article_id]["winner"]) { // Gebot stammt von Hoechstbieter
			if(bid >= maxbid + 1) {
				_articles[article_id]["maxbid"] = bid;
				_articles[article_id]["client"] = client_id; 
			}
			else {
				response_payload["rc"] = -1;
			}

			publish_response_message(response_topic, response_payload);

			return MOSQ_ERR_SUCCESS;
		}

		if(bid >= maxbid + 1) { // erfolgreich ueberboten
			response_payload["rc"] = 1;

			publish_response_message(response_topic, response_payload);

			string previous_winner_response_topic = "mqttfetch/aabay/" + (string)_articles[article_id]["client"] + "/to/-1";
			cout << previous_winner_response_topic << endl;

			Json::Value previous_winner_response_payload;
			previous_winner_response_payload["rc"] = -1;

			publish_response_message(previous_winner_response_topic, previous_winner_response_payload);

			string bid_notice_topic = "aabay/bids/" + article_id;
			string bid_notice_payload = to_string(maxbid+1); 
			publish_response_message(bid_notice_topic, bid_notice_payload);

			_articles[article_id]["bid"] = maxbid + 1;
			_articles[article_id]["maxbid"] = bid;
			_articles[article_id]["winner"] = nickname;
			_articles[article_id]["client"] = client_id;

			return MOSQ_ERR_SUCCESS;
		}

		if(stoi(_articles[article_id]["bid"]) >  0) {  // neues gebot niedriger als hoechstgebot. Gebot wird erhoeht. TODO: Weshalb der Check hier? // TODO: Code dokumentiert sich nicht selbst -> Mehr Refactoring
			_articles[article_id]["bid"] = bid;

			string bid_notice_topic = "aabay/bids/" + article_id;
			string bid_notice_payload = to_string(bid);
			publish_response_message(bid_notice_topic, bid_notice_payload);

			response_payload["rc"] = -2;

			publish_response_message(response_topic, response_payload);
			return MOSQ_ERR_SUCCESS;
		}

		return MOSQ_ERR_SUCCESS;
	} 

	response_payload["rc"] = -1;

	publish_response_message(response_topic, response_payload);

	return MOSQ_ERR_SUCCESS;
}

static int callback_tick(int event, void *event_data, void *userdata) 
{
	// return receive_mq_messages();
	// return get_and_send_spooled_messages();
	return MOSQ_ERR_SUCCESS;
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
	
	//user_data beschreiben - derzeit nicht genutzt, nur zum Lernen!
	// char userdata_text[] = "123";
	// *user_data = mosquitto_malloc(sizeof(userdata_text));
	// memcpy(*user_data, userdata_text, sizeof(userdata_text));

	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL, *user_data)
		|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, *user_data);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	remove(ci_fn);

	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_message, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL);
}