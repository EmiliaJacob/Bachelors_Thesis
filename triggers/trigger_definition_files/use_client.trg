-*
+^articles(articleId=*,"bid") -commands=Set -xecute="d &mqttclient.publishMessage(""aabay/bids/""_articleId,$ZTVALUE)" -name=client
