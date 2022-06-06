import sys
from xml.dom import minidom
import os
import copy

      
class AppListHandler():
    """Crude extractor for quotations.dtd compliant XML document"""
    def __init__(self, inFileName,OutFileName,a_portbase=9000,a_shmbase=2000,userid="1"):
        self.xmldoc=  minidom.parse('tcmcenter.xml.tpl')
        self.outFileName = OutFileName

        self.configDoc=minidom.parse(inFileName)
        self.configDocRoot = self.configDoc.getElementsByTagName('applist')[0]
        self.configAppList = self.configDocRoot.getElementsByTagName('app')
                
        self.domroot = self.xmldoc.getElementsByTagName('tcmcenter')[0]
        self.cluster = self.domroot.getElementsByTagName('cluster')[0]
        self.world = self.cluster.getElementsByTagName('world')[0]
        self.zone = self.world.getElementsByTagName('zone')[0]
        self.clusterDeploy = self.domroot.getElementsByTagName('ClusterDeploy')[0]
        self.hostTab = self.domroot.getElementsByTagName('HostTab')[0]
        self.host = self.hostTab.getElementsByTagName('Host')[0]
        self.tbusConf = self.domroot.getElementsByTagName('TbusConf')[0]
        self.deployWorld=self.clusterDeploy.getElementsByTagName('world')[0]
        #print self.cluster.toxml()
        self.string=''
        self.inread=0
        self.funcID = 1
        self.baseDir = ''
        self.chmodFile='#!/bin/sh'
        self.confFile='#!/bin/sh\n'

        #shm and port init
        self.shmBase=a_shmbase
        self.ShmList = {}
        self.ShmScript='#!/bin/sh\n'
        self.ShmScript+= 'cd '+self.configDocRoot.getAttribute('baseDir')+'\n'
        self.portBase=a_portbase
        self.PortList = {}
        self.GenShmList()
        self.GenPortList()
        self.globalMacro=''

        for shm in self.ShmList:
            #print shm+'='+self.ShmList[shm]
            self.globalMacro += '|\n    sed "s:##'+shm+'##:'+self.ShmList[shm]+':g"'
        for port in self.PortList:
            self.globalMacro += '|\n    sed "s:##'+port+'##:'+self.PortList[port]+':g"'
        
        #print self.ShmScript
        procList=self.configDocRoot.getElementsByTagName('Proc')
        procId = 1;
        for proc in procList:
            procId = procId +1
            proc.setAttribute('FuncID',str(procId))
			
			
		# I have to find the db and use it:
		
        databaseList=self.configDocRoot.getElementsByTagName('Database')
        database = databaseList[0]
        self.dbuser=database.getAttribute('username')
        self.dbpwdShort =database.getAttribute('password')
        #print 'Db got'
        if self.dbpwdShort == '':
            self.dbpwd =''
        else:
            self.dbpwd = ' -p '+self.dbpwdShort

        self.globalMacro += '|\n    sed "s:##'+database.getAttribute('name').upper()+'_USR'+'##:'+self.dbuser+':g"'
        self.globalMacro += '|\n    sed "s:##'+database.getAttribute('name').upper()+'_PWD'+'##:'+self.dbpwdShort+':g"'
		
        #I have to add the db config to all the db.
        basedir=self.configDocRoot.getAttribute('baseDir')
        #print basedir
        dblist=self.configDocRoot.getElementsByTagName('db')
        dbCount=0
        self.initdb='#!/bin/sh\n\n'
        for db in dblist:
            #dbCount = dbCount+1
            dbName=db.getAttribute('name')+'_'+userid
            self.initdb += 'mysql -u '+self.dbuser+self.dbpwd+' -e "drop database if exists '+dbName+'"\n'
            self.initdb += 'mysql -u '+self.dbuser+self.dbpwd+' -e "create database '+dbName+'"\n'
            pwd = db.getAttribute('dir')
            self.initdb += 'cd '+'../'+pwd+'\n'
            sqllist=db.getElementsByTagName('createSql')
            for sql in sqllist:
                sqlName=sql.getAttribute('script')
                self.initdb += 'mysql -u '+self.dbuser+self.dbpwd+' '+dbName+' < '+basedir+'../../'+pwd+sqlName+'\n'
            self.globalMacro += '|\n    sed "s:##'+db.getAttribute('name').upper()+'##:'+dbName+':g"'
        #print self.initdb
        #print self.globalMacro

        #I have to add the IP macro here.
        HostList=self.configDocRoot.getElementsByTagName('Host')
        innerIP=HostList[0].getAttribute('innerIP')
        outterIP=HostList[0].getAttribute('outterIP')
        
        ipList=self.configDocRoot.getElementsByTagName('BindIP')

        for ip in ipList:
            if(ip.getAttribute('type')) == 'innerIP':
                self.globalMacro += '|\n    sed "s:##'+'IP_'+ip.getAttribute('name').upper()+'##:'+innerIP+':g"' 
            elif ip.getAttribute('type') == 'outterIP':
                self.globalMacro += '|\n    sed "s:##'+'IP_'+ip.getAttribute('name').upper()+'##:'+outterIP+':g"' 
            else:
                print 'Wrong IP type'
                
    def GenShmList(self):
        portList = self.configDocRoot.getElementsByTagName('Shm')
        i = 0
        for port in portList:
            #print port.getAttribute('name')
            macroName = 'SHM_'+port.getAttribute('name').upper()
            if not self.ShmList.has_key(macroName):
                self.ShmList[macroName]=str(i+self.shmBase)
            if port.getAttribute('type')=='metabase':
                self.ShmScript += 'echo "[METABASE]" > tmng.conf\n'
                self.ShmScript += 'echo "    Key='+str(i+self.shmBase)+'" >> tmng.conf\n'
                self.ShmScript += 'echo "    Count	= 100" >> tmng.conf\n'
                self.ShmScript += 'echo "    Size	= 10000000" >> tmng.conf\n'
                self.ShmScript += '../tools/tmng --destroy mib\n'
                self.ShmScript += '../tools/tmng --create metabase\n'
                metalist=port.getElementsByTagName('metalib')
                for metalib in metalist:
                    metalibFile=metalib.getAttribute('file')
                    self.ShmScript += '../tools/tmng --loadmeta '+metalibFile+'\n'
            i = i+1
    def GenPortList(self):
        portList = self.configDocRoot.getElementsByTagName('Port')
        i = 0
        for port in portList:
            #print port.getAttribute('name')
            macroName = 'PORT_'+port.getAttribute('name').upper()
            if not self.PortList.has_key(macroName):
                self.PortList[macroName]=str(i+self.portBase)
            i = i+1

    def getAppByName(self, AppName):
        for app in self.configAppList:
            if(app.getAttribute('name') == AppName):
                return app

    def ProcToFunc(self,AppName,FuncName):
        #print AppName + '__'+ FuncName
        for app in self.configAppList:
            if app.getAttribute('name') == AppName:
                #print 'Find app '+AppName
                #print app.toxml()
                procList=app.getElementsByTagName('Proc')
                for proc in procList:
                    #print proc.getAttribute('FuncName')
                    if proc.getAttribute('FuncName') == FuncName:
                        #print proc.getAttribute('ProcName')
                        return proc.getAttribute('ProcName')+proc.getAttribute('FuncID')
        print 'Error Getting Proc Name'
        return ''
    def getNeighbour(self,AppName,FuncName):
        InterName=AppName+':'+FuncName
        result={}
        for app in self.configAppList:
            if app.getAttribute('name') == AppName:
                channelList=app.getElementsByTagName('Channel')
                for channel in channelList:
                    if channel.getAttribute('ProcSet1') == FuncName:
                        result[channel.getAttribute('ProcSet2')]=self.ProcToFunc(AppName,channel.getAttribute('ProcSet2'))
                    if channel.getAttribute('ProcSet2') == FuncName:
                        result[channel.getAttribute('ProcSet1')]=self.ProcToFunc(AppName,channel.getAttribute('ProcSet1'))
        procChannelList=self.configDocRoot.getElementsByTagName('IChannel')
        for procChannel in procChannelList:
            if procChannel.getAttribute('Port1')== AppName+':'+FuncName:
                PortName = procChannel.getAttribute('Port2')
                seperator=':'
                if not (PortName and seperator):
                    print 'Error'
                    return 
                #print PortName
                res = PortName.partition(seperator)
                appName = res[0]
                funcName = res[2]
                #print PortName
                #print appName
                #print FuncName
                result[funcName]=self.ProcToFunc(appName,funcName)
            if procChannel.getAttribute('Port2')== AppName+':'+FuncName:
                PortName = procChannel.getAttribute('Port1')
                seperator=':'
                if not (PortName and seperator):
                    print 'Error '
                    return 
                #print PortName
                res = PortName.partition(seperator)
                appName = res[0]
                funcName = res[2]
                result[funcName]=self.ProcToFunc(appName,funcName)
        return result
        #Config App
    
    def convert(self):
        #Set Proc
        self.cluster.setAttribute('WorkPath',self.configDocRoot.getAttribute('baseDir'))
        
        for app in self.configAppList:
            #Create Proc Grp
            procGrp = self.xmldoc.createElement('ProcGroup')
            self.domroot.insertBefore(procGrp,self.hostTab)
            procGrp.setAttribute('Name',app.getAttribute('name'))
            procGrp.setAttribute('Layer','cluster')
            
            #Create Deploy
            procDeploy = self.xmldoc.createElement('DeloyGroup')
            procDeploy.setAttribute('Group',app.getAttribute('name'))
            procDeploy.setAttribute('Host',self.host.getAttribute('Name'))
            #self.clusterDeploy.appendChild(procDeploy)
            self.deployWorld.appendChild(procDeploy)
            baseDir=self.configDocRoot.getAttribute('baseDir')
            appDir = app.getAttribute('dir')
            #Append Proc
            procList = app.getElementsByTagName('Proc')
            appName  = app.getAttribute('name')
            for proc in procList:
                funcName=app.getAttribute('name')+'_'+proc.getAttribute('FuncName')
                app_funcName = proc.getAttribute('FuncName')
                #Create Proc Grp
                procElement = self.xmldoc.createElement('Proc')
                procElement.setAttribute('FuncName',funcName)
                procGrp.appendChild(procElement)
                                
                #Create Proc
                NewProc = copy.deepcopy(proc)
                
                NewProc.setAttribute('WorkPath',appDir+'/'+proc.getAttribute('WorkPath'))
                NewProc.setAttribute('FuncName',funcName)
                NewProc.setAttribute('FuncID',proc.getAttribute('FuncID'))
                self.cluster.appendChild(NewProc)
                self.chmodFile += '\nchmod 06755 '+baseDir+'/'+NewProc.getAttribute('WorkPath') +'/'+NewProc.getAttribute('ProcName')
                self.confFile += 'N=`echo $APPID | grep \'[0-9]\+.[0-9]\+.'+proc.getAttribute('FuncID')+'.[0-9]\+\'| wc -l`\n'

                self.confFile += 'if [ $N -eq 1 ];then\n'



                self.confFile += '    echo "$APPID creating config"\n'


                
                neigh = self.getNeighbour(appName,app_funcName)
                replaceStatment=''
                for name in neigh:
                    #print name
                    replaceStatment+= '|\n    sed "s:##'+name.upper()+'##:${'+neigh[name]+'}:g"'
                # I have to list all the address here.
                replaceStatment+='|\n    sed "s:##MYNAME##:${APPID}:g"'
                fileList=proc.getAttribute('ConfigFileName').split('#')
                for File in fileList:
                    self.confFile += '    cat ${LIBPATH}/'+File
                    replaceStatment += self.globalMacro
                    self.confFile += replaceStatment
                    self.confFile += ' > $OUTPUT_PATH/'+File+'\n'
                    self.confFile += '    cat $OUTPUT_PATH/'+File+'\n'



                self.confFile += 'fi\n'
                self.funcID = self.funcID+1
            #I haveto create channel infor for all the apps.
            channelList=app.getElementsByTagName('Channel')
            for Channel in channelList:
                #channel = copy.deepcopy(ochannel)
                procset1 = Channel.getAttribute('ProcSet1')
                procset2 = Channel.getAttribute('ProcSet2')
                Channel.setAttribute('ProcSet1',app.getAttribute('name')+'_'+procset1)
                Channel.setAttribute('ProcSet2',app.getAttribute('name')+'_'+procset2)
                self.tbusConf.appendChild(Channel)               

        #Config Inter App Communication
        procChannelList=self.configDocRoot.getElementsByTagName('IChannel')
        for procChannel in procChannelList:
            channel = self.xmldoc.createElement('Channel')
            channel.setAttribute('Mask','0.0.0.0')
            #I have to check whether the Port exists.
            port1 = procChannel.getAttribute('Port1')
            port2 = procChannel.getAttribute('Port2')
            ProcSet1 = ''
            ProcSet2 = ''
            res = self.checkAppPort(port1)
            if res:
                ProcSet1= res
            else:
                print port1
                print 'No such channel'
                exit

            res = self.checkAppPort(port2)
            if res:
                ProcSet2 = res
            else:
                print port2
                print 'No such channel'
                exit
            channel.setAttribute('ProcSet1',ProcSet1)
            channel.setAttribute('ProcSet2',ProcSet2)
            self.tbusConf.appendChild(channel)
            
            #I have to init a channel here. 
    def checkAppPort(self, PortName):
        seperator=':'
        if not (PortName and seperator):
            return 

        res = PortName.partition(seperator)
        appName = res[0]
        procName = res[2]
        #print appName
        appNode = self.getAppByName(appName)
        if not appNode:
            print 'No such app'
            return

        portList = appNode.getElementsByTagName('ExportBusChannel')

        for port in portList:
            if(procName == port.getAttribute('FuncName')):
                return appName+'_'+procName
        

