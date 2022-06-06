#!/usr/local/bin/python
import os
import shutil
import os.path

cwd=os.getcwd()
tdr=os.path.abspath(os.path.join(cwd,'../../tools/tdr' ))
print tdr

shutil.copy(os.path.abspath('../../lib_src/tbus/tbus_macros.xml'),'./')

os.system(tdr+'  --xml2h tbus_macros.xml tbusd_desc.xml ')
os.system(tdr+' --xml2c ../../lib_src/tbus/tbus_macros.xml tbusd_desc.xml  -o tbusd_desc.c')
os.system(tdr+' --xml2h tbusd_ctrl_proto.xml ')
os.system(tdr+' --xml2c tbusd_ctrl_proto.xml  -o tbusd_ctrl_proto.c')