    if $ZTVALUE=0  do
    .set mosquittoFifo="/app/mosquittoFifo"
    .open mosquittoFifo:fifo
    .set topic="/deactivated"
    .set message=articleId
    .use mosquittoFifo write topic_" "_message,!
    .close mosquittoFifo
