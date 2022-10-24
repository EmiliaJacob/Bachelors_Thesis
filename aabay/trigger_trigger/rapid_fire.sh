#!/bin/bash

for i in {1..5000}
do
  ./a.out 123
  echo $i
done
