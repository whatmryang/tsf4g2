#coding=gbk

import yaml
from os.path import *

class Directory:
    
    def __init__(self):
        dirfile = join(dirname(realpath(__file__)), r'../directory.yaml')
        self.info = yaml.load(open(dirfile).read().decode('gbk'))
        
    def GetDirPath(self, htmname):
        dirpath = [[u'主页', u'index.htm']]
        self._FindDirPath(self.info['root'], htmname, dirpath)
        return dirpath
    
    def _FindDirPath(self, nodes, htmname, dirpath):
        for node in nodes:
            if node['uri'] == htmname:
                dirpath.append([node['desc'], node['uri']])
                return True
            else:
                if node.has_key('children'):
                    dirpath.append([node['desc'], node['uri']])
                    if self._FindDirPath(node['children'], htmname, dirpath):
                        return True
                    del dirpath[-1]

if __name__ == '__main__':
    dirs = Directory()
    print dirs.GetDirPath('newpage22.htm')
    