	set mosquittoFifo="/app/mosquittoFifo"
	open mosquittoFifo:fifo
	set topic="/test"
	set message=$ZTVALUE
	use mosquittoFifo write $topic_" "_message,!
    close mosquittoFifo