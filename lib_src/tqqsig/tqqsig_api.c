#include "tqqsig/tqqsig_api.h"
#include "tqqsig/tini_api.h"

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


#if defined (_WIN32) || defined (_WIN64)
    #include <io.h>
    #define F_OK  0	
    #define access(path, mode)   _access(path, mode)
#else   
    #include <unistd.h>
#endif


#if defined (_WIN32) || defined (_WIN64)
     #define snprintf _snprintf
#endif
    
#if defined (_WIN32) || defined (_WIN64)
     #include <direct.h>
     #define mkdir(path, mode)			_mkdir(path)
#else
     #include <sys/stat.h>
     #include <sys/types.h>
     #define LOG_DIR_MODE (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP| S_IXGRP | S_IROTH | S_IXOTH)
#endif


#if defined (_WIN32) || defined (_WIN64)
         
	#define	DIRSEP				'\\'
#else
	#define	DIRSEP				'/'
#endif 

#if defined (_WIN32) || defined (_WIN64)
struct tm* localtime_r(const time_t *timep, struct tm *result)
{
	struct tm* pstRet;

	pstRet	=	localtime(timep);

	if( !pstRet )
		return NULL;

	memcpy(result, pstRet, sizeof(*result));

	return result;
}
#endif 



#define FILE_MAX_LEN 1024
#define LOG_MAX_LEN  4096
#define IPV4_NETCOUNT 32


struct tagTQQSigCtx
{
	 int iCheckUin;    //必选项

	  int iCheckSigTime; //必选项
	 int iTimeOutValue;
	 
	 int iCheckCltIP;   //必选项
	 int iAllowTimeGap; 
	 int iNetMaskBitCount;
	 
	 int iCheckServiceID;
	 int iCheckCltVer;
	 
	 
	 /*日志配置*/
	 char szLogDIR[FILE_MAX_LEN];
	 char szLogCurDIR[FILE_MAX_LEN];
	 char szLogCurFile[FILE_MAX_LEN];
	 FILE *fp;
};


int tlogapi_openlog(IN TQQSIGCTX *pSigCtx,const char* szLogDir)
{
	int iRet=0;

	if(access(szLogDir,F_OK) < 0)
	{
		iRet = mkdir(szLogDir, LOG_DIR_MODE);
		if( iRet < 0)
		{
		    return -1;
		}
	}
	strncpy(pSigCtx->szLogDIR,szLogDir,FILE_MAX_LEN);
	pSigCtx->szLogCurDIR[0]=0;
	pSigCtx->szLogCurFile[0]=0;
	pSigCtx->fp=NULL;

	return 0;
		
}

