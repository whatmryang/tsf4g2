#coding=gbk

import sys, pprint
from plaincode import *

tDirective = Combine(Literal("#") + restOfLine)
tDirective.setParseAction(lambda pos, ts: [['type_directive', pos, i] for i in ts])

#--------------------------------------------------------------------
# ����ʱ��
#--------------------------------------------------------------------
#def trace(tok) : print tok
#tDirective.addParseAction(trace)

def dumpDirective(obj, rdump, preBlank):
    print preBlank + 'directive:', obj