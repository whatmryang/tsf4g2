/*
 **  @file $RCSfile: tlogload.c,v $
 **  general description of this module
 **  $Id: tlogload.c,v 1.7 2009-03-27 06:17:19 kent Exp $
 **  @author $Author: kent $
 **  @date $Date: 2009-03-27 06:17:19 $
 **  @version $Revision: 1.7 $
 **  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
 **  @note Platform: Linux
 */

#include "pal/pal.h"
#include "tmng/tmib.h"
#include "tloghelp/tlogload.h"
#include "tdr/tdr.h"
#include "../lib_src/tlog/tlog_i.h"
#include "tlogload_i.h"

LPTLOGCTX tlog_init_from_file(const char *a_pstzPath)
{
    int iRet;
    LPTLOGCONF pstConf = NULL;
    LPTLOGCTX pstCtx = calloc(1, sizeof(TLOGCTX));

    if (NULL == pstCtx)
    {
        goto err;
    }

    pstConf = calloc(1, sizeof(TLOGCONF));
    if (NULL == pstConf)
    {
        goto err;
    }

    iRet = tlog_init_cfg_from_file(pstConf, a_pstzPath);

    if (iRet)
    {
        goto err;
    }

    iRet = tlog_init(pstCtx, pstConf);

    if (iRet)
    {
        goto err;
    }

    return pstCtx;

    err: if (pstConf)
    {
        free(pstConf);
        pstConf = NULL;
    }

    if (pstCtx)
    {
        free(pstCtx);
        pstCtx = NULL;
    }

    return NULL;

}

LPTLOGCTX tlog_init_default(const char *a_pstzPath)
{
    int iRet;
    LPTLOGCONF pstConf = NULL;
    LPTLOGCTX pstCtx = calloc(1, sizeof(TLOGCTX));

    if (NULL == pstCtx)
    {
        goto err;
    }

    pstConf = calloc(1, sizeof(TLOGCONF));
    if (NULL == pstConf)
    {
        goto err;
    }

    iRet = tlog_init_cfg_default(pstConf, a_pstzPath);

    if (iRet)
    {
        goto err;
    }

    iRet = tlog_init(pstCtx, pstConf);

    if (iRet)
    {
        goto err;
    }

    return pstCtx;

    err: if (pstConf)
    {
        free(pstConf);
    }

    if (pstCtx)
    {
        free(pstCtx);
    }

    return NULL;
}

int tlog_fini_ctx(LPTLOGCTX* a_ppstCtx)
{
    TLOGCATEGORYINNERINST* pstCatInst;

    if (a_ppstCtx)
    {
        LPTLOGCTX a_pstCtx = *a_ppstCtx;
        if (a_pstCtx)
        {
            while (a_pstCtx->stInst.iCount > 0)
            {
                a_pstCtx->stInst.iCount--;

                pstCatInst = a_pstCtx->stInst.astCatInsts + a_pstCtx->stInst.iCount;

                if (pstCatInst->iInited)
                {
                    tlogany_fini(CATINST2DEVANY(pstCatInst));
                    pstCatInst->iInited = 0;
                }
#ifdef TSF4G_TLOG_THREAD_SAFE
				pthread_mutex_destroy(&pstCatInst->stLogMutex);
#endif

            }

            free(a_pstCtx->pstConf);
            free(a_pstCtx);
            *a_ppstCtx = NULL;
            return 0;
        }
    }

    return -1;
}

LPTLOGCONF tlog_conf_alloc(void)
{
    void * res = calloc(1, sizeof(TLOGCONF));
    return res;
}

TLOGCONF* tlog_get_cfg_from_mib(void *a_pMib, const char *a_pszDomain, const char *a_pszName, int a_iProcID)
{
    int iRet = 0;
    TMIBDATA stData;

    memset(&stData, 0, sizeof(stData));
    snprintf(stData.szDomain, sizeof(stData.szDomain), "%s", a_pszDomain);
    snprintf(stData.szName, sizeof(stData.szName), "%s", a_pszName);
    stData.iProcID = a_iProcID;

    iRet = tmib_get_data((HTMIB) a_pMib, &stData, 1);

    if (0 == iRet && stData.iSize < (int) sizeof(TLOGCONF))
        iRet = -1;

    return -1 == iRet ? NULL : (TLOGCONF*) stData.pszData;
}

extern unsigned char g_szMetalib_tlogdef[];

