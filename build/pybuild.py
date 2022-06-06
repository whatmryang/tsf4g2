import os
import shutil
from optparse import OptionParser
import subprocess
from xml.dom import minidom
from time import strftime, localtime
from datetime import timedelta, date
import tarfile
import xml
#Import('OutPutDir')

SConscript('TSF4G_CONF.py')
SConscript('tsf4gversion.py')
#SConscript('../tversion.py')
#Import('version')
Import('tsf4gVersion')
Import('buildConf')

def test_modify(target,source,env):
    return target,source

def test(target,source,env):
    stdoutstr =""
    stderrstr=""
    returnv=""
    expectedRetv = env['TESTRETV']
    cmd = os.path.abspath(str(source[0]))

    print "Testing %s" % cmd
    p2 = subprocess.Popen(cmd,shell=True,cwd=os.path.dirname(str(source[0])),stdout=subprocess.PIPE,stderr= subprocess.PIPE)
    res2 = p2.wait()

    stdoutstr = p2.stdout.read()
    stderrstr = p2.stderr.read()

    impl = xml.dom.minidom.getDOMImplementation()
    dom = impl.createDocument(None, 'ttestres', None)
    res = dom.getElementsByTagName('ttestres')

    res[0].setAttribute('stdout',stdoutstr);
    res[0].setAttribute('stderr',stderrstr);
    res[0].setAttribute('cmd',cmd);

    if (stdoutstr.count('!!!FAILURES!!!')):
        print "Failure found"
        print stdoutstr
        print stderrstr
        Exit(1)

    if (stderrstr.count('!!!FAILURES!!!')):
        print "Failure found"
        print stdoutstr
        print stderrstr
        Exit(1)

    if (not res2==expectedRetv ):
        print "%s expected res is %d but got %d"%(cmd,expectedRetv,res2);
        print stdoutstr
        print stderrstr
        Exit(1)
    print "OK"

    #print dom.toprettyxml()
    open(str(target[0]),"w").write(dom.toprettyxml())
    return

def makeNsis_modify(target,source,env):
    return [env['TAGNAME']+".exe"],source

def makeNsis(target,source,env):
    ttarget= env['TAGNAME']
    print ttarget
    nsisScript="""
; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply
; prompts the user asking them where to install, and drops a copy of example1.nsi
; there.

;--------------------------------

; The name of the installer
Name "tsf4g"

; The file to write
OutFile \"%s\"

; The default installation directory
InstallDir $PROGRAMFILES\\Tencent\\tsf4g

; Request application privileges for Windows Vista
RequestExecutionLevel user

;--------------------------------

; Pages

Page directory
Page instfiles

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put file there
  File /r release\\*
SectionEnd ; end the section
    """% (str(ttarget)+".exe")
    print nsisScript
    open("tsf4g.nsi",'w').write(nsisScript)
    cmd = 'makensis tsf4g.nsi'
    p2 = subprocess.Popen(cmd,shell=True)
    res2 = p2.wait()

    return

def xml2cpp_modify_targets(target,source,env):
    targlist=list()
    for src in source:
        source_base_name=(str(src))
        [source_base_name,ext] = os.path.splitext(source_base_name)
        targlist.append(os.path.join(os.getcwd(),source_base_name+'.h'))
        targlist.append(os.path.join(os.getcwd(),source_base_name+'.cpp'))
	#target = os.path.abspath(os.getcwd())
    return target,source

def xml2h_modify_targets(target,source,env):
    targlist=list()
    for src in source:
        source_base_name=(str(src))
        [source_base_name,ext] = os.path.splitext(source_base_name)
        targlist.append(os.path.join(os.getcwd(),source_base_name+'.h'))
    return targlist,source

def soname_modify_function(target,source,env):
    target = list()
    for realName in source:
        sourceDir = os.path.dirname(str(realName))
        sourceBaseName = os.path.basename(str(realName))
        libName = str(sourceBaseName).split('.')
        linkName = libName[0]+'.'+libName[1]
        soName = libName[0]+'.'+libName[1]+'.'+libName[2]
        target += [os.path.join(sourceDir,soName)]
        target += [os.path.join(sourceDir,linkName)]
    return target,source

def soname_function(target,source,env):
    target = list()
    for realName in source:
        sourceDir = os.path.dirname(str(realName))
        sourceBaseName = os.path.basename(str(realName))
        libName = str(sourceBaseName).split('.')
        linkName = libName[0]+'.'+libName[1]
        soName = libName[0]+'.'+libName[1]+'.'+libName[2]
        target += [os.path.join(sourceDir,soName)]
        target += [os.path.join(sourceDir,linkName)]

        currentDir = os.getcwd()
        os.chdir(sourceDir)
        lnCmd = "ln -s "+sourceBaseName+" "+ soName
        os.system(lnCmd)
        lnCmd = "ln -s "+soName+" "+ linkName
        os.system(lnCmd)
        os.chdir(currentDir)
    return None


def runShell_function(target,source,env):
    shCmd = "sh "
    for shellName in source:
        shCmd = shCmd + str(shellName) + " "

    shellDir = os.path.dirname(str(source[0]))
    currDir = os.getcwd()
    os.chdir(shellDir)
    os.system(shCmd)
    os.chdir(currDir)
    return None


# I have to add the module info here.
def getSVNRev(srcDir):
    cmd = 'svn info --xml %s' %(srcDir)
    p2 = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
    res2 = p2.wait()
    doc = minidom.parse(p2.stdout)
    commit=doc.getElementsByTagName('commit')
    url = doc.getElementsByTagName('url')
    return commit[0].getAttribute('revision')


def getSVNRepo(DIR):
    cmd = 'svn info --xml %s' %(DIR)
    p2 = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
    res2 = p2.wait()
    doc = minidom.parse(p2.stdout)
    commit=doc.getElementsByTagName('commit')
    url = doc.getElementsByTagName('url')
    return url[0].childNodes[0].nodeValue
    #return commit[0].getAttribute('revision')

def svnTag(tagName,srcDir,tagDst,forceOverWrite=True,MODULE="base"):
    cmd = 'svn ci -m \"ci for tag %s\" ' % tagName
    p = subprocess.Popen(cmd,shell=True)
    res = p.wait()

    cmd = 'svn up'
    p = subprocess.Popen(cmd,shell=True)
    res = p.wait()

    cmd = 'svn list %s/%s/' % (tagDst,tagName)
    p = subprocess.Popen(cmd,shell=True)
    res = p.wait()
    if res == 0:
        print "The tag :[%s] already exists would you like to remove it(Y/N,defautl no)" %tagName
        if not forceOverWrite:
            s = raw_input("(Y or N):")
        else:
            s = 'Y'
        if s == 'Y':
            print "Deleting the old tag %s" % tagName
            cmd = 'svn del -m \"remove old tags\" %s/%s/' % (tagDst, tagName)
            #print cmd
            p1 = subprocess.Popen(cmd,shell=True)
            res1 = p1.wait()
            if res1 != 0:
                print "Failed to remove the dir"
                exit(1)
        else:
            exit(1)
    tag = '%s/%s/' % (tagDst,tagName)
    print "Creating tag dir"
    cmd='svn mkdir -m "tag for publish" %s/%s' % (tagDst,tagName)
    p1 = subprocess.Popen(cmd,shell=True)
    p1.wait()

    print "Label dir"
    cmd = 'svn copy -m "tag for publish" %s %s' %(getSVNRepo(srcDir),tag)
    p2 = subprocess.Popen(cmd,shell=True)
    res2 = p2.wait()
    if res2 != 0:
        print "Failed to copy tag dir"
        exit(1)

def generate_version(target,source,env):
    buildData = strftime("%Y%m%d",localtime())

    version = "#ifndef __VERSION_AUTO_H\n#define __VERSION_AUTO_H\n#define MAJOR %s\n#define MINOR %s\n#define REV %s\n#define BUILD %s\n#endif\n\n" % (env['major'],env['minor'],getSVNRev(env['TAGDIR']),buildData)
    for file in target:
        fd = open(str(file),'w')
        fd.write(version)
        fd.close()
    return None

def generate_version_modify(target,source,env):
    source = list()
    if len(target) == 0:
        #print "Generate target %s" % os.getcwd()
        return ['version.h'],source
    else:
        #print "Generate target header %s %s" % (os.getcwd(),target)
        t = list()
        for tar in target:
            t += [str(tar)+'.h']
        return t,source

