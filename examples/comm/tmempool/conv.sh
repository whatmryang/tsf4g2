#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH
chmod 777 *.sh
tdr --xml2h  tmempool_conf_desc.xml
tdr --xml2c  -o tmempool_conf_desc.c tmempool_conf_desc.xml
