import os

class setup():
	def __init__(self):
		self.pDict = dict()
		self.pDict["Win32"] = dict()
		self.pDict["Win32"]["IP"]="10.2.146.39"
		self.pDict["Win32"]["ID"]="1.2.3.4"
		
		
		self.pDict["Linux32"] =dict()
		self.pDict["Linux32"]["IP"] = "10.6.221.149"
		self.pDict["Linux32"]["ID"]="1.2.3.5"
	
		self.pDict["Linux64"] =dict()
		self.pDict["Linux64"]["IP"] = "10.6.221.144"
		self.pDict["Linux64"]["ID"]="1.2.3.6"	



	def __isLinux(self):
		return os.name=='posix'

	def __is32(self):
		if self.__isLinux():
				[sysname,nodename,release,version,machine]=os.uname()
				if machine=='x86_64':
						return False
		return True

	def getName(self):
		if hasattr(self,'name'):
			return self.name
		if self.__isLinux() and self.__is32():
			self.name = "Linux32"
		
		if self.__isLinux() and not self.__is32():
			self.name = "Linux64"
		
		if not self.__isLinux():
			self.name = "Win32"
		return self.name
		
	def Generate(self):
		self.tbusd_relay = """
<RelayMnger>
  <RelayShmKey>12344</RelayShmKey>
		"""
		
		self.GCIM = """
<TbusGCIM>
	<GCIMShmKey>12345</GCIMShmKey>
"""
	
		for (cos) in self.pDict:
			conf = self.pDict[cos]
			if(cos == self.getName()):
				#Generate all the os info here.
				self.tbusd_xml = """<?xml version="1.0" encoding="gbk" standalone="yes" ?>
<!-- This file is for tbus relay application -->

<Tbusd version="2147483647">
  <RelayShmKey>12344</RelayShmKey> 
  <GCIMShmKey>12345</GCIMShmKey> 

  <Listen>tcp://%s:9883?reuse=1</Listen>

  <NeedAckPkg>1</NeedAckPkg>  
  <DisabledChannelTimeGap>10</DisabledChannelTimeGap>
  <RefreshTbusConfGap>1</RefreshTbusConfGap>
</Tbusd>""" % (conf['IP'])

				if(self.__isLinux()):
					open("tbustestserver/start.sh","w").write("./tbustestserver  --id=%s --conf-file tbustestserver.xml --bus-key=12345 --log-level 600 --log-file tbustestserver -D start"%(conf["ID"]))
					open("tbustestserver/stop.sh","w").write("./tbustestserver  --id=%s stop"%(conf["ID"]))
					open("tbustestserver/reload.sh","w").write("./tbustestserver  --id=%s reload"%(conf["ID"]))
					open("tbustestserver/control.sh","w").write("./tbustestserver  --id=%s control"%(conf["ID"]))
					os.system("chmod 777 tbustestserver/*.sh")
				else:
					open("tbustestserver/start.bat","w").write("tbustestserver  --id=%s --conf-file tbustestserver.xml --bus-key=12345 --log-level 600 --log-file tbustestserver -D start"%(conf["ID"]))
					open("tbustestserver/stop.bat","w").write("tbustestserver  --id=%s stop"%(conf["ID"]))
					open("tbustestserver/reload.bat","w").write("tbustestserver  --id=%s reload"%(conf["ID"]))
					open("tbustestserver/control.bat","w").write("tbustestserver  --id=%s control"%(conf["ID"]))				

			else:
				relay = """
<Relays>
    <Addr>%s</Addr>  <!-- ip format: xxx.xxx.xxx.xxx, primary key, should be unique in share memory -->
    <Priority>8</Priority>
    <MConn>tcp://%s:9883</MConn>  <!-- master connection information -->
    <Strategy>1</Strategy> <!-- strategy between master and slave connection, not implemented yet -->
    <Desc>Some description for this relay</Desc>
  </Relays> """ %(conf['ID'],conf['IP'])
				
				self.tbusd_relay += relay
				


				gcim = """
	<Channels>
		<Priority>8</Priority>
		<Address>%s</Address>
		<Address>%s</Address>
		<SendSize>10240</SendSize>
		<RecvSize>10240</RecvSize>
		<Desc>Some description for this route</Desc>
	</Channels>				
				""" % (self.pDict[self.getName()]['ID'],conf['ID'])
				
				self.GCIM += gcim
		self.tbusd_relay += """
</RelayMnger>"""		
		#print self.tbusd_xml
		#print self.tbusd_relay
		self.GCIM += """
</TbusGCIM>
		"""
		#print self.GCIM
		
		
		open("tbusd/trelay_mgr.xml","w").write(self.tbusd_relay)
		open("tbusd/tbus_mgr.xml","w").write(self.GCIM)
		open("tbusd/tbusd.xml","w").write(self.tbusd_xml)
		
		
		
		
t = setup()
t.Generate()
# Set up the machine 
