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
#include <fstream>
#include <string>
#include "ydb-global.h"
#include <regex>
#include "json.h"
#include <vector>
#include <sstream>
#include <chrono>
#include <ctime>
using std::istringstream; // TODO: does this make sense?
using std::string;
using std::cout;
using std::vector;


char *sync_mode = "client";
bool time_measure = false;
mqd_t mq_descriptor = -1;
int max_mq_receive_per_tick = 10;

c_ydb_global _mqttspool("^ms"); 
c_ydb_global dummy("dummy");


// message callback
Json::StreamWriterBuilder builder;
c_ydb_global _articles("^articles");
bool literal_to_json(Json::Value *json, char *literal);
bool publish_mqtt_message(string topic, Json::Value *payload); 
bool publish_mqtt_message(string topic, string payload);

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

		cout << "RECEIVE: " << (string)dummy[iterator]["message"] << endl;

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
	if(mq_descriptor == -1) {
		int errsv = errno;
		mosquitto_log_printf(MOSQ_LOG_INFO, "Invalid mq_descriptor: %s %s" , strerrorname_np(errsv), strerror(errsv));
		return MOSQ_ERR_SUCCESS;
	}

	struct mq_attr attr;

	if(mq_getattr(mq_descriptor, &attr) == -1) {
		int errsv = errno;
		mosquitto_log_printf(MOSQ_LOG_INFO, "Invalid mq_descriptor: %s %s" , strerrorname_np(errsv), strerror(errsv));
		return MOSQ_ERR_SUCCESS;
	}

	vector<char> buffer(attr.mq_msgsize);

	for (int i=0; i<max_mq_receive_per_tick; i++) {
		if(mq_receive(mq_descriptor, buffer.data(), attr.mq_msgsize, NULL) == -1) { 
			return MOSQ_ERR_SUCCESS;
		}
		else {

			if(!regex_match(buffer.data(), regex("(aabay/bids/)([0-9]+)(\\s)([0-9]+)"))) {
				mosquitto_log_printf(MOSQ_LOG_INFO, "Invalid mq message format" );
				return MOSQ_ERR_SUCCESS;
			}

			vector<char>::iterator delimiter_element = find(buffer.begin(), buffer.end(), ' ');
			vector<char> topic(buffer.begin(), delimiter_element );
			vector<char> payload(delimiter_element + 1, buffer.end());
			
			cout << "RECEIVE: " << payload.data() << endl;

			publish_mqtt_message(topic.data(), payload.data());
		}
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

bool publish_mqtt_message(string topic, string payload) {
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

bool publish_mqtt_message(string topic, Json::Value &payload) {  
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

void client_time_measure(struct mosquitto_evt_message *ed){
	if(!strcmp(sync_mode, "client") && time_measure == true){
		if(!strcmp(ed->topic, "time_measure")){
			cout << "Send Time: " << (char*)ed->payload << endl;
			auto current_time = std::chrono::high_resolution_clock::now();
			time_t tt;
			tt = std::chrono::system_clock::to_time_t(current_time);
			cout << "Receive Time: " << ctime(&tt) << endl;
		}
	}
}

static int callback_message(int event, void *event_data, void *userdata) // TODO: client trigger wird momentan bei allen sync_modes weitergeleitet
{
	struct mosquitto_evt_message *ed = (mosquitto_evt_message*)event_data; 

	client_time_measure(ed);

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
		publish_mqtt_message(response_topic, response_payload);

		return MOSQ_ERR_SUCCESS; 
	}

	if(request_payload["action"] == "get_articles") {
		string iterator = "";
		int json_array_index = 0;
		
		while(iterator=_articles[iterator].nextSibling(), iterator!="") {
			response_payload["articles"][json_array_index]["id"] = iterator;
			response_payload["articles"][json_array_index]["title"] = (string)_articles[iterator]["title"];
			response_payload["articles"][json_array_index]["bid"] = (string)_articles[iterator]["bid"];
			json_array_index += 1;
		}

		publish_mqtt_message(response_topic, response_payload);

		return MOSQ_ERR_SUCCESS;
	} 

	if(request_payload["action"] == "get_article") {
		string requested_article_id = request_payload["id"].asString(); 

		if(_articles[requested_article_id].hasChilds()) {
			response_payload["article"]["id"] = requested_article_id;
			response_payload["article"]["title"] = (string)_articles[requested_article_id]["title"];
			response_payload["article"]["bid"] = (string)_articles[requested_article_id]["bid"];
			response_payload["article"]["text"] = (string)_articles[requested_article_id]["text"];
		}
		else {
			response_payload["rc"] = -1;
		}

		publish_mqtt_message(response_topic, response_payload);
	
		return MOSQ_ERR_SUCCESS;
	}

	if(request_payload["action"] == "bid") { // TODO: add check for non-selected article
		string article_id = request_payload["id"].asString();
		string nickname = request_payload["nickname"].asString(); 

		int bid = stoi(request_payload["bid"].asString()); // TODO: maybe change everywhere to int
		int maxbid = stoi(_articles[article_id]["maxbid"]); 

		if(!_articles[article_id].hasChilds()) {
			response_payload["rc"] = -3;
			publish_mqtt_message(response_topic, response_payload);

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

			publish_mqtt_message(response_topic, response_payload);

			return MOSQ_ERR_SUCCESS;
		}

		if(bid >= maxbid + 1) { // erfolgreich ueberboten
			response_payload["rc"] = 1;

			publish_mqtt_message(response_topic, response_payload);

			string previous_winner_response_topic = "mqttfetch/aabay/" + (string)_articles[article_id]["client"] + "/to/-1";
			cout << previous_winner_response_topic << endl;

			Json::Value previous_winner_response_payload;
			previous_winner_response_payload["rc"] = -1;

			publish_mqtt_message(previous_winner_response_topic, previous_winner_response_payload);

			string bid_notice_topic = "aabay/bids/" + article_id;
			string bid_notice_payload = to_string(maxbid+1); 
			publish_mqtt_message(bid_notice_topic, bid_notice_payload);

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
			publish_mqtt_message(bid_notice_topic, bid_notice_payload);

			response_payload["rc"] = -2;

			publish_mqtt_message(response_topic, response_payload);
			return MOSQ_ERR_SUCCESS;
		}

		return MOSQ_ERR_SUCCESS;
	} 

	response_payload["rc"] = -1;

	publish_mqtt_message(response_topic, response_payload);

	return MOSQ_ERR_SUCCESS;
}

int counter = 0;

struct Timer
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<float> duration;
	ofstream time_log;

	Timer()
	{
		if(counter % 100 == 0) {
			cout << "SEND: " << counter << endl;
			time_log.open("/home/emi/ydbay/aabay/mosquitto_plugin/time_logs/global_100.md", ios_base::app);
			start = std::chrono::high_resolution_clock::now();
			_articles["123"]["bid"] = counter;
		}
	}

	~Timer()
	{
		if(counter % 100 == 0) {
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;

			float ms = duration.count() * 1000.0f;

			std::cout << "TIMER FINISHED ON COUNTER " << counter << std::endl;
			
			time_log << to_string(ms) + "\n";
			time_log.close();
		}
	}
};

static int callback_tick(int event, void *event_data, void *userdata) 
{

	if(!strcmp(sync_mode, "mq")) {
		counter += 1;
		Timer timer; // TODO: Es ist nicht garantiert, dass der aktuelle Trigger auch im selben Tick Aufruf wieder empfangen wird
		receive_mq_messages(); 
		return MOSQ_ERR_SUCCESS;
	}

	else if(!strcmp(sync_mode, "global")) {
		counter += 1;
		Timer timer;
		get_and_send_spooled_messages();
		return MOSQ_ERR_SUCCESS;
	}

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
		else if(!strcmp(opts[i].key, "time_measure")) {
			if(!strcmp(opts[i].value, "true")) {
				time_measure = true;
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