MOSQUITTO	;

	; Aufruf durch MOSQUITTO

AUTH(clid,user,pass)
	n (user,pass,clid)
	s a=$i(^dummy)
	i user="",pass="" q 0
	i user="W",pass="B" q 0
	q 1

MSG(clid,user,topic,msg,ok)
	n (clid,user,topic,msg)
	; jetzt ein paar direkte Antworten
	d spool("m",clid,"TestTopic","Hallo!") ; nur an Client selber
	d spool("m","","test/in","blubb") ; an alle
	d spool("^MQTTSPOOL","","to/all",$H) ; Gespoolt - zum Testen
	q $$convert(.m)

ACL(clid,user,topic,access,ok)
	n (clid,user,topic,access,ok)
	i access=4 d spool("m",clid,"wel","come") ; Begruessung
	s ok=0 ; subscibe und write erlaubt
	q $$convert(.m)

TICK ;; ToDo: evtl Lock
	n
	i '$D(^MQTTSPOOL) q $C(0)
	m m=^MQTTSPOOL k ^MQTTSPOOL
	q $$convert(.m)

	; Interne Hilfsroutinen
	; (ausser spool, das kann von ueberall her gerufen werden wenn ^MQTTSPOOL uebergeben wird)

spool(dest,clid,topic,message) ; ToDo: evtl Lock
	i $E(dest)="^" d
	. n (dest,clid,topic,message)
	e  d
	. n (dest,@dest,clid,topic,message)
	s nr=$i(@dest)
	s dummy("c")=clid,dummy("t")=topic,dummy("m")=message
	m @(dest_"("_nr_")")=dummy
	q

convert(m)
	n (m)
	s str="_",n=0,idx=""
	f  s idx=$O(m(idx)) q:idx=""  d
	. s n=n+1
	. i $G(m(idx,"c"))'="" d
	. . s str=str_$C($L(m(idx,"c"))\256,$L(m(idx,"c"))#256)_m(idx,"c")_$C(0)
	. e  d
	. . s str=str_$C(0,0,0)
	. s str=str_$C($L(m(idx,"t"))\256,$L(m(idx,"t"))#256)_m(idx,"t")_$C(0)
	. s str=str_$C($L(m(idx,"m"))\256,$L(m(idx,"m"))#256)_m(idx,"m")_$C(0)
	s $E(str,1)=$C(n)
	q str


