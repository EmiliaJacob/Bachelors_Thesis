-*
+^articles(articleId=*,"bid") -commands=Set -xecute="d spool^MOSQUITTO(""^ms"","""",""aabay/bids/""_articleId,$ZTVALUE)" -name=spool