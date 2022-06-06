#!/bin/sh
./tqossvr --id 1.0.113.2 --conf-file ../../cfg/tqossvr/tqossvr_withproxy1.xml --log-level 700 --log-file tqossvr_withproxy1 --use-bus  --daemon start
sleep 1
./tqossvr --id 1.0.113.3 --conf-file ../../cfg/tqossvr/tqossvr_withproxy2.xml --log-level 700 --log-file tqossvr_withproxy2 --use-bus --daemon start
