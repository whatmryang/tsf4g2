#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

echo ${PATH}

tdr --xml2c  -o transfer_conf_desc.c transfer_conf_desc.xml
tdr --xml2h  transfer_conf_desc.xml
tdr --xml2h  ../protocol/transfer_fore_proto.xml
tdr --xml2c  -o transfer_fore_proto.c ../protocol/transfer_fore_proto.xml
tdr --xml2h  ../protocol/transfer_backsvr_proto.xml
tdr --xml2c  -o transfer_backsvr_proto.c ../protocol/transfer_backsvr_proto.xml
