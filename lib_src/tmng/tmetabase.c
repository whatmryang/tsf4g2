/* ************************************************************
 @file $RCSfile: tmetabase.c,v $
  meta lib managing 
 $Id: tmetabase.c 10522 2010-07-20 02:22:08Z tomaswang $
 @author $Author: tomaswang $
 @date $Date: 2010-07-20 10:22:08 +0800 (星期二, 2010-07-20) $
 @version 1.0
 @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
 @note Platform: Linux
 **************************************************************/

#include "pal/pal.h"
#include "tmng/terror.h"
#include "tmng/tmng_errno.h"
#include "tmng/tmetabase.h"
#include "tdr/tdr_metalib_kernel_i.h"

#define MB_ACS_FLAG 0664

// local function prototype
LPTDRMETALIB tmb_find_unlocked(IN HTMBDESC, IN const char *, IN int32_t);


int32_t tmb_create(OUT HTMBDESC   *a_phDesc,
                   IN  const char *a_pszKey,
                   IN  int32_t     a_iMaxLibs,
                   IN  int32_t     a_iSize)
{
    HTMBDESC hDesc;
    int32_t  iLen;
    int32_t  iRet;

    iRet = TMNG_ERR_ARGS;
    if (NULL == a_phDesc)
    {
        goto err_tc_return;
    }

    iLen = offsetof(TMETABASE, offs) + sizeof(int32_t) * a_iMaxLibs;
    if (NULL == a_pszKey || a_iMaxLibs <= 0 || a_iSize <= iLen)
    {
        goto err_tc_normal;
    }

    iRet = TMNG_ERR_MEMORY;
    hDesc = (HTMBDESC)calloc(1, sizeof(TMBDESC));
    if (NULL == hDesc)
    {
        goto err_tc_normal;
    }

    iRet = TMNG_ERR_LOCKOPEN;
    hDesc->iLock = tlockopen(a_pszKey,
                             TLOCKF_CREATE|MB_ACS_FLAG,
                             TMB_IPC_TYPE);
    if ((HANDLE)-1 == hDesc->iLock)
    {
        goto err_tc_free;
    }

    iRet = TMNG_ERR_SHMOPEN;
    hDesc->iShm  = tshmopen(a_pszKey,
                            a_iSize,
                            TSHMF_CREATE|MB_ACS_FLAG,
                            TMB_IPC_TYPE);
    if ((HANDLE)-1 == hDesc->iShm)
    {
        goto err_tc_unlock;
    }

    iRet = TMNG_ERR_SHMAT;
    hDesc->pstBase = (LPTMETABASE)tshmat(hDesc->iShm, 0);
    if (NULL == hDesc->pstBase)
    {
        goto err_tc_unlock;
    }

    iRet = TMNG_ERR_VERSION;
    if (0 == hDesc->pstBase->iVersion)
    {
        hDesc->pstBase->iVersion = TMB_VERSION;
        hDesc->pstBase->iSize    = a_iSize;
        hDesc->pstBase->iCurLibs = 0;
        hDesc->pstBase->iMaxLibs = a_iMaxLibs;
    }
    else if (TMB_VERSION != hDesc->pstBase->iVersion)
    {
        goto err_tc_shmdt;
    }

    *a_phDesc = hDesc;

    return 0;

err_tc_shmdt:
    tshmdt(hDesc->pstBase);

err_tc_unlock:
    tlockclose(hDesc->iLock, 0);

err_tc_free:
    free(hDesc);

err_tc_normal:
    *a_phDesc = NULL;

err_tc_return:
    iRet = -iRet;
    T_ERROR_PUSH(iRet, NULL);
    return iRet;
}

int32_t tmb_destroy(IN const char *a_pszKey)
{
    if (NULL == a_pszKey)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    tlockdelete(a_pszKey, TMB_IPC_TYPE);

    if (0 > tshmdelete(a_pszKey, TMB_IPC_TYPE))
    {
        return -TMNG_ERR_SHMDEL;
    }
    return 0;
}

