//============================================================================
// @Id:       $Id: tpy_ptr_c.c 10522 2010-07-20 02:22:08Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-07-20 10:22:08 +0800 #$
// @Revision: $Revision: 10522 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdio.h>
#include <string.h>
#include "tpy_ptr_c.h"


//----------------------------------------------------------------------------
// 两个函数提供串和指针之间转换的功能
//----------------------------------------------------------------------------
void *_tpy_s_2_p(const char *s)
{
    unsigned char tmp[sizeof(void *)];
    void *p;

    int i;

    if (NULL == s)
    {
        return NULL;
    }

    for (i = 0; i < sizeof(void *) * 2;)
    {
        unsigned char a;
        unsigned char b;

#define PARSE_C(C) \
    do \
    { \
        switch (C) \
        { \
        case 'A' ... 'F': \
            (C) -= 'A' - '\x0a'; \
            break; \
        case '0' ... '9': \
            (C) -= '0'; \
            break; \
        default: \
            return NULL; \
            break; \
        } \
    } \
    while (0)
        a = s[i++];
        PARSE_C(a);
        b = s[i++];
        PARSE_C(b);
#undef PARSE_C

        tmp[sizeof(void *) + 1 - i / 2] = ((a << 4) & 0xf0) | (b & 0x0f);
    }

    memcpy(&p, tmp, sizeof(void *));

    return p;
}

int _tpy_p_2_s(const void *p, char *s)
{
    char c_table[] = "0123456789ABCDEF";
    unsigned char tmp[sizeof(void *)];
    int i;
    int j;

    if (NULL == s)
    {
        return -1;
    }
    s[0] = '\0';

    memcpy(tmp, &p, sizeof(void *));

    for (i = sizeof(void *) - 1, j = 0; i >= 0; --i)
    {
        unsigned char c = tmp[i];

        s[j++] = c_table[c / 16];
        s[j++] = c_table[c % 16];
    }
    s[j] = '\0';

    return 0;
}


//============================================================================
//
//----------------------------------------------------------------------------
static char _tpy_ptr_c__doc__[] = "...";

static struct PyMemberDef _tpy_ptr_c_members[] = {
    { "name",  T_OBJECT,   offsetof(TpyPtrC, name),  RO, "type name"  },
    { "stars", T_INT,      offsetof(TpyPtrC, stars), RO, "star count" },
    { "data",  T_PYSSIZET, offsetof(TpyPtrC, data),  RO, "data ptr"   },
    { NULL }
};

static PyMethodDef _tpy_ptr_c_methods[] = {
    { NULL, NULL }
};

static void _tpy_ptr_c_dealloc(TpyPtrC *self)
{
    PyObject_GC_UnTrack(self);

    /*
    if (self->data)
    {
        free(self->data);
    }
    */

    self->ob_type->tp_free((PyObject *)self);
}

static PyObject *_tpy_ptr_c_getattr(TpyPtrC *self, char *name)
{
    PyObject *res = Py_FindMethod(_tpy_ptr_c_methods, (PyObject *)self, name);

    if (NULL == res)
    {
        PyMemberDef *i;

        PyErr_Clear();

        for (i = _tpy_ptr_c_members; i->name != NULL; ++i)
        {
            if (0 == strcmp(name, i->name))
            {
                return PyMember_GetOne((char *)self, i);
            }
        }
        PyErr_SetString(PyExc_AttributeError, name);
    }

    return res;
}

static int _tpy_ptr_c_setattr(TpyPtrC *self, char *name, PyObject *v)
{
    PyMemberDef *i;

    if (NULL == v)
    {
        PyErr_SetString(PyExc_AttributeError, "Can't delete attributes");
        return -1;
    }

    for (i = _tpy_ptr_c_members; i->name != NULL; ++i)
    {
        if (0 == strcmp(name, i->name))
        {
            return PyMember_SetOne((char *)self, i, v);
        }
    }

    PyErr_SetString(PyExc_AttributeError, name);

    return -1;
}

static PyObject *_tpy_ptr_c_str(TpyPtrC *self)
{
    TPY_PTR_C_STR(str);

    char buf[300];
    char stars[300];
    int  i;

    if (self->stars < 0 || self->stars > 10)
    {
        PyErr_SetString(PyExc_AttributeError, "stars");
        return NULL;
    }

    if (0 == self->stars)
    {
        stars[0] = '\0';
    }
    else
    {
        stars[0] = ' ';
        for (i = 0; i < self->stars; ++i)
        {
            stars[i + 1] = '*';
        }
        stars[i + 1] = '\0';
    }

    _tpy_p_2_s(TPY_PTR_C_DATA(self), str);
    snprintf(buf,
             sizeof(buf),
             "(%s%s)0x%s",
             PyString_AsString(self->name),
             stars,
             str);
    buf[sizeof(buf) - 1] = '\0';

    return PyString_FromString(buf);
}

static PyObject *_tpy_ptr_c_repr(TpyPtrC *self)
{
    char buf[300];

    sprintf(buf, "<TpyPtrC at %p: %p>", (void *)self, TPY_PTR_C_DATA(self));

    return PyString_FromString(buf);
}

static int _tpy_ptr_c_traverse(TpyPtrC *self, visitproc visit, void *arg)
{
    return NULL == self->conv ? 0 : visit(self->conv, arg);
}

static int _tpy_ptr_c_clear(TpyPtrC *self)
{
    if (self->conv)
    {
        Py_XDECREF(self->conv);
        self->conv = NULL;
    }

    return 0;
}