class Tsf4gDirManager():
    def __init__(self):

        self.__InitOsInfo()
        self.__InitOptions()
        self.__InitCompilers()
        self.baseDir = self.__autoFindRootDir()
        self.__InitInstalls()
        self.__InitDirs()
        self.__buildModules = "base"

        return

    def genDoxyFile(self,src,html,chm,module="TSF4G"):
        tsf4g_dir = self.baseDir
        srcdir = os.path.abspath(os.path.join(tsf4g_dir,src))
        htmldir = os.path.abspath(os.path.join(tsf4g_dir,html))
        chmdir = os.path.abspath(os.path.join(tsf4g_dir,chm))

        doxyfile="""
# Doxyfile 1.5.1-p1

#---------------------------------------------------------------------------
# Project related configuration options
#---------------------------------------------------------------------------
PROJECT_NAME           = %s
PROJECT_NUMBER         =
OUTPUT_DIRECTORY       = %s
CREATE_SUBDIRS         = NO
OUTPUT_LANGUAGE        = Chinese
USE_WINDOWS_ENCODING   = YES
BRIEF_MEMBER_DESC      = YES
REPEAT_BRIEF           = YES
ABBREVIATE_BRIEF       =
ALWAYS_DETAILED_SEC    = NO
INLINE_INHERITED_MEMB  = NO
FULL_PATH_NAMES        = NO
STRIP_FROM_PATH        =
STRIP_FROM_INC_PATH    =
SHORT_NAMES            = NO
JAVADOC_AUTOBRIEF      = NO
MULTILINE_CPP_IS_BRIEF = NO
DETAILS_AT_TOP         = NO
INHERIT_DOCS           = YES
SEPARATE_MEMBER_PAGES  = NO
TAB_SIZE               = 8
ALIASES                =
OPTIMIZE_OUTPUT_FOR_C  = YES
OPTIMIZE_OUTPUT_JAVA   = NO
BUILTIN_STL_SUPPORT    = NO
DISTRIBUTE_GROUP_DOC   = NO
SUBGROUPING            = YES
#---------------------------------------------------------------------------
# Build related configuration options
#---------------------------------------------------------------------------
EXTRACT_ALL            = NO
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = NO
EXTRACT_LOCAL_CLASSES  = YES
EXTRACT_LOCAL_METHODS  = NO
HIDE_UNDOC_MEMBERS     = YES
HIDE_UNDOC_CLASSES     = YES
HIDE_FRIEND_COMPOUNDS  = NO
HIDE_IN_BODY_DOCS      = NO
INTERNAL_DOCS          = NO
CASE_SENSE_NAMES       = YES
HIDE_SCOPE_NAMES       = NO
SHOW_INCLUDE_FILES     = YES
INLINE_INFO            = YES
SORT_MEMBER_DOCS       = YES
SORT_BRIEF_DOCS        = NO
SORT_BY_SCOPE_NAME     = NO
GENERATE_TODOLIST      = YES
GENERATE_TESTLIST      = YES
GENERATE_BUGLIST       = YES
GENERATE_DEPRECATEDLIST= YES
ENABLED_SECTIONS       =
MAX_INITIALIZER_LINES  = 30
SHOW_USED_FILES        = YES
SHOW_DIRECTORIES       = NO
FILE_VERSION_FILTER    =
#---------------------------------------------------------------------------
# configuration options related to warning and progress messages
#---------------------------------------------------------------------------
QUIET                  = NO
WARNINGS               = YES
WARN_IF_UNDOCUMENTED   = YES
WARN_IF_DOC_ERROR      = YES
WARN_NO_PARAMDOC       = NO
WARN_FORMAT            = "$file:$line: $text"
WARN_LOGFILE           =
#---------------------------------------------------------------------------
# configuration options related to the input files
#---------------------------------------------------------------------------
INPUT                  = %s
FILE_PATTERNS          =
RECURSIVE              = YES
EXCLUDE                =
EXCLUDE_SYMLINKS       = NO
EXCLUDE_PATTERNS       =
EXAMPLE_PATH           =
EXAMPLE_PATTERNS       =
EXAMPLE_RECURSIVE      = NO
IMAGE_PATH             =
INPUT_FILTER           =
FILTER_PATTERNS        =
FILTER_SOURCE_FILES    = NO
#---------------------------------------------------------------------------
# configuration options related to source browsing
#---------------------------------------------------------------------------
SOURCE_BROWSER         = NO
INLINE_SOURCES         = NO
STRIP_CODE_COMMENTS    = YES
REFERENCED_BY_RELATION = NO
REFERENCES_RELATION    = NO
REFERENCES_LINK_SOURCE = YES
USE_HTAGS              = NO
VERBATIM_HEADERS       = NO
#---------------------------------------------------------------------------
# configuration options related to the alphabetical class index
#---------------------------------------------------------------------------
ALPHABETICAL_INDEX     = NO
COLS_IN_ALPHA_INDEX    = 5
IGNORE_PREFIX          =
#---------------------------------------------------------------------------
# configuration options related to the HTML output
#---------------------------------------------------------------------------
GENERATE_HTML          = YES
HTML_OUTPUT            = %s
HTML_FILE_EXTENSION    = .html
HTML_HEADER            =
HTML_FOOTER            =
HTML_STYLESHEET        =
HTML_ALIGN_MEMBERS     = YES
GENERATE_HTMLHELP      = YES
CHM_FILE               = %s
HHC_LOCATION           = "C:/Program Files/HTML Help Workshop/hhc.exe"
GENERATE_CHI           = NO
BINARY_TOC             = NO
TOC_EXPAND             = NO
DISABLE_INDEX          = NO
ENUM_VALUES_PER_LINE   = 4
GENERATE_TREEVIEW      = NO
TREEVIEW_WIDTH         = 250
#---------------------------------------------------------------------------
# configuration options related to the LaTeX output
#---------------------------------------------------------------------------
GENERATE_LATEX         = NO
LATEX_OUTPUT           = latex
LATEX_CMD_NAME         = latex
MAKEINDEX_CMD_NAME     = makeindex
COMPACT_LATEX          = NO
PAPER_TYPE             = a4wide
EXTRA_PACKAGES         =
LATEX_HEADER           =
PDF_HYPERLINKS         = YES
USE_PDFLATEX           = YES
LATEX_BATCHMODE        = NO
LATEX_HIDE_INDICES     = NO
#---------------------------------------------------------------------------
# configuration options related to the RTF output
#---------------------------------------------------------------------------
GENERATE_RTF           = NO
RTF_OUTPUT             = rtf
COMPACT_RTF            = NO
RTF_HYPERLINKS         = NO
RTF_STYLESHEET_FILE    =
RTF_EXTENSIONS_FILE    =
#---------------------------------------------------------------------------
# configuration options related to the man page output
#---------------------------------------------------------------------------
GENERATE_MAN           = NO
MAN_OUTPUT             = man
MAN_EXTENSION          = .3
MAN_LINKS              = NO
#---------------------------------------------------------------------------
# configuration options related to the XML output
#---------------------------------------------------------------------------
GENERATE_XML           = NO
XML_OUTPUT             = xml
XML_SCHEMA             =
XML_DTD                =
XML_PROGRAMLISTING     = YES
#---------------------------------------------------------------------------
# configuration options for the AutoGen Definitions output
#---------------------------------------------------------------------------
GENERATE_AUTOGEN_DEF   = NO
#---------------------------------------------------------------------------
# configuration options related to the Perl module output
#---------------------------------------------------------------------------
GENERATE_PERLMOD       = NO
PERLMOD_LATEX          = NO
PERLMOD_PRETTY         = YES
PERLMOD_MAKEVAR_PREFIX =
#---------------------------------------------------------------------------
# Configuration options related to the preprocessor
#---------------------------------------------------------------------------
ENABLE_PREPROCESSING   = YES
MACRO_EXPANSION        = NO
EXPAND_ONLY_PREDEF     = NO
SEARCH_INCLUDES        = YES
INCLUDE_PATH           =
INCLUDE_FILE_PATTERNS  =
PREDEFINED             =
EXPAND_AS_DEFINED      =
SKIP_FUNCTION_MACROS   = YES
#---------------------------------------------------------------------------
# Configuration::additions related to external references
#---------------------------------------------------------------------------
TAGFILES               =
GENERATE_TAGFILE       =
ALLEXTERNALS           = NO
EXTERNAL_GROUPS        = YES
PERL_PATH              = /usr/bin/perl
#---------------------------------------------------------------------------
# Configuration options related to the dot tool
#---------------------------------------------------------------------------
CLASS_DIAGRAMS         = YES
HIDE_UNDOC_RELATIONS   = YES
HAVE_DOT               = NO
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
GROUP_GRAPHS           = YES
UML_LOOK               = NO
TEMPLATE_RELATIONS     = NO
INCLUDE_GRAPH          = YES
INCLUDED_BY_GRAPH      = YES
CALL_GRAPH             = NO
CALLER_GRAPH           = NO
GRAPHICAL_HIERARCHY    = YES
DIRECTORY_GRAPH        = YES
DOT_IMAGE_FORMAT       = png
DOT_PATH               =
DOTFILE_DIRS           =
MAX_DOT_GRAPH_WIDTH    = 1024
MAX_DOT_GRAPH_HEIGHT   = 1024
MAX_DOT_GRAPH_DEPTH    = 0
DOT_TRANSPARENT        = NO
DOT_MULTI_TARGETS      = NO
GENERATE_LEGEND        = YES
DOT_CLEANUP            = YES
#---------------------------------------------------------------------------
# Configuration::additions related to the search engine
#---------------------------------------------------------------------------
SEARCHENGINE           = NO

    """ %(module,htmldir,srcdir,"",chmdir)

        #print doxyfile
        open('Doxyfile','w').write(doxyfile)
        cmd="doxygen"
        p2 = subprocess.Popen(cmd,shell=True)
        res2 = p2.wait()


    def SetBuildModule(self,MODULE):
        self.__buildModules = MODULE
        return

    def GetBuildModule(self):
        return self.__buildModules


    def __InitOptions(self):
        if self.isWin32():
            AddOption('--publish-docs',
            dest = 'publis_docs',
            type = 'int',
            nargs=1,
            action='store',
            metavar='BOOL',
            default=0,
            help='Do you want to publish docs,default 0, avaible options are 1 or 0')

            isPublish = GetOption('publis_docs')

            self._publish_docs = isPublish
        else:
            self._publish_docs = False
        return

    def isLinux(self):
        return self._isLinux

    def isWindows(self):
        return self._isWindows

    def is32(self):
        return self._is32

    def is64(self):
        return self._is64

    def isLinux32(self):
        return self._is32 and self._isLinux

    def isWin32(self):
        return self._is32 and self._isWindows

    def isWin64(self):
        return self._is64 and self._isWindows

    def getStaticLibName(self,a_libName):
        if self._isLinux:
            return 'lib'+a_libName+'.a'
        elif self._isWindows:
            if self._isDebug:
                return 'lib'+a_libName+'_d.lib'
            else:
                return 'lib'+a_libName+'.lib'
        else:
            print "Error getting static lib name"
            Exit(1)

    def getStaticLib(self,a_libName):
        return os.path.abspath(os.path.join(self.libBinPath,self.getStaticLibName(a_libName)))

    def getStaticLibPdbName(self,a_libName):
        if self._isWindows:
            if self._isDebug:
                pdbName = 'lib'+self.vcVariant+a_libName+'_d.pdb'
            else:
                pdbName = 'lib'+self.vcVariant+a_libName+'.pdb'
            if self.isWin32():
                return os.path.abspath(os.path.join(self.baseDir,"win32_pdb",pdbName))
            elif self.isWin64():
                return os.path.abspath(os.path.join(self.baseDir,"win64_pdb",pdbName))

        else:
            return ''

    def getStaticLibPdb(self,a_libName):
        return os.path.abspath(os.path.join(self.libBinPath,self.getStaticLibPdbName(a_libName)))

    def getStaticLibInstallPdb(self,a_libName):
        return os.path.abspath(os.path.join(self.libInstallPath ,self.getStaticLibPdbName(a_libName)))

    def getToolsName(self,a_toolsName):
        return a_toolsName+self._execSuffix

    def getTools(self,a_toolName):
        if self.dirDict.has_key('tools_path'):
            self.toolsBinPath = self.dirDict['tools_path']
        return os.path.abspath(os.path.join(self.toolsBinPath,self.getToolsName(a_toolName)))

    def getToolsPdbName(self,a_toolName):
        if self.isWin32():
            return os.path.join(self.baseDir,"win32_pdb",a_toolName+self.vcVariant+'.pdb')
        elif self.isWin64():
            return os.path.join(self.baseDir,"win64_pdb",a_toolName+self.vcVariant+'.pdb')
        else:
            return ""

    def getToolsPdb(self,a_toolName):
        if self.isWin32():
            return os.path.abspath(os.path.join(self.baseDir,"win32_pdb",self.getToolsPdbName(a_toolName)))
        elif self.isWin64():
            return os.path.abspath(os.path.join(self.baseDir,"win64_pdb",self.getToolsPdbName(a_toolName)))

    def getToolsInstallPdb(self,a_toolName):
        return os.path.abspath(os.path.join(self.toolsInstallPath,self.getToolsPdbName(a_toolName)))

    def getServicesName(self,a_process_name):
        return a_process_name+self._execSuffix

    def getServicesPath(self,a_service_name):
        return os.path.abspath(os.path.join(self.servicesBinPath,a_service_name+'_service'))

    def getServicesInstallPath(self,a_service_name):
        return os.path.abspath(os.path.join(self.servicesInstallPath,a_service_name+'_service'))

    def getServices(self,a_service_name,a_process_name):
        return os.path.abspath(os.path.join(self.servicesBinPath,a_service_name+'_service',self.getServicesName(a_process_name)))

    def getServicesInstall(self,a_service_name,a_process_name):
        return os.path.abspath(os.path.join(self.servicesInstallPath,a_service_name+'_service',self.getServicesName(a_process_name)))

    def getServicesPdbName(self,a_process_name):
        if self._isWindows:
            return a_process_name+self.vcVariant+'.pdb'
        else:
            return ""

    def getServicesPdb(self,a_service_name,a_process_name):
        if self.isWin32():
            return os.path.abspath(os.path.join(self.baseDir,"win32_pdb",self.getServicesPdbName(a_process_name)))
        elif self.isWin64():
            return os.path.abspath(os.path.join(self.baseDir,"win64_pdb",self.getServicesPdbName(a_process_name)))

    def getServicesInstallPdb(self,a_service_name,a_process_name):
        return os.path.abspath(os.path.join(self.servicesInstallPath,a_service_name+'_service',self.getServicesPdbName(a_process_name)))

    def getUnittestsName(self, a_process_name):
        return a_process_name+self._execSuffix

    def getUnittestsPath(self,a_unittests_name):
        return os.path.abspath(os.path.join(self.unittestsBinPath ,a_unittests_name))

    def getUnittestsInstallPath(self,a_unittests_name):
        return os.path.abspath(os.path.join(self.unittestsInstallPath,a_unittests_name))

    def getUnittests(self,a_unittests_name,a_process_name):
        return os.path.abspath(os.path.join(self.unittestsBinPath,a_unittests_name,self.getUnittestsName(a_process_name)))

    def getUnitestsInstall(self,a_unittests_name,a_process_name):
        return os.path.abspath(os.path.join(self.unittestsInstallPath,a_unittests_name,self.getUnittestsName(a_process_name)))

    def getUnittestsPdbName(self,a_process_name):
        if self._isWindows:
            return a_process_name+self.vcVariant+'.pdb'
        else:
            return ""

    def getUnittestsPdb(self,a_unittests_name,a_process_name):
        return os.path.abspath(os.path.join(self.unittestsBinPath,a_unittests_name,self.getUnittestsPdbName(a_process_name)))

    def getUnittestsInstallPdb(self,a_unittests_name,a_process_name):
        return os.path.abspath(os.path.join(self.unittestsInstallPath,a_unittests_name,self.getUnittestsPdbName(a_process_name)))

    def getExamplesName(self, a_process_name):
        return a_process_name+self._execSuffix

    def getExamplesPath(self,a_examples_name):
        return os.path.abspath(os.path.join(self.examplesBinPath ,a_examples_name))

    def getExamplesInstallPath(self,a_examples_name):
        return os.path.abspath(os.path.join(self.examplesInstallPath,a_examples_name))

    def getExamples(self,a_examples_name,a_process_name):
        return os.path.abspath(os.path.join(self.examplesBinPath,a_examples_name,self.getExamplesName(a_process_name)))

    def getExamplesInstall(self,a_examples_name,a_process_name):
        return os.path.abspath(os.path.join(self.examplesInstallPath,a_examples_name,self.getExamplesName(a_process_name)))

    def getExamplesPdbName(self,a_process_name):
        if self._isWindows:
            return a_process_name+self.vcVariant+'.pdb'
        else:
            return ""

    def getExamplesPdb(self,a_examples_name,a_process_name):
        return os.path.abspath(os.path.join(self.examplesBinPath,a_examples_name,self.getExamplesPdbName(a_process_name)))

    def getExamplesInstallPdb(self,a_examples_name,a_process_name):
        return os.path.abspath(os.path.join(self.examplesInstallPath,a_examples_name,self.getExamplesPdbName(a_process_name)))


    #Internal Implementation
    def __InitCompilers(self):
        #This should set the isDebug isRelease vcversion and Other informations.
        AddOption('--enable-debug',
        dest = 'enable_debug',
        type = 'int',
        nargs=1,
        action='store',
        metavar='BOOL',
        default=1,
        help='Do you want to buidl the debug version default is 1, avaible options are 1 or 0')

        isdebug = GetOption('enable_debug')

        self._isDebug = True

        if isdebug == 1:
                self._isDebug = True
        else:
                self._isDebug = False

        self.isRelease = not self._isDebug


        AddOption('--builddeps',
        dest = 'build_deps',
        type = 'int',
        nargs=1,
        action='store',
        metavar='BOOL',
        default=0,
        help='Do you want to build the depends lib 1 or 0')

        buildDeps = GetOption('build_deps')

        self._buildDeps = False;

        if buildDeps == 1:
                self._buildDeps = True
        else:
                self._buildDeps = False


        #Vc version
        if self.isWindows:
            AddOption('--vcversion',
                    dest = 'vcversion',
                    type = 'string',
                    nargs=1,
                    action='store',
                    metavar='STRING',
                    default='vc8',
                    help='specify vc version to build avaible values are vc6 vc7 vc8 vc9')

            vcversion = GetOption('vcversion')
            isdebug = self._isDebug

            if vcversion=='vc8':
                self.vcVariant = vcversion
                self.vcVersion = '8.0'
            elif vcversion == 'vc9':
                self.vcVariant = vcversion
                self.vcVersion = '9.0'
            elif vcversion == 'vc7':
                self.vcVariant = vcversion
                self.vcVersion = '7.1'
            elif vcversion == 'vc6':
                self.vcVariant = vcversion
                self.vcVersion = '6.0'
            else:
                print 'Unknown vc version '
                Exit(1)

        if self._isLinux:
            self._execSuffix = ""
        else:
            self._execSuffix = ".exe"


        AddOption('--vcLinkType',
                dest = 'linktype',
                type = 'string',
                nargs=1,
                action='store',
                metavar='STRING',
                default='MD',
                help='specify Link method ,avaible values are MD MT,default is MD')


        linktype = GetOption('linktype')
        self._linkType=linktype

        return
    def __autoFindRootDir(self):
        #print OutPutDir
        #return OutPutDir
        dir='SConstruct'
        maxLevel = 10
        while(maxLevel > 0):
            if os.path.isfile(os.path.abspath(dir)):
                return os.path.abspath(os.path.dirname(dir)) + '/'
                maxLevel=maxLevel-1
                dir='../'+dir
        print "Error Failed to find tsf4g root"
        Exit(1)

    def __isLinux(self):
        return os.name=='posix'

    def __is32(self):
        if self.__isLinux():
                [sysname,nodename,release,version,machine]=os.uname()
                if machine=='x86_64':
                    return False
                else:
                    return True;
        else:

            AddOption('--targetArch',
                    dest = 'targetarch',
                    type = 'string',
                    nargs=1,
                    action='store',
                    metavar='STRING',
                    default='i386',
                    help='specify machine architecture, avaible values are x86, i386(for 32bits); amd64, emt64, x86_64 (for 64 bits); and ia64 (Itanium). default is i386')

            self.targetArch = GetOption('targetarch')

            if self.targetArch=="x86" or self.targetArch=="i386":
                return True;
            else:
                return False;

    def __InitOsInfo(self):
        self._isLinux = self.__isLinux()
        self._isWindows = not self._isLinux

        self._is32 = self.__is32()
        self._is64 = not self._is32

        self._isLinux32 = self._is32 and self._isLinux
        self._isLinux64 = self._is64 and self._isLinux

        self._isWin32 = self._is32 and self._isWindows
        self._isWin64 = self._is64 and self._isWindows

        return

    def __InitDirs(self):
        self.baseDir = self.__autoFindRootDir()
        if self._isLinux32:
            print "Linux32"
            self.__Linux32InitDirs()
        elif self._isLinux64:
            print "Linux64"
            self.__Linux64InitDirs()
        elif self._isWin32:
            print "Windows32"
            self.__Win32InitDirs()
        elif self._isWin64:
            print "Windows64"
            self.__Win64InitDirs()
        else:
            print "Error Unknown OS detected"
            Exit(1)

        return

    def __absPath(self,path):
        return os.path.abspath(os.path.join(self.baseDir,path))

    def __Linux32InitDirs(self):
        print "Initing linux 32"
        self.libPath = self.__absPath("lib_src")
        self.libBinPath = self.__absPath("lib")
        self.libInstallPath = self.__absPath(self.getInstallBase()+"/"+"lib")

        self.toolsPath = self.__absPath("tools_src")
        self.toolsBinPath = self.__absPath('tools')
        self.toolsInstallPath = self.__absPath(self.getInstallBase()+"/"+'tools')

        self.servicesPath = self.__absPath("services_src")
        self.servicesBinPath = self.__absPath("services")
        self.servicesInstallPath = self.__absPath(self.getInstallBase()+"/"+"services")

        self.unittestsPath = self.__absPath("unittests_src")
        self.unittestsBinPath = self.__absPath("unittests")
        self.unittestsInstallPath = self.__absPath(self.getInstallBase()+"/"+"unittests")

        self.examplesPath = self.__absPath("examples")
        self.examplesBinPath = self.__absPath("examples")
        self.examplesInstallPath = self.__absPath(self.getInstallBase()+"/"+"examples")

        return

    def __Linux64InitDirs(self):

        print "Initing linux 64"
        self.libPath = self.__absPath("lib_src")
        self.libBinPath = self.__absPath("lib")
        self.libInstallPath = self.__absPath(self.getInstallBase()+"/"+"lib")

        self.toolsPath = self.__absPath("tools_src")
        self.toolsBinPath = self.__absPath('tools')
        self.toolsInstallPath = self.__absPath(self.getInstallBase()+"/"+'tools')

        self.servicesPath = self.__absPath("services_src")
        self.servicesBinPath = self.__absPath("services")
        self.servicesInstallPath = self.__absPath(self.getInstallBase()+"/"+"services")

        self.unittestsPath = self.__absPath("unittests_src")
        self.unittestsBinPath = self.__absPath("unittests64")
        self.unittestsInstallPath = self.__absPath(self.getInstallBase()+"/"+"unittests")

        self.examplesPath = self.__absPath("examples")
        self.examplesBinPath = self.__absPath("examples")
        self.examplesInstallPath = self.__absPath(self.getInstallBase()+"/"+"examples")
        return

    def __Win32InitDirs(self):
        print "Initing win32 libPath"

        if self._isDebug:
            dbgVariant = "debug"
        else:
            dbgVariant = "release"

        self.libPath = self.__absPath("lib_src")
        self.libBinPath = self.__absPath("win32_lib/"+self.vcVariant+'/')
        self.libInstallPath = self.__absPath(self.getInstallBase()+"/"+"lib/")

        self.toolsPath = self.__absPath("tools_src")
        self.toolsBinPath = self.__absPath('tools')
        self.toolsInstallPath = self.__absPath(self.getInstallBase()+"/tools/")

        self.servicesPath = self.__absPath("services_src")
        self.servicesBinPath = self.__absPath("services")
        self.servicesInstallPath = self.__absPath(self.getInstallBase()+"/services/")

        self.unittestsPath = self.__absPath("unittests_src")
        self.unittestsBinPath = self.__absPath("unittests")
        self.unittestsInstallPath = self.__absPath(self.getInstallBase()+"/unittests/")

        self.examplesPath = self.__absPath("examples")
        self.examplesBinPath = self.__absPath("examples")
        self.examplesInstallPath = self.__absPath(self.getInstallBase()+"/examples/")
        return

    def __Win64InitDirs(self):
        print "Initing win64 libPath"

        if self._isDebug:
            dbgVariant = "debug"
        else:
            dbgVariant = "release"

        self.libPath = self.__absPath("lib_src")
        self.libBinPath = self.__absPath("win64_lib/"+self.vcVariant+'/')
        self.libInstallPath = self.__absPath(self.getInstallBase()+"/"+"lib/")

        self.toolsPath = self.__absPath("tools_src")
        self.toolsBinPath = self.__absPath('tools')
        self.toolsInstallPath = self.__absPath(self.getInstallBase()+"/tools/")

        self.servicesPath = self.__absPath("services_src")
        self.servicesBinPath = self.__absPath("services")
        self.servicesInstallPath = self.__absPath(self.getInstallBase()+"/services/")

        self.unittestsPath = self.__absPath("unittests_src")
        self.unittestsBinPath = self.__absPath("unittests")
        self.unittestsInstallPath = self.__absPath(self.getInstallBase()+"/unittests/")

        self.examplesPath = self.__absPath("examples")
        self.examplesBinPath = self.__absPath("examples")
        self.examplesInstallPath = self.__absPath(self.getInstallBase()+"/examples/")
        return

    def getInstallBase(self):
        if self._isLinux32:
            path = 'i686'
        elif self._isLinux64:
            path = 'x86_64'
        elif self._isWin32:
            path = 'win32'+self.vcVariant+self._linkType
        else:
            path = 'win64'+self.vcVariant+self._linkType

        dirName = '%s' %(path)
        return os.path.abspath(os.path.join(self.baseDir,'release',dirName))

    def getCompiler(self):
        if self._isLinux32:
            path = 'i686'
        elif self._isLinux64:
            path = 'x86_64'
        elif self._isWin32:
            path = 'win32'+self.vcVariant+self._linkType
        else:
            path = 'win64'+self.vcVariant+self._linkType

        return path

    def isInstall(self,moduleName):
        if not self._isInstall:
            return False

        if not self.__installModules.has_key(moduleName):
            return False

        return True

    def isPublish(self,moduleName):
        return self.isInstall(moduleName)

    def __getAppInstallPath(self,appName):
        return os.path.abspath(os.path.join(self._installBase,'apps',appName))

    def getAppInstallBinPath(self,appName):
        return os.path.abspath(os.path.join(self._installBase,'apps',appName,'bin'))

    def getAppInstallcfgPath(self,appName):
        return os.path.abspath(os.path.join(self._installBase,'apps',appName,'cfg'))

    def getAppInstallexamplesPath(self,appName):
        return os.path.abspath(os.path.join(self._installBase,'apps',appName,'examples'))

    def InstallDir(self,dest,sourceDir):
        for root, dirs, files in os.walk(sourceDir):
            if '.svn' in dirs:
                dirs.remove('.svn')  # don't visit CVS directories

            for file in files:
                src = os.path.join(root,file)
                print "InstallIng File %s" %(src)

                if file != 'SConscript':
                    InstallAs(os.path.join(str(dest),os.path.relpath(str(src),sourceDir)),str(src))

    #Install the examples.
    def InstallAppExamples(self,appName):
        #Source Path
        srcPath = os.path.abspath(os.path.join(self.baseDir,'apps',appName,'examples'))
        destPath = self.getAppInstallexamplesPath(appName)

    def __InitInstalls(self):

        #The enable install option
        AddOption('--tsf4g-publish',
        dest = 'tsf4g_install',
        type = 'int',
        nargs=1,
        action='store',
        metavar='BOOL',
        default=0,
        help='Do you want to install the result default is 0, avaible options are 1 or 0')

        tsf4g_install = GetOption('tsf4g_install')

        self._isInstall = (tsf4g_install == 1)

        if self._isInstall:
            tag = 'svn ci -m \"ci publish\"'
            p = subprocess.Popen(tag,shell=True)
            res = p.wait()

            tag = 'svn up'
            p = subprocess.Popen(tag,shell=True)
            res = p.wait()


        AddOption('--publish-webfiles',
        dest = 'publish_webFiles',
        type = 'int',
        nargs=1,
        action='store',
        metavar='BOOL',
        default=0,
        help='would you like to publish web files 1 or 0,default 0')

        publish_webFiles = GetOption('publish_webFiles')
        self._publish_webfiles = publish_webFiles

        AddOption('--clean-release',
        dest = 'cleanRelease',
        type = 'int',
        nargs=1,
        action='store',
        metavar='BOOL',
        default=1,
        help='would you like to clean release dir 1 or 0,default 1')

        cleanRelease = GetOption('cleanRelease')
        self._clean_release = cleanRelease

        #Which app to install or just the base to install
        AddOption('--publish-modules',
        dest = 'install_modules',
        type = 'string',
        nargs=1,
        action='store',
        metavar='Modules',
        default='none',
        help='Which module would you like to install\n\tdefault value: base')

        modules = GetOption('install_modules')

        if modules == 'none':
            #The default module is base, Or I will just dist the libs.
            modules='docs base tools services examples benchmarks'
            self.InstallModuleName='base'
        else:
            self.InstallModuleName = Split(modules)[0]

        self.__installModules = dict()
        for module in Split(modules):
            self.__installModules[module] = True

        #if not self.isInstall('baselibs'):
        #    self.__installModules['baselibs'] = True

        if self._isInstall:
            self._installBase = self.getInstallBase()
            self.getAppInstallPath = self.__getAppInstallPath

        #Parse the install options.
    def getLinkMethod(self):
        #This should return MT MTD MD MDD
        if self.isWindows:
            linktype = self._linkType

            if self._isDebug:
                return '/'+linktype+'d'
            else:
                return '/'+linktype

