
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

#define UNUSED(A) (void)(A)

static mosquitto_plugin_id_t * mosq_pid = NULL;
//static char *spooled_messages;

static const int QOS_SPOOL = 0;
static const bool RETAIN_SPOOL = false;
static mosquitto_property *PROPERTIES_SPOOL = NULL;

int get_and_send_spooled_messages();

int receive_mq_messages();

int get_and_send_spooled_messages()
{
	c_ydb_global _mqttspool("^ms"); // TODO: Make static
	c_ydb_global dummy("dummy");

	string iterator = "";

	if(!_mqttspool.hasChilds())
		return MOSQ_ERR_SUCCESS;

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

int receive_mq_messages() 
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

bool literal_to_json(Json::Value *json, char *literal){
	istringstream literal_stream (literal);
	try {
		literal_stream >> *json;
	} catch(exception& e) {
		return false;
	}
	return true;
}

bool publish_response_message(string topic, string payload) { // TODO: Add predeclaration | One Version with JSON and one with String
	int result = mosquitto_broker_publish_copy( 
		NULL,
		topic.c_str(),
		strlen(payload.c_str()), // TODO: Maybe switch to CPP wrapper of mosquitto
		payload.c_str(),
		QOS_SPOOL,
		RETAIN_SPOOL,
		PROPERTIES_SPOOL // TODO: const wert fuer aabay anpassen
	);
	return (result == MOSQ_ERR_SUCCESS);
}

static int callback_message(int event, void *event_data, void *userdata)
{ // 	struct mosquitto_evt_message {
// 	void *future;
// 	struct mosquitto *client;
// 	char *topic;
// 	void *payload;
// 	mosquitto_property *properties;
// 	char *reason_string;
// 	uint32_t payloadlen;
// 	uint8_t qos;
// 	uint8_t reason_code;
// 	bool retain;
// 	void *future2[4];
// };

	struct mosquitto_evt_message * ed = (mosquitto_evt_message*)event_data;

	const char *clid = mosquitto_client_id(ed->client);

	string request_topic(ed->topic); // TODO: vllt variabel entfernen
	string response_topic = regex_replace(request_topic, regex("/fr/"), "/to/");

	Json::Value request_payload;
	Json::Value response_payload;
	response_payload["rc"] = 0;

	//c_ydb_global _articles("^articles");

	if(!literal_to_json(&request_payload, (char*)ed->payload)) {
		response_payload["rc"] = -1;
		string serialized_response_payload = Json::writeString(builder, response_payload);
		publish_response_message(response_topic, serialized_response_payload);

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

		//Json::StreamWriterBuilder builder; // TODO: move out
		string serialized_response_payload = Json::writeString(builder, response_payload);
		publish_response_message(response_topic, serialized_response_payload);

		return MOSQ_ERR_SUCCESS;
	} 

	if(request_payload["action"] == "get_article") {
		string requested_article_id = request_payload["id"].asString();
		c_ydb_entry article = _articles[requested_article_id]; 

		if(article.hasChilds()) {
			response_payload["article"]["id"] = requested_article_id;
			response_payload["article"]["title"] = (string)article["title"];
			response_payload["article"]["bid"] = (string)article["bid"];
			response_payload["article"]["text"] = (string)article["text"];
		}
		else {
			response_payload["rc"] = -1;
		}

		//Json::StreamWriterBuilder builder; // TODO move levels up
		string serialized_response_payload = Json::writeString(builder, response_payload);
		publish_response_message(response_topic, serialized_response_payload);
	
		return MOSQ_ERR_SUCCESS;
	}

	if(request_payload["action"] == "bid") {
		string article_id = request_payload["id"].asString();
		c_ydb_entry article = _articles[article_id];
		string nickname = request_payload["id"].asString(); 
		int bid = stoi(request_payload["bid"].asString());// TODO: maybe change everywhere to int
		int maxbid = stoi(article["maxbid"]); 

		if(!article.hasChilds()) {
			response_payload["rc"] = -3;
			string serialized_response_payload = Json::writeString(builder, response_payload);
			publish_response_message(response_topic, serialized_response_payload);

			return MOSQ_ERR_SUCCESS;
		}

		if(nickname == (string)article["winner"]) { // Gebot stammt von Hoechstbieter
			if(bid >= maxbid + 1) {
				article["maxbid"] = bid;
				article["client"] = clid; // TODO: rename var
			}
			else {
				response_payload["rc"] = -1;
			}

			//Json::StreamWriterBuilder builder; // TODO move levels up
			string serialized_response_payload = Json::writeString(builder, response_payload);
			publish_response_message(response_topic, serialized_response_payload);

			return MOSQ_ERR_SUCCESS;
		}

		if(bid >= maxbid + 1) { // erfolgreich ueberboten
			// Json::StreamWriterBuilder builder; // TODO move levels up
			response_payload["rc"] = 1;
			string serialized_response_payload = Json::writeString(builder, response_payload);
			publish_response_message(response_topic, serialized_response_payload);

			string previous_winner_response_topic = regex_replace(response_topic, regex(clid), (string)article["client"]);

			Json::Value previous_winner_response_payload;
			previous_winner_response_payload["rc"] = -1;
			//Json::StreamWriterBuilder builder; // TODO move levels up
			string serialized_previous_winner_response_payload = Json::writeString(builder, previous_winner_response_payload);
			publish_response_message(previous_winner_response_topic, serialized_previous_winner_response_payload);

			string bid_notice_topic = "aabay/bids/" + article_id;
			string bid_notice_payload = to_string(maxbid+1); 
			publish_response_message(bid_notice_topic, bid_notice_payload);

			article["bid"] = maxbid + 1;
			article["maxbid"] = bid;
			article["winner"] = nickname;
			article["client"] = clid;

			return MOSQ_ERR_SUCCESS;
		}

		if(stoi(article["bid"]) >  0) {  // neues gebot niedriger als hoechstgebot. Gebot wird erhoeht. TODO: Weshalb der Check hier? // TODO: Code dokumentiert sich nicht selbst -> Mehr Refactoring
			article["bid"] = bid;

			string bid_notice_topic = "aabay/bids/" + article_id;
			string bid_notice_payload = to_string(bid);
			publish_response_message(bid_notice_topic, bid_notice_payload);

			response_payload["rc"] = -2;
			string serialized_response_payload = Json::writeString(builder, response_payload);
			publish_response_message(response_topic, serialized_response_payload);
			return MOSQ_ERR_SUCCESS;
		}

		return MOSQ_ERR_SUCCESS;
	} 

	response_payload["rc"] = -1;
	//Json::StreamWriterBuilder builder; // TODO move levels up
	string serialized_response_payload = Json::writeString(builder, response_payload);
	publish_response_message(response_topic, serialized_response_payload);
	return MOSQ_ERR_SUCCESS;
}

static int callback_tick(int event, void *event_data, void *userdata) 
{
	//return receive_mq_message();
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