if __name__ == '__main__':
    try:
        count = 0
        for t in sys.argv:
            count=count+1
# print count
        if count == 2:
            userid=sys.argv[1]
            confFile='applist.xml.tmp'
        else:
            userid='1'
            confFile='archtecture.xml'
        
        portbase = int(userid)*100 + 9000
        shmbase  = int(userid)*100 + 1001
        handler = AppListHandler(confFile,'tcmcenter.xml',portbase,shmbase,userid)
        handler.convert()
        #print handler.xmldoc.toprettyxml()
        tcmConf=handler.xmldoc.toprettyxml()
        count = 0
        for Port in handler.PortList:
            tcmConf=tcmConf.replace('##'+Port+'##',handler.PortList[Port])

        for Shm in handler.ShmList:
            tcmConf=tcmConf.replace('##'+Shm+'##',handler.ShmList[Shm])
            #print Shm+'=>'+handler.ShmList[Shm]
        #print tcmConf
        fd = open('tcmcenter.xml','w')
        fd.write(tcmConf)
        fd.close()

        fd = open('chmod.sh','w')
        fd.write(handler.chmodFile)
        fd.close()

        fd = open('shminit.sh','w')
        fd.write(handler.ShmScript)
        fd.close()


        fd = open('../../mmog_deps/tsf4g_install/apps/tcm/lib/config.sh','w')
        fd.write(handler.confFile)
        fd.close()

        #print self.initdb

        fd = open('dbinit.sh','w')
        fd.write(handler.initdb)
        fd.close()
        
        #print handler.confFile


        #print handler.chmodFile
    except:
        import traceback
        traceback.print_exc()
    finally:
        print 'Done'
