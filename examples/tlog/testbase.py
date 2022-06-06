import os
import os,sys
import sys
import subprocess

def getTsf4gHome():
	dir='CHANGES'
	maxLevel = 10
	while(maxLevel > 0):
		if os.path.isfile(os.path.abspath(dir)):
			return os.path.abspath(os.path.dirname(dir))
		maxLevel=maxLevel-1
		dir='../'+dir
	return ''
	
def rmIfExist(fileName):
	if os.path.isfile(fileName):
		os.remove(fileName)
		
def osinfo():
	return os.name

	
def exitCleanup():
	cleanup()
	exit(1)	
	
def checkFileLines(fileName,Lines):
	print "Checking file "+fileName
	f1 = open(fileName,'r')
	content = f1.read()
	sum = 0
	lines = content.split('\n')
	for item in lines:
		sum = sum+1
	f1.close()
	if Lines != sum:
		print "File check error "+fileName+" expected "+str(Lines)+" Found "+str(sum)
		print sum
		exit(-1)
	print "Check OK"
	return True
	
def checkAndDel(fileName):
	if os.path.isdir(fileName):
		os.rmdir(fileName)
	elif os.path.isfile(fileName):
		os.remove(fileName)
		
def runCmd(cmd):
	p = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
	res = not p.wait()
	child_stdout = p.stdout.read()
	child_stderr = p.stderr.read()    

	if not res:
		print "Failed to exec "+cmd
		print child_stdout
		print child_stderr
		exit(-1)
	return res  

def isLinux():
	return os.name == 'posix'