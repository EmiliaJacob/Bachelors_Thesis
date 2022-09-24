MOSQUITTO	;

	; Zentrale Unterroutinen

TICK ;; // Evtl besser in C ???
	;; Besser wÃ¤re mit $O rein und bei vor/bei ^MQTTSPOOL aufhÃ¶ren?
	n
	i '$D(^MQTTSPOOL) q $C(0)
	l +^MQTTSPOOL
	m m=^MQTTSPOOL k ^MQTTSPOOL
	l -^MQTTSPOOL
	q $$convert(.m)

spool(dest,clid,topic,message) ; ToDo: evtl Lock
	i $E(dest)="^" d
	. n (dest,clid,topic,message)
	. l +@dest
	e  d
	. n (dest,@dest,clid,topic,message)
	s nr=$i(@dest)
	s dummy("c")=clid,dummy("t")=topic,dummy("m")=message
	m @(dest_"("_nr_")")=dummy
	i $E(dest)="^" l -@dest
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

MSG(clid,user,topic,msg,ok) ; Not needed
	s ok=0;
	q $C(0)
	n (clid,user,topic,msg)
	; jetzt ein paar direkte Antworten
	d spool("m",clid,"t","Hallo!") ; nur an Client selber
	d spool("m","","t","blubb") ; an alle
	d spool("^MQTTSPOOL","","to/all",$H) ; Gespoolt - zum Testen
	q $$convert(.m)
