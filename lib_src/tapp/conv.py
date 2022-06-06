#!/usr/local/bin/python
import os
import shutil
import os.path

cwd=os.getcwd()
tdr=os.path.join(cwd,'../../tools/tdr' )
print tdr

os.system(tdr+' --xml2h tapp_basedata_def.xml') 
os.system(tdr+'  --xml2c -o tapp_basedata_def.c tapp_basedata_def.xml ') 
os.system(tdr+'  --xml2h tapp_rundata_timer_def.xml') 
os.system(tdr+'  --xml2c -o tapp_rundata_timer_def.c tapp_rundata_timer_def.xml') 
os.system(tdr+'  --xml2h tappctrl_proto.xml') 
os.system(tdr+'  --xml2c tappctrl_proto.xml -o tappctrl_proto.c') 
