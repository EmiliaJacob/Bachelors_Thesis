 if $ZTVALUE=0  do
 .set mosquittoFifo="/app/mosquittoFifo"
 .open mosquittoFifo:fifo
 .use mosquittoFifo write sel,!
 .close mosquittoFifo