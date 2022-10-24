
 w $ztrigger("item","+^articles(articleId=*,""bid"") -commands=Set -xecute=""d &mqtrigger.addMqttMessage(""""aabay/bids/""""_articleId,$ZTVALUE)"" -name=mq")
 w $ztrigger("item","-^articles(articleId=*,""bid"") -commands=Set -xecute=""d &mqttclient.publishMessage(""""aabay/bids/""""_articleId,$ZTVALUE)"" -name=client")
 w $ztrigger("item","-^articles(articleId=*,""bid"") -commands=Set -xecute=""d spool^MOSQUITTO(""""^ms"""","""""""",""""aabay/bids/""""_articleId,$ZTVALUE)"" -name=spool")