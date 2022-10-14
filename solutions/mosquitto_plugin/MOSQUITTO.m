MOSQUITTO	;
	;
	; Zentrale Unterroutinen
	;
TICK ;; // Evtl besser in C ???
	;; Besser wÃ¤re mit $O rein und bei vor/bei ^MQTTSPOOL aufhÃ¶ren?
	new
	if '$DATA(^MQTTSPOOL) quit $CHAR(0) ;; check if mqttspool global var exists
	lock +^MQTTSPOOL ;; Bezeichner ^MQTTSPOOL wird gelockt
	merge m=^MQTTSPOOL kill ^MQTTSPOOL ;; Inhalt von ^MQTTSPOOL wird in globale Var m kopiert
	lock -^MQTTSPOOL ;; MQTTSPOOL wird wieder freigegeben
	quit $$convert(.m) ;; actualname: m wird per referenz gepasst. muss bei scope-wechsel noch nicht definiert sein
	;
spool(dest,clid,topic,message) ; ToDo: evtl Lock
	if $EXTRACT(dest)="^" do ; erstes zeichen wird extrahiert. es wird geprueft ob es sich um eine globale variabel handelt
	. new (dest,clid,topic,message) ;; alle anderen lokalen vars bis auf die parameter werden im aktuellen scope resettet. das bringt bessere performance
	. lock +@dest ; zielvariabelnname wird ueber indirektion ermittelt und gelockt. ; ist praktisch mqttspool
	else  do ;; wenn globale var gepasst wurde, dann wird auch sie vom new ausgenommen. ;
	. new (dest,@dest,clid,topic,message)
	set nr=$INCREMENT(@dest) ; die variable scheint als wert einen nachrichtencounter zu haben. er wird hier erhoeht
	set dummy("c")=clid,dummy("t")=topic,dummy("m")=message ;es wird eine neue lokal variable dummy in diesem scope hier gesetzt. sie bekommt client id, topic und message als subscript befuellt. ;
	merge @(dest_"("_nr_")")=dummy ; die variabel hat den nachrichten-index als subscript. diesem subscript wird die dummy variable angehaengt. ; es wird gemerget, da sonst nur referenzen zu den bald ungueltigen lokalen parameter variabeln gespeichert wuerden. die dummy variabel erlaubt ein nur einmaliges mergen. ;
	if $EXTRACT(dest)="^" lock -@dest ; falls die zielvariable global ist, wird sie wieder entlockt. ;
	quit
	;
convert(m) ;; konvertiert varibel mit subscripts in string und returnt diesen
	new (m) ;; alle anderen lokalen vars bis auf m werden im aktuellen scope resettet
	set str="_",numberOfMessages=0,index="" ;; index wird auf leeren string initiert. Index ist der Index der Nachricht es koennen mehere Nachrichten eingefuegt worden sein
	for  set index=$O(m(index)) quit:index=""  do ;; for does not generate an additional scope level for variables. it has no argument, so two spaces have to follow. its execution scope is in the same line, but can be extenden with a do. Die Schleife durchlaeuft einmal alle subscripts von m, was zuvor mqttspool war.  
	. set numberOfMessages=numberOfMessages+1
	. if $GET(m(index,"c"))'="" do ;;c ist vmtl client --> abaendern!
	. . set str=str_$CHAR($LENGTH(m(index,"c"))\256,$LENGTH(m(index,"c"))#256)_m(index,"c")_$C(0)
	. else  do
	. . set str=str_$CHAR(0,0,0) ; erster teil des strings bekommt null als wert, wenn kein client bekannt ist
	. set str=str_$CHAR($LENGTH(m(index,"t"))\256,$LENGTH(m(index,"t"))#256)_m(index,"t")_$CHAR(0)
	. set str=str_$CHAR($LENGTH(m(index,"m"))\256,$LENGTH(m(index,"m"))#256)_m(index,"m")_$CHAR(0)
	set $EXTRACT(str,1)=$CHAR(numberOfMessages) ; fuegt die anzahl der nachrichten dem string hinzu
	quit str
	;
MSG(clid,user,topic,msg,ok) ; Not needed
	set ok=0;
	quit $C(0)
	new (clid,user,topic,msg)
	; jetzt ein paar direkte Antworten
	do spool("m",clid,"t","Hallo!") ; nur an Client selber
	do spool("m","","t","blubb") ; an alle
	do spool("^MQTTSPOOL","","to/all",$H) ; Gespoolt - zum Testen
	quit $$convert(.m)
	;