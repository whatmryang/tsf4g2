#!/usr/local/bin/python
print "Testing "
import os
import os,sys
import sys
import shutil
import time

def autoFindRootDir():
	dir='testbase.py'
	maxLevel = 10
	while(maxLevel > 5):
		if os.path.isfile(os.path.abspath(dir)):
			return os.path.abspath(os.path.dirname(dir)) + '/'
			maxLevel=maxLevel-1
		dir='../'+dir
	print "Error Failed to find tsf4g root"
	Exit(1)
path = autoFindRootDir()
#print path
sys.path.insert(0, os.path.abspath(path))
import testbase	

def cleanup():
	testbase.checkAndDel('example.log')
	testbase.checkAndDel('example.error')
	testbase.checkAndDel('example_tbus.log')
	testbase.checkAndDel('tbus.log')
	testbase.checkAndDel('example_rundata.log')

def startTappTlog():
	if testbase.isLinux():
		testbase.runCmd('./start_example.sh')
	else:
		testbase.runCmd('start_example.bat')	
		
def main():
	print "Testing TappReload"
	"""
	cleanup()
	shutil.copy('example_log_filter.xml','example_log.xml')
	startTappTlog()
	time.sleep(1)
	testbase.checkFileLines('example.log', 7)
	testbase.checkFileLines('example.error', 1)
	
	
	cleanup()
	"""
	
	os.system('./test.sh')
main()
