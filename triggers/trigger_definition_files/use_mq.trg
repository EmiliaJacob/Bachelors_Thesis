-*
+^articles(articleId=*,"bid") -commands=SET -xecute="d &mqtrigger.addMqttMessage(""aabay/bids/""_articleId,$ZTVALUE)" -name=mq
