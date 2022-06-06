#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

cp ../../../../lib_src/tapp/tappctrl_proto.xml .
tdr --xml2h tappctrl_proto.xml
#tdr --xml2c tappctrl_proto.xml -o tappctrl_proto.c
