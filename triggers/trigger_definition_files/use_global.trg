-*
+^articles(articleId=*,"title") -commands=Set -xecute="d appendMessage^syncBuffer(""^globalSyncBuffer"",""aabay/title/""_articleId,$ZTVALUE)" -name=spool
