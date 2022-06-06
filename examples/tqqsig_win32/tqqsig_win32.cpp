// tqqsig_win32.cpp : Defines the entry point for the console application.
//
#include "tqqsig/tqqsig_api.h"
#include <winsock2.h>
#include <stdio.h>
#include <time.h>


#define CONFPATHLEN 128
#define LOOPCOUNT 3

struct tagTQQSIGENV
{
     char *pszConfilePath;
     TQQSIGCHECKLIST stSigInfo;
     TQQConnInfo          stConnInfo;
     LPTQQSIGCTX            pstSigCtx;	 
};


typedef struct tagTQQSIGENV TQQSIGENV;
static TQQSIGENV  gs_QQSigEnv;

int main(int argc, char* argv[])
{
	int iRet=0;
	int i=0;
    if(argc != 2)
	{
	   printf("Error:need to specify conf path\n");
	   return -1;
	}
	printf("conf path=[%s]\n",argv[1]);
	gs_QQSigEnv.pszConfilePath = argv[1];

    iRet = tsigapi_init(&gs_QQSigEnv.pstSigCtx,gs_QQSigEnv.pszConfilePath);
	if(iRet != 0)
	{
	   printf("Error:fail to init tsigapi!iRet=[%d]\n",iRet);
	   return -1;
	}

	while( i < LOOPCOUNT )
	{
		/*模拟从当前连接中获取信息*/
		gs_QQSigEnv.stConnInfo.uin = 10000;
		gs_QQSigEnv.stConnInfo.uCltIP = ntohl(inet_addr("10.1.1.2"));
		gs_QQSigEnv.stConnInfo.uAppID = 20001;
		gs_QQSigEnv.stConnInfo.uCltVer = 1;
		
		/*模拟从签名中获取信息*/	
		gs_QQSigEnv.stSigInfo.uin = 10001;
		gs_QQSigEnv.stSigInfo.utimestamp = time(NULL);
		gs_QQSigEnv.stSigInfo.uCltIP = ntohl(inet_addr("10.1.1.1"));
		gs_QQSigEnv.stSigInfo.uAppID = 20001;
		gs_QQSigEnv.stSigInfo.uCltVer = 1;
		
		iRet = tsigapi_check(gs_QQSigEnv.pstSigCtx, &gs_QQSigEnv.stConnInfo, &gs_QQSigEnv.stSigInfo);
		if( iRet < 0)
		{
			printf("Error:check siginfo unpassed:iRet=[%d]\n",iRet);
		}
		i++;
	}

	tsigapi_fini(&gs_QQSigEnv.pstSigCtx);
	printf("done!\n");
	return 0;
}

