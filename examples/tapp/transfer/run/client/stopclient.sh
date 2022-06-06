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
    ./tdrclient  --pid-file=/tmp/tdrclient$i.pid  stop
done
