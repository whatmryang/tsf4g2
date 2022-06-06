
class version():

    def __init__(self):
        svns = 'https://sz-svn4.tencent.com/'
        self.dirDict = {}
        self.dirDict['baseDir']='base'
        self.dirDict['export'] = {}
        self.dirDict['checkOut'] = [
            (svns + 'ied/ied_tsf4g_rep/tsf4g_proj/trunk/', '')
        ]
        self.dirDict['tag'] = [
            ('', svns + 'ied/ied_tsf4g_rep/tsf4g_proj/tags/')
        ]

Export('version')
