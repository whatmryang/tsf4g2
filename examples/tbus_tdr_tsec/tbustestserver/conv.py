#!/usr/local/bin/python
import os
import shutil
import os.path

cwd=os.getcwd()
tdr=os.path.abspath(os.path.join(cwd,'../../../tools/tdr' ))
print tdr

os.system(tdr+' --xml2h  tbustestserver_conf_desc.xml')
os.system(tdr+' --xml2c -o tbustestserver_conf_desc.c tbustestserver_conf_desc.xml')
os.system(tdr+'  --xml2h tbustestCS.xml')
os.system(tdr+'  --xml2c -o tbustestCS.c tbustestCS.xml')



