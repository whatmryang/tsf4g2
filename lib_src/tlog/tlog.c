/*
 **  @file $RCSfile: tlog.c,v $
 **  general description of this module
 **  $Id: tlog.c,v 1.7 2009-03-27 06:17:02 kent Exp $
 **  @author $Author: kent $
 **  @date $Date: 2009-03-27 06:17:02 $
 **  @version $Revision: 1.7 $
 **  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
 **  @note Platform: Linux
 */

#include "tlog_i.h"
#include "tdr/tdr.h"

#if !defined (_WIN32) && !defined (_WIN64)
#include <sys/resource.h>
#endif

#define DECLARE
int tlog_resolve_i(TLOGCTX* a_pstCtx);

extern unsigned char g_szMetalib_tlogdef[];

const void* tlog_get_meta_data()
{
    return (const void*) g_szMetalib_tlogdef;
}

TLOGCTX* tlog_alloc(void)
{
    TLOGCTX* pstCtx;
    pstCtx = (TLOGCTX*) calloc(1, sizeof(TLOGCTX));
    return pstCtx;
}

int tlog_free(TLOGCTX** a_ppstCtx)
{
    if (*a_ppstCtx)
    {
        free(*a_ppstCtx);

        *a_ppstCtx = NULL;

        return 0;
    }

    return -1;
}

int tlog_resolve_i(TLOGCTX* a_pstCtx)
{
    int i;
    int j;
    TLOGCATEGORYINNERINST* pstCatInnerInst1;
    TLOGCATEGORYINNERINST* pstCatInnerInst2;
    char szParentName[TLOG_NAME_LEN];
    char *pcTmp;

    for (i = 0; i < a_pstCtx->stInst.iCount; i++)
    {
        pstCatInnerInst1 = a_pstCtx->stInst.astCatInsts + i;
        pstCatInnerInst1->pstParent = NULL;
        pstCatInnerInst1->pstForward = NULL;

        pcTmp = strrchr(pstCatInnerInst1->szName, TLOG_CHAR_DOT);

        szParentName[0] = 0;
        if (pcTmp)
        {
            size_t iLen;

            iLen = pcTmp - &pstCatInnerInst1->szName[0];

            if (iLen > 0 && iLen < sizeof(szParentName))
            {
                memcpy(szParentName, pstCatInnerInst1->szName, iLen);
                szParentName[iLen] = 0;
            }
        }

        for (j = 0; j < a_pstCtx->stInst.iCount; j++)
        {
            if (i == j)
                continue;

            pstCatInnerInst2 = a_pstCtx->stInst.astCatInsts + j;

            if (NULL == pstCatInnerInst1->pstParent && szParentName[0] && 0
                    == strcasecmp(szParentName, pstCatInnerInst2->szName))
            {
                pstCatInnerInst1->pstParent = pstCatInnerInst2;
            }

            if (NULL == pstCatInnerInst1->pstForward
                    && pstCatInnerInst1->pstCat->szForwardCat[0] && 0
                    == strcasecmp(pstCatInnerInst1->pstCat->szForwardCat,
                            pstCatInnerInst2->szName))
            {
                pstCatInnerInst1->pstForward = pstCatInnerInst2;
            }

            if (pstCatInnerInst1->pstParent && pstCatInnerInst1->pstForward)
            {
                break;
            }
        }
    }

    return 0;
}

static int tlog_check_forward_loop(TLOGCTX* pstCtx)
{
    TLOGCATEGORYINNERINST *pstCatInst;
    TLOGCATEGORYINNERINST *pstCatHead;
    TLOGCATEGORYINNERINST *pstCatTail;
    int iNumMemDev;
    int i, j, k;

    for (i = 0; i < pstCtx->stInst.iCount; i++)
    {
        iNumMemDev = 0;
        pstCatInst = &pstCtx->stInst.astCatInsts[i];
        pstCatHead = pstCatTail = pstCatInst;
        k = 0;
        do
        {
            // head移动一步
            if (pstCatHead->pstForward)
            {
                k++;
                pstCatHead = pstCatHead->pstForward;
                if (k && pstCatHead->pstCat->stDevice.iType == TLOG_DEV_MEM)
                {
                    return 2;
                }
            }

            // tail移动两步
            for (j = 0; j < 2; j++)
            {
                if (pstCatTail->pstForward)
                {
                    pstCatTail = pstCatTail->pstForward;
                }
            }
        } while (pstCatHead != pstCatTail);

        // 如果没有环，pstCatHead和pstCatTail都应该在最后
        if (pstCatHead->pstForward)
        {
            return 1;
        }
    }

    return 0;
}

