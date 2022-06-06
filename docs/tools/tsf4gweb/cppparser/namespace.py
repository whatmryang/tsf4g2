#coding = gbk

import sys, pprint
from plaincode import *
from cppclass import *

tNamespace = Group(Literal("namespace") + tIdent + Literal('{') + Group(ZeroOrMore(tCpp)) + Literal('}') + Optional(Literal(';')))
tNamespace.setParseAction(lambda pos, ts: [['type_namespace', pos, i] for i in ts])

class Namespace:
    def __init__(self, rawInfo):
        self._raw = rawInfo
    def name(self):
        return self._raw[2][1]
    def body(self):
        return self._raw[2][3]
    def findClass(self, className):
        for i in self.body():
            if i[0] == 'type_class':
                cclass = Class(i)
                if cclass.name() == className:
                    return cclass
    def findTemplate(self, templateName):
        for i in self.body():
            if i[0] == 'type_template':
                tem = Template(i)
                if tem.name() == templateName:
                    return tem
    def position(self):
        return self._raw[1]
    
def dumpNamespace(obj, rdump, preBlank):
    o = Namespace(obj)
    print preBlank + 'namespace:', o.name()
    rdump(o.body(), preBlank+' '*2)
    
if __name__ == '__main__':
    tCpp << (tFunDeclare|tFunDefine|tClassPreDefine|tClass)
    text = """
      
        
    namespace Tenio
    {
    class GOOD;
        class B
        {
        void SayGood();
        };
    }

    """
    tBNF = ZeroOrMore(tNamespace)
    tBNF.parseWithTabs()
    result = tBNF.parseString(text)
    print result
    