int tlogapi_logfile(IN TQQSIGCTX *pSigCtx,IN unsigned int dwuin, IN unsigned int dwtype, IN unsigned int iConnInfo,IN unsigned int iSigInfo,IN unsigned int dwReason)
{
	
	char szLogFilePath[FILE_MAX_LEN];
	char szTmp[LOG_MAX_LEN];
	struct tm  stDate;
	struct tm *pstDate;
	time_t tCur;

	char szTime[128];
	
	time(&tCur);
	localtime_r(&tCur,&stDate);

	pstDate = &stDate;
	
	strftime(szTmp,sizeof(szTmp),"%Y%m%d",pstDate);
	
	if( pSigCtx->szLogCurDIR[0] == 0 )
	{
	    strncpy(pSigCtx->szLogCurDIR,szTmp,FILE_MAX_LEN);	
	}
	
	/*切换日期目录*/
	if( strncmp(pSigCtx->szLogCurDIR,szTmp,FILE_MAX_LEN))
	{
		 strncpy(pSigCtx->szLogCurDIR,szTmp,FILE_MAX_LEN);
		 if( pSigCtx->fp )
		 {
		    fclose(pSigCtx->fp);	
		    pSigCtx->fp = NULL;
		 }	
	}
	
	
	snprintf(szLogFilePath,sizeof(szLogFilePath),"%s%c%s",pSigCtx->szLogDIR,DIRSEP,pSigCtx->szLogCurDIR);
	if( access(szLogFilePath,F_OK) < 0 )
	{
	    if( 0 > mkdir(szLogFilePath,LOG_DIR_MODE))
	    {
	       printf("Error:fail to create dir[%s],errorstring=%s\n",szLogFilePath,strerror(errno));
	       return -1;	
	    } 	
	}
	
	/*切换日期文件*/
	strftime(szTmp,sizeof(szTmp),"%Y%m%d%H",pstDate);
	
	if( pSigCtx->szLogCurFile[0] == 0 )
	{
		strncpy(pSigCtx->szLogCurFile,szTmp,FILE_MAX_LEN);	
	}
	
	if(strncmp(pSigCtx->szLogCurFile,szTmp,FILE_MAX_LEN))
	{
	       strncpy(pSigCtx->szLogCurFile,szTmp,FILE_MAX_LEN);
	       if( pSigCtx->fp )
	       {
	            fclose(pSigCtx->fp);
	            pSigCtx->fp = NULL;
	       }	
	}
	
	if( pSigCtx->fp == NULL )
	{
	      snprintf(szLogFilePath,sizeof(szLogFilePath),"%s%c%s%c%s",pSigCtx->szLogDIR,DIRSEP,pSigCtx->szLogCurDIR,DIRSEP,pSigCtx->szLogCurFile);
	       pSigCtx->fp = fopen(szLogFilePath,"a+");
	      if( pSigCtx->fp == NULL )
	      {
	           printf("Error:fail to open file[%s],errorstring=%s\n",szLogFilePath,strerror(errno));
	           return -1;	
	      }	
	}
	
	/*Log file*/
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + pstDate->tm_year,
			   pstDate->tm_mon + 1, pstDate->tm_mday, pstDate->tm_hour, pstDate->tm_min, pstDate->tm_sec );
	if( dwReason != TQQSIG_REASON_NONE)
	{
		snprintf(szTmp,sizeof(szTmp),"%s | %u | %u | %u | %u | %u \n",szTime,dwuin,dwtype,iConnInfo,iSigInfo,dwReason);
	}
	else
	{
		snprintf(szTmp,sizeof(szTmp),"%s | %u | %u | %u | %u \n",szTime,dwuin,dwtype,iConnInfo,iSigInfo);
	}		   
	
	fputs(szTmp,pSigCtx->fp);
	fflush(pSigCtx->fp);
	
	return 0;
}


int tlogapi_closelog(IN TQQSIGCTX *pSigCtx)
{

	if( NULL != pSigCtx->fp )
	{
           fclose(pSigCtx->fp);
	    pSigCtx->fp = NULL;	   
	}

	return 0;


}

