 set mosquittoFifo="/ydbay/FIFO_Demo/MqttFifo"
 open mosquittoFifo:fifo
 set topic="aabay/title/"_articleId
 set payload=$ZTVALUE
 use mosquittoFifo 
 write topic_" "_payload,!
 close mosquittoFifo