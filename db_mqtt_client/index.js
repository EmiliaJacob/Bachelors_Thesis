const mqtt = require('mqtt');
const client = mqtt.connect('mqtt://mosquitto:1883');

console.log('attempting to connect to broker');

client.on('connect', () => {
  console.log("ejfskld;flkjfdslkfjsdlksdfj");
  client.subscribe('/test', (err) => {
    console.log('subbed');
  });
});

client.on('message', ()=> {
  console.log('dls;fjadlkfjdskl;fjdsalk;fjdsalk;fdjsflkasdfjadkl');
  //console.log(message.toString());
});
