#!/bin/bash

for i in {1..1000}
do
  sleep $1
  ./set_global_var 123 1 0
done
