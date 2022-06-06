#!/usr/local/bin/python
import os
import shutil
import os.path

..\..\tools\tdr --xml2h tapp_basedata_def.xml 
..\..\tools\tdr --xml2c -o tapp_basedata_def.c tapp_basedata_def.xml 
..\..\tools\tdr --xml2h tapp_rundata_timer_def.xml
..\..\tools\tdr --xml2c -o tapp_rundata_timer_def.c tapp_rundata_timer_def.xml
..\..\tools\tdr --xml2h tappctrl_proto.xml
..\..\tools\tdr --xml2c tappctrl_proto.xml -o tappctrl_proto.c
