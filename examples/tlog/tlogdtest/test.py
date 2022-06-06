#!/usr/local/bin/python
print "Testing "
import os
import os,sys
import sys

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
	testbase.checkAndDel('TappTlog.log')
	testbase.checkAndDel('TappTlog.error')
	testbase.checkAndDel('TappTlog_rundata.log')
	testbase.checkAndDel('TappTlog_tbus.log')


def startTappTlog():
	if testbase.isLinux():
		testbase.runCmd('./start_TappTlog.sh')
	else:
		testbase.runCmd('start_TappTlog.bat')	
		
def main():
	print "Testing TappTlog"
	os.system('./test.sh')
main()
