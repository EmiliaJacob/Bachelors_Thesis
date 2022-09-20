//Establishing connection to Broker
var client = mqtt.connect('ws:localhost:9001')

//data Object is the mirrored Object
var mirrorObj = {}

const proxyUsr = new Proxy(mirrorObj, userhandler)

const proxyMsg = new Proxy(mirrorObj, msghandler)

createDatasetTable()