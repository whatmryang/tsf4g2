/**************************************************************
 @file $RCSfile: tmng.c,v $
  tsf managing interface
  $Id: tmng.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
 @author $Author: kent $
 @date $Date: 2008-05-28 07:35:00 $
 @version 1.0
 @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
 @note Platform: Linux
 **************************************************************/
#include    <time.h>
#include    "tbus/tbus.h"
#include    "comm/tini.h"
#include    "tmng/tmng.h"
#include    "tmng/tmng_def.h"
int  tmng_init_mem(INOUT HTMNG   *a_phMng, IN const char* a_szMibKey, IN const char* a_szMetabaseKey)
{
    int     iRet;
    HTMNG   hMng = NULL;

    if ( NULL == *a_phMng  )
    {
        hMng = (HTMNG) malloc(sizeof(TMNGHANDLER));
        if ( NULL == hMng )
        {   
            return -1;
        }
        memset(hMng, 0, sizeof(TMNGHANDLER));
    }
    else 
    {
        hMng = *a_phMng;
    }
    
    if( NULL!=a_szMibKey  &&  0!=strlen(a_szMibKey) )
        strncpy( hMng->szMibKey, a_szMibKey, sizeof(hMng->szMibKey)-1);
    else
        strncpy( hMng->szMibKey, DEFAULT_MIB_KEY, sizeof(hMng->szMibKey)-1);

    if ( NULL!=a_szMetabaseKey  &&  0!=strlen(a_szMetabaseKey) )
        strncpy( hMng->szMetabaseKey, a_szMetabaseKey, sizeof(hMng->szMetabaseKey)-1);
    else 
        strncpy( hMng->szMetabaseKey, DEFAULT_METABASE_KEY, sizeof(hMng->szMetabaseKey)-1);
    
    iRet = tmib_open(&hMng->hMib, hMng->szMibKey);
    if( iRet )
    {
        return -2;
    }
    
    iRet = tmb_open(&hMng->hDesc, hMng->szMetabaseKey, 1);
    if( iRet )
    {
        return -3;
    }

    if ( NULL == *a_phMng  )
    {
        *a_phMng = hMng;
    }

    return 0;
}


int tmng_get_agentid(INOUT HTMNG   a_hMng)
{
    int             iRet;
    LPHOSTDEPLOY    pstDeploy = NULL;
    char*           pchDeploy = NULL;
    
    iRet = tmng_get_cfg(a_hMng, DEPOLY_CFG, 0,  &pchDeploy);
    if ( iRet )
    {   
        printf( "tmng_get_cfg return %d\n", iRet);
        return -1;
    }

    pstDeploy = (LPHOSTDEPLOY)pchDeploy;    
    a_hMng->iAgentID = inet_addr(pstDeploy->szAgentId);
    return 0;
}

