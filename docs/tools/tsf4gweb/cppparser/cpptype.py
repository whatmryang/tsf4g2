#coding=gbk

from pyparsing import *
from plaincode import *

tType = Forward()
tTemplateType = tIdent + Group(Literal('<') + delimitedList(tType|tInteger) + Literal('>'))
tType << (tTemplateType|OneOrMore(tIdent))

if __name__ == '__main__':
    text = """
        TSimpleControlImpl
		<
		CToolTipBinder, 
		TComponentImpl<IToolTipBinder, CToolTipBinder, COMPONENTID_TOOLTIPBINDER, 1, FALSE>, 
		COMPONENTID_TOOLTIPBINDER>

    """
    print tType.parseString(text)