import os
import shutil

OutPutDir = os.path.abspath('.')
print OutPutDir
Export('OutPutDir')

# Try to find the pybuild.py. And then build it.
# May I have some default value?.
def findBuildManager():
	dir='buildManager'
	maxLevel = 10
	while(maxLevel > 0):
		if os.path.isdir(os.path.abspath(dir)):
			return os.path.abspath(os.path.dirname(dir)) + '/'
		maxLevel=maxLevel-1
		dir='../'+dir
	print "Error Failed to find tsf4g root"
	Exit(1)

script = os.path.abspath(os.path.join(findBuildManager() , 'buildManager/build/pybuild.py'))
print script
SConscript(script)
Import('tsf4g_build')
SConscript("version.py")
Import('version')

class build(tsf4g_build,version):
	def __init__(self):
		version.__init__(self)
		tsf4g_build.__init__(self)

buildenv = build()
Export("buildenv")
buildenv.tsf4g_libs_dict['expat']=True
buildenv.tsf4g_libs_dict['scew']=True

SConscript("SConscript")

buildenv.Package()
