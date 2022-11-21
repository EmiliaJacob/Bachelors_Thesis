-*
+^articles(articleId=*,"title") -commands=Set -xecute="d &mqttclient.publishMessage(""aabay/title/""_articleId,$ZTVALUE)" -name=client
