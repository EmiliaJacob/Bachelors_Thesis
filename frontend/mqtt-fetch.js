/*
 Version 20200107
 
 ToDo:
 */
class mqtt_fetch {
	constructor (prefix) {
		this.prefix = "mqttfetch/" + prefix + "/";
		this.mqtt_topicIndex = 0;
		this.mqtt_topicMap = new Map; // Map ist ein JS dictionary mit Reihenfolge
		this.timeout = 1000;
		this.qos = 0;
		console.log("Ende Konstruktor " );
	};
	
	init(host, port, uri, connect_prm) { // return promise. bei erfolg wird mqttfetch/prefix/clid/to/+ subskribiert
		const that = this;
		if (connect_prm == undefined)
			connect_prm = {};
		if(uri == undefined)
			uri = "";
		return new Promise(function(resolve) {
			connect_prm.onSuccess = function() {
				console.log("mqtt_fetch_onConnect");
				that.subscribe(that.prefix + that.mqtt_client.clientId + "/to/+").then(value => {resolve(value);});
			};
			connect_prm.onFailure = function() {
				console.log("mqtt_connect_err"); 
				resolve(-1);
			};
			that.mqtt_client = new Paho.MQTT.Client(host, port, uri, "mqtt-fetch-js-client-" + Math.floor(Math.random() * 1E15).toString()); // hier wird clid berechnet
			that.mqtt_client.onMessageArrived = function(msg) {that.mqtt_fetch_rx(msg, that)};
			that.mqtt_client.onConnectionLost = function () {console.log("mqtt_fetch_onConnectionLost");};
			console.log("Connecting....");
			that.mqtt_client.connect(connect_prm); // hier wird promise resolved
			console.log("Done");
		});
	};
	
	
	subscribe(topic) { // TODO: vllt clid mitschicken um direktantwort von Broker aus zu schicken
		var that = this;
		console.log("subscribing " + topic);
		return new Promise(function(resolve) {
			that.mqtt_client.subscribe(topic, {
				qos: that.qos,
				onSuccess: function () {console.log("mqtt_fetch_subscription " + topic + " ok"); resolve(0);},
				onFailure: function () {console.log("mqtt_fetch_subscription " + topic + " err"); resolve(1);}
			});
		});
	}
	
	unsubscribe(topic) {
		var that = this;
		console.log("unsubscribing " + topic);
		return new Promise(function(resolve) {
			that.mqtt_client.unsubscribe(topic, {
			onSuccess: function () {console.log("mqtt_fetch_unsubscription " + topic + " ok"); resolve(0);},
			onFailure: function () {console.log("mqtt_fetch_unsubscription " + topic + " err"); resolve(1);}
			});
		});
	}
	

	send(v) { // TODO: Vllt clid flag statt subtopic verwenden | Sendet msg an prefix/clid/fr/topicIndex
		var is_object = typeof v === "object";
		if (is_object)
			v = JSON.stringify(v);
		const that = this;
		return new Promise(function(resolve) {
		    console.log("OUTMESSAGEPAYLOAD: " + v);
			var message = new Paho.MQTT.Message(v);
			message.destinationName = that.prefix + that.mqtt_client.clientId + "/fr/" + that.mqtt_topicIndex; // topicindex wird bei jeder message hochgezaehlt. Warum TOPICindex?
			message.qos = that.qos;
			that.mqtt_topicMap.set(that.mqtt_topicIndex, [resolve,setTimeout(that.mqtt_fetch_error, that.timeout, that.mqtt_topicIndex, that), is_object]); //topicIndex ist key

		    console.log("OUTMESSAGETOPIC: " + message.destinationName);
			that.mqtt_client.send(message);
			that.mqtt_topicIndex++;
		});
	}
	
	mqtt_fetch_rx (msg, that) { // wird aufgerufen wenn message empfangen wurde
		var topic = msg.destinationName.split("/"); // aabay,bids,artid
		 console.log("rx " + msg.destinationName + " " + msg.payloadString);
		if (msg.destinationName.substring(0,that.prefix.length) == that.prefix) { // checkt ob topic der message den selben prefix hat? mqttfetch/aabay
			var nr=+topic[topic.length - 1], dummy = that.mqtt_topicMap.get(nr); // nr wird auf den counter gesetzt (letztes subtopic), dummy wird ueber den counter das entsprechende objekt aus mqtt_map zugewiesen

			if (dummy != undefined) { // fuer den counter ist ein eintrag in der map enthalten
				if (nr >= 0) {
					clearTimeout(dummy[1]); // cleart ein zuvor gesetztes timeout
					that.mqtt_topicMap.delete(nr); // die antwort wurde empfangen. deshalb kann nun der eintrag wieder geloescht werden
				}
				dummy[0]((dummy[2] == true) ? JSON.parse(msg.payloadString) : msg.payloadString); // Promise einloesen oder callback-Fkt.!
			}
			else {
				console.log("Verworfenes Topic " + msg.destinationName + " " + msg.payloadString);
			}
		}
		else if (that.mqtt_topicMap.has(msg.destinationName)) { // fuer nicht-fetch nachrichten die empfangen wurden
			console.log("DELLO");
			var r = that.mqtt_topicMap.get(msg.destinationName);
			r[0](msg.destinationName, (r[2] == true) ? JSON.parse(msg.payloadString) : msg);
		}
		else {
			console.log("Sollte nie passieren: " + msg.destinationName + " " + msg.payloadString);
		}
	}
	
	mqtt_fetch_error(nr, that) {
		console.log("error: " + nr);
		if (that.mqtt_topicMap.has(nr))
			that.mqtt_topicMap.delete(nr);
		if (that.mqtt_error != undefined)
			that.mqtt_error(nr);
	}
	
	//key:topic, [0]:callback function, [1]:timeout, [2]:is_object
	async set_callback(index, f, is_object) { // index=topic f=>callback funktion
		var rc = 0;
		if (index != parseInt(index, 10) ) { // Es wird getestet ob index keine gerade zahl ist
			rc = await this.subscribe(index, {});
			console.log("rc = " + rc);
		}
		this.mqtt_topicMap.set(index, [f, undefined , is_object == true]); // dem key: topic wird in der map die callback fkt hinzugefuegt
		return rc;
	}
	
	async delete_callback(index) {
		var rc = 0;
		if (index != parseInt(index, 10) ) {
			rc = await this.unsubscribe(index, {});
			console.log("rc = " + rc);
		}
		this.mqtt_topicMap.delete(index);
		return rc;
	}
};