static int _tpy_ptr_c_init(TpyPtrC *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "stars", "data", "conv", NULL };
    const char  *name  = NULL;
    int          stars = -1;
    Py_ssize_t   data  = Py_INVALID_SIZE;
    PyObject    *conv  = NULL;

    self->conv = NULL;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwargs,
                                     "|sinO:init",
                                     kwlist,
                                     &name,
                                     &stars,
                                     &data,
                                     &conv))
    {
        return -1;
    }

    if (conv)
    {
        Py_INCREF(conv);
    }
    else
    {
        conv = PyDict_New();
    }

    if (!conv)
    {
        return -1;
    }

    self->conv  = conv;
    self->name  = Py_BuildValue("s", name);
    self->stars = stars;
    self->data  = data;

    return 0;
}

PyTypeObject _tpy_ptr_c_Type = {
    PyObject_HEAD_INIT(NULL)
    0,
    "_tpy_ptr_c",                             // (char *)tp_name for printing
    sizeof(TpyPtrC),
    0,
    (destructor)_tpy_ptr_c_dealloc,           // tp_dealloc
    0,                                        // tp_print
    (getattrfunc)_tpy_ptr_c_getattr,          // tp_getattr
    (setattrfunc)_tpy_ptr_c_setattr,          // tp_setattr
    0,                                        // tp_compare
    (reprfunc)_tpy_ptr_c_repr,                // tp_repr

    0,                                        // tp_as_number
    0,                                        // tp_as_sequence
    0,                                        // tp_as_mapping

    0,                                        // (hashfunc)tp_hash
    0,                                        // (ternaryfunc)tp_call
    (reprfunc)_tpy_ptr_c_str,                 // (reprfunc)tp_str
    0,                                        // (getattrofunc) tp_getattro
    0,                                        // (setattrofunc)tp_setattro

    0,                                        // (PyBufferProcs *)tp_as_buffer

    // Flags to define presence of optional/expanded features
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE,

    _tpy_ptr_c__doc__,                        // (char *)tp_doc

    (traverseproc)_tpy_ptr_c_traverse,        // tp_traverse

    (inquiry)_tpy_ptr_c_clear,                // tp_clear

    0,                                        // (richcmpfunc)tp_richcompare

    0,                                        // (long)tp_weaklistoffset

    0,                                        // (getiterfunc)tp_iter
    0,                                        // (iternextfunc)tp_iternext

    (struct PyMethodDef *)_tpy_ptr_c_methods, // tp_methods
    (struct PyMemberDef *)_tpy_ptr_c_members, // tp_members
    0,                                        // (struct getsetlist *)tp_getset
    0,                                        // (struct _tpyeobject *)tp_base
    0,                                        // (PyObject *)tp_dict
    0,                                        // (descrgetfunc)tp_descr_get
    0,                                        // (descrsetfunc)tp_descr_set
    0,                                        // (long)tp_dictoffset
    (initproc)_tpy_ptr_c_init,                // tp_init
    NULL,                                     // tp_alloc
    NULL,                                     // tp_new
    NULL,                                     // tp_free
    0,                                        // (PyObject *)tp_bases
    0,                                        // (PyObject *)tp_mro
    0                                         // (PyObject *)tp_defined
};


//============================================================================
// 
//----------------------------------------------------------------------------
static PyObject *tpy_ptr_c_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    TpyPtrC *r = (TpyPtrC *)_tpy_ptr_c_Type.tp_alloc(&_tpy_ptr_c_Type, 0);

    if (r && _tpy_ptr_c_init(r, args, kwargs))
    {
        Py_DECREF(r);
        r = NULL;
    }

    return (PyObject *)r;
} 

static char typ_ptr_c_new__doc__[] = "create tpy_ptr_c object:\n\n"
    "\tnew(str type_name, int star_count, long ptr_value)";


static PyMethodDef tpy_ptr_c_methods[] = {
    {
        "new",
        (PyCFunction)tpy_ptr_c_new,
        METH_VARARGS | METH_KEYWORDS,
        typ_ptr_c_new__doc__
    },
    { NULL, NULL }
};

static char tpy_ptr_c__doc__[] = "...";


//============================================================================
// 
//----------------------------------------------------------------------------
DL_EXPORT(void) init_tpy_ptr_c(void)
{
    PyObject *dict;
    PyObject *module;

    module = Py_InitModule4("_tpy_ptr_c",
                            tpy_ptr_c_methods,
                            tpy_ptr_c__doc__,
                            (PyObject *)NULL,
                            PYTHON_API_VERSION); 
    if (NULL == module)
    {
        return;
    }

    _tpy_ptr_c_Type.ob_type  = &PyType_Type;
    _tpy_ptr_c_Type.tp_alloc = PyType_GenericAlloc;
    _tpy_ptr_c_Type.tp_new   = PyType_GenericNew;
    _tpy_ptr_c_Type.tp_free  = _PyObject_GC_Del;

    dict = PyModule_GetDict(module);
    if (NULL == dict)
    {
        goto error;
    }

    if (PyDict_SetItemString(dict, "__version__", PyString_FromString("1.0")))
    {
        goto error;
    }

    Py_INCREF(&_tpy_ptr_c_Type);

error:
    if (PyErr_Occurred())
    {
        PyErr_SetString(PyExc_ImportError, "_tpy_ptr_c: init failed");
    }

    return;
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
