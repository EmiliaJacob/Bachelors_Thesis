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
}let wiki_1 = {}
let wiki_1.path = '~/vimwiki/'
let wiki_1.path_html = '~/vimwiki_html/'

let wiki_2 = {}
let wiki_2.path = '~/private/'
let wiki_2.path_html = '~/private_html/'

let g:vimwiki_list = [wiki_1, wiki_2]


function onTimeUpMessage(message) {
}

function onBidMessage(message) {
}
// TODO: Weshalb Proxy verwenden? 
