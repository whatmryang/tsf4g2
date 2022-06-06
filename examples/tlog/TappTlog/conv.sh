#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH
chmod 777 *.sh
tdr --xml2h  TappTlog_conf_desc.xml
tdr --xml2c  -o TappTlog_conf_desc.c TappTlog_conf_desc.xml
