#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2h  example_conf_desc.xml
tdr --xml2c  -o example_conf_desc.c example_conf_desc.xml
