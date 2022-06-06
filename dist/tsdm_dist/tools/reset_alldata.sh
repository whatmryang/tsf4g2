#!/bin/sh

./stopall.sh
echo "please wait ...."
sleep 10 
./rmshm.sh
sleep 1
../sql/clear.sh
rm -rf ../transd/tid.counter
rm -rf ../operd/counter.*