int32_t tmb_open(OUT HTMBDESC   *a_phDesc,
                 IN  const char *a_pszKey,
                 IN  int32_t     a_fReadOnly)
{
    HTMBDESC hDesc;
    int32_t  iRet;

    iRet = TMNG_ERR_ARGS;
    if (NULL == a_phDesc)
    {
        goto err_to_return;
    }

    if (NULL == a_pszKey)
    {
        goto err_to_normal;
    }

    iRet = TMNG_ERR_MEMORY;
    hDesc = (HTMBDESC)calloc(1, sizeof(TMBDESC));
    if (NULL == hDesc)
    {
        goto err_to_normal;
    }

    iRet = TMNG_ERR_SHMOPEN;
    hDesc->iShm  = tshmopen(a_pszKey, 0, 0, TMB_IPC_TYPE);
    if ((HANDLE)-1 == hDesc->iShm)
    {
        goto err_to_free;
    }

    iRet = TMNG_ERR_SHMAT;
    hDesc->pstBase = (LPTMETABASE)tshmat(hDesc->iShm,
                                         a_fReadOnly ? TSHMF_RDONLY : 0);
    if (NULL == hDesc->pstBase)
    {
        goto err_to_shmdt;
    }

    iRet = TMNG_ERR_VERSION;
    if (TMB_VERSION != hDesc->pstBase->iVersion)
    {
        goto err_to_shmdt;
    }

    hDesc->iLock = (HANDLE)-1;
    *a_phDesc = hDesc;

    return 0;

err_to_shmdt:
    tshmdt(hDesc->pstBase);

err_to_free:
    free(hDesc);

err_to_normal:
    *a_phDesc = NULL;

err_to_return:
    iRet = -iRet;
    T_ERROR_PUSH(iRet, NULL);
    return iRet;
}

int32_t tmb_close(INOUT HTMBDESC *a_phDesc)
{
    HTMBDESC hDesc;

    if (NULL == a_phDesc || NULL == *a_phDesc)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    hDesc = *a_phDesc;

    if (NULL != hDesc->pstBase)
    {
        tshmdt(hDesc->pstBase);
        hDesc->pstBase = NULL;
    }

    if ((HANDLE)-1 != hDesc->iShm)
    {
        tshmclose(hDesc->iShm, 0);
        hDesc->iShm = (HANDLE)-1;
    }

    if ((HANDLE)-1 != hDesc->iLock)
    {
        tlockclose(hDesc->iLock, 0);
        hDesc->iLock = (HANDLE)-1;
    }

    free(hDesc);
    *a_phDesc = NULL;

    return 0;
}

int32_t tmb_lock(IN HTMBDESC a_hDesc)
{
    if (NULL == a_hDesc || (HANDLE)-1 == a_hDesc->iLock)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }
    return tlockop(a_hDesc->iLock, 1);
}

int32_t tmb_unlock(IN HTMBDESC a_hDesc)
{
    if (NULL == a_hDesc || (HANDLE)-1 == a_hDesc->iLock)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }
    return tlockop(a_hDesc->iLock, 0);
}

int32_t tmb_dump(IN HTMBDESC  a_hDesc,
                 IN int32_t   a_fDetail,
                 IN FILE     *a_fp)
{
    TMETABASE    *pstBase;
    LPTDRMETALIB  pstLib;
    size_t        iOff;
    int32_t       i;

    if (NULL == a_hDesc)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstBase = a_hDesc->pstBase;

    if (pstBase->iCurLibs <= 0)
        iOff = offsetof(TMETABASE, offs) + sizeof(int32_t) * pstBase->iMaxLibs;
    else
    {
        iOff = pstBase->offs[pstBase->iCurLibs - 1];
        pstLib = (LPTDRMETALIB)((size_t)pstBase + iOff);
        iOff += tdr_size(pstLib);
    }

    fprintf(a_fp,
            "MetaBase Info:\n"
            "Size=%d Used=%Zd MaxLibs=%d CurLibs=%d:\n\n",
            pstBase->iSize, (size_t)iOff, pstBase->iMaxLibs, pstBase->iCurLibs);

    if (!a_fDetail)
        return 0;

    for (i = 0; i < pstBase->iCurLibs; ++i)
    {
        pstLib = (LPTDRMETALIB)((size_t)pstBase + pstBase->offs[i]);
        fprintf(a_fp, "LibInfo (Index=%d):\n\n", i);
        tdr_dump_metalib(pstLib, a_fp);
    }

    return 0;
}

