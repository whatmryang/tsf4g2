

class tsf4gVersion():
    def __init__(self):
        self.versionDict = dict()

        self.versionDict['base'] = dict()
        self.versionDict['base']['major'] = "2"
        self.versionDict['base']['minor'] = "4"
        self.versionDict['base']['checkOut'] = [("https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk","")]
        self.versionDict['base']['commit'] = ["https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk"]
        self.versionDict['base']['tag'] = [""]

        self.versionDict['tconnd'] = dict()
        self.versionDict['tconnd']['major'] = "2"
        self.versionDict['tconnd']['minor'] = "1"

        self.versionDict['tdir'] = dict()
        self.versionDict['tdir']['major'] = "2"
        self.versionDict['tdir']['minor'] = "1"


        self.versionDict['torm'] = dict()
        self.versionDict['torm']['major'] = "3"
        self.versionDict['torm']['minor'] = "4"

        self.versionDict['tsdm'] = dict()
        self.versionDict['tsdm']['major'] = "2"
        self.versionDict['tsdm']['minor'] = "3"

        self.versionDict['tmail'] = dict()
        self.versionDict['tmail']['major'] = "2"
        self.versionDict['tmail']['minor'] = "0"

        self.versionDict['tmart'] = dict()
        self.versionDict['tmart']['major'] = "2"
        self.versionDict['tmart']['minor'] = "0"
        self.versionDict['tmart']['checkOut'] = [\
		("https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/tags/tsf4g_base-2_1_13989_i686_release/trunk/",""),\
		("https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/tags/TSF4G_RESCONV_01_007/apps/ResConvertBasedDR/","apps/ResConvertBasedDR/"),\
		("https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/tags/TSF4G_RESCONV_01_007/apps/ResConvertBasedDR/","apps/ResConvertBasedDR/"),\
		("https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tmart_system_proj/trunk/","apps/tmart/"),\
		]
        self.versionDict['tmart']['commit'] = ["apps/tmart/"]
        self.versionDict['tmart']['tag'] = [\
		("https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tmart_system_proj/trunk/","https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tmart_system_proj/tags/")]

        self.versionDict['tcenterd'] = dict()
        self.versionDict['tcenterd']['major'] = "2"
        self.versionDict['tcenterd']['minor'] = "0"

        self.versionDict['tdirty'] = dict()
        self.versionDict['tdirty']['major'] = "2"
        self.versionDict['tdirty']['minor'] = "0"

        self.versionDict['tproxy'] = dict()
        self.versionDict['tproxy']['major'] = "2"
        self.versionDict['tproxy']['minor'] = "1"

        self.versionDict['tcm'] = dict()
        self.versionDict['tcm']['major'] = "2"
        self.versionDict['tcm']['minor'] = "1"

        self.versionDict['tagent'] = dict()
        self.versionDict['tagent']['major'] = "2"
        self.versionDict['tagent']['minor'] = "0"

    def addNewModule(self,ModuleName,Major,Minor):
        if self.versionDict.has_key(ModuleName):
            print "Warning::the module(%s) already exists" % ModuleName
        self.versionDict[ModuleName] = dict()
        self.versionDict[ModuleName]['major'] = Major
        self.versionDict[ModuleName]['minor'] = Minor

    def getMajor(self,ModuleName):
        return self.versionDict[ModuleName]['major']

    def getMinor(self,ModuleName):
        return self.versionDict[ModuleName]['minor']



Export('tsf4gVersion')
