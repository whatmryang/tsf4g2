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
	testbase.checkAndDel('test_vec1.log')
	testbase.checkAndDel('test_vec2.log')

def startTappTlog():
	if testbase.isLinux():
		testbase.runCmd('./VecFilter')
	else:
		testbase.runCmd('VecFilter.exe')	
		
def main():
	print "Testing VecFilter"
	cleanup()
	startTappTlog()
	testbase.checkFileLines('test_vec1.log', 3)
	testbase.checkFileLines('test_vec2.log', 3)
	cleanup()
main()
