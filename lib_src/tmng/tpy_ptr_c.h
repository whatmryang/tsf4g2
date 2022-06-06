#ifndef _TPY_PTR_C_H_
#define _TPY_PTR_C_H_
//============================================================================
// @Id:       $Id: tpy_ptr_c.h 10522 2010-07-20 02:22:08Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-07-20 10:22:08 +0800 #$
// @Revision: $Revision: 10522 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdio.h>
#include <string.h>
#include <Python.h>
#include <structmember.h>


//----------------------------------------------------------------------------
// 两个函数提供串和指针之间转换的功能
//----------------------------------------------------------------------------
void *_tpy_s_2_p(const char *s);
int _tpy_p_2_s(const void *p, char *s);

#define TPY_PTR_C_STR(P) char P[sizeof(void *) * 2 + 1]


//----------------------------------------------------------------------------
// TpyPtrC 结构
//----------------------------------------------------------------------------
typedef struct
{
    PyObject_HEAD
    PyObject   *conv;
    PyObject   *name;
    int         stars;
    Py_ssize_t  data;
} TpyPtrC;

#define TPY_PTR_C_DATA(P) ((void *)((void *)0 + (size_t)((TpyPtrC *)P)->data))


//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
PyObject *tpy_ptr_c(PyObject *self, PyObject *args, PyObject *kwargs);


//----------------------------------------------------------------------------
// THE END
//============================================================================
#endif