int tlog_init_cfg_from_file(TLOGCONF* a_pstConf, const char *a_pszPath)
{
    LPTDRMETALIB pstLib;
    LPTDRMETA pstMeta;
    TDRDATA stData;
    int iRet;

    pstLib = (LPTDRMETALIB) tlog_get_meta_data();

    pstMeta = tdr_get_meta_by_name(pstLib, "TLOGConf");

    if (!pstMeta)
        return -1;

    stData.pszBuff = (char*) a_pstConf;
    stData.iBuff = (int) sizeof(*a_pstConf);

    iRet = tdr_input_file(pstMeta, &stData, a_pszPath, 0, 0);
    if (iRet < 0)
    {
        printf("input tlog conf file err:%s\n", tdr_error_string(iRet));
        return iRet;
    }
    else
        return 0;
}

int tlog_init_cfg_default(TLOGCONF* a_pstConf, const char* a_pszPath)
{
    TLOGCATEGORY* pstCat;

    memset(a_pstConf, 0, sizeof(*a_pstConf));

    a_pstConf->iMagic = TLOG_CONF_MAGIC;
    a_pstConf->iCount = 6;
    a_pstConf->iPriorityHigh = TLOG_PRIORITY_NULL;
    a_pstConf->iPriorityLow = TLOG_PRIORITY_DEBUG;
    a_pstConf->iDelayInit = 0;
    a_pstConf->iSuppressError = 1;

    // text category
    pstCat = a_pstConf->astCategoryList + 0;

    pstCat->iPriorityHigh = TLOG_PRIORITY_INFO;
    pstCat->iPriorityLow = TLOG_PRIORITY_TRACE;

    STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_TEXTROOT, sizeof(pstCat->szName));
    STRNCPY(pstCat->szForwardCat, TLOG_DEF_CATEGORY_TEXTTRACE, sizeof(pstCat->szForwardCat));

    //
    //vinson 20110418 change to TLOG_DEV_NO for close Memlog.
    pstCat->stDevice.iType = TLOG_DEV_NO;
    //pstCat->stDevice.iType = TLOG_DEV_MEM;

	
    pstCat->stDevice.stDevice.stMem.iBuffSize = 10000000;
    pstCat->stDevice.stDevice.stMem.dwSizeLimit = 10000000;
    snprintf(pstCat->stDevice.stDevice.stMem.szUri, sizeof(pstCat->stDevice.stDevice.stMem.szUri),
            "tsm-data://12/test:1.3.5.7/log/%s/memlog", a_pszPath);
    snprintf(pstCat->stDevice.stDevice.stMem.szPattern, sizeof(pstCat->stDevice.stDevice.stMem.szPattern), "%s.memlog", a_pszPath);

    pstCat->stFilter.iCount = 1;
    pstCat->stFilter.astFilters[0].stClsFilter.iStart = 0;
    pstCat->stFilter.astFilters[0].stClsFilter.iCount = 100;
    pstCat->stFilter.astFilters[0].stIDFilter.iStart = 0;
    pstCat->stFilter.astFilters[0].stIDFilter.iCount = 100000000;

    //texttrace
    pstCat = a_pstConf->astCategoryList + 1;
    STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_TEXTTRACE, sizeof(pstCat->szName));
    STRNCPY(pstCat->szFormat, TLOG_DEF_LAYOUT_FORMAT, sizeof(pstCat->szFormat));
    STRNCPY(pstCat->szForwardCat, TLOG_DEF_CATEGORY_TEXTERR, sizeof(pstCat->szForwardCat));
    pstCat->iPriorityHigh = TLOG_PRIORITY_NULL;
    pstCat->iPriorityLow = TLOG_PRIORITY_TRACE;
    pstCat->iLevelDispatch = 0;
    pstCat->iMustSucc = 0;
    pstCat->iMaxMsgSize = 0;

    pstCat->stFilter.iCount = 0;

    pstCat->stDevice.iType = TLOG_DEV_FILE;
    pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1] = 0;
    snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1, "%s.log", a_pszPath);
    pstCat->stDevice.stDevice.stFile.iSyncTime = 0;
    pstCat->stDevice.stDevice.stFile.iPrecision = 1;
    pstCat->stDevice.stDevice.stFile.dwSizeLimit = 10 * 1024 * 1024;
    pstCat->stDevice.stDevice.stFile.iMaxRotate = 2;

    //texterr
    pstCat = a_pstConf->astCategoryList + 2;
    STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_TEXTERR, sizeof(pstCat->szName));
    STRNCPY(pstCat->szFormat, TLOG_DEF_LAYOUT_FORMAT, sizeof(pstCat->szFormat));
    pstCat->iPriorityHigh = TLOG_PRIORITY_NULL;
    pstCat->iPriorityLow = TLOG_PRIORITY_ERROR;
    pstCat->iLevelDispatch = 0;
    pstCat->iMustSucc = 0;
    pstCat->iMaxMsgSize = 0;
    pstCat->stDevice.iType = TLOG_DEV_FILE;
    pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1] = 0;
    snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1, "%s.error", a_pszPath);
    pstCat->stDevice.stDevice.stFile.iSyncTime = 0;
    pstCat->stDevice.stDevice.stFile.iPrecision = 1;
    pstCat->stDevice.stDevice.stFile.dwSizeLimit = 10 * 1024 * 1024;
    pstCat->stDevice.stDevice.stFile.iMaxRotate = 2;

    pstCat->stFilter.iCount = 0;

    //texttrace.bus
    pstCat = a_pstConf->astCategoryList + 3;
    STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_BUS, sizeof(pstCat->szName));
    STRNCPY(pstCat->szFormat, "[%d%u]%p %m %F:%l%n", sizeof(pstCat->szFormat));
    pstCat->iPriorityHigh = TLOG_PRIORITY_NULL;
    pstCat->iPriorityLow = TLOG_PRIORITY_ERROR;
    pstCat->iLevelDispatch = 1;
    pstCat->iMustSucc = 0;
    pstCat->iMaxMsgSize = 0;
    pstCat->stDevice.iType = TLOG_DEV_FILE;
    pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1] = 0;
    snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1, "%s_tbus.log", a_pszPath);
    pstCat->stDevice.stDevice.stFile.iSyncTime = 0;
    pstCat->stDevice.stDevice.stFile.iPrecision = 1;
    pstCat->stDevice.stDevice.stFile.dwSizeLimit = 5 * 1024 * 1024;
    pstCat->stDevice.stDevice.stFile.iMaxRotate = 3;

    pstCat->stFilter.iCount = 0;
    pstCat->stFilter.astFilters[0].stClsFilter.iStart = 0;
    pstCat->stFilter.astFilters[0].stClsFilter.iCount = 100;
    pstCat->stFilter.astFilters[0].stIDFilter.iStart = 0;
    pstCat->stFilter.astFilters[0].stIDFilter.iCount = 100000000;

    //data
    pstCat = a_pstConf->astCategoryList + 4;
    STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_DATAROOT, sizeof(pstCat->szName));
    STRNCPY(pstCat->szFormat, TLOG_DEF_LAYOUT_FORMAT, sizeof(pstCat->szFormat));
    pstCat->iPriorityHigh = TLOG_PRIORITY_NULL;
    pstCat->iPriorityLow = TLOG_PRIORITY_INFO;
    pstCat->iLevelDispatch = 0;
    pstCat->iMustSucc = 0;
    pstCat->iMaxMsgSize = 0;
    pstCat->stDevice.iType = TLOG_DEV_FILE;
    pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1] = 0;
    snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1, "%s_rundata.log",
            a_pszPath);
    pstCat->stDevice.stDevice.stFile.iSyncTime = 0;
    pstCat->stDevice.stDevice.stFile.iPrecision = 1;
    pstCat->stDevice.stDevice.stFile.dwSizeLimit = 20 * 1024 * 1024;
    pstCat->stDevice.stDevice.stFile.iMaxRotate = 3;

    pstCat->stFilter.iCount = 0;
    pstCat->stFilter.astFilters[0].stClsFilter.iStart = 0;
    pstCat->stFilter.astFilters[0].stClsFilter.iCount = 100;
    pstCat->stFilter.astFilters[0].stIDFilter.iStart = 0;
    pstCat->stFilter.astFilters[0].stIDFilter.iCount = 100000000;



    // bill
    pstCat = a_pstConf->astCategoryList + 5;
    STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_BILL, sizeof(pstCat->szName));
    STRNCPY(pstCat->szFormat, TLOG_DEF_BILL_LAYOUT_FORMAT, sizeof(pstCat->szFormat));
    pstCat->iPriorityHigh = TLOG_PRIORITY_NULL;
    pstCat->iPriorityLow = TLOG_PRIORITY_ERROR;
    pstCat->iLevelDispatch = 0;
    pstCat->iMustSucc = 0;
    pstCat->iMaxMsgSize = 0;
    pstCat->stDevice.iType = TLOG_DEV_FILE;
    pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1] = 0;
    snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern) - 1, "%s.bill",
            a_pszPath);
    pstCat->stDevice.stDevice.stFile.iSyncTime = 0;
    pstCat->stDevice.stDevice.stFile.iPrecision = 1;
    pstCat->stDevice.stDevice.stFile.dwSizeLimit = 20 * 1024 * 1024;
    pstCat->stDevice.stDevice.stFile.iMaxRotate = 0;

    pstCat->stFilter.iCount = 0;
    return 0;
}

