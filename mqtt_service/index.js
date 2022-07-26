import { createClient } from 'redis';
import * as mqtt from 'mqtt';
const mqttTopics = {newArticle: 'newArticle', timeUp: 'timeUp', bid: 'bid'}
const redisClient = createClient();

redisClient.on('error', (err) => console.log('Redis Client Error', err));

redisClient.connect();

const mqttClient = mqtt.connect(); // TODO: What is the role of aync here?

mqttClient.on('message', (topic, message) => {
  console.log(message.toString());
  
  switch(topic) {
    case mqttTopics.newArticle:
      onNewArticleMessage(message);
      break;
    case mqttTopics.timeUp:
      onTimeUpMessage(message);
      break;
    case mqttTopics.bid:
      onBidMessage(message);
      break;
  }
});

Object.values(mqttTopics).forEach((topic) => {
  mqttClient.subscribe(topic);
});

function onNewArticleMessage(message) {
}

function onTimeUpMessage(message) {
}

function onBidMessage(message) {
}
