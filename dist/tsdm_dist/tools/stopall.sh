#!/bin/sh

cd ../bin
./stop_operd.sh
sleep 1

./stop_transd.sh
sleep 1

./stop_dmsd.sh
sleep 1

