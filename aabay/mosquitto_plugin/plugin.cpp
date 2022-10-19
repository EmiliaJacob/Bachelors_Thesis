#include <string.h> // TODO: use only c++ includes?
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

// Mosquitto
#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
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
#include <vector>
#include <sstream>
using std::istringstream; // TODO: does this make sense?
using std::string;
using std::cout;
using std::vector;


char *sync_mode = "client";
mqd_t mq_descriptor = -1;

// message callback
Json::StreamWriterBuilder builder;
c_ydb_global _articles("^articles");
bool literal_to_json(Json::Value *json, char *literal);
bool publish_response_message(string topic, Json::Value *payload); 
bool publish_response_message(string topic, string payload);

#define UNUSED(A) (void)(A)

static mosquitto_plugin_id_t * mosq_pid = NULL;

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
	if(mq_descriptor == -1) {
		int errsv = errno;
		// cout << "open: " << strerrorname_np(errsv) << "  " << strerror(errsv) << endl; // TODO: couts durch mosquitto logs ersetzen
		return MOSQ_ERR_SUCCESS;
	}

	struct mq_attr attr;

	if(mq_getattr(mq_descriptor, &attr) == -1) {
		int errsv = errno;
		// cout << "attr: " << strerrorname_np(errsv) << "  " << strerror(errsv) << endl; //TODO: activate again and make a log message out of it?
		return MOSQ_ERR_SUCCESS;
	}

	vector<char> buffer(attr.mq_msgsize);

	if(mq_receive(mq_descriptor, buffer.data(), attr.mq_msgsize, NULL) == -1) { 
		int errsv = errno;
	}
	else {
		vector<char>::iterator delimiter_element = find(buffer.begin(), buffer.end(), ' '); //TODO: sollte Format der message irgendo ueberprueft werden?
		vector<char> topic(buffer.begin(), delimiter_element );
		vector<char> payload(delimiter_element + 1, buffer.end());

		publish_response_message(topic.data(), payload.data()); // TODO: rename function
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
		mosquitto_broker_publish_copy( // TODO: do you also have to this in ACL check?
			NULL,
			ed->topic,
			strlen((char*)ed->payload), 
			ed->payload,
			QOS_RESPONSE,
			RETAIN_RESPONSE,
			PROPERTIES_RESPONSE
		);
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
	if(!strcmp(sync_mode, "mq")) // TODO: replace char* by string
		return receive_mq_messages(); 

	else if(!strcmp(sync_mode, "global"))
		return get_and_send_spooled_messages();

	else { // sync_mode = "client"
		return MOSQ_ERR_SUCCESS;
	}
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

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	mosq_pid = identifier;

	mosquitto_log_printf(MOSQ_LOG_INFO, "Init %d\n", opt_count);

	for (int i = 0; i < opt_count; i++) {
		if(!strcmp(opts[i].key, "sync_mode")) {
			if(!strcmp(opts[i].value,"client") || !strcmp(opts[i].value,"mq") || !strcmp(opts[i].value,"global")) {
				mosquitto_log_printf(MOSQ_LOG_INFO, "Selected Sync Mode: %s", opts[i].value);
				sync_mode = opts[i].value;
			}
			else {
				mosquitto_log_printf(MOSQ_LOG_INFO, "Invalid Sync Mode %s" , sync_mode);
			}
		}
	}

	if(!strcmp(sync_mode, "mq")){
		mq_descriptor = mq_open("/mqttspool", O_RDONLY | O_CREAT | O_NONBLOCK, S_IRWXU, NULL); 
	}

	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL, *user_data)
		|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, *user_data);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	if(!strcmp(sync_mode, "mq")){
		if(mq_descriptor != -1) {
			mq_close(mq_descriptor);
		}
	}

	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_message, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL);
}