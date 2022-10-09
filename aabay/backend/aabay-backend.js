/*

 Testat "AAbay" Loesungsvorlage
 Stand 20220531
 Version fuer MQTT-Backend
 
*/
const articles = {
	123: {title: "The art of Computer Programming", "text": "Sehr guter Zustand", bid: 0, maxbid: 0, winner: "", client: ""},
	234: {title: "Algorithmen - Eine Einführung", "text": "Ein Klassiker...", bid: 5, maxbid: 10, winner: "", client: ""}
};

exports.aabay = function(topic, message) {
	let client_id = topic.split("/")[2];
	console.log("client_id:", client_id);
	console.log("topic: ", topic);
	console.log("message: ", message);

	var response = [{topic: topic.replace("/fr/","/to/"), payload: {rc: 0}}]; // Index 0: mqtt-fetch-response
	o = JSON.parse(message);
	console.log(o);
	if (o.action == "get_articles") {
		response[0].payload.articles = [];
		for (i in articles)
			response[0].payload.articles.push({id: i, title: articles[i].title, bid:articles[i].bid});
	}
	else if (o.action == "get_article") {
		console.log(typeof o.id);
		if (typeof o.id == "number" && articles[o.id] != undefined)
			response[0].payload.article = {id: o.id, title: articles[o.id].title, bid:articles[o.id].bid, text: articles[o.id].text};
		else
			response[0].rc = -1;
	}
	else if (o.action == "bid") {
		if (!o.nickname || !o.bid || !o.id || articles[o.id] == undefined) { // Darf ergaenzt werden
			response[0].payload.rc = -3; // No valid data
		}
		// ToDo Start
		else if (articles[o.id].winner == o.nickname) { // Hoechstbietender bietet nochmals
			if(o.bid >= articles[o.id].maxbid+1) {
				articles[o.id].maxbid = o.bid;
				articles[o.id].client = client_id;
			}
			else {
				response[0].rc  = -1;
			}
		}
		else { // Anderer Hoechstbietender
			if(o.bid >= articles[o.id].maxbid+1) { 
        let dummy = Object.assign({}, articles[o.id]);
        console.log(JSON.stringify(dummy));

				articles[o.id].bid = articles[o.id].maxbid + 1;
				articles[o.id].maxbid = o.bid;
				articles[o.id].winner = o.nickname;
				articles[o.id].client = client_id;		

        //new winner msg
				response[0].payload.rc = 1;
        //old winner msg
        response.push(
          {topic: `${topic.split('/')[0]}/${topic.split('/')[1]}/${dummy.client}/to/${topic.split('/')[4]}`, payload: {rc: -1} }
        );
        //universal new bid msg
        response.push(
          {topic: `aabay/bids/${o.id}`, payload: `${dummy.maxbid+1}`}
        );
			}
			else{
        let dummy = articles[o.id].bid; 
        if(dummy > 0){
          articles[o.id].bid = o.bid;
          response.push(
            {topic: `aabay/bids/${o.id}`, payload: `${o.bid}`}
          );
          response[0].payload.rc = -2;
        }
			}
		}
		// ToDo End
	}
	else
		response[0].payload.rc = -1; // No valid action
	response.pus
	for (let i = 0; i < response.length; i++)
		response[i].payload = JSON.stringify(response[i].payload);
	return response;
}


//const mqtt_fetch = require('./mqtt-fetch');

const mqtt = require('mqtt');
const client = mqtt.connect(); //localhost:1883

client.on('connect', () => {
	console.log("hello");
	client.subscribe('aabay/mqtt_fetch/#', (err) => {
		if(!err) {
			console.log("jjj");
		}
	});
})

client.on('message', (topic, message) => {
	console.log(message.toString());
});
