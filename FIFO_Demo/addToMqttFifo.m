 set mosquittoFifo="/ydbay/FIFO_Demo/MqttFifo"
 open mosquittoFifo:fifo
 set topic="aabay/title/"_articleId
 set message=$ZTVALUE
 use mosquittoFifo write topic_" "_message,!
 close mosquittoFifo