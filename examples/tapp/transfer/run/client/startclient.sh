#!/bin/bash


if [ -z "$1" ];then
  count=1
else
  count=$1
fi

i=0
while [ $i -lt $count ]
do
  i=`expr $i + 1`
  ./tdrclient  --online=1--package=1024 --max-speed=1 --conf-file=./tdrclient_def_data.xml --timer=1000 --pid-file=/tmp/tdrclient$i.pid -D start
done
