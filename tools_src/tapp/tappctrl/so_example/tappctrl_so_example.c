#include "tapp/tappctrlapi.h"
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include <string.h>
#include <stdio.h>

typedef struct SOEnv
{
    CIPlugin stCIPlugin;
    const char* pszMetaName;
    LPTDRMETA pstMeta;
    LPTDRMETALIB pstMetaLib;
} SOEnv;

static SOEnv gs_stSOEnv;

static int InitMeta(const char* pszDescFile, const char* pszMetaName)
{
    int iRet = 0;

    iRet = tdr_create_lib_file(&gs_stSOEnv.pstMetaLib, pszDescFile, 0, stderr);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        tlog_error(gs_stSOEnv.stCIPlugin.pstLogCat, 0, 0,
                "tdr_create_lib_file by file<%s> error: <%s>\n", pszDescFile, tdr_error_string(iRet));
        return -1;
    }

    gs_stSOEnv.pstMeta = tdr_get_meta_by_name(gs_stSOEnv.pstMetaLib, pszMetaName);
    if (NULL == gs_stSOEnv.pstMeta)
    {
        tlog_error(gs_stSOEnv.stCIPlugin.pstLogCat, 0, 0,
                "tdr_get_meta_by_name by name<%s> error\n", pszMetaName);
        tdr_free_lib(&gs_stSOEnv.pstMetaLib);
        return -1;
    }

    return iRet;
}

// InitPlugin is necessary
// InitPlugin will be called when tapptappctrl starts
// if InitPlugin return !0, tapptappctrl exits by "exit(1)"
int InitPlugin(CIPlugin* pstCIPlugin)
{
    int iRet = 0;

    // save log instance and callbacker for sending msg
    gs_stSOEnv.stCIPlugin.tappctrl_send = pstCIPlugin->tappctrl_send;
    gs_stSOEnv.stCIPlugin.pstLogCat = pstCIPlugin->pstLogCat;

    tlog_trace(gs_stSOEnv.stCIPlugin.pstLogCat, 0, 0, "come into InitPlugin");

    // do other initializing
    gs_stSOEnv.pszMetaName = "Pkg";
    iRet = InitMeta("desc.xml", gs_stSOEnv.pszMetaName);

    return iRet;
}

// FreePlugin is optional
// FreePlugin will be called before tapptappctrl exits by "tapp_exit_mainloop()"
void FreePlugin()
{
    tlog_trace(gs_stSOEnv.stCIPlugin.pstLogCat, 0, 0, "come into FreePlugin\n");
    // do housekeeping
    tdr_free_lib(&gs_stSOEnv.pstMetaLib);
}

// InputProcess is necessary
// InputProcess will be called when tapptappctrl gets an input that can't be processed by itself
int InputProcess(unsigned short argc, const char** argv)
{
    tlog_trace(gs_stSOEnv.stCIPlugin.pstLogCat, 0, 0, "come into InputProcess\n");
    printf("MSG FROM SO:\nGot an input, but not interested in it.\n");

    return 0;
}

// BinMsgProcess is necessary
// BinMsgProcess will be called when tapptappctrl process 'sendbin' command
int BinMsgProcess(const char* pszMetaName, const char* pBuf, int iBufLen)
{
    TDRDATA stHostData;

    stHostData.pszBuff = (char*)pBuf;
    stHostData.iBuff = iBufLen;

    tlog_trace(gs_stSOEnv.stCIPlugin.pstLogCat, 0, 0, "come into BinMsgProcess\n");

    printf("MSG FROM SO:\nRecv a msg for \'%s\', tdr_fprintf result:\n", pszMetaName);
    if (0 == strcmp(pszMetaName, gs_stSOEnv.pszMetaName))
    {
        if (NULL != gs_stSOEnv.pstMeta)
        {
            tdr_fprintf(gs_stSOEnv.pstMeta, stdout, &stHostData, 0);
        }else
        {
            tlog_trace(gs_stSOEnv.stCIPlugin.pstLogCat, 0, 0, "no meta for %s", pszMetaName);
        }
    }

    return 0;
}
