MOSQUITTO	;
	;
	; Zentrale Unterroutinen
	;
spool(dest,clid,topic,message) ; ToDo: evtl Lock
	if $EXTRACT(dest)="^" do ; erstes zeichen wird extrahiert. es wird geprueft ob es sich um eine globale variabel handelt
	. new (dest,clid,topic,message) ;; alle anderen lokalen vars bis auf die parameter werden im aktuellen scope resettet. das bringt bessere performance
	. lock +@dest ; zielvariabelnname wird ueber indirektion ermittelt und gelockt. ; ist praktisch mqttspool
	else  do ;; wenn lokale var gepasst wurde, dann wird auch sie vom new ausgenommen. ;
	. new (dest,@dest,clid,topic,message) ; dest kann als string oder als refernz uebergeben werden
	set nr=$INCREMENT(@dest) ; die variable scheint als wert einen nachrichtencounter zu haben. er wird hier erhoeht. Der Wert der WurzelVar ist die Anzahl der Nachrichten (Subscripts)
	set dummy("client")=clid,dummy("topic")=topic,dummy("message")=message ;es wird eine neue lokal variable dummy in diesem scope hier gesetzt. sie bekommt client id, topic und message als subscript befuellt. ;
	merge @(dest_"("_nr_")")=dummy ; die variabel hat den nachrichten-index als subscript. diesem subscript wird die dummy variable angehaengt. ; es wird gemerget, da sonst nur referenzen zu den bald ungueltigen lokalen parameter variabeln gespeichert wuerden. die dummy variabel erlaubt ein nur einmaliges mergen. ;
	if $EXTRACT(dest)="^" lock -@dest ; falls die zielvariable global ist, wird sie wieder entlockt. ;
	quit
	;