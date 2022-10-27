#!/bin/bash

for i in {1..5000}
do
  ./trigger_trigger 123
  echo $i
done