int tsigapi_init(IN LPTQQSIGCTX *ppSigCtx,IN const char *szConfilePath)
{
	int iRet=0;
	LPTQQSIGCTX pstCtx=NULL;
	char szTemp[128];
	
	if( !szConfilePath || !ppSigCtx)
	{
	   return -1;		
	}

	pstCtx =(LPTQQSIGCTX)calloc(1,sizeof(TQQSIGCTX));

	if( !pstCtx)
	{
           return -12;

	}

	*ppSigCtx = pstCtx;

	

	
	
	iRet = tiniapi_read(szConfilePath,"General","CheckSigTime",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
	   printf("Error:fail to read CheckSigTime[%d] in conf[%s]\n",iRet,szConfilePath);
	   return -2;			
	} 
	
	pstCtx->iCheckSigTime = atoi(szTemp);
	if( pstCtx->iCheckSigTime == 1 )
	{
	   iRet = tiniapi_read(szConfilePath,"General","TimeOutValue",szTemp,sizeof(szTemp));	
	   if( iRet < 0)
	   {
	      printf("Error:fail to read TimeOutValue[%d] in conf[%s]\n",iRet,szConfilePath);
	      return -3;	
	   }		
	   pstCtx->iTimeOutValue = atoi(szTemp);   
	}
	
	iRet = tiniapi_read(szConfilePath,"General","CheckUin",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read CheckUin[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -4;	
	}
	pstCtx->iCheckUin = atoi(szTemp);
	
	iRet = tiniapi_read(szConfilePath,"General","CheckCltIP",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read CheckCltIP[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -5;	
	
	}
	pstCtx->iCheckCltIP = atoi(szTemp);
	
	if( pstCtx->iCheckCltIP == 1 )
	{
	    iRet = tiniapi_read(szConfilePath,"General","AllowedTimeGap",szTemp,sizeof(szTemp));
	    if( iRet < 0 )
	    {
		      printf("Error:fail to read AllowedTimeGap[%d] in conf[%s]\n",iRet,szConfilePath);
		      return -6;	
	    }

	    pstCtx->iAllowTimeGap = atoi(szTemp);	

	    iRet = tiniapi_read(szConfilePath,"General","NetMaskBitCount",szTemp,sizeof(szTemp));
	    if( iRet < 0 )
	    {
		      printf("Error:fail to read NetMaskBitCount[%d] in conf[%s]\n",iRet,szConfilePath);
		      return -7;	

	    }
	    pstCtx->iNetMaskBitCount = atoi(szTemp);
	    if(pstCtx->iNetMaskBitCount<=0
		||pstCtx->iNetMaskBitCount>32)
	    {

	          pstCtx->iNetMaskBitCount = 24;
	    }		
    	}
     
       
   
 
	iRet = tiniapi_read(szConfilePath,"General","CheckServiceID",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read CheckServiceID[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -8;	
	}
	pstCtx->iCheckServiceID = atoi(szTemp);

	iRet = tiniapi_read(szConfilePath,"General","CheckCltver",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read CheckCltver[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -9;	
	}
	pstCtx->iCheckCltVer = atoi(szTemp);

	iRet = tiniapi_read(szConfilePath,"General","logdir",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read logdir[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -10;	
	}
	 
	iRet =  tlogapi_openlog(pstCtx,szTemp);
	if( iRet < 0 )
	{
		  printf("Error:fail to open logdir[%s],errorstrint=%s\n",szTemp,strerror(errno));	
		  return -11;		
	}
	
	return 0;
}



int tsigapi_reload(IN TQQSIGCTX *pSigCtx,IN const char *szConfilePath)
{
    
	int iRet=0;
	LPTQQSIGCTX pstCtx=NULL;
	
	char szTemp[128];
	
	if( !szConfilePath || !pSigCtx)
	{
	   return -1;		
	}

       iRet = tiniapi_read(szConfilePath,"General","CheckSigTime",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
	   printf("Error:fail to read CheckSigTime[%d] in conf[%s]\n",iRet,szConfilePath);
	   return -2;			
	} 
	pstCtx = pSigCtx;
	
	pstCtx->iCheckSigTime = atoi(szTemp);
	if( pstCtx->iCheckSigTime == 1 )
	{
	   iRet = tiniapi_read(szConfilePath,"General","TimeOutValue",szTemp,sizeof(szTemp));	
	   if( iRet < 0)
	   {
	      printf("Error:fail to read TimeOutValue[%d] in conf[%s]\n",iRet,szConfilePath);
	      return -3;	
	   }		
	   pstCtx->iTimeOutValue = atoi(szTemp);   
	}
	
	iRet = tiniapi_read(szConfilePath,"General","CheckUin",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read CheckUin[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -4;	
	}
	pstCtx->iCheckUin = atoi(szTemp);
	
	iRet = tiniapi_read(szConfilePath,"General","CheckCltIP",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read CheckCltIP[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -5;	
	
	}
	pstCtx->iCheckCltIP = atoi(szTemp);
	
	if( pstCtx->iCheckCltIP == 1 )
	{
	    iRet = tiniapi_read(szConfilePath,"General","AllowedTimeGap",szTemp,sizeof(szTemp));
	    if( iRet < 0 )
	    {
		      printf("Error:fail to read AllowedTimeGap[%d] in conf[%s]\n",iRet,szConfilePath);
		      return -6;	
	    }

	    pstCtx->iAllowTimeGap = atoi(szTemp);	

	    iRet = tiniapi_read(szConfilePath,"General","NetMaskBitCount",szTemp,sizeof(szTemp));
	    if( iRet < 0 )
	    {
		      printf("Error:fail to read NetMaskBitCount[%d] in conf[%s]\n",iRet,szConfilePath);
		      return -7;	

	    }
	    pstCtx->iNetMaskBitCount = atoi(szTemp);
	    if(pstCtx->iNetMaskBitCount<=0
		||pstCtx->iNetMaskBitCount>32)
	    {

	          pstCtx->iNetMaskBitCount = 24;
	    }		
    	} 
 
	iRet = tiniapi_read(szConfilePath,"General","CheckServiceID",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read CheckServiceID[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -8;	
	}
	pstCtx->iCheckServiceID = atoi(szTemp);

	iRet = tiniapi_read(szConfilePath,"General","CheckCltver",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read CheckCltver[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -9;	
	}
	pstCtx->iCheckCltVer = atoi(szTemp);

	iRet = tiniapi_read(szConfilePath,"General","logdir",szTemp,sizeof(szTemp));
	if( iRet < 0 )
	{
		  printf("Error:fail to read logdir[%d] in conf[%s]\n",iRet,szConfilePath);
		  return -10;	
	}
	 
	iRet =  tlogapi_openlog(pstCtx,szTemp);
	if( iRet < 0 )
	{
		  printf("Error:fail to open logdir[%s],errorstrint=%s\n",szTemp,strerror(errno));	
		  return -11;		
	}
	
	return 0;
	
	
}


/*同一个网段返回1,否则返回0*/
int tsigapi_netmask_cmp(int iNetMaskBitCount,unsigned int  uCltIP1,unsigned int uCltIP2)
{

      int iLeftShiftCount;
    

      iLeftShiftCount = IPV4_NETCOUNT - iNetMaskBitCount;

      if(( (uCltIP1>> iLeftShiftCount)<< iLeftShiftCount ) == (  (uCltIP2>> iLeftShiftCount)<< iLeftShiftCount  ) )	  
      {
               return 1;
      }
      else
      {

                return 0;
	}	  

}



int tsigapi_check(IN TQQSIGCTX *pSigCtx,IN TQQConnInfo *pstConnInfo,IN TQQSIGCHECKLIST *pstSigInfo)
{
	time_t tCur;
	unsigned int dwuin;

	if( !pSigCtx ||
		!pstConnInfo||
		!pstSigInfo)
	{
            return -1;
	}

	dwuin=pstSigInfo->uin;
	time(&tCur);

	if( pSigCtx->iCheckUin == 1
	    && dwuin != pstConnInfo->uin )
	{
            tlogapi_logfile(pSigCtx, dwuin, TQQSIG_UIN,pstConnInfo->uin,dwuin, TQQSIG_REASON_NONE);
	     return -2;		
	}

	if( pSigCtx->iCheckSigTime == 1)
	{
            
	     if( tCur - pstSigInfo->utimestamp >(unsigned int) pSigCtx->iTimeOutValue)
	     {
                tlogapi_logfile(pSigCtx, dwuin, TQQSIG_TIMESTAMP, tCur,pstSigInfo->utimestamp,TQQSIG_REASON_NONE);
		  return -3;		
	     }
	}

	if( pSigCtx->iCheckCltIP == 1
		&& pstConnInfo->uCltIP != pstSigInfo->uCltIP )
	{
	      if(pSigCtx->iAllowTimeGap == 0 || tCur - pstSigInfo->utimestamp <= (unsigned int)pSigCtx->iAllowTimeGap)
	      {
	          /*未超时*/
		   tlogapi_logfile(pSigCtx, dwuin, TQQSIG_IP, pstConnInfo->uCltIP,pstSigInfo->uCltIP,TQQSIG_IP_TIME);	  

	      }
	      else if( tsigapi_netmask_cmp(pSigCtx->iNetMaskBitCount, pstConnInfo->uCltIP, pstSigInfo->uCltIP) == 1)
	      {
                 /*在同一个网段*/
		   tlogapi_logfile(pSigCtx, dwuin, TQQSIG_IP, pstConnInfo->uCltIP, pstSigInfo->uCltIP, TQQSIG_IP_NETMASK);

	       }
		else
		{
                   
                 tlogapi_logfile(pSigCtx,dwuin,TQQSIG_IP, pstConnInfo->uCltIP, pstSigInfo->uCltIP, TQQSIG_REASON_NONE);
		   return -4;		 
		}
	}

	if(  pSigCtx->iCheckServiceID == 1
		&& pstConnInfo->uAppID != pstSigInfo->uAppID )
	{
             tlogapi_logfile(pSigCtx,dwuin,TQQSIG_SERVICEID, pstConnInfo->uAppID, pstSigInfo->uAppID,TQQSIG_REASON_NONE);
             return -5;			 
	}

	if( pSigCtx->iCheckCltVer == 1
		&& pstConnInfo->uCltVer != pstSigInfo->uCltVer )
	{
             tlogapi_logfile(pSigCtx, dwuin, TQQSIG_CLTVER, pstConnInfo->uCltVer, pstSigInfo->uCltVer, TQQSIG_REASON_NONE);
	      return -6;		 
	}


	return 0;	
}



int tsigapi_fini(IN LPTQQSIGCTX *ppSigCtx)
{
	
    if(!ppSigCtx)
    {
          return -1;

     }

     tlogapi_closelog(*ppSigCtx);

     free(*ppSigCtx);

     *ppSigCtx = NULL;	 
	 
      return 0;		
}
