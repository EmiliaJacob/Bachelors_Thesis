var mqtt = require('mqtt')
var prompt = require('prompt')
var client = mqtt.connect('mqtt://localhost:1883')

process.on('SIGINT', () => process.exit(1));

//Konzept: msgSent Objekt enthält topic:nachricht die der Client an den Broker published
var msgSent = {}

const publishOptions = {
    qos: 1,
    retain: true
}

const proxyMsgSent = new Proxy(msgSent, {
    get: function (target, prop){
        return target[prop]
    },
    set: function (target, prop, value){
        console.log(target)
        console.log(prop, ': ', value)
        client.publish(prop.toString(), value.toString(), publishOptions)
        target[prop] = value
        console.log(target)
    }

})

function startPrompt(){
    prompt.start()
    prompt.get(['topic', 'message'], function(err, result){
        if(err){
            console.log(err)
            return 1
        }
        proxyMsgSent[result.topic] = result.message
        startPrompt()
    })
}

startPrompt()