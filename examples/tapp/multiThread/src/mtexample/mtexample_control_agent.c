#include "mtexample_control_agent.h"


int example_controller_fini(TAPPCTX *pstAppCtx, void *argv)
{
    int iRet = 0;

    LPMTEXAMPLEENV pstEnv = (LPMTEXAMPLEENV)argv;
    tlog_error(pstEnv->pstLogCat, 0, 0, "example controller fini do nothing");
    return iRet;
}


int mtexample_proc_stdin(unsigned short argc, const char** argv)
{
    //tlog_error(gs_stEnv.pstLogCat, 0, 0, "recv command: %s, but not interested in it", argv[0]);
    return 0;
}

int mtexample_controller_init(TAPPCTX *pstAppCtx, void *argv)
{
    int iRet = 0;

    LPMTEXAMPLEENV pstEnv = (LPMTEXAMPLEENV)argv;
	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (void *)(&pstEnv->pstLogCat)))	
	{		
		printf("tapp_get_category run fail\n");
		return -1;	
	}	

    tlog_error(pstEnv->pstLogCat, 0, 0, "example controller init do nothing");

    return iRet;
}


