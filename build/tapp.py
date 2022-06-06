import os
import shutil
from optparse import OptionParser
import subprocess
from xml.dom import minidom
from time import strftime, localtime
from datetime import timedelta, date
import tarfile
import xml


class tappBuilder:
    def __init__(self):
        return
    
    def TappScripts(self,destPath,Name,CFGPATH="../cfg/",LOGPATH="../log/",ID="1.2.3.4"):
        appName = "%s --id=%s " %(Name,ID)
        cfgPrefix = CFGPATH+Name
        logPrefix = LOGPATH+Name
        cfgFile = "--tlogconf=%s_log.xml --conf-file=%s.xml --log-file=%s" %(cfgPrefix,cfgPrefix,logPrefix)
        if self._isLinux:
            open('%s/start_%s.sh'%(destPath,Name),'w').write("./%s %s -D start" %(appName,cfgFile))
            open('%s/stop_%s.sh'%(destPath,Name),'w').write("./%s  stop"%(appName))
            open('%s/reload_%s.sh'%(destPath,Name),'w').write("./%s  reload" %(appName))
            open('%s/control_%s.sh'%(destPath,Name),'w').write("./%s  control"%(appName))
        else:
            open('%s/start_%s.bat'%(destPath,Name),'w').write("%s %s -D start" %(appName,cfgFile))
            open('%s/stop_%s.bat'%(destPath,Name),'w').write("%s  stop"%(appName))
            open('%s/reload_%s.bat'%(destPath,Name),'w').write("%s  reload" %(appName))
            open('%s/control_%s.bat'%(destPath,Name),'w').write("%s  control"%(appName))            
        return
Export('tappBuilder')