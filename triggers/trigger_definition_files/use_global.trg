-*
+^articles(articleId=*,"title") -commands=Set -xecute="d spool^MOSQUITTO(""^ms"","""",""aabay/title/""_articleId,$ZTVALUE)" -name=spool
