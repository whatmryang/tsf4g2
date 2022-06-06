#coding = gbk

import sys, pprint
from plaincode import *
from function import *
from cpptype import *

tModifier = Literal("public") | Literal("protected") | Literal("private")

tClassParent = tIdent + Optional(Literal("<")+ delimitedList(tIdent|tInteger) +Literal(">"))
tClassParents = Group(Suppress(":") + delimitedList(Group(tModifier + tClassParent)))
tClassModifier = tModifier + Suppress(":")
tClassModifier.setParseAction(lambda pos, ts: [['type_classModifier', pos, i] for i in ts])

tClassBody = Optional(Group(ZeroOrMore(tClassModifier | tCpp)), '')
tClass = Group(Literal("class") + tIdent + Optional(tClassParents, '') + Literal('{') + Optional(tClassBody) + Literal('}') + Literal(';'))
tClass.setParseAction(lambda pos, ts: [['type_class', pos, i] for i in ts])

tClassPreDefine = Group(Literal('class') + tIdent + Literal(';'))
tClassPreDefine.setParseAction(lambda pos, ts: [['type_classpredefine', pos, i] for i in ts])

tTemplate = Group(Suppress('template') + Suppress('<') + Group(delimitedList(tType|tInteger)) + Suppress('>') + tClass)
tTemplate.setParseAction(lambda pos, ts: [['type_template', pos, i] for i in ts])

class Class:
    def __init__(self, rawInfo):
        self._raw = rawInfo
        self._publicMethods = [[], []] #declare, define
        self._protectedMethods = [[], []]
        self._privateMethods = [[], []]
        currList = self._protectedMethods
        for i in self._raw[2][4]:
            if i[0] == 'type_classModifier':
                if i[2] == 'public':
                    currList = self._publicMethods
                elif i[2] == 'protected':
                    currList = self._protectedMethods
                elif i[2] == 'private':
                    currList = self._privateMethods
            elif i[0] == 'type_functionDeclare':
                no = FunDeclare(i)
                currList[0].append(no)
            elif i[0] == 'type_functionDefine':
                no = FunDefine(i)
                currList[1].append(no)                            
    def name(self):
        return self._raw[2][1]
    def publicFunDeclares(self):
        return self._publicMethods[0]
    def publicFunDefines(self):
        return self._publicMethods[1]
    def findPublicFunDefine(self, inFun):
        for i in self.publicFunDefines():
            if i.name() == inFun.name() and len(i.paramList()) == len(inFun.paramList()):
                lhs = [k.type() for k in i.paramList()]
                rhs = [k.type() for k in inFun.paramList()]
                if lhs == rhs:
                    return i

class Template:
    def __init__(self, rawInfo):
        self._raw = rawInfo
        self._class = Class(self._raw[2][1])
    def name(self):
        return self._raw[2][1][2][1]
    def classObj(self):
        return self._class

def dumpTemplate(obj, rdump, preBlank):
    o = Template(obj)
    print preBlank + 'template:', o.name()
    dumpClass(o.classObj(), rdump, preBlank+'  ')
    
def dumpClassPredefine(obj, rdump, preBlank):
    print preBlank + 'classPredefine:', obj[1]

def dumpClassObj(o, rdump, preBlank):
    print preBlank + 'class :', o.name()
    print preBlank + '  public method declares :'
    for i in o.publicFunDeclares():
        dumpFunDeclareObj(i, rdump, preBlank + '    ')
    print preBlank + '  public method defines :'
    for i in o.publicFunDefines():
        dumpFunDefineObj(i, rdump, preBlank  + '     ')
    
def dumpClass(obj, rdump, preBlank):
    o = Class(obj)
    dumpClassObj(o, rdump, preBlank)
 
def testClass():
    text = """
    class CPreDefine;
    class CObject
    {
    void SayGood();
    public:
        void SayHello();
        unsigned int Calc(int p)
        {
            return p;
        }
    protect:
    };
    
    class COther : public CA, public CB, protect ITemplate<CObj, 1>
    {
    };
    """
    tCpp << (tFunDeclare|tFunDefine|tClassPreDefine|tClass)
    tBNF = ZeroOrMore(tClass|tClassPreDefine)
    tBNF.parseWithTabs()
    result = tBNF.parseString(text)
    for i in result:
        print i[0]
        if i[0] == 'type_class':
            dumpClass(i[2], 0, '')
        elif i[0] == 'type_classpredefine':
            dumpClassPredefine(i[2], 0, '')

def testTemplate():
    text = """
	template
	<
		class TInterface, 
		class TSubclass,
		class TCoreClass,
		DWORD tdwComponentID, 
		DWORD tdwVersion
	>
	class TWinlessImpl :
		public TComponentImpl<TInterface, TSubclass, tdwComponentID, tdwVersion>,
		public TLogHelp<TSubclass, tdwComponentID>
            {
            };
    """
    print tTemplate.parseString(text)
    
if __name__ == '__main__':
    testTemplate();
