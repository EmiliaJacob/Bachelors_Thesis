-*
+^articles(articleId=*,"title") -commands=Set -xecute="d appendMessage^syncBuffer(""^ms"",""aabay/title/""_articleId,$ZTVALUE)" -name=spool
