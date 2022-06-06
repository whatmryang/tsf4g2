

class buildConf():
	def __init__(self):
		if self.isLinux():
			print "Creating linux conf"
			self.LDLIBPATH=''
			self.CPPFLAGS=''
		
			if self.isLinux32():
				print "Creating linux32 conf"
				
			
			if self._isLinux64:
				print "Creating linux64 conf"
			
		
		if self._isWindows:
			if self._is32:
				print "Creating win32 conf"
				
			if self._is64:
				print "Creating win64 conf"
		
		
		return 
		
Export('buildConf')