int tmng_init(  OUT HTMNG*      a_phMng, 
                IN  int         a_Argc,
                IN  const char* a_Argv[],
                IN  int         a_iProcID, 
                IN  const char* a_szMibKey, 
                IN  const char* a_szMetabaseKey)
{
    int     i,iRet;
    HTMNG   hMng = NULL;
    TMIBDATA stData;
    char        szCmd[MAX_CMD_LEN], szTmp[80];

    assert(a_phMng);
    
    hMng = (HTMNG) malloc(sizeof(TMNGHANDLER));
    if ( NULL == hMng )
    {   
        return -1;
    }
    memset(hMng, 0, sizeof(TMNGHANDLER));

    
    hMng->argc = a_Argc;
    hMng->argv = a_Argv;
    
    iRet = tmng_init_mem(&hMng, a_szMibKey, a_szMetabaseKey);
    if ( iRet )     return -1;
    
    hMng->iProcID  = a_iProcID;

    if ( hMng->iProcID )
    {
        /*注册进程状态记录*/
        memset(&stData, 0, sizeof(TMIBDATA));
        strcpy( stData.szDomain, PROC_LIST_DOMAIN);
        strcpy(stData.szName, inet_ntoa(*(struct in_addr *)&hMng->iProcID));
        strcpy( stData.szLib,  MNG_LIB);
        strcpy( stData.szMeta, PROC_ATTR);
        stData.iSize = sizeof(MNGPROCATTR);
        
        iRet = tmib_register_data(hMng->hMib, &stData);
        if ( iRet )     return -2;
        tmib_get_data(hMng->hMib, &stData, 0);

        hMng->pstProcAttr = (MNGPROCATTR*)stData.pszData;      
        if ( hMng->argc && hMng->argv )
        {
            
            strcpy(hMng->pstProcAttr->szProcName, basename(hMng->argv[0]));
            strcpy(hMng->pstProcAttr->szWorkDir, getenv("PWD"));
            strcpy(hMng->pstProcAttr->szUser, getenv("USER"));
            szCmd[0]=0;
            for(i=0; i<hMng->argc; i++)
            {
                sprintf(szTmp, " %s", hMng->argv[i]);
                strcat(szCmd, szTmp );        
            }   
            strncpy(hMng->pstProcAttr->szStartCmd, 
                    szCmd,
                    sizeof(hMng->pstProcAttr->szStartCmd)-1);
        }
            
        strcpy(hMng->pstProcAttr->szProcID, inet_ntoa(*(struct in_addr *)&hMng->iProcID));
        hMng->pstProcAttr->iPid = getpid();
        time(&hMng->pstProcAttr->dwStart);
        hMng->pstProcAttr->dwActive= hMng->pstProcAttr->dwStart;
        hMng->pstProcAttr->iStatus= 1;
        iRet = tmng_get_agentid( hMng );
    }        
    
    
    if ( hMng->iProcID && hMng->iAgentID)
    {
        /*初始化bus,建立和agent的连接*/        
        iRet = tbus_init ( 0, 0 ) ;

        if ( !iRet ) iRet = tbus_new ( &hMng->iBus ) ;

        if ( !iRet ) iRet = tbus_bind ( hMng->iBus, hMng->iProcID ) ;

        if ( !iRet ) iRet = tbus_connect ( hMng->iBus,  hMng->iAgentID) ;

        if ( iRet )     /*bus init fail*/
        {
            hMng->iBus=0;
        }
    }

    *a_phMng = hMng;
    return 0;
}
    

int tmng_fini_mem(INOUT HTMNG   *a_phMng )
{
    int     iRet;
    HTMNG   hMng=NULL;

    assert(a_phMng);

    hMng = *a_phMng;
    assert(hMng);

    iRet = tmib_close(&hMng->hMib);
    iRet = tmb_close(&hMng->hDesc);

    free( hMng );
    *a_phMng = NULL;
    return 0;
}

int tmng_fini(INOUT HTMNG   *a_phMng )
{
    HTMNG   hMng = NULL;
       
    assert(a_phMng);
    
    tbus_fini ( );
    
    hMng = *a_phMng;

    if ( hMng->iProcID || hMng->pstProcAttr )
    {
        time(&hMng->pstProcAttr->dwStop);
        hMng->pstProcAttr->iStatus= 0;
        hMng->pstProcAttr->iPid= 0;
    }
    return tmng_fini_mem(a_phMng );
}

int tmng_get_cfg(IN  HTMNG       a_hMng,
                 IN  const char* a_szCfgName, 
                 IN  int         a_iVersion, 
                 OUT char**      a_ppCfgData )
{
    int     iRet;
    TMIBDATA    stCfgData;

    assert( a_hMng&& a_szCfgName && a_ppCfgData);

    if ( NULL == a_hMng->hDesc || NULL == a_hMng->hMib )    return -1;

    memset ( &stCfgData, 0, sizeof(TMIBDATA));
    strcpy(stCfgData.szName, a_szCfgName);
    strcpy(stCfgData.szDomain, CFG_DOMAIN );
    stCfgData.iVersion = a_iVersion;
    iRet = tmib_get_data(a_hMng->hMib, &stCfgData, 1);
    if( iRet )
    {
        return -2;
    }
    
    if ( NULL == stCfgData.pszData ) 
    {
        *a_ppCfgData = NULL;
        return -1;
    }
        
    *a_ppCfgData = stCfgData.pszData;
    return 0;
}