LPTLOGCONF tlog_create_file_cfg(const char *a_pszCatName, int iPriorityLow, const char *a_pszFilePattern, int iMaxRotate, size_t iSizeLimit, int iRotateStick)
{
    LPTLOGCONF pstConf;
    TLOGCATEGORY* pstCat;
    pstConf = calloc(1, sizeof(TLOGCONF));
    if (!pstConf)
    {
        goto error;
    }

    if(0 == iPriorityLow)
    {
        iPriorityLow = -1;
    }

    pstConf->iMagic = TLOG_CONF_MAGIC;
    pstConf->iCount = 1;
    pstConf->iPriorityHigh = TLOG_PRIORITY_NULL;
    pstConf->iPriorityLow = iPriorityLow;
    pstConf->iDelayInit = 0;
    pstConf->iSuppressError = 0;

    // text category
    pstCat = pstConf->astCategoryList + 0;

    pstCat->iPriorityHigh = TLOG_PRIORITY_NULL;
    pstCat->iPriorityLow = iPriorityLow;
    pstCat->iMustSucc = 1;

    STRNCPY(pstCat->szName, a_pszCatName, sizeof(pstCat->szName));
    STRNCPY(pstCat->szFormat, TLOG_DEF_LAYOUT_FORMAT, sizeof(pstCat->szFormat));
    pstCat->stDevice.iType = TLOG_DEV_FILE;
    pstCat->stDevice.stDevice.stFile.iBuffSize = 0;
    pstCat->stDevice.stDevice.stFile.iMaxRotate = iMaxRotate;
    pstCat->stDevice.stDevice.stFile.iRotateStick = iRotateStick;
    pstCat->stDevice.stDevice.stFile.dwSizeLimit = iSizeLimit;
    pstCat->stDevice.stDevice.stFile.iPrecision = 10;
    STRNCPY(pstCat->stDevice.stDevice.stFile.szPattern, a_pszFilePattern,
            sizeof(pstCat->stDevice.stDevice.stFile.szPattern));

    return pstConf;
    error: if (pstConf)
    {
        free(pstConf);
    }

    return NULL;
}

