#coding=gbk

import sys, pprint
from plaincode import *

tEmptyParam = Word(alphas + "_", alphas + nums + "_$")
tEmptyParam.setParseAction(lambda ts:[ts[0], ''])
tParam =  Group(tVarType + tIdent + Optional(Literal('=')+tAnyWord)) | Group(tEmptyParam)
tParamList = Group(Optional(delimitedList(tParam)))
tVFunHead = Optional(Literal("virtual"), default="")
tVFunTail = Optional(Group(Literal("=") + Literal("0")).setParseAction(lambda ts:['=0']), default='')
tFunName = Word(alphas + "_~", alphas + nums + "_$")
tFunHead =  Optional(tVarType, default='') + Group(tFunName + Optional(Literal('::')+tFunName)) + Suppress("(") + tParamList + Suppress(")")
tFunDeclare = Group(tVFunHead + tFunHead + tVFunTail + Literal(";"))
tFunDeclare.setParseAction(lambda pos, ts : [['type_functionDeclare', pos, i] for i in ts])
tFunDefine = Group(tFunHead + Literal('{') + tCodeBlock + Literal('}') + Optional(Literal(';')))
tFunDefine.setParseAction(lambda pos, ts : [['type_functionDefine', pos, i] for i in ts])
tParser = OneOrMore(tFunDeclare|tFunDefine)

#--------------------------------------------------------------------
# 调试时打开
#--------------------------------------------------------------------
#def trace(tok) : print tok
#tFunDeclare.addParseAction(trace)
#tFunDefine.addParseAction(trace)

class Param:
    def __init__(self, rawInfo):
        self._raw = rawInfo
    def type(self):
        return ''.join(self._raw[0])
    def name(self):
        return self._raw[1]
        
class FunDeclare:
    def __init__(self, rawInfo):
        self._raw = rawInfo
        self._paramList = []
        for p in self._raw[2][3]:
            np = Param(p)
            self._paramList.append(np)
    def isVirtual(self):
        return self._raw[2][0] == 'virtual'    
    def retType(self):
        return ' '.join(self._raw[2][1])
    def name(self):
        return self._raw[2][2][0]
    def isPure(self):
        return self._raw[2][4] != ''
    def paramList(self):
        return self._paramList
    def getPos(self):
        return self._raw[1]

class FunDefine:
    def __init__(self, rawInfo):
        self._raw = rawInfo
        self._paramList = []
        for p in self._raw[2][2]:
            np = Param(p)
            self._paramList.append(np)
    def name(self):
        return self._raw[2][1][0]
    def retType(self):
        return ' '.join(self._raw[2][0])
    def paramList(self):
        return self._paramList
    def findBodyBeginPos(self, cpp):
        text = cpp.getText()
        return text.find('{', self._raw[1]) + 1

def dumpFunDeclareObj(o, rdump, preBlank):
    print preBlank + 'FunctionDeclare :'
    print preBlank + '  name :', o.name()
    print preBlank + '  isVirtual :', o.isVirtual()
    print preBlank + '  isPure :', o.isPure()
    print preBlank + '  retType :', o.retType()
    print preBlank + '  paramList :'
    pl = o.paramList()
    for i, p in enumerate(pl):
        print preBlank + '    param', i+1, ':', p.type(), p.name()
    
def dumpFunDeclare(body, rdump, preBlank):
    o = FunDeclare(body)
    dumpFunDeclareObj(o, rdump, preBlank)

def dumpFunDefineObj(o, rdump, preBlank):
    print preBlank + 'FunctionDefine :'
    print preBlank + '  name :', o.name()
    print preBlank + '  retType :', o.retType()
    print preBlank + '  paramList :'
    pl = o.paramList()
    for i, p in enumerate(pl):
        print preBlank + '    param', i+1, ':', p.type(), p.name()
    
def dumpFunDefine(body, rdump, preBlank):
    o = FunDefine(body)
    dumpFunDefineObj(o, rdump, preBlank)

if __name__ == '__main__':
    text = """
        virtual unsigned int Create(unsigned int & lpParam, ISocketEvent* pEventHandler) = 0;
        int SayHello(int p, int*& n);
        int noparamfun();
        
        int dosomthing()
        {
        printf("hello");
        };
        
        void iamok();
    """
    tParser.ignore(cStyleComment)
    tParser.ignore(cppStyleComment)
    tParser.parseWithTabs()

    for i in tParser.parseString(text).asList():
        if i[0] == 'type_functionDeclare':
            print i[0], 'pos=', i[1]
            dumpFunDeclare(i[2])
        elif i[0] == 'type_functionDefine':
            print i[0], 'pos=', i[1]
            dumpFunDefine(i[2])