int32_t tmb_dump_files(IN HTMBDESC    a_hDesc,
                       IN int32_t     a_fDetail,
                       IN const char *a_szStyle,
                       IN const char *a_szPath)
{
    TMETABASE      *pstBase;
    LPTDRMETALIB    pstLib;
    int32_t         iOff;
    int32_t         i;
    int32_t         iNameLen;
    FILE           *fp;
    char            szFile[512];

    if (NULL == a_hDesc || NULL == a_szStyle)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstBase = a_hDesc->pstBase;

    if (pstBase->iCurLibs <= 0)
        iOff = offsetof(TMETABASE, offs) + sizeof(int32_t) * pstBase->iMaxLibs;
    else
    {
        iOff = pstBase->offs[pstBase->iCurLibs - 1];
        pstLib = (LPTDRMETALIB)((size_t)pstBase + iOff);
        iOff += (int32_t)tdr_size(pstLib);
    }

    fprintf(stderr,
            "MetaBase Info:\n"
            "Size=%d Used=%d MaxLibs=%d CurLibs=%d:\n\n",
            pstBase->iSize, iOff, pstBase->iMaxLibs, pstBase->iCurLibs);

    if (!a_fDetail)
        return 0;

    for (i = 0; i < pstBase->iCurLibs; ++i)
    {
        pstLib = (LPTDRMETALIB)((size_t)pstBase + pstBase->offs[i]);

        if (a_szPath && a_szPath[0])
        {
            iNameLen = snprintf(szFile,
                                sizeof(szFile),
                                "%s%c%s.%s",
                                a_szPath, TOS_DIRSEP,
                                pstLib->szName, a_szStyle);
            if ((size_t)iNameLen >= sizeof(szFile))
            {
                fprintf(stderr, "name(%s) too long...\n", pstLib->szName);
                continue;
            }

            fp = fopen(szFile, "w+");
            if (NULL == fp)
            {
                fprintf(stderr, "open(%s) fail\n", szFile);
                continue;
            }
        }
        else
            fp = stdout;

        fprintf(stderr, "INFO (index=%d; name=%s)\n\n", i, pstLib->szName);
        if (!strcmp("xml", a_szStyle))
            tdr_save_xml_fp(pstLib, fp);
        else if (!strcmp("tdr", a_szStyle))
        {
            if (stdout != fp)
                tdr_save_metalib_fp(pstLib, fp);
            else
                fprintf(stderr, "warning: binary data print to stdout!\n");
        }
        else if (!strcmp("txt", a_szStyle))
            tdr_dump_metalib(pstLib, fp);
        else
            fprintf(stderr, "format(%s) unknown...\n", a_szStyle);

        if (a_szPath && a_szPath[0])
        {
            fflush(fp);
            fclose(fp);
        }
    }

    return 0;
}

// local function
LPTDRMETALIB tmb_find_unlocked(IN HTMBDESC    a_hDesc,
                               IN const char *a_pszName,
                               IN int32_t     a_iVersion)
{
    TMETABASE    *pstBase;
    LPTDRMETALIB  pstLib;
    LPTDRMETALIB  pstFind;
    int32_t       i;
    int32_t       iVersion;

    if (NULL == a_hDesc || NULL == a_pszName)
    {
        T_ERROR_PUSH(-1, NULL);
        return NULL;
    }

    if (!a_iVersion)
        a_iVersion = TDR_MAX_VERSION;

    pstBase = a_hDesc->pstBase;
    pstFind = NULL;

    for (i = 0; i < pstBase->iCurLibs; ++i)
    {
        pstLib = (LPTDRMETALIB)((size_t)pstBase + pstBase->offs[i]);

        if (0 == stricmp(tdr_get_metalib_name(pstLib), a_pszName))
        {
            iVersion = tdr_get_metalib_version(pstLib);
            if (iVersion > a_iVersion)
                continue;

            if (!pstFind || tdr_get_metalib_version(pstFind) < iVersion)
                pstFind = pstLib;
        }
    }

    return pstFind;
}

