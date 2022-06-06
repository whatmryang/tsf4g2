#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2h  tlogtest_conf_desc.xml
tdr --xml2c  -o tlogtest_conf_desc.c tlogtest_conf_desc.xml
