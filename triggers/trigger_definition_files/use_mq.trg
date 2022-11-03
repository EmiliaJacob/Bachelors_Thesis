-*
+^articles(articleId=*,"bid") -commands=SET -xecute="d &mqtrigger.addMqttMessage($ZTLEVEL,""aabay/bids/""_articleId,$ZTVALUE)" -name=mq
