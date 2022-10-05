const mqtt = require('mqtt');
const aabay_backend = require('./aabay-backend');

const client = mqtt.connect(); //localhost:1883

client.on('connect', function () {
  client.subscribe('mqttfetch/aabay/+/fr/+', function (err) {
    if (!err) {
      console.log('backend Client successfully subscribed\n');
    }
  });
});

client.on('message', function (topic, message) {
  let responses = aabay_backend.aabay(topic, message);
  responses.forEach(response => {
    console.log("JLFJSlfdkj");
    console.log(response.topic);
    client.publish(response.topic, response.payload);
  });
});