int tlog_init(TLOGCTX* a_pstCtx, TLOGCONF* a_pstConf)
{
    TLOGCATEGORY* pstCat;
    TLOGCATEGORYINNERINST* pstCatInnerInst;
    int iErr;
    int iRet = 0;
    int i = 0;
    assert(a_pstCtx && a_pstConf);

    if (a_pstConf->iMagic != TLOG_CONF_MAGIC)
    {
        return -1;
    }

    memset(a_pstCtx, 0, sizeof(*a_pstCtx));

    a_pstCtx->pstConf = a_pstConf;

    snprintf(a_pstCtx->szHostName, sizeof(a_pstCtx->szHostName), "0.0.0.0");
    snprintf(a_pstCtx->szModuleName, sizeof(a_pstCtx->szModuleName), "unknown");

    iErr = 0;

    for (i = 0; i < TLOG_MAX_CATEGORY; i++)
    {
        a_pstCtx->stInst.astCatInsts[i].iIndex = i;
    }

    while (a_pstCtx->stInst.iCount < a_pstConf->iCount)
    {
        pstCat = a_pstConf->astCategoryList + a_pstCtx->stInst.iCount;
        pstCatInnerInst = a_pstCtx->stInst.astCatInsts
                + a_pstCtx->stInst.iCount;

        if (pstCat->iMaxMsgSize <= 0)
        {
            pstCat->iMaxMsgSize = TLOG_DEFAULT_MAX_MSG_SIZE;
        }

        pstCatInnerInst->stCatInst.piPriorityHigh = &a_pstConf->iPriorityHigh;
        pstCatInnerInst->stCatInst.piPriorityLow = &a_pstConf->iPriorityLow;

        STRNCPY(pstCatInnerInst->szName, pstCat->szName, sizeof(pstCatInnerInst->szName));
        pstCatInnerInst->pstCat = pstCat;

        // 无论如何，都给category分配内存空间
        pstCatInnerInst->pszBuff = calloc(1, pstCat->iMaxMsgSize);
        if (!pstCatInnerInst->pszBuff)
        {
            iErr++;
            break;
        }

        if (!a_pstConf->iDelayInit)
        {
            iRet = tlogany_init(CATINST2DEVANY(pstCatInnerInst),
                    &pstCat->stDevice);
            if (0 == iRet)
            {
                pstCatInnerInst->iInited = 1;
            }
            else
            {
                if (!a_pstConf->iSuppressError || pstCat->iMustSucc)
                {
                    iErr++;
                    break;
                }
            }
        }

#ifdef TSF4G_TLOG_THREAD_SAFE
		iRet = pthread_mutex_init(&pstCatInnerInst->stLogMutex, NULL);
		if (0 != iRet)
		{
			printf("tlog_init: pthread_mutex_init fail\n");
			iErr++;
			tlogany_fini(CATINST2DEVANY(pstCatInnerInst));
			pstCatInnerInst->iInited = 0;
			break;
		}
#endif

        a_pstCtx->stInst.iCount++;
    }/*while (a_pstCtx->stInst.iCount < a_pstConf->iCount)*/

    if (iErr)
    {
        tlog_fini(a_pstCtx);

        return iRet;
    }

    tlog_resolve_i(a_pstCtx);

    if ((iRet = tlog_check_forward_loop(a_pstCtx)))
    {
        printf("tlog conf forward category form loop %d\n", iRet);
        tlog_fini(a_pstCtx);
        return TLOG_ERR_MAKE_ERROR(TLOG_ERROR_LOOP);
    }

    return 0;
}

int tlog_set_module_name(TLOGCTX *a_pstCtx, const char *a_pszModuleName)
{
    if (NULL == a_pszModuleName || NULL == a_pstCtx)
    {
        return -1;
    }
    STRNCPY(a_pstCtx->szModuleName,a_pszModuleName,sizeof(a_pstCtx->szModuleName));
    return 0;
}

int tlog_set_host_name(TLOGCTX *a_pstCtx, const char *a_pszHostName)
{
    if (NULL == a_pszHostName || NULL == a_pstCtx)
    {
        return -1;
    }
    STRNCPY(a_pstCtx->szHostName,a_pszHostName,sizeof(a_pstCtx->szHostName));
    return 0;
}
static int tlog_renameFile(char *originalName, size_t len, const char *NewName);
static int tlog_renameFile(char *originalName, size_t len, const char *NewName)
{
    char *ptr = originalName;
    int i = 0;
    while (*ptr)
    {
        i++;
        ptr++;
    }

#define issep(ptr) ((*(ptr) == '.')||(*(ptr) == ':')||(*(ptr) == '/') || (*(ptr) == '\\'))

    while ((!issep(ptr)) && i > 0)
    {
        ptr--;
        i--;
    }

    //#define issep2(ptr) ((*(ptr) == ':')||(*(ptr) == '/') || (*(ptr) == '\\'))


    if (issep(ptr))
    {
        if (*ptr == '.')
        {
            *ptr = '_';
        }
        i++;
        ptr++;
    }

    STRNCPY(ptr,NewName,len-i);
    return 0;
}

