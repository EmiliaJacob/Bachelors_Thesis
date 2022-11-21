#!/bin/bash 
while true
do
        fifoContent=$(cat MqttFifo)
        topic=$(echo $fifoContent | cut -d" " -f1)
        payload=$(echo $fifoContent | cut -d" " -f2)
        mosquitto_pub -t $topic -m $payload
done
