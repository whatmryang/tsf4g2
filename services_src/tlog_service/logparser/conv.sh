#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2h  logparser_desc.xml
tdr --xml2c  -o logparser_desc.c logparser_desc.xml