// local function
int32_t tmb_append_unlocked(INOUT HTMBDESC     a_hDesc,
                            IN    LPTDRMETALIB a_pstLib)
{
    TMETABASE    *pstBase;
    LPTDRMETALIB  pstFind;
    char         *pszCur;
    size_t        iOff;
    int32_t       iLibVersion;

    if (NULL == a_hDesc || NULL == a_pstLib)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstBase = a_hDesc->pstBase;
    if (pstBase->iCurLibs >= pstBase->iMaxLibs)
    {
        T_ERROR_PUSH(-2, NULL);
        return -TMNG_ERR_FULL;
    }

    iLibVersion = tdr_get_metalib_version(a_pstLib);
    pstFind = tmb_find_unlocked(a_hDesc,
                                tdr_get_metalib_name(a_pstLib),
                                iLibVersion);

    if (pstFind && iLibVersion == tdr_get_metalib_version(pstFind))
    {
        if (tdr_size(a_pstLib) != tdr_size(pstFind))
        {
            T_ERROR_PUSH(-3, NULL);
            return -TMNG_ERR_SIZE;
        }
        return tdr_get_metalib_build_version(a_pstLib) \
            == tdr_get_metalib_build_version(pstFind)
                ? 0 : -TMNG_ERR_COMP;
    }

    if (pstBase->iCurLibs > 0)
    {
        iOff   = pstBase->offs[pstBase->iCurLibs - 1];
        pszCur = (char *)((size_t)pstBase + iOff);

        iOff  += tdr_size((LPTDRMETALIB)pszCur);
        pszCur = (char *)((size_t)pstBase + iOff);
    }
    else
    {
        iOff   = offsetof(TMETABASE, offs)\
                 + sizeof(int32_t) * pstBase->iMaxLibs;
        pszCur = (char *)((size_t)pstBase + iOff);
    }

    if (iOff + tdr_size(a_pstLib) > (size_t)pstBase->iSize)     
    {
        T_ERROR_PUSH(-4, NULL);
        return -TMNG_ERR_SIZE;
    }

    memcpy(pszCur, a_pstLib, tdr_size(a_pstLib));
    pstBase->offs[pstBase->iCurLibs++] = (int32_t)iOff;

    return 0;
}

int32_t tmb_append(INOUT HTMBDESC    a_hDesc,
                   IN    const char *a_pszPath)
{
    LPTDRMETALIB pstLib;
    int32_t      iRet;

    if (NULL == a_hDesc || NULL == a_pszPath || '\0' == a_pszPath[0])
    {
        return -TMNG_ERR_ARGS;
    }

    if (tdr_load_metalib(&pstLib, a_pszPath) < 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_TDR_LOAD_METALIB;
    }

    tmb_lock(a_hDesc);
    iRet = tmb_append_unlocked(a_hDesc, pstLib);
    tmb_unlock(a_hDesc);

    tdr_free_lib(&pstLib);

    return iRet;
}

int32_t tmb_open_metalib(INOUT HTMBDESC    a_hDesc,
                         IN    const char *a_pszLib,
                         IN    int32_t     a_iVersion)
{
    LPTDRMETALIB pstLib;

    if (NULL == a_hDesc || NULL == a_pszLib)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstLib = tmb_find_unlocked(a_hDesc, a_pszLib, a_iVersion);

    if (NULL == pstLib)
    {
        T_ERROR_PUSH(-2, NULL);
        return -TMNG_ERR_TMB_FIND_UNLOCKED;
    }

    a_hDesc->pstLib = pstLib;

    return 0;
}

int32_t tmb_meta_by_name(IN    HTMBDESC    a_hDesc,
                         IN    const char *a_pszName,
                         INOUT LPTDRMETA  *a_ppstMeta)
{
    int32_t    iID;
    char      *pszPos;
    LPTDRMETA  pstMeta;

    if (NULL == a_hDesc || NULL == a_hDesc->pstLib
                        || NULL == a_pszName
                        || NULL == a_ppstMeta)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iID  = strtol(a_pszName, &pszPos, 0);

    if (strlen(a_pszName) == (size_t)(pszPos - a_pszName))
        pstMeta = tdr_get_meta_by_id(a_hDesc->pstLib, iID);
    else
        pstMeta = tdr_get_meta_by_name(a_hDesc->pstLib, a_pszName);

    *a_ppstMeta = pstMeta;

    return NULL == pstMeta ? -TMNG_ERR_NULLMETA : 0;
}

int32_t tmb_meta_by_id(IN    HTMBDESC   a_hDesc,
                       IN    int32_t    a_iID,
                       INOUT LPTDRMETA *a_ppstMeta)
{
    if (NULL == a_hDesc || NULL == a_hDesc->pstLib || NULL == a_ppstMeta)
    {
        T_ERROR_PUSH(-1, "");
        return -TMNG_ERR_ARGS;
    }

    *a_ppstMeta = tdr_get_meta_by_id(a_hDesc->pstLib, a_iID);

    return NULL == *a_ppstMeta ? -TMNG_ERR_NULLMETA : 0;
}

