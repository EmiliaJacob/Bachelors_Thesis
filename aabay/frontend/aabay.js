
 
 var m;
window.onload = async function() {
	m = new mqtt_fetch("aabay");
	await m.init("localhost", 1884); // MQTT over websockets!!
	m.set_callback(-1, rx_status, true);
	make_list();
	document.getElementById("send").addEventListener("click", tx_bid);
}

async function make_list() {
	var articles = await m.send({action: "get_articles"});
	console.log(articles);
	for (let i = 0; i < articles.articles.length; i++) {
		let node = document.createElement("li");
		node.setAttribute("class", "list-item");
		node.setAttribute("id", "list_" + articles.articles[i].id)
		node.appendChild(document.createTextNode(articles.articles[i].title));
		node.appendChild(document.createElement("br"));
		node.appendChild(document.createElement("text"));
		node.lastChild.setAttribute("class", "bid-" + articles.articles[i].id)
		node.lastChild.appendChild(document.createTextNode(articles.articles[i].bid));
		node.appendChild(document.createTextNode(" \u20ac"));
		node.addEventListener("click", show);
		document.getElementById("list").appendChild(node);
		m.set_callback("aabay/bids/" +  articles.articles[i].id, rx_bid, true);
	}
}

async function show(prm) {
	let id = +prm.target.id.substring(5);
	console.log(id);
	let article = await m.send({action: "get_article", id: id});
	console.log(article);
	document.getElementById("id").firstChild.nodeValue = article.article.id;
	document.getElementById("title").firstChild.nodeValue = article.article.title;
	document.getElementById("text").firstChild.nodeValue = article.article.text;
	document.getElementById("bid").firstChild.nodeValue = article.article.bid;
	document.getElementById("bid").setAttribute("class", "bid-" + article.article.id)
}

function rx_bid(topic, data) {
	console.log(topic, data);
	let id = topic.split("/");
	id = id[id.length - 1];
	console.log("bid-" + id);
	for (i=0; i < document.getElementsByClassName("bid-" + id).length; i++) {
		console.log(i);
		document.getElementsByClassName("bid-" + id)[i].firstChild.nodeValue = data;
	}
}

function rx_status(data) {
	console.log("rx_status", data, JSON.stringify(data));
	document.getElementById("status").firstChild.nodeValue = JSON.stringify(data);
}

async function tx_bid() {
	var result = await m.send({
		action: "bid",
		id: document.getElementById("id").firstChild.nodeValue,
		nickname: document.getElementById("nickname").value,
		bid: +document.getElementById("my-bid").value
	});
	document.getElementById("response").firstChild.nodeValue = JSON.stringify(result);

}