SConscript("tapp.py")
Import("tappBuilder")

class tsf4g_build(Tsf4gDirManager,buildConf,tsf4gVersion,tappBuilder):
    def __init__(self):
        Tsf4gDirManager.__init__(self)
        tsf4gVersion.__init__(self)
        buildConf.__init__(self)
    #    version.__init__(self)
        if os.path.isfile("ModuleConf"):
            self.SetBuildModule(SConscript("ModuleConf"))
        else:
            self.SetBuildModule("base")
        (dir,tag) = self.dirDict['tag'][0]
        self._svrRev = getSVNRev(dir)
        if self._isWindows:
            if self._isDebug:
                debug = "debug"
            else:
                debug = "release"

            if self._isWin32:
                print "Building win32 %s %s" % (self.vcVersion, debug)
            else:
                print "Building win64 %s %s" % (self.vcVersion, debug)

            self.env = Environment(tools=['default','packaging'],MSVC_VERSION=self.vcVersion, TARGET_ARCH=self.targetArch)
            self.env.Append(CPPPATH=self.__getIncludePath())
            self.env.Append(CPPDEFINES=self.__getBuildDefines())
            self.env.Append(LIBPATH=self.libBinPath)
            if self._isWin32:
                if self._isDebug:
                    self.env.Append(LIBPATH=Split(os.path.abspath(os.path.join(self.baseDir,'deps/mysql/lib/debug'))))
                else:
                    self.env.Append(LIBPATH=Split(os.path.abspath(os.path.join(self.baseDir,'deps/mysql/lib/opt'))))
            else:
                if self._isDebug:
                    self.env.Append(LIBPATH=Split(os.path.abspath(os.path.join(self.baseDir,'deps/mysql/lib64/debug'))))
                else:
                    self.env.Append(LIBPATH=Split(os.path.abspath(os.path.join(self.baseDir,'deps/mysql/lib64/opt'))))


            if self.dirDict.has_key('libPath'):
                self.env.Append(LIBPATH=self.dirDict['libPath'])
            self.env.Append(LINKFLAGS='WinMm.lib kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib ole32.lib odbc32.lib odbccp32.lib ws2_32.lib')

            if self.vcVariant == 'vc8' or self.vcVariant=='vc9':
                self.env.AppendUnique(LINKCOM  = [self.env['LINKCOM'], 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;1'])

            if self._isDebug:
                if self._isWin32:
                    self.env.Append(CCFLAGS=Split('/Od /Gm /EHsc  /nologo /ZI /Gd /W1'))
                else:
                    self.env.Append(CCFLAGS=Split('/Od /Gm /EHsc  /nologo /Zi /Gd /W1'))
            else:
                self.env.Append(CCFLAGS=Split('/O2  /EHsc /nologo /Zi /Gd /W1'))

            self.env.Append(CCFLAGS = Split(self.__getLinkMethod()))
        else:
            if self._isDebug:
                debug = "debug"
            else:
                debug = "release"

            if self._is32:
                arch = 'i686'
            else:
                arch = 'x86_64'
            print "Building linux %s %s" % (arch, debug)

            self.env = Environment(tools=["default",'packaging'])

            self.env.Append(CPPPATH=self.__getIncludePath())
            self.env.Append(CPPDEFINES=self.__getBuildDefines())
            self.env.Append(LIBPATH=self.libBinPath)

            if self.dirDict.has_key('libPath'):
                self.env.Append(LIBPATH=self.dirDict['libPath'])

            if self._is64:
                self.env.Append(CCFLAGS=['-fPIC', '-m64'])
            else:

                self.env.Append(CCFLAGS=['-m32','-fPIC'])

            AddOption('--no-werr',
                dest = 'no_werr',
                type = 'string',
                nargs=1,
                action='store',
                metavar='STRING',
                default='0',
                help='Without werr 0 or 1')

            no_werr = GetOption('no_werr')

            if '0'== no_werr:
                werr = " -Werror "
            else:
                werr = ""

            if self._isDebug:
                self.env.Append(CCFLAGS=Split('-g -Wall  -pg '+werr))
                self.env.Append(CFLAGS=Split('-g -Wall  -Werror-implicit-function-declaration'+werr))
                self.env.Append(LINKFLAGS=" -pg ")
                #self.env.Append(CFLAGS=Split('-g -pg -Wall -Werror -fprofile-arcs -ftest-coverage -Werror-implicit-function-declaration'))
            else:
                self.env.Append(CCFLAGS=Split('-g -O2 -Wall   -DNDEBUG -fno-strict-aliasing '+werr))
                self.env.Append(CFLAGS=Split('-g -O2 -Wall   -DNDEBUG -fno-strict-aliasing -Werror-implicit-function-declaration'+werr))
                self.env.Append(LINKFLAGS=" -O2 ")
            if os.environ.has_key('TERM'):
                self.env.Append(ENV = {'PATH' : os.environ['PATH'],
                             'TERM' : os.environ['TERM'],
                             'HOME' : os.environ['HOME']})

        self.__initShellRun()
        self.__initTdrTools()
        self.__initSoNameBuilder()
        self.__init_version_header()

        if self._isInstall:
            self.__makeRelease()


        self.tsf4g_libs_dict = dict()

        if self.isInstall and os.path.isdir('release') and self._clean_release:
            shutil.rmtree('release',ignore_errors=True)

        return

    def __getLinkMethod(self):
        return self.getLinkMethod()

    def __makeRelease(self):
        return

    def SConscript(self,script,IgnorMissing=True):
        if IgnorMissing and not os.path.isfile(script):
            return
        return SConscript(script)

    def buildStaticObj(self,src,**argv):
        target="tsf4g"
        argv = self.__parseOption(target,src,**argv)
        if self._isWindows:
            pdbfile = self.getStaticLibPdb(target)
            argv['CCPDBFLAGS']=' /FD /Fd'+pdbfile
        targetName = target

        lib = self.env.SharedObject(src,**argv)
        return lib

    def buildStaticLib(self,target,src,**argv):
        argv = self.__parseOption(target,src,**argv)
        if self._isWindows:
            pdbfile = self.getStaticLibPdb(target)
            argv['CCPDBFLAGS']=' /FD /Fd'+pdbfile
        targetName = target

        lib = self.env.StaticLibrary(self.getStaticLibName(target),src,**argv)
        lib = Install(self.libBinPath,lib)

        #print "isInstall ?? %s" % argv["MODULE"]
        if self.isInstall(argv["MODULE"]):
            #print "Install %s->%s" %(lib,self.libInstallPath )
            Install(self.libInstallPath ,lib)

        #if self.isInstall(argv["MODULE"]) and self._isWindows:
            #InstallAs(str(self.getStaticLibInstallPdb(targetName)),str(self.getStaticLibPdb(targetName)))
        self.tsf4g_libs_dict[target] = True
        return
    def buildSharedLib(self,a_libName,a_src,**argv):
        if self._isLinux:
            argv = self.__parseOption(a_libName,a_src,**argv)
            mod = argv.get('MODULE','None')

            a_major = self.versionDict.get(mod,dict()).get('major',self._a_major)
            a_minor = self.versionDict.get(mod,dict()).get('minor',self._a_minor)
            a_build = self._svrRev;

            OutSoName='lib'+a_libName+'.so.'+a_major
            OutLibRealName='lib'+a_libName+'.so.'+a_major+'.'+a_minor+'.'+a_build
            OutLibName='lib'+a_libName+'.so'
            OutLibPath=self.libBinPath
            CopyDest=os.path.abspath(os.path.join(OutLibPath,OutLibRealName))
            CopySrc=OutLibName

            env=self.env.Clone()
            #env.Append(LIBFLAGS = '-z defs -Wl,-Bstatic -lexpat -lscew  -Wl,-Bdynamic -lgcov -Wl,-soname,'+OutSoName)
            libflag = '-Wl,-Bstatic -lexpat -lscew  -Wl,-Bdynamic -lgcov -Wl,-soname,'+OutSoName
            if argv.has_key('_LIBFLAGS'):
                argv['_LIBFLAGS'] = argv['_LIBFLAGS'] + libflag
            else:
                argv['_LIBFLAGS'] = libflag
            result=env.SharedLibrary(a_libName,a_src,**argv)

            if argv.has_key('a_tsf4g_static_lib'):
                for key in Split(argv['a_tsf4g_static_lib']):
                    Depends(result,self.getStaticLib(key))

            env.InstallAs(CopyDest,result)
            env.soname(source=CopyDest)
            #print self.env.Dump()
            if self.isInstall(argv['MODULE']):
                res = self.__Install(self.libInstallPath,CopyDest)
                env.soname(source=res)
        else: # Windows
            argv = self.__parseOption(a_libName,a_src,**argv)
            OutLibPath=self.libBinPath
            OutLibRealName='lib'+a_libName+'.dll'
            CopyDest=os.path.abspath(os.path.join(OutLibPath,OutLibRealName))
            env=self.env.Clone()
            if self.isWin32():
                pdbfile = os.path.abspath(os.path.join(self.baseDir,"win32_pdb",'lib'+a_libName+self.vcVariant+'.pdb'))
            else:
                pdbfile = os.path.abspath(os.path.join(self.baseDir,"win64_pdb",'lib'+a_libName+self.vcVariant+'.pdb'))
            argv['CCPDBFLAGS']=' /Fd'+pdbfile
            pdbRele = os.path.relpath(pdbfile,self.baseDir)
            argv['LINKFLAGS'] = self.env['LINKFLAGS']+(' /DEBUG /PDB:\"'+str(pdbRele)+'\"')

            result=env.SharedLibrary("lib"+a_libName,a_src,**argv)

            if argv.has_key('a_tsf4g_static_lib'):
                for key in Split(argv['a_tsf4g_static_lib']):
                    Depends(result,self.getStaticLib(key))

            env.InstallAs(CopyDest,OutLibRealName)
            if self.isInstall(argv['MODULE']):
                res = self.__Install(self.libInstallPath,CopyDest)
        return

    def __sonamecmd(self,a_libName,a_major,a_minor,a_build):
        libdir=self.libBinPath
        SharedLibName='lib'+a_libName+'.so'
        MiddleLibName=SharedLibName+'.'+a_major
        RealLibName=MiddleLibName+'.'+a_minor+'.'+a_build
        result =  'cd '+os.path.abspath(libdir)+';'
        result = result + 'rm -f '+ MiddleLibName +';'
        result = result + 'rm -f  '+ SharedLibName +';'
        result = result + 'ln -s '+ RealLibName + ' '+MiddleLibName+';'
        result = result + 'ln -s '+ MiddleLibName + ' '+SharedLibName+';'
        return result

    def buildExec(self,a_exe_name,src,**argv):
        argv = self.__parseOption(a_exe_name,src,**argv)

        if self._isWindows:
            if self.isWin32():
                pdbfile = os.path.abspath(os.path.join(self.baseDir,"win32_pdb",a_exe_name+self.vcVariant+'.pdb'))
            elif self.isWin64():
                pdbfile = os.path.abspath(os.path.join(self.baseDir,"win64_pdb",a_exe_name+self.vcVariant+'.pdb'))
            argv['CCPDBFLAGS']=' /Fd'+pdbfile
            pdbRele = os.path.relpath(pdbfile,self.baseDir)
            argv['LINKFLAGS'] = argv.get('LINKFLAGS',list())+self.env['LINKFLAGS']+(' /DEBUG /PDB:\"'+str(pdbRele)+'\"')

        target = self.env.Program(a_exe_name,src,**argv)

        if argv.has_key('a_tsf4g_static_lib'):
            for key in Split(argv['a_tsf4g_static_lib']):
                Depends(target,self.getStaticLib(key))
        return

    def buildTools(self,tool_name,src,**argv):
        argv = self.__parseOption(tool_name,src,**argv)

        if self._isWindows:
            cppdbflags = self.getToolsPdb(tool_name)
            argv['CCPDBFLAGS'] = '/Fd'+cppdbflags
            pdbRele = os.path.relpath(cppdbflags,self.baseDir)
            argv['LINKFLAGS'] = self.env['LINKFLAGS']+(' /DEBUG /PDB:\"'+str(pdbRele)+'\"')

        target = self.env.Program(tool_name,src,**argv)

        if argv.has_key('a_tsf4g_static_lib'):
            for key in Split(argv['a_tsf4g_static_lib']):
                Depends(target,self.getStaticLib(key))

        self.env.Install(self.toolsBinPath,target)

        if self._isLinux and self.isInstall("base"):
            self.__Install(self.toolsInstallPath,target)

        if self.isInstall("base") and self._isWindows:#and self.vcVariant== 'vc8' and (not self._isDebug):
            Install(self.toolsInstallPath,target)
            #InstallAs(self.getToolsInstallPdb(tool_name),self.getToolsPdb(tool_name))
        return

    def buildServices(self,a_service_name,a_process_name,src,*argc,**argv):
        args = dict(argv)
        args['a_service_name'] = a_service_name
        argv = self.__parseOption(a_process_name,src,**argv)

        if self._isWindows:
            cppdbflags = self.getServicesPdb(a_service_name,a_process_name)
            argv['CCPDBFLAGS'] = ' /Fd'+cppdbflags
            pdbRele = os.path.relpath(cppdbflags,self.baseDir)
            argv['LINKFLAGS'] = self.env['LINKFLAGS']+(' /DEBUG /PDB:\"'+str(pdbRele)+'\"')

        target = self.env.Program(a_process_name,src,**argv)

        if argv.has_key('a_tsf4g_static_lib'):
            for key in Split(argv['a_tsf4g_static_lib']):
                Depends(target,self.getStaticLib(key))

        self.env.Install(self.getServicesPath(a_service_name),target)

        if self._isLinux and self.isInstall('base'):
            Install(self.getServicesInstallPath(a_service_name),target)

        if self.isInstall('base')  and self._isWindows:
            #InstallAs(self.getServicesInstallPdb(a_service_name,a_process_name),
            #    self.getServicesPdb(a_service_name,a_process_name))
            Install(self.getServicesInstallPath(a_service_name),target)
        return

    def buildUnittests(self,a_unittests_name,a_process_name,src,*argc,**argv):
        args = dict(argv)
        args['a_unittests_name'] = a_unittests_name
        argv = self.__parseOption(a_process_name,src,**argv)

        if self._isWindows:
            cppdbflags = self.getUnittestsPdb(a_unittests_name,a_process_name)
            argv['CCPDBFLAGS'] = ' /Fd'+cppdbflags
            pdbRele = os.path.relpath(cppdbflags,self.baseDir)
            argv['LINKFLAGS'] = self.env['LINKFLAGS']+(' /DEBUG /PDB:\"'+str(pdbRele)+'\"')

        target = self.env.Program(a_process_name,src,**argv)

        if argv.has_key('a_tsf4g_static_lib'):
            for key in Split(argv['a_tsf4g_static_lib']):
                Depends(target,self.getStaticLib(key))

        self.env.Install(self.getUnittestsPath(a_unittests_name),target)

        return

    def buildExamples(self,a_examples_name,a_process_name,src,*argc,**argv):
        args = dict(argv)
        args['a_examples_name'] = a_examples_name
        argv = self.__parseOption(a_process_name,src,**argv)

        if self._isWindows:
            cppdbflags = self.getExamplesPdb(a_examples_name,a_process_name)
            argv['CCPDBFLAGS'] = ' /Fd'+cppdbflags
            pdbRele = os.path.relpath(cppdbflags,self.baseDir)
            argv['LINKFLAGS'] = self.env['LINKFLAGS']+(' /DEBUG /PDB:\"'+str(pdbRele)+'\"')

        target = self.env.Program(a_process_name,src,**argv)

        if argv.has_key('a_tsf4g_static_lib'):
            for key in Split(argv['a_tsf4g_static_lib']):
                Depends(target,self.getStaticLib(key))

        return

    def runShell(self,src,**argv):
        args = dict(argv)
        if args.has_key('OPTIONS'):
            args['SHELLFLAGS'] = args['OPTIONS']
        args['SHDIR'] = os.path.abspath(os.path.dirname(src))
        args['CURDIR'] = os.getcwd()
        src = str(os.path.basename(src))
        target = self.env.runshell(source=src,**args)
        return target

    def xml2h(self,src,a_install_dir='',**argv):
        res = dict(argv)
        if res.has_key('OPTIONS'):
            res['XML2HFLAGS'] = res['OPTIONS']

        target= self.env.xml2h(source=src,**res)
        Depends(target,self.getTools('tdr'))
        if a_install_dir != "":
            target = self.env.Install(os.path.abspath(a_install_dir),target)
        return target

    def xml2hs(self,target,src,a_target_name='',a_install_dir=""):


            target= self.env.xml2hs(target,source=src)
            Depends(target,self.getTools('tdr'))
            return target

    def xml2c(self,target,src,**argv):
            res = dict(argv)
            if res.has_key('OPTIONS'):
                res['XML2CFLAGS'] = res['OPTIONS']

            target= self.env.xml2c(target,src,**res)
            Depends(target,self.getTools('tdr'))
            return target

    def xml2dr(self,target,src, **argv):
            res = dict(argv)
            if res.has_key('OPTIONS'):
                res['XML2HFLAGS'] = res['OPTIONS']

            target = self.env.xml2dr(target,src,**res)
            Depends(target,self.getTools('tdr'))
            return target

    def xml2cpp(self,src, **argv):
            res = dict(argv)
            if res.has_key('OPTIONS'):
                res['XML2CPPFLAGS'] = res['OPTIONS']

            target =  self.env.xml2cpp(src,**res)
            Depends(target,self.getTools('tdr'))
            return target

    def tmeta2tab(self,target,src,a_target_name='',a_install_dir="",**argv):
            res = dict(argv)
            if res.has_key('OPTIONS'):
                res['XML2HFLAGS'] = res['OPTIONS']
            target= self.env.tmeta2tab(target,source=src,**res)
            Depends(target,self.getTools('tmeta2tab'))
            return target

    def deps(self,*argc,**argv):
        print "Warining deplicated function deps"
        return
    def fixDeps(self,*argc,**argv):
        print "Warning deplicated function fixDeps"
        return

    def __initShellRun(self):
        runShell = Builder(action='cd $SHDIR;'+'sh $SOURCES $SHELLFLAGS;'+'cd $CURDIR')
        self.env.Append(BUILDERS={'runshell':runShell})

    def __initTdrTools(self):
        if self.dirDict.has_key('tools_path'):
            toolspath=os.path.abspath(os.path.join(self.dirDict['tools_path'],'tdr'+self._execSuffix))
        else:
            toolspath=os.path.abspath(os.path.join(self.toolsBinPath,'tdr'+self._execSuffix))
        xml2h = Builder(action=toolspath+' --xml2h $XML2HFLAGS  $SOURCES',
                                        suffix='.h',src_suffix='.xml',
                                        emitter=xml2h_modify_targets)

        xml2hs = Builder(action=toolspath+' --xml2h  $SOURCES  -o $TARGET',
                                        suffix='.h',src_suffix='.xml')

        xml2c = Builder(action=toolspath+' --xml2c $XML2CFLAGS $SOURCES -o  $TARGET',
                                        suffix='.c',src_suffix='.xml')

        xml2cpp = Builder(action=toolspath+' --xml2cpp $XML2CPPFLAGS $SOURCES',emitter=xml2cpp_modify_targets)

        xml2tdr = Builder(action=toolspath+' --xml2dr  $SOURCES -o  $TARGET',
                                        suffix='.tdr',src_suffix='.xml')


        if self.dirDict.has_key('tools_path'):
            toolspath=os.path.abspath(os.path.join(self.dirDict['tools_path'],'tmeta2tab'+self._execSuffix))
        else:
            toolspath=os.path.abspath(os.path.join(self.toolsBinPath,'tmeta2tab'+self._execSuffix))
        tmeta2tab = Builder(action=toolspath+' $FLAGS -B $SOURCES  -o $TARGET',
                                        suffix='.tdr',src_suffix='.sql')

        self.env.Append(BUILDERS={'xml2h':xml2h})
        self.env.Append(BUILDERS={'xml2hs':xml2hs})
        self.env.Append(BUILDERS={'xml2c':xml2c})
        self.env.Append(BUILDERS={'xml2dr':xml2tdr})
        self.env.Append(BUILDERS={'xml2cpp':xml2cpp})
        self.env.Append(BUILDERS={'tmeta2tab':tmeta2tab})

        return

    def __initSoNameBuilder(self):
        self._a_major = '2'
        self._a_minor = '1'
        self._a_rev   = '0'



        builder = Builder(action=soname_function,emitter=soname_modify_function)
        self.env.Append(BUILDERS={'soname':builder})

    def __parseOption(self,target,source,**argv):
        options='CPPDEFINES CPPPATH LINKFLAGS'
        keyWords = dict()
        for option in Split(options):
            keyWords[option]=True

        res = dict()
        for (key,option) in argv.items():
            #print key
            if keyWords.has_key(key):
                res[key] = argv[key]+self.env[key]
            else:
                res[key] = option

        if not res.has_key("MODULE"):
            res["MODULE"] = "None"

        thirdPartyLib = " "
        tsf4gLib=' '


        if res.has_key('LIBS'):
            for lib in Split(res['LIBS']):
                if self.tsf4g_libs_dict.has_key(lib):
                    tsf4gLib = tsf4gLib + lib +' '
                else:
                    thirdPartyLib = thirdPartyLib + lib + ' '

        if res.has_key('a_tsf4g_static_lib'):
            for lib in Split(res['a_tsf4g_static_lib']):
                if self.tsf4g_libs_dict.has_key(lib):
                    tsf4gLib = tsf4gLib + lib +' '
                else:
                    thirdPartyLib = thirdPartyLib + lib + ' '

        res['LIBS'] = thirdPartyLib
        res['a_tsf4g_static_lib'] = tsf4gLib


        if self._isLinux:
            if res.has_key('a_tsf4g_static_lib'):
                libflags = ''
                a_tsf4g_static_lib = res['a_tsf4g_static_lib']
                for lib in Split(a_tsf4g_static_lib):
                    libPath=self.getStaticLib(lib)
                    libflags = ' '+libflags+' '+libPath+' '
                res['_LIBFLAGS'] = libflags

            if self._is32:
                libflags = ' -L/usr/local/lib  -L/usr/lib/mysql -Wl,-Bstatic '
            else:
                libflags = ' -L/usr/lib64 -Wl,-Bstatic '

            if res.has_key('a_thirdparty_static_lib'):
                thireparty_static_lib = res['a_thirdparty_static_lib']
                for lib in Split(thireparty_static_lib):
                    libflags += ' -l'+lib+' '

            libflags += '-lscew -lexpat -Wl,-Bdynamic -lpthread -lgcov'
            if res.has_key('_LIBFLAGS'):
                res['_LIBFLAGS'] = res['_LIBFLAGS']+libflags
            else:
                res['_LIBFLAGS'] = libflags

            libflags = ''
            if res.has_key('LIBS'):
                for lib in Split(res['LIBS']):
                    libflags += ' -l%s ' % lib
                if res.has_key('_LIBFLAGS'):
                    res['_LIBFLAGS'] = res['_LIBFLAGS']+libflags
                else:
                    res['_LIBFLAGS'] = libflags

            if not self._isDebug:
                #Add o2 here.
                if res.has_key('_LIBFLAGS'):
                    res['_LIBFLAGS'] = " -O2 " + res['_LIBFLAGS']
                else:
                    res['_LIBFLAGS'] = " -O2 "
        else:
            res['LIBS'] = ""
        return res

    def __buildexe(**args):
        #I have to build all the exes here for it.
        args = self.__parseOption(**args)
        dirEnv = args['a_dirEnv']
        a_target = args['a_target']
        a_source = args['a_source']

        if self._isWindows:
            cppdbflags = dirEnv['cppdbdir']
            argv['CCPDBFLAGS'] = ' /Fd'+cppdbflags
            pdbRele = os.path.relpath(cppdbflags,self.baseDir)
            argv['LINKFLAGS'] = self.env['LINKFLAGS']+(' /DEBUG /PDB:\"'+str(pdbRele)+'\"')


        target = self.env.Program(a_target,a_source,**argv)

        if argv.has_key('a_tsf4g_static_lib'):
            for key in Split(argv['a_tsf4g_static_lib']):
                Depends(target,self.getStaticLib(key))

        self.env.Install(self.getExamplesPath(a_examples_name),target)
        self.__Install(self.getExamplesInstallPath(a_examples_name),target)
        return

    def __getIncludePath(self):
        if self._isWindows:
                newPath=[os.path.abspath(os.path.join(self.baseDir,'include'))]
        else:
                newPath=[os.path.abspath(os.path.join(self.baseDir,'include')),'/usr/include/mysql','/usr/local/include/mysql']

        if self.dirDict.has_key('includePath'):
             newPath += self.dirDict['includePath']

        if self._isWindows:
            includeList=['include','deps/scew/','deps/expat/lib','deps/mysql/include','deps/zlib']
        else:
            includeList=['include','deps/scew/','deps/expat/lib']

        for path in includeList:
            newPath=newPath+[os.path.abspath(os.path.join(self.baseDir,path))]
        return newPath

    def __getBuildDefines(self):

        AddOption('--no-tag',
        dest = 'noTag',
        type = 'string',
        nargs=1,
        action='store',
        metavar='STRING',
        default='0',
        help='Whether to make a tag during install: 0 or 1 default 0')

        noTag = GetOption('noTag')

        if noTag == '1':
            self._noTag = True
        elif noTag == '0':
            self._noTag = False
        else:
            self._noTag = False


        if self._isWindows:

                AddOption('--char-encoding',
                dest = 'charEncodingUtf8',
                type = 'string',
                nargs=1,
                action='store',
                metavar='STRING',
                default='0',
                help='Whether to use utf8 or othre encoding:\n\t1:uincode\n\t2:multibyte\n\t3:not specify')

                tagName = GetOption('charEncodingUtf8')

                if tagName == '1':
                    defines = ' _UNICODE  UNICODE  '
                elif tagName == '2':
                    defines = " _MBCS "
                else:
                    defines = ''

                if self.isWin32():
                    defines = defines + '_WIN32 WIN32 XML_STATIC TSF4G_STATIC _NEW_LIC '
                elif self.isWin64():
                    defines = defines + '_WIN64 WIN64 XML_STATIC TSF4G_STATIC _NEW_LIC '
                if self._isWin32:
                    defines = defines + '_USE_32BIT_TIME_T'
        else:
                defines = '_GNU_SOURCE XML_STATIC TSF4G_STATIC _USE_32BIT_TIME_T   _NEW_LIC _GNU_SOURCE'

        if self._isDebug:
            defines =defines+ ' _DEBUG DEBUG'
        else:
            defines = defines + ' NDEBUG '
        return Split(defines)

    def __Install(self,dest,source):
        if self._isInstall:
            return self.env.Install(dest,source)

    def Install(self,target,source):
        Install(target,source)

    def InstallAs(self,target,source):
        InstallAs(target,source)

    def __init_version_header(self):
        #self.versionDict = dict()
        builder = Builder(action=generate_version,emitter=generate_version_modify)
        self.env.Append(BUILDERS={'VersionHeader':builder})

        builder = Builder(action=makeNsis,emitter=makeNsis_modify)
        self.env.Append(BUILDERS={'Makensis':builder})

        builder = Builder(action=test,emitter=test_modify)
        self.env.Append(BUILDERS={'ttest':builder})

    def VersionHeader(self,*argc,**argv):
        argv1 = dict()
        for (key,value) in argv.items():
            argv1[key]=value
        argv = argv1

        if argv.has_key('MODULE'):
            mod = argv['MODULE']
            if self.versionDict.has_key(mod):
                #self.versionDict[mod] = dict()
                #self.versionDict[mod]['major'] =
                argv['major'] = self.versionDict[mod]['major']
                argv['minor'] = self.versionDict[mod]['minor']
                #self.versionDict[mod]['minor'] = argv['minor']
                #print "Error duplicate version key :%s" % mod
                #Exit(1)
            else:
                #print "Adding %s"% mod
                self.versionDict[mod] = dict()
                self.versionDict[mod]['major'] = argv['major']
                self.versionDict[mod]['minor'] = argv['minor']
        # If is install I have to set must build flag
        #
        #print "Version called %s" % os.getcwd()
        # major minor and others.
        argv['TAGDIR'] = self.dirDict['tag'][0][0]
        target = self.env.VersionHeader(*argc,**argv)
        if self._isInstall or True:
            AlwaysBuild(target)

    def Package(self,*argc,**argv):
        if self._isInstall:
            verDic = self.versionDict[self.InstallModuleName]
            version = "%s_%s_%s_%s" %(verDic['major'],verDic['minor'],getSVNRev(self.dirDict['tag'][0][0]),self.getCompiler())
            name = 'tsf4g_%s' % self.InstallModuleName

            if self._isDebug:
                version = version + '_debug'
            else:
                version = version + '_release'

            tagName = "%s-%s" %(name,version)

            p = subprocess.Popen("svn info",stdout=subprocess.PIPE,shell=True)
            p.wait()
            verinfo = p.stdout.read()
            vf = open(os.path.join('version.txt'),'w')
            vf.write('TagName:'+tagName+'\n')
            vf.write(verinfo)
            vf.close()

            if not os.path.isdir(self.getInstallBase()):
                os.makedirs(self.getInstallBase())
            Install(self.getInstallBase(),'version.txt')

            sources = self.env.FindInstalledFiles()
            new_sources = list()
            if self._isLinux:
                s='/'
            else:
                s = '\\'
            for src in sources:
                #print (str(os.path.relpath(self.getInstallBase(),self.baseDir)).split('/'))[0]
                if (str(src).split(s))[0] == (str(os.path.relpath(self.getInstallBase(),self.baseDir)).split(s))[0]:
                    #print "Install %s" % src
                    new_sources += [src]
            for src in new_sources:
                print "Install %s" % src

            if len(new_sources):
                if self._isLinux:
                    self.env.Package(source = new_sources,
                        NAME=name,
                        VERSION=version,
                        PACKAGEVERSION = 0,
                        PACKAGETYPE='targz')
                    target = name+"-"+version+".tar.gz"

                else:
                    print "Calling build package"
                    t = self.env.Makensis(source = new_sources,TAGNAME=tagName)
                    self.env.Package(source = t,
                        NAME=name,
                        VERSION=version,
                        PACKAGEVERSION = 0,
                        PACKAGETYPE='zip')
                    target = name+"-"+version +".zip"
                Install(os.path.abspath(os.path.join(self.baseDir,"dist")),target)

                if not self._noTag:
                    for (src,dst) in self.dirDict['tag']:
                        print "tag(%s=>%s)" %(src,dst)
                        svnTag(tagName,src,dst,forceOverWrite = True,MODULE=self.GetBuildModule())

    def ttest(self,*args,**argv):
        return
        #return self.env.ttest(*args,**argv)

Export('tsf4g_build')
