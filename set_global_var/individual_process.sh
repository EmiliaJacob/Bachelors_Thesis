#!/bin/bash

for (( i=0; i<$1; i++))
do
  sleep $2
  ./set_global_var $3 1 0
done
