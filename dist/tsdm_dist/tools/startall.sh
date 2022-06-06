#!/bin/sh

cd ../bin
./start_dmsd.sh
sleep 1

./start_transd.sh
sleep 1

./start_operd.sh

