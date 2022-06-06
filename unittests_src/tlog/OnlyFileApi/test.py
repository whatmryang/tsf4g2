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
	testbase.rmIfExist("test.log")
	
def exitCleanup():
	cleanup()
	exit(1)	
	

def main():
	cleanup()
	if testbase.isLinux():
		testbase.runCmd('./OnlyFileApi')
	else:
		testbase.runCmd('OnlyFileApi.exe')
	testbase.checkFileLines('test.log',2)
	cleanup()
main()