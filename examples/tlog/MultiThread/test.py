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
	testbase.checkAndDel('thread0_0.log')
	testbase.checkAndDel('thread1_0.log')
	testbase.checkAndDel('thread2_0.log')


def startTappTlog():
	if testbase.isLinux():
		testbase.runCmd('./start_MultiThread.sh')
	else:
		testbase.runCmd('start_MultiThread.bat')	
		
def main():
	print "Testing TappTlog"

	cleanup()
	startTappTlog()
	testbase.checkFileLines('thread0_0.log', 2)
	testbase.checkFileLines('thread1_0.log', 2)
	testbase.checkFileLines('thread2_0.log', 2)
	
	cleanup()
main()
