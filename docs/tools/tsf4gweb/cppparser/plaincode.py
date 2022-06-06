#coding=gbk

import sys, pprint
from pyparsing import *
from pyparsing import *

cplusplusLineComment = Literal("//") + restOfLine
tIdent = Word(alphas + "_", alphas + nums + "_$")
tAnyWord = Word(alphas + nums + "_-.")
tInteger = Word( nums )
tPreWords = OneOrMore((tIdent+FollowedBy(tIdent|Literal("*")|Literal("&"))))
tVarType = Group(tPreWords + Optional(Word('*&')))
tValidChar = Word(alphanums + '~`!@#$%^&*()_-+=|\\[]:;\',<.>?')|quotedString|cplusplusLineComment|Literal('/')
tPlainCode = Forward()
tBraceCode = Literal('{') + ZeroOrMore(tPlainCode)+Literal('}')
tPlainBraceCode = tBraceCode + Optional(tPlainCode)
tPlainCode << (tPlainBraceCode | (OneOrMore(tValidChar) + Optional(tPlainBraceCode)))
tCodeBlock = Optional(Group(tPlainCode))
tCodeBlock.setParseAction(  lambda pos, ts :[['type_codeblck', pos]]  )
tCpp = Forward()

tUnknownLine = Combine(Word(alphas + "_", alphas + nums + "_$") + restOfLine)
tUnknownLine.setParseAction(lambda pos, ts: [['type_unknownLine', pos, i] for i in ts])

#--------------------------------------------------------------------
# 调试时打开
#--------------------------------------------------------------------
#def trace(tok) : print tok
#tUnknownLine.addParseAction(trace)
#tBraceCode.addParseAction(trace)

if __name__ == '__main__':
        text = """
        {
        {
        dd
        }
            int n=100;
            for(int i=0; i<n; i++)
            {
                printf("hello %d", 100);
                {
                }
            }
            
            int m = 1000;
        }
        """
        text = '\t\t' + text
        tTest = Literal('{') + tCodeBlock + Literal('}')
        tTest.parseWithTabs()
        result = tTest.parseString(text)
        print result
