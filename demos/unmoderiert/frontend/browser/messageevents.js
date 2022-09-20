client.on('connect', (connack)=>{
    console.log('Connection to Broker established');
})

client.on('error', (err)=>{
    console.log('An error ocurred: ${err}');
})

client.on('message', (topic, message)=>{
    proxyMsg[topic] = message;
})