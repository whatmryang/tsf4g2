#include "mtexample.h"
#include "mtexample_control.h"

// when tapp recv a "help" msg, it will call this interface to process it
const char* mtexample_console_help(void)
{
    static const char* pszHelp;

    //tlog_info(gs_stEnv.pstLogCat, 0, 0, "request for help info");

    pszHelp = "This is an demo.\n"
        "All recved binary msgs will be send back.\n"
        "No cmdline msgs are supported.\n";

    return pszHelp;
}

int mtexample_proc_cmdline_req(TAPPCTX *pstAppCtx, void *pvData, unsigned short argc, const char** argv)
{
    int i = 0;
	LPMTEXAMPLEENV pstEnv = pvData;
    tlog_info(pstEnv->pstLogCat, 0, 0, "recv cmdline msg: argc<%d>", argc);
    printf("recv cmd line msg: ");
    tappctrl_send_string("Error: unsupported command");
    for (i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    return 0;
}

int mtexample_inject_msg()
{
	return 0;
}

int mtexample_deal_inject_res()
{
	return 0;
}


int mtexample_proc_binary_res(const char* pszMetaName, const char* pBuf, int tLen)
{
    int iRet = 0;

    printf("recv response for meta: %s\n", pszMetaName);
    //tlog_error(gs_stEnv.pstLogCat, 0, 0, "recv response for meta: %s", pszMetaName);

    return iRet;
}