int tmng_send_i(IN  HTMNG  a_hMng,  IN const void *a_pvData, IN const size_t a_iLen)
{   
    assert( a_hMng && a_pvData && a_iLen);
   
    return tbus_send(a_hMng->iBus, &a_hMng->iProcID, &a_hMng->iAgentID, a_pvData, a_iLen, 0);
}

int tmng_send_alert(IN  HTMNG  a_hMng, const LPMNGALERT a_pstAlert)
{
    MNGPKG  stPkg;

    assert(a_hMng && a_pstAlert);
    memset( &stPkg, 0, sizeof(MNGPKG));
    stPkg.stHead.bMsgType=MNG_ALERT;
    stPkg.stHead.iSrcID = a_hMng->iProcID;
    stPkg.stHead.iDestID = a_hMng->iAgentID;
    memcpy(&stPkg.stBody, a_pstAlert, sizeof(MNGALERT));

    //tdr_hton (IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstNet, INOUT LPTDRDATA a_pstHost, IN int a_iVersion) 
    tmng_send_i( a_hMng,  &stPkg, sizeof(MNGPKG));
    return 0;
}

int tmng_on_cycle(IN  HTMNG  a_hMng)
{
    //int iRet=0;
    //int iSrc=0;

    assert( a_hMng );
    time(&a_hMng->pstProcAttr->dwActive);
    
    /*接收管理消息*/
    /*
    iRet = tbus_send(a_hMng->iBus, &iSrc, &a_hMng->iAgentID, "MNG TEST!", 10, 0);
    
    if ( iRet )
        printf ("tbus_send dst=%x, ret=%d\n ", a_hMng->iAgentID, iRet);
    */
    return 0;
}

int tmng_get_rdo(IN  HTMNG       a_hMng,
                 IN  const char* a_szName, 
                 IN  int         a_iPeriods,
                 IN  int         a_iVersion, 
                 OUT char**      a_ppData ,
                 OUT char**      a_ppDefine)

{
    LPTMIBINFO pstInfo=NULL;
    LPTMIBENTRY pstEntry=NULL;
    int iPageSize;
    int i;

    assert( a_hMng && a_ppData && a_ppDefine );

    pstInfo = a_hMng->hMib->pstInfo;
    iPageSize = tos_get_pagesize();

    for(i=0; i<pstInfo->iCurDatas; i++)
    {
        pstEntry = pstInfo->entries + i;

        if( pstEntry->bDelete )
        {
            continue;
        }

        if( 0==stricmp(pstEntry->szDomain, RDO_DOMAIN) &&
            0==stricmp(pstEntry->szName, a_szName) &&
            ( !a_iVersion|| !pstEntry->iVersion || 
               a_iVersion ==pstEntry->iVersion) &&
            ( !a_iPeriods|| !pstEntry->iPeriods || 
               a_iPeriods==pstEntry->iPeriods) )
        {
            if( pstEntry->iOff % iPageSize )
            {
                return -1;
            }

            *a_ppDefine = pstEntry;
            
            if ( pstEntry->bExtern )
            {
                *a_ppData = (char*) tshmat(pstEntry->iExShm, 0);
            }
            else
            {
                *a_ppData  = (char*)(((int)pstInfo) + pstEntry->iOff);
            }

            if( mprotect(*a_ppData,  pstEntry->bExtern?pstEntry->iExSize:pstEntry->iSize, PROT_RDWR)<0 )
            {
                return 0;
            }
            else
            {
                return 0;
            }

        }
    }
    
    return -1;
}

