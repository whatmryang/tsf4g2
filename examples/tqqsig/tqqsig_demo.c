#include "tqqsig/tqqsig_api.h"
#include "tapp/tapp.h"





#define CONFPATHLEN 128


struct tagTQQSIGENV
{
     char *pszConfilePath;
     TQQSIGCHECKLIST stSigInfo;
     TQQConnInfo          stConnInfo;
     LPTQQSIGCTX            pstSigCtx;	 
};


typedef struct tagTQQSIGENV TQQSIGENV;
typedef struct tagTQQSIGENV *LPQQSigEnv;


static TQQSIGENV  gs_QQSigEnv;
static TAPPCTX      gs_QQSigCtx;


int  tqqsig_def_opt(LPTAPPCTX pstCtx,LPQQSigEnv pstEnv)
{
       int opt;
	int iOldOptErr;
	int iRet=0;
	static struct option s_astLongOptions[] = {
		{"conf-file", 1, NULL, 'c'},
		{0, 0, 0, 0}
	};


	assert(0 < pstCtx->argc);
	assert(NULL != pstCtx->argv);

	iOldOptErr	=	opterr;
	opterr	=	0;	
	while (1)
	{
		int option_index = 0;
		
		opt = getopt_long (pstCtx->argc,pstCtx->argv, "c:",s_astLongOptions, &option_index);

		if (opt == -1)
		{
			break;
		}
		
		switch(opt)
		{
		case 'c':
			pstEnv->pszConfilePath= optarg;
			break;
		default:
			break;
		}/*switch(opt)*/
	}/*while (1)*/


	/* restore the getopt environment. */	opterr	=	iOldOptErr;
	optarg	=	NULL;	
	optind	=	1;	
	optopt	=	'?';

       return 0;

}




int tqqsig_init(TAPPCTX* pstAppCtx, LPQQSigEnv pstEnv)
{
    int iRet=0;


    iRet = tsigapi_init(&pstEnv->pstSigCtx,pstEnv->pszConfilePath);
    if( iRet < 0)
    {
         printf("Error:fail to init tsigapi:conf=%s,iRet=%d\n",pstEnv->pszConfilePath,iRet);
         return -1;
    }

    printf("start success\n");	

    return 0;	
}

int tqqsig_reload(TAPPCTX* pstAppCtx, LPQQSigEnv pstEnv)
{
   int iRet=0;
   iRet = tsigapi_reload(pstEnv->pstSigCtx,pstEnv->pszConfilePath);
   if( iRet < 0)
    {
         printf("Error:fail to reload tsigapi:conf=%s,iRet=%d\n",pstEnv->pszConfilePath,iRet);
         return -1;
    }

    printf("reload success\n");	
    return 0;	
}

int tqqsig_fini(TAPPCTX* pstAppCtx, LPQQSigEnv pstEnv)
{
    tsigapi_fini(&pstEnv->pstSigCtx);
    return 0;
}


int tqqsig_proc(TAPPCTX* pstAppCtx, LPQQSigEnv pstEnv)
{
   int iRet=0;
    /*模拟从当前连接中获取信息*/
    pstEnv->stConnInfo.uin = 10000;
    pstEnv->stConnInfo.uCltIP = NTOHL(inet_addr("10.1.1.2"));
    pstEnv->stConnInfo.uAppID = 20001;
    pstEnv->stConnInfo.uCltVer = 1;

    /*模拟从签名中获取信息*/	
    pstEnv->stSigInfo.uin = 10000;
    pstEnv->stSigInfo.utimestamp = time(NULL);
    pstEnv->stSigInfo.uCltIP = NTOHL(inet_addr("10.1.1.1"));
    pstEnv->stSigInfo.uAppID = 20001;
    pstEnv->stSigInfo.uCltVer = 1;

    iRet = tsigapi_check(pstEnv->pstSigCtx, &pstEnv->stConnInfo, &pstEnv->stSigInfo);
    if( iRet < 0)
    {
        printf("Error:check siginfo unpassed:iRet=%d\n",iRet);
    }
    sleep(1);	

     return -1;	
	
}






  

int main(int argc, char* argv[])
{
       int iRet;
	void *pvArg = &gs_QQSigEnv;

	memset(&gs_QQSigEnv, 0, sizeof(gs_QQSigEnv));
	memset(&gs_QQSigCtx, 0, sizeof(gs_QQSigCtx));
	
	gs_QQSigCtx.argc	=	argc;
	gs_QQSigCtx.argv	=	argv;
	gs_QQSigCtx.iTimer	=	1000;
	gs_QQSigCtx.iNoLoadConf = 1;
	
	gs_QQSigCtx.pfnInit	=	(PFNTAPPFUNC)tqqsig_init;
	gs_QQSigCtx.pfnFini	=	(PFNTAPPFUNC)tqqsig_fini;
	gs_QQSigCtx.pfnProc	=	(PFNTAPPFUNC)tqqsig_proc;
	gs_QQSigCtx.pfnReload =   (PFNTAPPFUNC)tqqsig_reload; 

	
       tqqsig_def_opt(&gs_QQSigCtx, pvArg);

	iRet = tapp_def_init(&gs_QQSigCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: tapp_def_init failed, ret: %d\n", iRet);
		return iRet;
	}

	iRet = tapp_def_mainloop(&gs_QQSigCtx, pvArg);

	tapp_def_fini(&gs_QQSigCtx, pvArg);

	printf("stop success\n");	

	return iRet;    



}





