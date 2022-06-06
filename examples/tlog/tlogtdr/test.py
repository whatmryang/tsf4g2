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
	testbase.checkAndDel('tlogtest.log')
	testbase.checkAndDel('tlogtest.error')

def startTappTlog():
	if testbase.isLinux():
		testbase.runCmd('./start_tlogtest.sh')
	else:
		testbase.runCmd('start_tlogtest.bat')	
		
def main():
	print "Testing Tlogtdr"

	cleanup()
	startTappTlog()
	testbase.checkFileLines('tlogtest.log', 11)
	testbase.checkFileLines('tlogtest.error', 4)
	cleanup()
main()