// 克隆一个URI，需要重命名uri并配置新的名字，在这里我们应该如何重命名

int tlog_rename_uri_i(char *pszOrig, char *pszCatName, size_t iSize)
{
    int i = 0;
    int j;
    while (pszOrig[i] && i < iSize)
    {
        i++;
    }

    if (i == iSize)
    {
        return -1;
    }

    for (j = 0; pszCatName[j] && i < iSize; j++)
    {
        pszOrig[i++] = pszCatName[j];
    }

    if (i < iSize)
    {
        pszOrig[i] = 0;
    }

    return 0;
}

int tlog_clone_category(TLOGCATEGORYINST* a_pstParent,
        const char *a_pszChildName)
{
    int iRet;
    TLOGCATEGORYINNERINST *pstParentCategoryInnerInst = NULL;
    TLOGCTX* pstCtx;
    char szChildLogFileName[1024];
    TLOGCATEGORYINNERINST* pstChildCategoryInst;
    TLOGCATEGORY* pstChildCategoryConf;
    if (NULL == a_pstParent || NULL == a_pszChildName)
    {
        return -1;
    }

    pstParentCategoryInnerInst = container(TLOGCATEGORYINNERINST, stCatInst,
            a_pstParent);

    // 如果存在，直接返回
    if (NULL != tlog_get_category(CATINST2LOGCTX(pstParentCategoryInnerInst),
            a_pszChildName))
    {
        return -1;
    }
    pstCtx = CATINST2LOGCTX(pstParentCategoryInnerInst);
    if (pstCtx->stInst.iCount >= TLOG_MAX_CATEGORY)
    {
        return -2;
    }

    pstChildCategoryInst = pstCtx->stInst.astCatInsts + pstCtx->stInst.iCount;
    pstChildCategoryConf = pstCtx->pstConf->astCategoryList
            + pstCtx->pstConf->iCount;

    *pstChildCategoryInst = *pstParentCategoryInnerInst;
    pstChildCategoryInst->iIndex = pstCtx->stInst.iCount;

    pstChildCategoryInst->pstCloneParent = pstParentCategoryInnerInst;
    pstChildCategoryInst->pstCat = pstChildCategoryConf;
    pstChildCategoryInst->pstParent = NULL;
    pstChildCategoryInst->pstForward = NULL;
    assert(NULL != pstParentCategoryInnerInst->pstCat);
    pstChildCategoryInst->pszBuff = calloc(1,
            pstParentCategoryInnerInst->pstCat->iMaxMsgSize);
    if (NULL == pstChildCategoryInst->pszBuff)
    {
        return -3;
    }

    *pstChildCategoryConf = *(pstParentCategoryInnerInst->pstCat);

    STRNCPY(pstChildCategoryInst->szName, a_pszChildName, sizeof(pstChildCategoryInst->szName));
    STRNCPY(pstChildCategoryConf->szName, a_pszChildName, sizeof(pstChildCategoryConf->szName));

    snprintf(szChildLogFileName, sizeof(szChildLogFileName), "%s.log",
            a_pszChildName);

    // Reset the device part.
    memset(CATINST2DEVANY(pstChildCategoryInst), 0, sizeof(TLOGDEVANYINST));
#define RENAME(dst,src,len) tlog_renameFile((dst),(len),(src))

    if (pstChildCategoryConf->stDevice.iType == TLOG_DEV_FILE)
    {
        RENAME(pstChildCategoryConf->stDevice.stDevice.stFile.szPattern, szChildLogFileName, sizeof(pstChildCategoryConf->stDevice.stDevice.stFile.szPattern));
    }
    else if (pstChildCategoryConf->stDevice.iType == TLOG_DEV_VEC)
    {
        int i = 0;
        int fileCount = 0;
        for (i = 0; i < pstChildCategoryConf->stDevice.stDevice.stVec.iCount; i++)
        {
            if (pstChildCategoryConf->stDevice.stDevice.stVec.astElements[i].iType
                    == TLOG_DEV_FILE)
            {
                snprintf(szChildLogFileName, sizeof(szChildLogFileName),
                        "%s_%d.log", a_pszChildName, fileCount);
                fileCount++;
                RENAME(pstChildCategoryConf->stDevice.stDevice.stVec.astElements[i].stDevice.stFile.szPattern, szChildLogFileName, sizeof(pstChildCategoryConf->stDevice.stDevice.stVec.astElements[i].stDevice.stFile.szPattern));
            }
        }
    }
    else if (pstChildCategoryConf->stDevice.iType == TLOG_DEV_MEM)
    {
        // The child should not support forwarding. So I have to use the memlog dev.
        RENAME(pstChildCategoryConf->stDevice.stDevice.stMem.szPattern,szChildLogFileName,sizeof(pstChildCategoryConf->stDevice.stDevice.stMem.szPattern));
        tlog_rename_uri_i(pstChildCategoryConf->stDevice.stDevice.stMem.szUri,
                szChildLogFileName,
                sizeof(pstChildCategoryConf->stDevice.stDevice.stMem.szUri));

    }

    iRet = tlogany_init(CATINST2DEVANY(pstChildCategoryInst),
            &pstChildCategoryConf->stDevice);
    if (0 == iRet)
    {
        pstChildCategoryInst->iInited = 1;
    }
    else
    {
        if (!(pstCtx->pstConf->iSuppressError)
                || (pstChildCategoryConf->iMustSucc))
        {
            free(pstChildCategoryInst->pszBuff);
            pstChildCategoryInst->pszBuff = NULL;
            return -1;
        }
    }

#ifdef TSF4G_TLOG_THREAD_SAFE
	iRet = pthread_mutex_init(&pstChildCategoryInst->stLogMutex, NULL);
	if (0 != iRet)
	{
		printf("clone catelog, pthread_mutex_init fail\n");
		tlogany_fini(CATINST2DEVANY(pstChildCategoryInst));
		pstChildCategoryInst->iInited = 0;
		return -4;
	}
#endif

    pstCtx->stInst.iCount++;
    pstCtx->pstConf->iCount++;
    return 0;
}

