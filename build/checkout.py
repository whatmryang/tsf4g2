import os
import os.path

from optparse import OptionParser






if   __name__  ==  "__main__":
	parser = OptionParser()
	parser.add_option("-m", "--module", dest="moduleName",
	                  help="Which module to build(tmart tmail tqos ...)", metavar="Module")

	(options, args) = parser.parse_args()
	#print options
	if not options.moduleName:
		print parser.get_usage()
		exit(-1)

	MODULE=options.moduleName

	os.system("svn co https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk/build")
	os.system("svn co https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk/dist")
	os.system("svn cat https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk/SConstruct > SConstruct")

	if not os.path.isdir("apps"):
		os.mkdir("apps")
	cmd = "svn co https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk/apps/%s apps/%s" %(MODULE,MODULE)
	os.system(cmd)

	comf ="""
res = \"%s\"
Return(\"res\")

	"""% (MODULE)

	open('ModuleConf','w').write(comf)
