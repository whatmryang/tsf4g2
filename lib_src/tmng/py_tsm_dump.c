//============================================================================
// @Id:       $Id: py_tsm_dump.c 10522 2010-07-20 02:22:08Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-07-20 10:22:08 +0800 #$
// @Revision: $Revision: 10522 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <Python.h>
#include <sys/shm.h>

#include "tsm_base.h"


//============================================================================
// 
//----------------------------------------------------------------------------
static PyObject *py_tsm_dump(PyObject *self, PyObject *args)
{
    const char *pShm = NULL;
    int         iShmId;
    int         iSize;

    if (!PyArg_ParseTuple(args, "ii:tsm_dump", &iShmId, &iSize))
    {
        return NULL;
    }

    pShm = (const char *)shmat(iShmId, NULL, 0);

    return Py_BuildValue("s#", pShm, iSize);
}

static PyObject *py_tsm_k2i(PyObject *self, PyObject *args)
{
    int iKey;

    if (!PyArg_ParseTuple(args, "i:tsm_k2i", &iKey))
    {
        return NULL;
    }

    return Py_BuildValue("i", shmget(iKey, 0, 0));
}


//============================================================================
// 
//----------------------------------------------------------------------------
static PyMethodDef TsmDumpMethods[] = {
    { "dump", py_tsm_dump, METH_VARARGS, "返回共享内存内容"  },
    { "k2i",  py_tsm_k2i,  METH_VARARGS, "通过 key取得shmid" },
    { NULL,   NULL,         0,            NULL                }
};


//============================================================================
// 模块初始化
//----------------------------------------------------------------------------
PyMODINIT_FUNC inittsm_dump(void)
{
    PyObject *m = Py_InitModule("tsm_dump", TsmDumpMethods);

    if (NULL == m)
    {
        return;
    }

    PyModule_AddObject(m, "INDEX_KEY", Py_BuildValue("i", TSM_TDR_C_INDEX_KEY));
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
