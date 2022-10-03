MOSQUITTO	;

	; Aufruf durch MOSQUITTO

AUTH(clid,user,pass)
	n (user,pass,clid)
	s a=$i(^dummy)
	i user="",pass="" q 0
	i user="W",pass="B" q 0
	q 1

ACL(clid,user,access,topic,payloadlen,payload,resp)
	n (clid,user,access,topic,payloadlen,payload,resp)
	s ok=12 ;MOSQ_ERR_ACL_DENIED
	i access=4 d  ;;;; SUBSCRIBE
	. i topic="chat" d
	. . s ok=0
	. . d spool^MOSQUITTO("m","","chat",clid_" has joined")

	e  i access=2 d  ;;;; WRITE
	. s vgl="mqttfetch/chat/"_clid_"/fr/"
	. i $E(topic,1,$L(vgl))'=vgl q
	. s ok=0
	. s $E(topic,$L(clid)+16,$L(clid)+19)="/to/"
	. d spool^MOSQUITTO("m",clid,topic,"ok") ; mqttfetch-response
	. d spool^MOSQUITTO("m","","chat",payload) ; chat

	e  i access=1 d ;;;; READ
	. s ok=0
	s resp=$$convert^MOSQUITTO(.m)
	q ok
