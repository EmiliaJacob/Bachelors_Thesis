-*
+^articles(articleId=*,"title") -commands=SET -xecute="d &mqtrigger.addMqttMessage(""aabay/title/""_articleId,$ZTVALUE)" -name=mq
