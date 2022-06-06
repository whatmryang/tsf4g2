#!/usr/local/bin/python
import os
import shutil
import os.path



def getTsf4gHome():
	dir='pybuild'
	maxLevel = 10
	while(maxLevel > 0):
		if os.path.isfile(os.path.abspath(dir)):
			return os.path.abspath(os.path.dirname(dir)) + '/'
		maxLevel=maxLevel-1
		dir='../'+dir
	return ''
	

tsf4g_dir=getTsf4gHome()



def moveifexist(src,dest):
	if os.path.isfile(src):
		shutil.move(src,dest)

def tmvheader(tsf4gdir):
	cwd=os.path.join(tsf4gdir,'lib_src/tlog/')
	moveifexist(os.path.join(tsf4gdir,'tlogfilterdef.h'),os.path.join(cwd, '../../include/tlog/tlogfilterdef.h'))
	moveifexist(os.path.join(tsf4gdir,'tlogfiledef.h'), os.path.join(cwd,'../../include/tlog/tlogfiledef.h'))
	moveifexist(os.path.join(tsf4gdir,'tlognetdef.h'), os.path.join(cwd,'../../include/tlog/tlognetdef.h'))
	moveifexist(os.path.join(tsf4gdir,'tlogvecdef.h'), os.path.join(cwd,'../../include/tlog/tlogvecdef.h'))
	moveifexist(os.path.join(tsf4gdir,'tloganydef.h'),os.path.join(cwd,'../../include/tlog/tloganydef.h'))
	moveifexist(os.path.join(tsf4gdir,'tlogbindef.h'), os.path.join(cwd,'../../include/tlog/tlogbindef.h'))
	moveifexist(os.path.join(tsf4gdir,'tlog_priority_def.h'), os.path.join(cwd,'../../include/tlog/tlog_priority_def.h'))
	moveifexist(os.path.join(tsf4gdir,'tlog_category_def.h'), os.path.join(cwd,'../../include/tlog/tlog_category_def.h'))
	moveifexist(os.path.join(tsf4gdir,'tlogconf.h'), os.path.join(cwd,'../../include/tlog/tlogconf.h'))
	return 
tmvheader(tsf4g_dir)