LPTLOGCTX tlog_init_file_ctx(const char *a_pszCatName, int iPriorityLow, const char *a_pszFilePattern, int iMaxRotate, size_t iSizeLimit,
        int iRotateStick)
{
    LPTLOGCTX pstCtx = NULL;
    LPTLOGCONF pstConf = NULL;
    int iRet;
    pstCtx = calloc(1, sizeof(TLOGCTX));
    if (!pstCtx)
    {
        goto error;
    }

    pstConf = tlog_create_file_cfg(a_pszCatName,  iPriorityLow,a_pszFilePattern, iMaxRotate, iSizeLimit, iRotateStick);
    if (!pstConf)
    {
        goto error;
    }

    iRet = tlog_init(pstCtx, pstConf);
    if (iRet)
    {
        goto error;
    }
    return pstCtx;
    error: if (pstCtx)
    {
        free(pstCtx);
    }

    if (pstConf)
    {
        free(pstConf);
    }
    return NULL;
}

int tlog_set_ctx_priority(LPTLOGCTX pstCtx, int iPriorityHigh, int iPriorityLow)
{
    if (pstCtx->pstConf->iMagic != TLOG_CONF_MAGIC)
    {
        return -1;
    }

    pstCtx->pstConf->iPriorityHigh = iPriorityHigh;
    pstCtx->pstConf->iPriorityLow = iPriorityLow;
    return 0;
}

