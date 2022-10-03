MOSQUITTO	;
	;
	; Aufruf durch MOSQUITTO
	;
AUTH(clid,user,pass)
	new (user,pass,clid) ;; alle anderen lokalen vars bis auf die parameter werden im aktuellen scope resettet. das bringt bessere performance
	set a=$INCREMENT(^dummy)
	if user="",pass="" quit 0
	if user="W",pass="B" quit 0
	quit 1
	;
ACL(clid,user,access,topic,payloadlen,payload,resp)
	new (clid,user,access,topic,payloadlen,payload,resp)
	set ok=12 ;MOSQ_ERR_ACL_DENIED
	if access=4 do  ;;;; SUBSCRIBE
	. if topic="chat" do
	. . set ok=0
	. . do spool^MOSQUITTO("m","","chat",clid_" has joined") ;; kann entfernt werden
	;
	else  if access=2 do  ;;;; WRITE
	. set vgl="mqttfetch/chat/"_clid_"/fr/"
	. if $EXTRACT(topic,1,$L(vgl))'=vgl quit
	. set ok=0
	. set $EXTRACT(topic,$L(clid)+16,$L(clid)+19)="/to/"
	. do spool^MOSQUITTO("m",clid,topic,"ok") ; mqttfetch-response
	. do spool^MOSQUITTO("m","","chat",payload) ; chat
	;
	else  if access=1 do ;;;; READ
	. set ok=0
	set resp=$$convert^MOSQUITTO(.m)
	quit ok
	;