static TLOGCATEGORY* tlog_find_catgory_conf_by_name(TLOGCONF* a_pstConf,
        char *name)
{
    int i;
    TLOGCATEGORY* pstCat;
    for (i = 0; i < a_pstConf->iCount; i++)
    {
        pstCat = a_pstConf->astCategoryList + i;
        if (0 == strncmp(name, pstCat->szName, sizeof(pstCat->szName)))
        {
            return pstCat;
        }
    }

    return NULL;
}

static int tlog_reload_category_cfg(TLOGCATEGORY* pstCat,
        TLOGCATEGORY* pstNewCat)
{
    int i;
    pstCat->iPriorityHigh = pstNewCat->iPriorityHigh;
    pstCat->iPriorityLow = pstNewCat->iPriorityLow;

    /*add by vinsonzuo 20110620, fix bug: after reload, MaxMsgSize error*/
    if (pstNewCat->iMaxMsgSize <= 0)
    {
        pstNewCat->iMaxMsgSize = TLOG_DEFAULT_MAX_MSG_SIZE;
    }

    for (i = 0; i < TLOG_VEC_FILTER; i++)
    {
        pstCat->stFilter.astFilters[i] = pstNewCat->stFilter.astFilters[i];
    }
    pstCat->stFilter.iCount = pstNewCat->stFilter.iCount;

    if (pstCat->stDevice.iType == TLOG_DEV_MEM)
    {
        pstCat->stDevice.stDevice.stMem.iEnable
                = pstNewCat->stDevice.stDevice.stMem.iEnable;
    }
    return 0;
}

static int tlog_reload_category(TLOGCATEGORYINNERINST* pstInnerCat,
        TLOGCATEGORY* pstNewCat)
{
    if (!pstInnerCat)
    {
        return -1;
    }

    if (NULL != pstNewCat)
    {
        return tlog_reload_category_cfg(pstInnerCat->pstCat, pstNewCat);
    }
    else
    {
        TLOGCATEGORYINNERINST* pstInnerCatNew;
        pstInnerCatNew = pstInnerCat->pstCloneParent;
        while (NULL != pstInnerCatNew->pstCloneParent)
        {
            pstInnerCatNew = pstInnerCatNew->pstCloneParent;
        }
        return tlog_reload_category_cfg(pstInnerCat->pstCat,
                pstInnerCatNew->pstCat);
    }
}

