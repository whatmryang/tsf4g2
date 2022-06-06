#!/bin/sh

cd ${HOME}/tsf4g/src/tbus/relay

RELAY=`ps -e -o pid,comm | grep tbusd | awk '{print $1}'`
/bin/kill -SIGUSR1 ${RELAY}

