#coding = gbk

import sys, pprint
from pyparsing import *
from pyparsing import *

tM1 = Combine(Literal('TENIO_MAKE_TYPEID_MAP') + restOfLine)

tMacros = (tM1)
tMacros.setParseAction(lambda pos, ts: [['type_macro', pos, i] for i in ts])