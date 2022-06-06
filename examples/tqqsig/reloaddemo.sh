#!/bin/sh
DEMO_PID=`ps -e -o pid,comm|grep -v grep|grep -v sh|grep  tqqsigdemo|awk '{print $1}'`
echo "Reload demo $DEMO_PID"
/bin/kill -s SIGUSR2 $DEMO_PID



