#!/bin/bash 
while true
do
        message=$(cat mosquittoFifo)
        mosquitto_pub -t /test -m $message
done
