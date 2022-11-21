appendMessage(dest,topic,payload) 
	new (dest,topic,payload) 
	lock +@dest 
	set nr=$INCREMENT(@dest) 
	set dummy("topic")=topic,dummy("payload")=payload 
	merge @(dest_"("_nr_")")=dummy 
	lock -@dest 
	quit