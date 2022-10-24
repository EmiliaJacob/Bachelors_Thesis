// Mosquitto
#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"

// Posix Message Queue
#include <mqueue.h> 
#include <errno.h>

#include <iostream>
#include <fstream>
#include <string>
#include "ydb-global.h"
#include <regex>
#include "json.h"
#include <vector>
#include <sstream>
#include <chrono>

using namespace::std;
using namespace::std::chrono;

string sync_mode = "client";

bool time_measurement_trigger_to_publish = false;
bool time_measurement_read_out_function = false;

ofstream time_log_mq_trigger_to_publish;
ofstream time_log_mq_receive_mq_messages;
ofstream time_log_global_trigger_to_publish;
ofstream time_log_global_get_and_send_spooled_messages;
ofstream time_log_client_trigger_to_publish;

c_ydb_global _mqttspool("^ms"); 
c_ydb_global dummy("dummy");

mqd_t mq_descriptor = -1;
int max_mq_receive_per_tick = 10;

Json::StreamWriterBuilder builder;
c_ydb_global _articles("^articles");

static int callback_message(int event, void *event_data, void *userdata);

static int callback_tick(int event, void *event_data, void *userdata);

int get_and_send_spooled_messages();

int receive_and_send_mq_messages();

bool literal_to_json(Json::Value *json, char *literal);

bool publish_mqtt_message(string topic, Json::Value *payload); 

bool publish_mqtt_message(string topic, string payload);

const int QOS = 0;
const bool RETAIN = false;
mosquitto_property *PROPERTIES = NULL;

static mosquitto_plugin_id_t * mosq_pid = NULL;


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
				sync_mode = string(opts[i].value);
			}
			else {
				mosquitto_log_printf(MOSQ_LOG_INFO, "Invalid Sync Mode %s" , sync_mode);
			}
		}
		else if(!strcmp(opts[i].key, "time_measurement_trigger_to_publish")) {
			if(!strcmp(opts[i].value, "true")) {
				time_measurement_trigger_to_publish = true;
			}
		}
		else if(!strcmp(opts[i].key, "time_measurement_read_out_function")) {
			if(!strcmp(opts[i].value, "true")) {
				time_measurement_read_out_function = true;
			}

		}
	}

	if(sync_mode == "mq") {
		mq_descriptor = mq_open("/mqttspool", O_RDONLY | O_CREAT | O_NONBLOCK, S_IRWXU, NULL); 
	}

	if(time_measurement_trigger_to_publish){
		if(sync_mode == "mq") {
			time_log_mq_trigger_to_publish.open("/home/emi/ydbay/aabay/time_measurements/mq/trigger_to_publish"); 
		}
		if(sync_mode == "global"){
			time_log_global_trigger_to_publish.open("/home/emi/ydbay/aabay/time_measurements/global/trigger_to_publish");
		}
		if(sync_mode == "client"){
			time_log_client_trigger_to_publish.open("/home/emi/ydbay/aabay/time_measurements/client/trigger_to_publish");
		}
	}

	if(time_measurement_read_out_function) {
		if(sync_mode == "mq") {
			time_log_mq_receive_mq_messages.open("/home/emi/ydbay/aabay/time_measurements/mq/receive_and_send_mq_messages");
		}
		if(sync_mode == "global"){
			time_log_global_get_and_send_spooled_messages.open("/home/emi/ydbay/aabay/time_measurements/global/get_and_send_spooled_messages");
		}
		if(sync_mode == "client"){
		}
	}

	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL, *user_data)
		|| mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, *user_data);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	if(sync_mode == "mq") {
		if(mq_descriptor != -1) {
			mq_close(mq_descriptor);
		}
	}

	if(time_measurement_trigger_to_publish){
		if(sync_mode == "mq"){
			time_log_mq_trigger_to_publish.close();
		}
		if(sync_mode == "global"){
			time_log_global_trigger_to_publish.close();
		}
		if(sync_mode == "client"){
			time_log_client_trigger_to_publish.close();
		}
	}

	if(time_measurement_read_out_function) {
		if(sync_mode == "mq"){
			time_log_mq_receive_mq_messages.close();
		}
		if(sync_mode == "global"){
			time_log_global_get_and_send_spooled_messages.close();
		}
		if(sync_mode == "client"){
		}
	}

	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_message, NULL)
	|| mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_TICK, callback_tick, NULL);
}


