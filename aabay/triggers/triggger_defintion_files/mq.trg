
+^articles(articleId=*,"bid") -commands=Set -xecute="d &mqtrigger.addMqttMessage(""aabay/bids/""_articleId,$ZTVALUE)" -name=mq