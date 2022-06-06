import os
import sys
import yaml
from webgen import doc2html
from webgen import basetemp

def updatefiles():
    if len(sys.argv) != 2:
      print "parameter incorrect!usage:test.py $project"
      return 
    project = sys.argv[1]
    info = yaml.load(basetemp.readFile('updatefiles.yaml'))
    if info.has_key(project):
      projectinfo = info[project]
      for fileitem in projectinfo:
        srcfile = os.path.join(info['srchome'],fileitem['src'])
        dstfile = os.path.join(info['dsthome'],fileitem['dst'])
        locations = fileitem['loc']   
        doc2html.Word2Html(srcfile,dstfile,locations)
    else:
      print "unknow project %s"%(project)     
    return       



if __name__=='__main__':
    updatefiles()
    raw_input("Press enter key to close this window");