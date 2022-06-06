#coding = gbk

import sys, pprint, os
from plaincode import *
from function import *
from cppclass import *
from directive import *
from namespace import *
from cpptype import *

tTypedef = Group(Suppress('typedef')+ ~FollowedBy(Literal('struct'))+ tType + SkipTo(Suppress(';')))
tTypedef.setParseAction(lambda pos, ts: [['type_typedef', pos, i] for i in ts])
tCpp << (tDirective|tNamespace|tFunDeclare|tFunDefine|tClass|tClassPreDefine|tTemplate|tTypedef|tTypedef|Literal(';')|tUnknownLine|tBraceCode)
tCpp.ignore(cStyleComment)
tCpp.ignore(cppStyleComment)
tCpp.ignore(cplusplusLineComment)
tCppBNF = ZeroOrMore(tCpp)
tCppBNF.parseWithTabs()

class Cpp:
    def __init__(self, cppFilePath):
        self._path = cppFilePath
        self._text = open(cppFilePath).read()
        self._inserts = []
        self._grammar  = []
        try:
            self._grammar = tCppBNF.parseString(self._text, True)
        except ParseException, pfe:
            print 'Cpp parse error:'
            print 'Line no:', lineno(pfe.loc, text)
            print 'File :', self._path
            print 'Error :', pfe.msg
            print 'Input text :', line(pfe.loc, text)
    def findNamespace(self, namespaceName):
        for g in self._grammar:
            if g[0] == 'type_namespace':
                return Namespace(g)
    def assertFileDir(self, p):
        folder = os.path.dirname(p)
        if not os.path.isdir(folder):
            os.makedirs(folder)
    def saveAs(self, path):
        self.assertFileDir(path)
        if len(self._inserts) > 0:
            self._inserts.sort(lambda x, y : cmp(y[0], x[0]))
            newText = self._text
            for i in self._inserts:
                ipos = i[0]
                istr = i[1]
                newText = newText[:ipos] + istr+ newText[ipos:]
                open(path, 'w').write(newText)
        else:
            open(path, 'w').write(self._text)
    def getText(self):
        return self._text
    def insertText(self, pos, text):
        self._inserts.append([pos, text])
    def insertCount(self):
        return len(self._inserts)
        
def dumpCpp(objList, preBlank):
    for i in objList:
        typeName = i[0]
        if typeName == 'type_directive':
            dumpDirective(i[2], dumpCpp, preBlank)
        elif typeName == 'type_namespace':
            dumpNamespace(i, dumpCpp, preBlank)
        elif typeName == 'type_class':
            dumpClass(i, dumpCpp, preBlank)
        elif typeName == 'type_classpredefine':
            dumpClassPredefine(i[2], dumpCpp, preBlank)
        elif typeName == 'type_macro':
            print preBlank+'macro:', i[2]
        elif typeName == 'type_functionDeclare':
            dumpFunDeclare(i, dumpCpp, preBlank)
        elif typeName == 'type_functionDefine':
            dumpFunDefine(i, dumpCpp, preBlank)
        elif typeName == 'type_unknownLine':
            print preBlank+'unknownLine:', i[2]
        elif typeName == 'type_winlessimpl':
            print preBlank+'winlessimpl:', i[2]
        elif typeName == 'type_simcontrol':
            print preBlank+'simcontrol:', i[2]
        elif typeName == 'type_typedef':
            print preBlank+'typedef:', i[2]
        elif typeName == 'type_template':
            dumpTemplate(i, dumpCpp, preBlank)

if __name__ == '__main__':
    path = sys.argv[1]
    try:
        text = open(path).read()
        result = tCppBNF.parseString(text, True)
        dumpCpp(result, '')
    except ParseException, pfe:
        text = open(path).read()
        print "Error:"
        print path
        print pfe.msg
        print 'err location:', line(pfe.loc,text)
        print 'line no:', lineno(pfe.loc, text)
        