int tlog_reload(TLOGCTX* a_pstCtx, TLOGCONF* a_pstConf)
{
    TLOGCATEGORY* pstCat;
    TLOGCATEGORY* pstNewCat;
    TLOGCATEGORYINNERINST* pstCatInnerInst;
    int iErr;
    int i;
    char *ctxName = NULL;

    assert(a_pstCtx && a_pstConf);

    iErr = 0;

    a_pstCtx->pstConf->iPriorityHigh = a_pstConf->iPriorityHigh;
    a_pstCtx->pstConf->iPriorityLow = a_pstConf->iPriorityLow;

    for (i = 0; i < a_pstCtx->stInst.iCount; i++)
    {
        pstCatInnerInst = a_pstCtx->stInst.astCatInsts + i;
        pstCat = pstCatInnerInst->pstCat;

        ctxName = pstCat->szName;
        pstNewCat = tlog_find_catgory_conf_by_name(a_pstConf, ctxName);
        tlog_reload_category(pstCatInnerInst, pstNewCat); // Just reload the category.
    }
    return 0;
}

int tlog_fini(TLOGCTX* a_pstCtx)
{
    TLOGCATEGORYINNERINST* pstCatInnerInst;

    assert(a_pstCtx);

    while (a_pstCtx->stInst.iCount > 0)
    {
        a_pstCtx->stInst.iCount--;

        pstCatInnerInst = a_pstCtx->stInst.astCatInsts
                + a_pstCtx->stInst.iCount;

        if (pstCatInnerInst->iInited)
        {
            tlogany_fini(CATINST2DEVANY(pstCatInnerInst));
            pstCatInnerInst->iInited = 0;			
        }

        if (NULL != pstCatInnerInst->pszBuff)
        {
            free(pstCatInnerInst->pszBuff);
            pstCatInnerInst->pszBuff = NULL;
        }

#ifdef TSF4G_TLOG_THREAD_SAFE
		pthread_mutex_destroy(&pstCatInnerInst->stLogMutex);
#endif

    }

    return 0;
}

TLOGCATEGORYINST* tlog_get_category(TLOGCTX* a_pstCtx, const char* a_pszName)
{
    TLOGCATEGORYINNERINST* pstCatInnerInst;
    int i;

    if (NULL == a_pstCtx)
    {
        return NULL;
    }

    if (NULL == a_pszName)
    {
        return NULL;
    }

    for (i = 0; i < a_pstCtx->stInst.iCount; i++)
    {
        pstCatInnerInst = a_pstCtx->stInst.astCatInsts + i;

        if (0 == strcasecmp(pstCatInnerInst->szName, a_pszName))
            return &pstCatInnerInst->stCatInst;
    }

    return NULL;
}

// 获取账单日志空间，先根据日志空间名获取日志category，然后再完成初始化操作
/* 获取账单日志空间要求
 *  1. 日志必须为文件或者向量
 *  2. 文件必须是无限滚动的
 *  3. 日志category的layout必须为%m\n(不强制要求)
 *
 * */
TLOGCATEGORYINST *tlog_get_bill_category(TLOGCTX *a_pstCtx,
        const char *a_pszName)
{
    TLOGCATEGORYINST *pstCat = NULL;
    TLOGCATEGORYINNERINST *pstInnerCat = NULL;
    pstCat = tlog_get_category(a_pstCtx, a_pszName);
    if (!pstCat)
    {
        return NULL;
    }

    pstInnerCat = container(TLOGCATEGORYINNERINST, stCatInst, pstCat);
    // 进行检查。

    // 不应该有过滤器
    if (pstInnerCat->pstCat->stFilter.iCount)
    {
        return NULL;
    }

    if (pstInnerCat->pstCat->stDevice.iType == TLOG_DEV_FILE)
    {
        if (tlogfile_is_bill(&pstInnerCat->stTlogAnyInst.stInst.stAny.stFile))
        {
            return pstCat;
        }
    }

    if (pstInnerCat->pstCat->stDevice.iType == TLOG_DEV_VEC)
    {
        int i;
        for (i = 0; i < pstInnerCat->pstCat->stDevice.stDevice.stVec.iCount; i++)
        {
            // 必须有一个element没有过滤器而且是文件而且符合bill标准
            if (pstInnerCat->pstCat->stDevice.stDevice.stVec.astElements[i].stFilterVec.iCount
                    == 0
                    && pstInnerCat->pstCat->stDevice.stDevice.stVec.astElements[i].iType
                            == TLOG_DEV_FILE
                    && tlogfile_is_bill(
                            &pstInnerCat->stTlogAnyInst.stInst.stAny.stVec.stInst.astElements[i].stFileOrNet.stFile))
            {
                return pstCat;
            }
        }
    }

    return NULL;
}
