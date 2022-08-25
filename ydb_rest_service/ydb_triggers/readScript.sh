#!/bin/bash 
while true
do
        message=$(cat testFifo)
        mosquitto_pub -t /test -m $message
done
