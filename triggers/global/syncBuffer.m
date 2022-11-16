appendMessage(dest,topic,payload) 
	if $EXTRACT(dest)="^" do 
	. new (dest,topic,payload) 
	. lock +@dest 
	else  do 
	. new (dest,@dest,topic,payload) 
	set nr=$INCREMENT(@dest) 
	set dummy("topic")=topic,dummy("payload")=payload 
	merge @(dest_"("_nr_")")=dummy 
	if $EXTRACT(dest)="^" lock -@dest 
	quit