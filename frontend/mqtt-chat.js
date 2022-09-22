var chat, msg, mqtt;

window.onload = async function () {
	mqtt = new mqtt_fetch("chat");
	//await mqtt.init(`ws://${location.host}/mqtt/`); // nginx forwarding
	await mqtt.init(`ws://${location.host}:1884`); // ws-port!!!!!
	await mqtt.set_callback("chat", rx);
	document.getElementById("sb").addEventListener("click", tx);
	chat = document.getElementById("c"),
	msg = document.getElementById("m")
}

async function tx () {
	console.log(await mqtt.fetch(msg.value));
}

function rx(txt) {
	console.log(event);
	if (chat == undefined) {
		console.log("zu frÃ¼h", txt);
		return;
	}
	var n = chat.insertBefore(
		document.createElement("li"), chat.childNodes[0]);
	n.appendChild(document.createTextNode(txt));
	while (chat.childNodes.length > 10)
		chat.removeChild(chat.childNodes[10]);
}