static int callback_message(int event, void *event_data, void *userdata) 
{
	struct mosquitto_evt_message *ed = (mosquitto_evt_message*)event_data; 

	//TODO: Wird ACL davor ausgefuehrt ? Funktionstimecheck inkl ACL machen? Wann wird ACL - Callback aufgerufen?
	if(!regex_match(ed->topic, regex("(mqttfetch/aabay/)([^/]+)(/fr/)([0-9]+)"))) { // Hier wird sync Nachricht von Client Implementation ausgelesen
		if(sync_mode == "client") {

			if(time_measurement_trigger_to_publish) {
				system_clock::time_point stop_point = system_clock::now();
				duration<double> stop_duration = stop_point.time_since_epoch(); 

				double start_duration_rep = stod(((char*)ed->payload), NULL);
				duration<double> start_duration(start_duration_rep);

				duration<double> time_difference = stop_duration - start_duration;
				double time_difference_in_ms = time_difference.count() * 1000;

				time_log_client_trigger_to_publish << to_string(time_difference_in_ms) + "\n";
			}
			
			publish_mqtt_message(ed->topic, (char*)ed->payload);
		}

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

static int callback_tick(int event, void *event_data, void *userdata) 
{
	if(sync_mode == "mq") {
			receive_and_send_mq_messages(); 
			return MOSQ_ERR_SUCCESS;
	}

	else if(sync_mode == "global") {
			get_and_send_spooled_messages();
			return MOSQ_ERR_SUCCESS;
	}

	else { // sync_mode = "client"
		return MOSQ_ERR_SUCCESS;
	}
}


int get_and_send_spooled_messages()
{
	high_resolution_clock::time_point start_function_time = high_resolution_clock::now(); 

	if(!_mqttspool.hasChilds())
		return MOSQ_ERR_SUCCESS;

	int lock_inc_result =_mqttspool.lock_inc(0);

	if(lock_inc_result != YDB_OK)  // Lock konnte nicht gesetzt werden
		return MOSQ_ERR_SUCCESS;

	string interator_mqttspool = "";

	while(interator_mqttspool = _mqttspool[interator_mqttspool].nextSibling(), interator_mqttspool != "") { 
		dummy[interator_mqttspool] = interator_mqttspool;		
		dummy[interator_mqttspool]["topic"] = (string)_mqttspool[interator_mqttspool]["topic"];
		dummy[interator_mqttspool]["clientid"] = (string)_mqttspool[interator_mqttspool]["clientid"];
		dummy[interator_mqttspool]["message"] = (string)_mqttspool[interator_mqttspool]["message"];
	}

	_mqttspool.kill();
	_mqttspool.lock_dec();

	string iterator_dummy = "";
	bool first_iteration = true;

	high_resolution_clock::time_point start_point_get = high_resolution_clock::now();

	while(iterator_dummy = dummy[iterator_dummy].nextSibling(), iterator_dummy != "") {

		high_resolution_clock::time_point stop_point_get = high_resolution_clock::now();
		duration<double> time_difference_get = stop_point_get - start_point_get;
		double time_difference_get_in_ms = time_difference_get.count() * 1000;

			if(time_measurement_trigger_to_publish) { 
				system_clock::time_point stop_point = system_clock::now();
				duration<double> stop_duration = stop_point.time_since_epoch(); // Implicit cast

				double start_duration_rep = stod(((string)dummy[iterator_dummy]["message"]), NULL);
				duration<double> start_duration(start_duration_rep);

				duration<double> time_difference = stop_duration - start_duration;
				double time_difference_in_ms = time_difference.count() * 1000;

				time_log_global_trigger_to_publish << to_string(time_difference_in_ms) + "\n";
			}

			if(time_measurement_read_out_function && first_iteration){ 
				high_resolution_clock::time_point stop = high_resolution_clock::now();

				duration<double> time_difference = stop - start_function_time;
				double time_difference_in_ms = time_difference.count() * 1000;
				double time_difference_without_get = time_difference_in_ms - time_difference_get_in_ms;

				time_log_global_get_and_send_spooled_messages << to_string(time_difference_without_get) << "\n";
			}

			publish_mqtt_message((string)dummy[iterator_dummy]["topic"], (string)dummy[iterator_dummy]["message"]); // TODO: vllt ueberall den Begriff payload oder message verwenden

			first_iteration = false;
	}
	
	dummy.kill();
	return MOSQ_ERR_SUCCESS;
}

int receive_and_send_mq_messages() 
{
	high_resolution_clock::time_point start_function_time = high_resolution_clock::now(); 

	if(mq_descriptor == -1) {
		int errsv = errno;
		mosquitto_log_printf(MOSQ_LOG_INFO, "Invalid mq_descriptor: %s %s" , strerrorname_np(errsv), strerror(errsv));
		return MOSQ_ERR_SUCCESS;
	}

	struct mq_attr attr;

	if(mq_getattr(mq_descriptor, &attr) == -1) {
		int errsv = errno;
		mosquitto_log_printf(MOSQ_LOG_INFO, "Couldn't get mq Attributes: %s %s" , strerrorname_np(errsv), strerror(errsv));
		return MOSQ_ERR_SUCCESS;
	}

	vector<char> buffer(attr.mq_msgsize);

	for (int i = 0; i < max_mq_receive_per_tick; i++) {
		
		high_resolution_clock::time_point start_point_receive = high_resolution_clock::now();

		if(mq_receive(mq_descriptor, buffer.data(), attr.mq_msgsize, NULL) == -1) { 
			return MOSQ_ERR_SUCCESS;
		}

		high_resolution_clock::time_point stop_point_receive = high_resolution_clock::now();

		duration<double> time_difference_receive = stop_point_receive - start_point_receive;
		double time_difference_receive_in_ms = time_difference_receive.count() * 1000;

		if(!regex_match(buffer.data(), regex("(aabay/bids/)([0-9]+)(\\s)([0-9]+)(([.][0-9]+)?)"))) {
			mosquitto_log_printf(MOSQ_LOG_INFO, "Undesired mq message format" );
			return MOSQ_ERR_SUCCESS;
		}

		else {
			vector<char>::iterator delimiter_element = find(buffer.begin(), buffer.end(), ' ');
			vector<char> topic(buffer.begin(), delimiter_element );
			vector<char> payload(delimiter_element + 1, buffer.end());
			
			if(time_measurement_trigger_to_publish) {
				system_clock::time_point stop_point = system_clock::now();
				duration<double> stop_duration = stop_point.time_since_epoch(); 

				double start_duration_rep = stod(payload.data(), NULL);
				duration<double> start_duration(start_duration_rep);

				duration<double> time_difference = stop_duration - start_duration;
				double time_difference_in_ms = time_difference.count() * 1000;

				time_log_mq_trigger_to_publish << to_string(time_difference_in_ms) << "\n";
			}

			if(time_measurement_read_out_function && i == 0) { 
				high_resolution_clock::time_point stop = high_resolution_clock::now();

				duration<double> time_difference = stop - start_function_time;
				double time_difference_in_ms = time_difference.count() * 1000;
				double time_difference_without_receive = time_difference_in_ms - time_difference_receive_in_ms; 

				time_log_mq_receive_mq_messages << to_string(time_difference_without_receive) << "\n";

				publish_mqtt_message(topic.data(), payload.data());
			}

			publish_mqtt_message(topic.data(), payload.data());
		}
	}

	return MOSQ_ERR_SUCCESS;
}


bool literal_to_json(Json::Value *json, char *literal) 
{
	istringstream literal_stream (literal);

	try {
		literal_stream >> *json;
	} 
	catch(exception& e) {
		return false;
	}

	return true;
}

bool publish_mqtt_message(string topic, string payload) 
{
	int result = mosquitto_broker_publish_copy( 
		NULL,
		topic.c_str(),
		strlen(payload.c_str()),
		payload.c_str(),
		QOS,
		RETAIN,
		PROPERTIES 
	);

	return (result == MOSQ_ERR_SUCCESS);
}

bool publish_mqtt_message(string topic, Json::Value &payload) 
{  
	string serialized_payload = Json::writeString(builder, payload);

	int result = mosquitto_broker_publish_copy( 
		NULL,
		topic.c_str(),
		strlen(serialized_payload.c_str()), 
		serialized_payload.c_str(),
		QOS,
		RETAIN,
		PROPERTIES
	);

	return (result == MOSQ_ERR_SUCCESS);
}