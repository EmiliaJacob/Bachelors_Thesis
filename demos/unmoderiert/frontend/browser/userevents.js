sampleData = {
    "house": "22",
    "house/mainhall/light": "on",
    "house/kitchen/light": "off",
    "house/kitchen/stove": "off",
    "house/cellar/light": "off"
}

const subscribeOptions = {
    qos: 1
}

const checkTopicFormat = (topicString, pub) => {
    //true for publish mode (# and + not allowed)
    //conversion to ASCII without control chars
    topicString = topicString.replace(/[^\x20-\x7E]+/g, '');
    len = topicString.length;
    if(len == 0){ 
        alert('no empty topic allowed')
        return false
    }
    if(topicString.includes(' ')){
        alert('Malformed topic for publishing');
        return false;
    }
    if(topicString.charCodeAt(0) == 47 || topicString.charCodeAt(len-1) == 47){
        alert('no / at start or end of topic allowed')
        return false;
    }
    if(pub && (topicString.includes('#') || topicString.includes('+'))){
        alert('no wildcards allowed for publishing');
        return false;
    }
    return true
}

document.getElementById('publishButton').onclick = function () {
    var topic = document.querySelector('#topicField').value;
    var message = document.querySelector('#messageField').value
    console.log('published:',topic, message)
    if(message.length == 0){
        alert('please specify message')
        return
    }
    if(!checkTopicFormat(topic, true)) return;
    proxyUsr[topic] = message
}

document.getElementById('subscribeButton').onclick = function () {
    var topic = document.querySelector('#subTopicField').value;
    if(!checkTopicFormat(topic, false)) return;
    client.subscribe(topic, subscribeOptions);
}

document.getElementById('clearButton').onclick = function() {
    var topic = document.querySelector('#unsubTopicField').value;
    if(!checkTopicFormat(topic, false)) return;
    delete proxyUsr[topic];
}

document.getElementById('sampleDataButton').onclick = function() {
    for(var prop in sampleData){
        client.publish(prop, sampleData[prop], publishOptions)
    }
}

document.getElementById('defaultSubButton').onclick = function() {
    for(var prop in sampleData){
        client.subscribe(prop, subscribeOptions)
    }
}

