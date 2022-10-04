MOSQUITTO	;
	;
	; Aufruf durch MOSQUITTO
	;
AUTH(clid,user,pass) ; liefert 0 zurueck falls authentifizierung erfolgreich verlief
	new (user,pass,clid) ;; alle anderen lokalen vars bis auf die parameter werden im aktuellen scope resettet. das bringt bessere performance
	set a=$INCREMENT(^dummy) ; die globale variabel dummy scheint ein counter fuer die authentifizierungsversuche zu sein. ;
	if user="",pass="" quit 0 ; hier koennen die zugelassenen user festgelegt werden. ;
	if user="W",pass="B" quit 0 
	quit 1
	;
ACL(clid,user,access,topic,payloadlen,payload,resp)
	new (clid,user,access,topic,payloadlen,payload,resp) ; alle anderen lokalen vars bis auf die parameter werden im aktuellen scope resettet. das bringt bessere performance
	set ok=12 ;MOSQ_ERR_ACL_DENIED -> entsprechender mosquitto fehlercode
	if access=4 do  ;;;; SUBSCRIBE; Die Clients muessen eine unterschiedliche ID haben, sonst Endlosschleife. ;
	. if topic="chat" do ; es wird aktuell nur bei subscriptions auf das topic chat die status var auf erfolg gesetzt. ;
	. . set ok=0
	. . do spool^MOSQUITTO("m","","chat",clid_" has joined") ; als ziel fuer das spooling wird die lokale variable m gewaehlt. sie ist wahrscheinlich im shared memory zusammen mit den statischen variabeln des c triggers
	;
	else  if access=2 do  ;;;; WRITE wird aufgerufen, falls ein client eine Nachricht an ein Topic schickt. ;
	. set vgl="mqttfetch/chat/"_clid_"/fr/" ; Es kann nur von mqttfetch/chat/clid/fr geschrieben werden, sonst wird fkt abgebrochen und status code bleibt error. ;
	. if $EXTRACT(topic,1,$LENGTH(vgl))'=vgl quit
	. set ok=0
	. set $EXTRACT(topic,$LENGTH(clid)+16,$LENGTH(clid)+19)="/to/" ; Es wird die mqttfetch antwort gebildet, indem das /fr/ durch /to/ ersetzt wird
	. do spool^MOSQUITTO("m",clid,topic,"ok") ; mqttfetch-response wird der lokale var m hinzugefuegt
	. do spool^MOSQUITTO("m","","chat",payload) ; chatnachricht des clients wird an alle anderen chats subscriber releast
	;
	else  if access=1 do ;;;; READ tritt ein bevor eine Nachricht den Broker an einen Client verlassen soll
	. set ok=0
	set resp=$$convert^MOSQUITTO(.m) ; response wird auf alle gespoolten nachrichten in stringform gesetzt
	quit ok
	;
	; TODO: eventuell die mosquitto fehlercodes als constante speichern oder passen
	; TODO: Es kann mom nicht das mqttfetch-response topic subscribiert werden
	; TODO: ACL config in entsprechende config file auslagern