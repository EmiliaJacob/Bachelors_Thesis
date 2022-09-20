# mqtt-proxy
This Application is a PoC of how JS Proxies can be used to Mirror Object via MQTT
## Usage
1. Start the broker.js via node (node broker). This will start a MQTT Broker service running on your local machine. The Port for native MQTT is 1883 and for MQTT over Websockets is 8883
2. In /browser open index.html, needed MQTT.js Modules will be loaded via CDN
3. Open index.html in a different browser/window
4. Try subscribing to default and publish a sample dataset. Try your own topics, wildcards, unsubscribe (topics will only be shown if you subscribed on them, otherwise they only appear in the mirror object)
5. The mirror object is logged in the browser console each time it is updated

Note: MQTT-Wildcards sometimes behave unintuitive when subscribing, unsubscribing and then publishing on them.
https://groups.google.com/g/mqtt/c/eWf46f6d1wU describes how they are supposed to work. From my testing this implementation works the same as described.
