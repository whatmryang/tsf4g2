import os
import shutil
import subprocess
from xml.dom import minidom

def get_subdir_package(filename):
    tagpart = filename.split('_')
    ilen = len(tagpart)

    return tagpart[ilen-1]

def find_base_dir():
    filename = 'SConstruct'
    maxDepth = 10
    while maxDepth > 0:
        if (os.path.isfile(os.path.abspath(filename))):
            return str(os.path.abspath(os.path.dirname(filename)))
        maxDepth = maxDepth -1
        filename = '../'+filename
    print 'Can not find file Sconstruct'

def getSVNRev(srcDir):
    cmd = 'svn info --xml %s' %(srcDir)
    p2 = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
    res2 = p2.wait()
    doc = minidom.parse(p2.stdout)
    commit=doc.getElementsByTagName('commit')
    url = doc.getElementsByTagName('url')
    return commit[0].getAttribute('revision')

def build_package(module = 'base'):
    #version = tsf4gVersion()
    #major = version.versionDict['base']['major']
    #minor = version.versionDict['base']['minor']
    major = "2"
    minor = "4"
    basedir = find_base_dir()
    revision = getSVNRev(basedir)
    print 'revision:'+revision
    [sysname,nodename,release,version,machine] = os.uname()
    if ('posix' == os.name) and ('x86_64' != machine):
        arch = 'i686'
    else:
        arch= 'x86_64'
    strVersion = "%s_%s" % (major,minor)
    strSVNRev = "_%s" % (revision)
    strArch = "_%s" % (arch)
    strmodule = "tsf4g_%s" % (module)
    strtar = ".tar.gz"

    filelist = []
    rootlist = []

    print 'basedir:'+basedir
    for root,dirs,files in os.walk(basedir):
        for filename in files:
            rootlist += [root]
            filelist += [filename]

    for index in range(0,len(filelist)):
        filesrc = filelist[index]
        root = rootlist[index]
        if (-1 != filesrc.find(strtar)) and (-1 != filesrc.find(strArch)) and \
            (-1 != filesrc.find(strSVNRev)) and (-1 != filesrc.find(strmodule)):
            print 'base zip file:'+filesrc
            #os.system("tar -xvzf "+str(filesrc))
            #os.system("mv "+str(os.path.join(root,filesrc))+" "+str(os.path.join(root,filesrc+".backup")))
            cmd = 'tar -xvzf %s -C %s' % (str(os.path.join(root,filesrc)),root)
            p = subprocess.Popen(cmd,shell=True)
            res = p.wait()

            filepart = filesrc.split('.')
            file_no_suffix = filepart[0]
            print 'fileName:'+file_no_suffix
            release = get_subdir_package(file_no_suffix)
            soNamelist = []
            linkNamelist = []
            sofilelist = []
            for a_root,dirs,files in os.walk(str(os.path.join(root,file_no_suffix))+'/'+release+'/'+arch+'/lib'):
                print 'ln cmd directory:'+str(os.path.join(root,file_no_suffix))+'/'+release+'/'+arch+'/lib'
                for sofile in files:
                    sourceDir = os.path.dirname(str(sofile))
                    sourceBaseName = os.path.basename(str(sofile))
                    libName = str(sourceBaseName).split('.')
                    if (len(libName)>=5) and ('so'==libName[1]):
                        linkName = libName[0]+'.'+libName[1]
                        print sofile
                        soName = libName[0]+'.'+libName[1]+'.'+libName[2]
                        soNamelist += [soName]
                        linkNamelist += [linkName]
                        sofilelist += [sofile]
            currentDir = os.getcwd()
            os.chdir(str(os.path.join(root,file_no_suffix))+'/'+release+'/'+arch+'/lib')
            for i in range(0,len(soNamelist)):
                os.system("ln -s "+sofilelist[i]+" "+soNamelist[i])
                os.system("ln -s "+soNamelist[i]+" "+linkNamelist[i])
            os.chdir(currentDir)
            os.chdir(root)
            print 'cur dir:'+os.getcwd()
            print 'final gzip:'+filesrc
            os.system("tar -cvzf "+filesrc+" "+file_no_suffix)
            os.chdir(currentDir)
