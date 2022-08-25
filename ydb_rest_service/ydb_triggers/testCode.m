 set x="/root/trig/testFifo"
 open x:fifo
 use x write $ZTVALUE,!
 close x
