#coding=gbk

from pyparsing import *

tDxBrief = Suppress("@brief") + restOfLine
tDxParam = Suppress("@param") + Word(alphas + "_", alphas + nums + "_$") + restOfLine
tDxReturn = Suppress("@return") + restOfLine

class Docxy:
    def __init__(self):
        self.brief = ''
        self.ret = ''
        self.params = []

def cleanDocxy(text):
    cleantext = ''
    for l in text.split('\n'):
        l = l.strip()
        if not (l.startswith('/*') or l.startswith('*/')):
            l = l.lstrip('*')
            l = l.lstrip()
            cleantext += l
    pos = cleantext.find('@')
    while pos != -1:
        cleantext = cleantext[:pos] + '\n' + cleantext[pos:]
        pos = cleantext.find('@', pos+2)
    return cleantext
        
def parseDocxy(text):
    text = cleanDocxy(text)
    d = Docxy()
    for tokens, preloc, nextLoc in tDxBrief.scanString(text):
        d.brief = tokens[0].strip()
    for tokens, preloc, nextLoc in tDxParam.scanString(text):
        tokens[0] = tokens[0].strip()
        tokens[1] = tokens[1].strip()
        d.params.append(tokens)
    for tokens, preloc, nextLoc in tDxReturn.scanString(text):
        d.ret = tokens[0].strip()
    return d

if __name__ == '__main__':
    text = """
/**
		 *  @brief ����iconcount�������ͼƬ����Ϊbtn�ĸ���״̬ͼ��
ͨ��˳����4��̬����
		 *  @param lpszIconFileName ͼƬ�ļ���
		 *  @param IconCount ��������
		 */
"""
    d = parseDocxy(text)
    print d.brief
    print d.params
   
