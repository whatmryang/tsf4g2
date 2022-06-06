#!/bin/sh

# Fisrt of all, you should set PATH to xml2c and xml2h properly

PATH=${HOME}/tsf4g/tools/:${PATH}
export PATH

tdr --xml2h tbus_test_desc.xml -o tbus_test_desc.h
tdr --xml2h tbus_test_net.xml -o tbus_test_net.h

tdr --xml2c tbus_test_desc.xml -o tbus_test_desc.c
tdr --xml2c tbus_test_net.xml -o tbus_test_net.c

