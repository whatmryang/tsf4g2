#!/usr/local/bin/python
print "Testing hello world"
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
	testbase.rmIfExist("testfile.log")

def main():

	cleanup()
	if not testbase.isLinux():
		testbase.runCmd('HelloWorld.exe')		
	else:
		testbase.runCmd('./HelloWorld')
	testbase.checkFileLines('testfile.log',4)
	
main()
