#!/bin/bash 
while true
do
        fifoContent=$(cat mosquittoFifo)
        topic=$(echo $fifoContent | cut -d" " -f1)
        message=$(echo $fifoContent | cut -d" " -f2)
        mosquitto_pub -t $topic -m $message
done
