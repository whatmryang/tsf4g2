#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH
chmod 777 *.sh
tdr --xml2h  shtable_conf_desc.xml
tdr --xml2c  -o shtable_conf_desc.c shtable_conf_desc.xml
