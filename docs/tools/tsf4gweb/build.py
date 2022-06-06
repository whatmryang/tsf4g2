#coding=gbk

import os
import yaml
from webgen import basetemp

def q(msg):
    raw_input(msg)
    quit()

def getLine(text, url):
    str = r'<tr><td style =" padding: 10px; padding-top: 12px"><a href="$(url)">$(text)</a></td></tr>'
    str = str.replace('$(url)', url)
    str = str.replace('$(text)', text)
    return str


def make(filename, dirinfo, locations, outlist, outpath):
    content = ''
    for c in dirinfo:
        if c.has_key('children'):
            newFileName = os.path.join(outpath, c['uri'])
            locations.append([c['desc'], c['uri']])
            make(newFileName, c['children'], locations, outlist, outpath)
            del locations[-1]
            #content += getLine(c['desc'], os.path.basename(newFileName))
            #content += getLine(c['desc'], c['uri'])
            realurl = basetemp.getrealurl(c['uri'],filename)
            content += getLine(c['desc'], realurl)

        else:
            #content += getLine(c['desc'], c['uri'])
            realurl = basetemp.getrealurl(c['uri'],filename)
            content += getLine(c['desc'], realurl)


    if outlist:
        if os.path.basename(filename) in outlist:
            basetemp.replaceAndSave(content, locations, filename)
    else:
        basetemp.replaceAndSave(content, locations, filename)

def buildHtmls(outlist=None):
    if outlist == [] : return

    cfg = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'outputpath.txt')
    if not os.path.isfile(cfg) : q('请配置outputpath.txt')
    outpath = open(cfg).read().strip()
    if not os.path.isdir(outpath) : q('outputpath.txt 中配置不是一个合法的目录路径')

    info = yaml.load(basetemp.readFile('directory.yaml'))
    indexuri = os.path.join(outpath, r'index.htm')

    make(indexuri, info['root'], [[u'主页', u'index.htm']], outlist, outpath)

if __name__=='__main__':
    buildHtmls()
    raw_input('press enter ...')
