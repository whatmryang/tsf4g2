#coding=gbk

import os

def readFile(filename):
    root = os.path.dirname(os.path.realpath(__file__))
    return open(os.path.join(root, '..', filename)).read().decode('gbk')

def readoutpath():
    cfg = os.path.join(os.path.join(os.path.dirname(os.path.realpath(__file__)),'..'), 'outputpath.txt')
    tempDir = os.path.dirname(os.path.realpath(__file__))
    tempDir = os.path.split(tempDir)[0]
    return os.path.join(tempDir, open(cfg).read().strip())

def getPathDelta(filename):
    global outpath
    relpath = filename.replace(outpath,"")
    delta = ""
    sepstr = r".." + os.sep
    sepcount = relpath.count(os.sep)
    while sepcount > 1:
      delta += sepstr
      sepcount -= 1
    return   delta

def getRealPath(url, filename):
    return   getPathDelta(filename) + url


def getrealurl(url,filename):
    global outpath
    relpath = filename.replace(outpath,"")
    prefixurl = ""
    sepstr = r".." + os.sep
    sepcount = relpath.count(os.sep)
    while sepcount > 1:
      prefixurl += sepstr
      sepcount -= 1
    return   prefixurl+url



rootpath = os.path.dirname(os.path.realpath(__file__))
outpath = readoutpath()
basetemplate = readFile(r'templates\frame.htm')

def getWriteFile(filename):
    root = os.path.dirname(os.path.realpath(__file__))
    full = os.path.join(root, '..', filename)
    return open(full, 'w')

def replaceAndSave(listView, locations, filename):
    print filename
    locstr = []
    for l in locations:
        realhref =  getrealurl(l[1],filename)
        str = ur'<a href="%s" class="titlelink">%s</a>' % (realhref, l[0])
        locstr.append(str)
    navigation = u' - '.join(locstr)
    global basetemplate

    #创建目录
    realpath = os.path.realpath((os.path.dirname(filename)))
    if not os.path.exists(realpath):
        os.makedirs(realpath)

    target = getWriteFile(filename)
    realsrcfile = getrealurl("",filename)
    localframe = basetemplate.replace('$(listview)', listView).replace('$(location)', navigation).replace('$(resouce)',realsrcfile)
    localframe = localframe.encode('gbk')

    target.write(localframe)
    target.close()

if __name__=='__main__':
    outpath = readoutpath()
    print outpath
