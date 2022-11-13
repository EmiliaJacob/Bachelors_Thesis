appendMessage(dest,clid,topic,payload) 
	if $EXTRACT(dest)="^" do 
	. new (dest,clid,topic,payload) 
	. lock +@dest 
	else  do 
	. new (dest,@dest,clid,topic,payload) 
	set nr=$INCREMENT(@dest) 
	set dummy("clientid")=clid,dummy("topic")=topic,dummy("payload")=payload 
	merge @(dest_"("_nr_")")=dummy 
	if $EXTRACT(dest)="^" lock -@dest 
	quit