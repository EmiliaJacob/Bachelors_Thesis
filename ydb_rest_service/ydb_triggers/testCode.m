	set x="/app/testFifo"
	open x:fifo
	use x write $ZTVALUE,!
	close x