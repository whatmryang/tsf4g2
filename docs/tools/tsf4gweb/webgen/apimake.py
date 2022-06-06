#coding=gbk

import os
from cppparser.cpp import *
from cppparser.docxy import *
from directory import *
import basetemp

dirs = Directory()

def getPreComment(text, funpos):
    r = text.rfind(r'*/', 0, funpos)
    l = text.rfind(r'/*', 0, funpos)
    if r != -1 and l != -1 and text[r+2:funpos].strip() == '':
        return text[l:r+2]
    else:
        return ''

def makeItem(f, comm):
    head = '<table align=center style="width:840;">'
    tail = '</table><br><br>'
    str = '''        
        <tr>
            <td style =" padding: 5px; width=75; vertical-align:top;">$(td1)</td>
            <td style =" padding: 5px; vertical-align:top;">$(td2)</td>
        </tr>
        '''
    params = ', '.join([i.type()+' '+i.name() for i in f.paramList()])
    fundec = ' '.join([f.retType(), f.name(), '(', params, ')'])
    txt1 = str.replace('$(td1)', '函数原型 : ').replace('$(td2)', fundec)
    txt2 = str.replace('$(td1)', '函数简介 : ').replace('$(td2)', comm.brief)
    txt3 = str.replace('$(td1)', '返 回 值 : ').replace('$(td2)', comm.ret)
    
    txt4 = ''
    for i, p in enumerate(comm.params):
        if i == 0:
            txt = str.replace('$(td1)', '参数说明 : ').replace('$(td2)', p[0] + '  ' + p[1])
        else:
            txt = str.replace('$(td1)', ' 　　　　').replace('$(td2)', p[0] + ' ' + p[1])
        txt4 += txt
    return head + txt1 + txt2 + txt3 + txt4 + tail

def makeForHeader(full, outputpath):
    global frameTemplate
    content = ''
    cppf = Cpp(full)
    gram = cppf.findNamespace('Tenio')
    if not gram:
        print 'error, tenio namespace not found'
        print full
        return
    for i in gram.body():
        if i[0] == 'type_class':
            c = Class(i)
            ins = ''
            for f in c.publicFunDeclares():
                comm = getPreComment(cppf.getText(), f.getPos())
                if comm:
                    docxy = parseDocxy(comm)
                    ins += makeItem(f, docxy)
                    print 'Function with comment :', f.name()
                else:
                    doc = Docxy()
                    doc.brief = '(暂无注释)'
                    ins += makeItem(f, doc)
                    print 'Function without comment :', f.name()
                    print c.name() + '::' + f.name() + ' no comment !!'
            if ins == '':
                ins = '本接口没有定义函数'
                
            ins = ins.decode('gbk')
            ins = '<tr><td style="padding:10px">' + ins
            locs = dirs.GetDirPath(c.name()+'.htm')
            basetemp.replaceAndSave(ins, locs, os.path.join(outputpath, c.name()+'.htm'))
            ins = ''
