#!/usr/bin/env python
# -*- coding: utf-8 -*-
##============================================================================
## @Id:       $Id: tpy_c.py 10522 2010-07-20 02:22:08Z tomaswang $
## @Author:   $Author: tomaswang $
## @Date:     $Date: 2010-07-20 10:22:08 +0800 (星期二, 2010-07-20) $
## @Revision: $Revision: 10522 $
## @HeadURL:  $HeadURL: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk/lib_src/tmng/tpy_c.py $
##----------------------------------------------------------------------------


import os

from UserList import UserList

import _tpy_ptr_c


##----------------------------------------------------------------------------
## 
##----------------------------------------------------------------------------
class tpy_c_list(UserList):

    def __init__(self, *args, **argv):
        UserList.__init__(self, *args, **argv)

    def __call__(self, data):
        self.data.append(data)


class tpy_c:

    _ignore = {
        'IN':       '',
        'OUT':      '',
        'static':   '',
        }
    _ctypes = {
        'int':      'i',
        'size_t':   'n',
        'char':     '',
        'signed':   '',
        'unsigned': '',
        'const':    '',
        'void':     '',
        '*':        '',
        '(':        '',
        ')':        '',
        '[':        '',
        ']':        '',
        }

    def __init__(self, pname=''):
        self.pname = pname
        self.clear()

    def clear(self):
        self.option = ''
        self.macro = []
        self.heads = []
        self.funcs = []
        self._init_types()
        self.add_head('')
        self.add_head('<Python.h>')
        self.add_head('tpy_ptr_c.h')
        self.add_head('')

    def _init_types(self):
        self.types = {}
        for k, v in self._ignore.items():
            self.types[k] = v
        for k, v in self._ctypes.items():
            if self.types.has_key(k):
                raise
            self.types[k] = v

    def set_name(self, name):
        self.pname = name

    def set_option(self, opt):
        self.option = opt

    def add_macro(self, name, value):
        self.macro.append((name, value))

    def add_head(self, head=''):
        if '' == head:
            pass
        elif head[0] == '<' and head[-1] == '>':
            pass
        elif head[0] == head[-1] == '"':
            pass
        else:
            head = '"' + head + '"'
        self.heads.append(head)

    def add_func(self, doc, rtype, func, *args):
        r = ' '.join(rtype.split()).replace(' *', '*').replace('* ', '*')
        rl = len(r)
        r = r.rstrip('*')

        pargs = []
        for i in args:
            assert i
            s = ' '.join(i.split()).replace(' *', '*').replace('* ', '*')
            p = s.rfind('*') + 1
            if p:
                v = s[p:]
                s = s[:p]
                l = len(s)
                s = s.rstrip('*')
                pargs.append((s, l - len(s), v))
            else:
                s = s.split()
                v = s.pop()
                pargs.append((' '.join(s), 0, v))
        self.funcs.append((doc, (r, rl - len(r)), func, tuple(pargs)))

    def add_type(self, k, v=''):
        if self.types.has_key(k):
            raise
        self.types[k] = v

    def dump_var_define(self, r, *args):
        tmp = []
        tl = 0
        s  = 0
        for i, (t, n, v) in zip(range(len(args)), args):
            if n > 0:
                t = 'TpyPtrC'
                s = 1
            if len(t) > tl:
                tl = len(t)
            tmp.append((i + 1, t, n and 1 or 0))
        t, n = r
        if t != 'void' or n != 0:
            if n > 0:
                t = 'PyObject'
                s = 1
            if len(t) > tl:
                tl = len(t)
            tmp.insert(0, (0, t, n and 1 or 0))

        ret = tpy_c_list()
        for i, t, n in tmp:
            ret('%s%s %sarg%d;' % (
                t,
                ' ' * (tl - len(t)),
                s and (n and '*' or ' ') or '',
                i))
        return ret

    def dump_var_flags(self, *args):
        ret = tpy_c_list()
        for t, s, v in args:
            ret(s and 'O' or self.types[t])
        return ret

    def dump_var_address(self, *args):
        ret = tpy_c_list()
        for i in range(len(args)):
            ret('arg%d' % (i + 1,))
        return ret

    def dump_var_values(self, *args):
        ret = tpy_c_list()
        for t, v in args:
            ret(v)
        return ret

    def dump_funcs(self, _i, d, r, f, *a):
        ret = tpy_c_list()
        ret('static char tpyS_%s__doc__[] = ' % f)
        ret('%s"\\nprototype:\\n%s%s %s%s(\\n"' % (
            _i,
            _i,
            r[0],
            r[1] and '*' * r[1] or '',
            f))
        tl = 0
        sl = 0
        for t, s, v in a:
            if len(t) > tl:
                tl = len(t)
            if s > sl:
                sl = s
        for i, (t, s, v) in zip(range(len(a)), a):
            ret('%s"%s%s%s %s%s%s%s"' % (
                _i,
                _i * 2,
                t,
                ' ' * (tl - len(t)),
                ' ' * (sl - s),
                '*' * s,
                v,
                i == len(a) - 1 and ');\\n\\n' or ',\\n'))
        ret('%s"%s";' % (_i, self.quote_doc(d)))
        ret('')
        ret('static PyObject *tpyF_' + \
                f + \
                '(PyObject *self, PyObject *args)')
        ret('{')

        X = self.dump_var_define(r, *a)
        if X:
            for i in X:
                ret(_i + i)
            ret('')

        ret('%sif (!PyArg_ParseTuple(args,' % _i)
        if a:
            ret('%s                      "%s:func(%s)",' % (
                _i,
                ''.join(self.dump_var_flags(*a)),
                f))

            X = self.dump_var_address(*a)
            for i in X[:-1]:
                ret('%s                      &%s,' % (_i, i))
            ret('%s                      &%s))' % (_i, X[-1]))
        else:
            ret('%s                      ":func(%s)"))' % (_i, f))
        ret('%s{' % _i)
        ret('%s%sreturn NULL;' % (_i, _i))
        ret('%s}' % _i)
        ret('')

        for i, (t, s, v) in zip(range(len(a)), a):
            if s:
                ret('')
                ret('%sif (%d != arg%d->stars)' % (_i, s, i + 1))
                ret('%s{' % _i)
                ret('%s%sreturn NULL;' % (_i, _i))
                ret('%s}' % _i)
                ret('%selse' % _i)
                ret('%s{' % _i)
                ret('%sconst char *n = NULL;' % (_i * 2,))
                ret('%sconst char *a = "%s";' % (_i * 2, t))
                ret('')
                ret('%sif (!PyArg_Parse(arg%d->name, "s:->%d", &n))' % (
                    _i * 2,
                    i + 1,
                    i + 1))
                ret('%s{' % (_i * 2,))
                ret('%sreturn NULL;' % (_i * 3,))
                ret('%s}' % (_i * 2,))
                ret('')
                ret('%sif (!strncmp(n, a, sizeof(a)))' % (_i * 2,))
                ret('%s{' % (_i * 2,))
                ret('%sreturn NULL;' % (_i * 3,))
                ret('%s}' % (_i * 2,))
                ret('%s}' % _i)
                ret('')

        if r[1]:
            ret('%sarg0 = Py_BuildValue("sin", "%s", %d, %s(%s' % (
                _i,
                r[0],
                r[1],
                f,
                (not a) and '));' or ''))
        elif 'void' == r[0]:
            ret('%s%s(%s' % (_i, f, (not a) and ');' or ''))
        else:
            ret('%sarg0 = %s(%s' % (_i, f, (not a) and ');' or ''))

        for i in range(len(a)):
            ret('%s%sarg%d%s%s' % (
                _i * 2,
                a[i][1] and ('(%s %s)' % (a[i][0], '*' * a[i][1])) or '',
                i + 1,
                a[i][1] and '->data' or '',
                i != len(a) - 1 and ',' or ''))
        if a:
            ret('%s%s);' % (_i, r[1] and ')' or ''))

        ret('')
 
        if r[1]:
            ret('%sif (NULL == arg0)' % _i)
            ret('%s{' % _i)
            ret('%sreturn NULL;' % (_i * 2,))
            ret('%s}' % _i)
            ret('')
            ret('%sreturn (PyObject *)arg0;' % _i)
        elif 'void' == r[0]:
            ret('%sPy_INCREF(Py_None);' % _i)
            ret('%sreturn Py_None;' % _i)
        else:
            ret('%sreturn Py_BuildValue("%s", arg0);' % (
                _i,
                self.types[r[0]]))
 
        ret('}')
        ret('')
        return ret

    _quote_table = (
        ('\\', '\\\\'),
        ('\n', '\\n'),
        ('\r', '\\r'),
        ('\t', '\\t'),
        ('"',  '\\"'))
    def quote_doc(self, doc):
        for s, t in self._quote_table:
            doc = doc.replace(s, t)
        return doc

    def dump(self, _i_len=4):
        _i = ' ' * _i_len
        ret = tpy_c_list()

        for i in self.heads:
            ret(i and ('#include ' + i) or '')
        ret('')
        ret('')

        for d, r, f, a in self.funcs:
            ret.extend(self.dump_funcs(_i, d, r, f, *a))

        ret('static PyMethodDef tpyM_%sMethods[] = {' % self.pname)
        for d, r, f, a in self.funcs:
            ret('%s{ "%s", tpyF_%s, METH_VARARGS, tpyS_%s__doc__ },' % (
                _i,
                f,
                f,
                f))
        ret('%s{ NULL, NULL, 0, NULL }' % _i)
        ret('};')
        ret('')

        ret('PyMODINIT_FUNC init%s(void)' % self.pname)
        ret('{')
        ret('%sPyObject *m = Py_InitModule("%s", tpyM_%sMethods);' % (
            _i,
            self.pname,
            self.pname))
        ret('%s(void)m;' % _i)
        ret('}')
        ret('')

        return '\n'.join(ret)

    def compile(self):
        open('py_%s.c' % self.pname, 'w').write(self.dump())
        os.system('gcc -Wall -fPIC -shared -o %s.so py_%s.c %s %s' % (
            self.pname,
            self.pname,
            '-I /usr/local/include/python2.6',
            self.option))
        os.system('rm -f py_%s.c' % self.pname)

 
if __name__ == "__main__":
    obj = tpy_c('a')
    obj.add_head('<stdio.h>')
    obj.add_head('<sys/shm.h>')
    obj.add_func('func: shmat',
                 'const char *',
                 'shmat', 
                 'int shmid',
                 'const void *shmaddr',
                 'int shmflg')
    obj.add_func('func: sync',
                 'void',
                 'sync')
    #print obj.dump()
    obj.compile()


##----------------------------------------------------------------------------
## THE END
##============================================================================
