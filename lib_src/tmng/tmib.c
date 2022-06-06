
#include    "tmng/terror.h"
#include    "tmng/tmng_errno.h"
#include	"pal/ttypes.h"
#include    "tmng/tmib.h"
#define     MIB_ACS_FLAG 0664

#if defined (_WIN32) || defined (_WIN64)
#define stricmp _stricmp
#endif

// local function prototype
int32_t PageAligned_i(IN int32_t);
void FormatTime_i(time_t, char *, int32_t);
int32_t tmib_lock_i(IN HTMIB);
int32_t tmib_unlock_i(IN HTMIB);


// local function
int32_t PageAligned_i(IN int32_t iSize)
{
    int32_t iPageSize;
    int32_t iRet;

    iPageSize = tos_get_pagesize();
    if (iPageSize < 0 || iSize < 0)
    {
        T_ERROR_PUSH(-1, "args error");
        return iSize;
    }

    iRet = iSize % iPageSize;

    return iRet ? iSize + (iPageSize - iRet) : iSize;
}

// local function
void FormatTime_i(time_t tSecond, char *szBuf, int32_t iSize)
{
    struct tm stm;

    if (szBuf && iSize > 0)
    {
        szBuf[0] = '\0';
    }

    bzero(&stm, sizeof(struct tm));

    if (szBuf && iSize > 19 && tSecond > 0 && localtime_r(&tSecond, &stm))
    {
        strftime(szBuf, iSize, "%Y-%m-%d %H:%M:%S", &stm);
    }
}

// local function
int32_t tmib_lock_i(IN HTMIB a_hMib)
{
    if (NULL == a_hMib || (HANDLE)-1 == a_hMib->iLock)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }
    return tlockop(a_hMib->iLock, 1);
}

// local function
int32_t tmib_unlock_i(IN HTMIB a_hMib)
{
    if (NULL == a_hMib || (HANDLE)-1 == a_hMib->iLock)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }
    return tlockop(a_hMib->iLock, 0);
}


int32_t tmib_create(OUT HTMIB      *a_phMib,
                    IN  const char *a_pszKey,
                    IN  int32_t     a_iDatas,
                    IN  int32_t     a_iSize)
{
    HTMIB   hMib;
    int32_t iHeadSize;
    int32_t iRet;

    iRet = TMNG_ERR_ARGS;
    if (NULL == a_phMib)
    {
        goto err_tc_return;
    }

    if (!(a_pszKey && a_iDatas > 0 && a_iSize > 0))
    {
        goto err_tc_normal;
    }

    iRet = TMNG_ERR_SIZE;
    iHeadSize = PageAligned_i(offsetof(TMIBINFO, entries)
                            + sizeof(TMIBENTRY) * a_iDatas);
    if (a_iSize <= iHeadSize)
    {
        goto err_tc_normal;
    }

    iRet = TMNG_ERR_MEMORY;
    hMib = (HTMIB)calloc(1, sizeof(TMIBDESC));
    if (NULL == hMib)
    {
        goto err_tc_normal;
    }

    hMib->iLock   = (HANDLE)-1;
    hMib->iShm    = (HANDLE)-1;
    hMib->bCreate =  1;

    iRet = TMNG_ERR_LOCKOPEN;
    hMib->iLock = tlockopen(a_pszKey,
                            TLOCKF_CREATE|MIB_ACS_FLAG,
                            TMIB_IPC_TYPE);
    if ((HANDLE)-1 == hMib->iLock)
    {
        goto err_tc_free;
    }

    iRet = TMNG_ERR_SHMOPEN;
    hMib->iShm = tshmopen(a_pszKey,
                          a_iSize,
                          TSHMF_CREATE|MIB_ACS_FLAG,
                          TMIB_IPC_TYPE);
    if ((HANDLE)-1 == hMib->iShm)
    {
        goto err_tc_unlock;
    }

    iRet = TMNG_ERR_SHMAT;
    hMib->pstInfo = (LPTMIBINFO)tshmat(hMib->iShm, 0);
    if (NULL == hMib->pstInfo)
    {
        goto err_tc_shmdt;
    }

    iRet = TMNG_ERR_VERSION;
    if (0 == hMib->pstInfo->iVersion)
    {
        hMib->pstInfo->iVersion    = TMIB_VERSION;
        hMib->pstInfo->iSize       = a_iSize;
        hMib->pstInfo->iCurDatas   = 0;
        hMib->pstInfo->iMaxDatas   = a_iDatas;
        hMib->pstInfo->iHeadSize   = iHeadSize;
        hMib->pstInfo->iCheckSum   = 0;
        hMib->pstInfo->iExternSize = DEFAULT_MIB_DATA_SIZE;
    }
    else if (TMIB_VERSION != hMib->pstInfo->iVersion)
    {
        goto err_tc_shmdt;
    }

    *a_phMib = hMib;

    return 0;

err_tc_shmdt:
    tshmclose(hMib->iShm, 0);

err_tc_unlock:
    tlockclose(hMib->iLock, 0);

err_tc_free:
    free(hMib);

err_tc_normal:
    *a_phMib = NULL;

err_tc_return:
    iRet = -iRet;
    T_ERROR_PUSH(iRet, NULL);
    return iRet;
}

int32_t tmib_destroy(IN const char *a_pszKey)
{
    HTMIB      hMib;
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;
    int32_t    i;

    if (NULL == a_pszKey || tmib_open(&hMib, a_pszKey) < 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstInfo = hMib->pstInfo;

    for (i = 0; i < pstInfo->iCurDatas; ++i)
    {
        pstEntry = pstInfo->entries + i;
        if (pstEntry->bExtern)
        {
            tshmclose(pstEntry->iExShm, TSHMF_DELETE);
        }
    }

    tmib_close(&hMib);

    tlockdelete(a_pszKey, TMIB_IPC_TYPE);
    tshmdelete(a_pszKey, TMIB_IPC_TYPE);

    return 0;
}

int32_t tmib_open(OUT HTMIB      *a_phMib,
                  IN  const char *a_pszKey)
{
    HTMIB   hMib;
    int32_t iRet;

    iRet = TMNG_ERR_ARGS;
    if (NULL == a_phMib)
    {
        goto err_to_return;
    }

    if (NULL == a_pszKey)
    {
        goto err_to_normal;
    }

    iRet = TMNG_ERR_MEMORY;
    hMib = (HTMIB)calloc(1, sizeof(TMIBDESC));
    if (NULL == hMib)
    {
        goto err_to_normal;
    }

    iRet = TMNG_ERR_LOCKOPEN;
    hMib->iLock = tlockopen(a_pszKey, 0, TMIB_IPC_TYPE);
    if ((HANDLE)-1 == hMib->iLock)
    {
        goto err_to_free;
    }

    iRet = TMNG_ERR_SHMOPEN;
    hMib->iShm  = tshmopen(a_pszKey, 0, 0, TMIB_IPC_TYPE);
    if ((HANDLE)-1 == hMib->iShm)
    {
        goto err_to_unlock;
    }

    iRet = TMNG_ERR_SHMAT;
    hMib->pstInfo = (LPTMIBINFO)tshmat(hMib->iShm, 0);
    if (NULL == hMib->pstInfo || TMIB_VERSION != hMib->pstInfo->iVersion)
    {
        goto err_to_shmdt;
    }

    *a_phMib = hMib;

    return 0;

err_to_shmdt:
    tshmclose(hMib->iShm, 0);

err_to_unlock:
    tlockclose(hMib->iLock, 0);

err_to_free:
    free(hMib);

err_to_normal:
    *a_phMib = NULL;

err_to_return:
    iRet = -iRet;
    T_ERROR_PUSH(iRet, NULL);
    return -1;
}

int32_t tmib_close(IN HTMIB *a_phMib)
{
    HTMIB hMib;

    if (NULL == a_phMib || NULL == *a_phMib)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    hMib = *a_phMib;

    if (hMib->pstInfo)
    {
        tshmdt(hMib->pstInfo);
    }

    if ((HANDLE)-1 != hMib->iShm)
    {
        tshmclose(hMib->iShm, 0);
    }

    if ((HANDLE)-1 != hMib->iLock)
    {
        tlockclose(hMib->iLock, 0);
    }

    free(hMib);

    *a_phMib = NULL;

    return 0;
}

int32_t tmib_set_extern_size(INOUT HTMIB   a_hMib,
                             IN    int32_t a_iExSize)
{
    if (NULL == a_hMib || tmib_lock_i(a_hMib) < 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    if (a_iExSize <= 0)
    {
        a_iExSize = DEFAULT_MIB_DATA_SIZE;
    }

    a_hMib->pstInfo->iExternSize = PageAligned_i(a_iExSize);

    tmib_unlock_i(a_hMib);

    return 0;
}

int32_t tmib_set_updtime_unlock(INOUT HTMIB     a_hMib,
                                INOUT TMIBDATA *a_pstData,
                                IN    time_t    a_tUpdTime)
{
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;
    int32_t    iRet;
    int32_t    iEntry;

    if (NULL == a_hMib || NULL == a_pstData)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iRet = tmib_get_data(a_hMib, a_pstData, 0);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, NULL);
        return iRet;
    }

    pstInfo = a_hMib->pstInfo;
    iEntry  = a_pstData->iEntry;

    if (iEntry < 0 || iEntry >= a_hMib->pstInfo->iCurDatas)
    {
        T_ERROR_PUSH(-2, NULL);
        return -TMNG_ERR_FULL;
    }
    pstEntry = pstInfo->entries + iEntry;

    if (!a_hMib->bCreate)   /* enable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
    }

    a_hMib->pstInfo->entries[iEntry].tUpdateTime = a_tUpdTime;

    if (!a_hMib->bCreate)   /* disable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
    }

    return 0;
}

int32_t tmib_set_updtime_now(INOUT HTMIB     a_hMib,
                             INOUT TMIBDATA *a_pstData)
{
    int32_t iRet;

    if (NULL == a_hMib || NULL == a_pstData || tmib_lock_i(a_hMib) < 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iRet = tmib_set_updtime_unlock(a_hMib, a_pstData, time(NULL));

    tmib_unlock_i(a_hMib);

    return iRet;
}

int32_t tmib_find_data_unlocked(IN    HTMIB     a_hMib,
                                INOUT TMIBDATA *a_pstData,
                                IN    int32_t   a_bIncludeDelete)
{
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;
    int32_t    i;

    if (NULL == a_hMib || NULL == a_pstData)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstInfo = a_hMib->pstInfo;
    for (i = 0; i < pstInfo->iCurDatas; ++i)
    {
        pstEntry = pstInfo->entries + i;

#define STR_CMP(V) (stricmp(pstEntry->V, a_pstData->V))
#define INT_CMP(V) (pstEntry->V != a_pstData->V)
#define STR_NEQ(V) (a_pstData->V[0] && stricmp(pstEntry->V, a_pstData->V))
#define INT_NEQ(V) (a_pstData->V && pstEntry->V != a_pstData->V)
        if ((pstEntry->bDelete && !a_bIncludeDelete)
                || STR_NEQ(szLib)
                || STR_NEQ(szMeta)
                || STR_NEQ(szDomain)
                || STR_CMP(szName)
                || INT_NEQ(iVersion)
                || INT_NEQ(iPeriods)
                || INT_CMP(iProcID))
        {
            continue;
        }
#undef INT_NEQ
#undef STR_NEQ
#undef INT_CMP
#undef STR_CMP

        a_pstData->iVersion = pstEntry->iVersion;
        a_pstData->iPeriods = pstEntry->iPeriods;
        a_pstData->iEntry   = i;
        a_pstData->bExtern  = pstEntry->bExtern;

        if (pstEntry->bExtern)
        {
            a_pstData->iSize   = pstEntry->iExSize;
            a_pstData->pstHead = (LPTMIBHEAD)tshmat(pstEntry->iExShm, 0);
        }
        else
        {
            a_pstData->iSize   = pstEntry->iSize;
            a_pstData->pstHead = (LPTMIBHEAD)((size_t)pstInfo \
                    + pstEntry->iOff);
        }
        a_pstData->iSize  -= MIB_HEAD_SIZE;
        a_pstData->pszData = (char *)((size_t)a_pstData->pstHead \
                + MIB_HEAD_SIZE);

        return 0;
    }

    T_ERROR_PUSH(-2, NULL);
    return -TMNG_ERR_TMB_FIND_UNLOCKED;
}

int32_t tmib_register_data_unlocked(INOUT HTMIB     a_hMib,
                                    INOUT TMIBDATA *a_pstData)
{
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;
    int32_t    iOff;
    int32_t    iSize;

    if (NULL == a_hMib || NULL == a_pstData || a_pstData->iSize <= 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iSize = PageAligned_i(a_pstData->iSize + MIB_HEAD_SIZE);

    pstInfo = a_hMib->pstInfo;
    if (pstInfo->iCurDatas >= pstInfo->iMaxDatas)
    {
        T_ERROR_PUSH(-2, NULL);
        return -TMNG_ERR_FULL;
    }

    if (pstInfo->iCurDatas > 0)
    {
        pstEntry = pstInfo->entries + pstInfo->iCurDatas - 1;
        iOff = pstEntry->iOff + pstEntry->iSize;
    }
    else
    {
        iOff = pstInfo->iHeadSize;
    }

    if (!a_hMib->bCreate)   /* enable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
    }

    pstEntry = pstInfo->entries + pstInfo->iCurDatas;

#define V_COPY(V) memcpy(pstEntry->V, a_pstData->V, sizeof(a_pstData->V) - 1)
    V_COPY(szLib);
    V_COPY(szMeta);
    V_COPY(szDomain);
    V_COPY(szName);
#undef V_COPY

    pstEntry->iProcID  = a_pstData->iProcID;
    pstEntry->iOff     = iOff;
    pstEntry->iVersion = a_pstData->iVersion;
    pstEntry->iPeriods = a_pstData->iPeriods;

    if (a_pstData->bExtern || a_pstData->iSize > a_hMib->pstInfo->iExternSize)
    {
        pstEntry->bExtern = 1;
        pstEntry->iExSize = iSize;
        pstEntry->iSize  = 0;
        pstEntry->iExShm = tshmopen(NULL,
                                    pstEntry->iExSize,
                                    TSHMF_CREATE|TSHM_DFT_ACCESS|TSHMF_PRIVATE,
                                    TMIB_IPC_TYPE);
        a_pstData->pstHead = (LPTMIBHEAD)tshmat(pstEntry->iExShm, 0);
    }
    else
    {
        pstEntry->bExtern  = 0;
        pstEntry->iExSize  = 0;
        pstEntry->iSize    = iSize;
        a_pstData->pstHead = (LPTMIBHEAD)((size_t)pstInfo + pstEntry->iOff);
    }
    a_pstData->pstHead->iPeriods    = pstEntry->iPeriods;
    a_pstData->pstHead->tUpdateTime = 0;
    a_pstData->pstHead->tReportTime = 0;

    a_pstData->pszData = (char *)((size_t)a_pstData->pstHead + MIB_HEAD_SIZE);
    a_pstData->iSize   = iSize - MIB_HEAD_SIZE;
    a_pstData->iEntry  = pstInfo->iCurDatas++;

    if (!a_hMib->bCreate)   /* disable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
        mprotect(a_pstData->pstHead, pstEntry->iSize, PROT_READ);
    }

    return 0;
}

int32_t tmib_check_entry_unlocked(INOUT HTMIB  a_hMib,
                                  IN TMIBDATA *a_pstData)
{
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;

    if (NULL == a_hMib || NULL == a_pstData)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstInfo  = a_hMib->pstInfo;
    pstEntry = pstInfo->entries + a_pstData->iEntry;

    if (pstEntry->bDelete)
    {
        if (!a_hMib->bCreate)   /* enable write. */
        {
            mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
        }

        pstEntry->bDelete = 0;

        if (!a_hMib->bCreate)   /* disable write. */
        {
            mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
        }
    }

    if (!a_hMib->bCreate)   /* disable write. */
    {
        mprotect(a_pstData->pstHead,
                 a_pstData->iSize + MIB_HEAD_SIZE,
                 PROT_READ);
    }

    return 0;
}

int32_t tmib_register_data(INOUT HTMIB     a_hMib,
                           INOUT TMIBDATA *a_pstData)
{
    int32_t iRet;

    if (NULL == a_hMib || NULL == a_pstData || tmib_lock_i(a_hMib) < 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iRet = tmib_find_data_unlocked(a_hMib, a_pstData, 1) < 0
        ? tmib_register_data_unlocked(a_hMib, a_pstData)
        : tmib_check_entry_unlocked(a_hMib, a_pstData);

    tmib_unlock_i(a_hMib);

    return iRet;
}

int32_t tmib_register_data_by_mbhd(INOUT HTMIB     a_hMib,
                                   IN    HTMBDESC  a_hDesc,
                                   INOUT TMIBDATA *a_pstData)
{
    TMIBINFO  *pstInfo;
    LPTDRMETA  pstMeta;
    int32_t    iSize;
    int32_t    iTdrSize;
    int32_t    iRet;

    if (NULL == a_hMib || NULL == a_hDesc || NULL == a_pstData)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstInfo = a_hMib->pstInfo;

    iRet = tmb_open_metalib(a_hDesc, a_pstData->szLib, a_pstData->iVersion);
    if (iRet < 0)
    {
        T_ERROR_PUSH(-iRet, NULL);
        return iRet;
    }

    iRet = tmb_meta_by_name(a_hDesc, a_pstData->szMeta, &pstMeta);
    if (iRet < 0)
    {
        T_ERROR_PUSH(-iRet, NULL);
        return iRet;
    }

    iTdrSize = (int32_t)tdr_get_meta_size(pstMeta);

    iSize = PageAligned_i(a_pstData->iSize ? a_pstData->iSize : iTdrSize);
    if (iSize < iTdrSize)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_SIZE;
    }

    a_pstData->iSize = iSize;

    return tmib_register_data(a_hMib, a_pstData);
}

int32_t tmib_register_data_by_mbkey(INOUT HTMIB        a_hMib,
                                    IN    const char  *a_pszBase,
                                    INOUT TMIBDATA    *a_pstData)
{
    HTMBDESC hDesc;
    int32_t  iRet;

    if (tmb_open(&hDesc, a_pszBase, 1) < 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iRet = tmib_register_data_by_mbhd(a_hMib, hDesc, a_pstData);

    tmb_close(&hDesc);

    return iRet;
}

int32_t tmib_protect_data(IN TMIBDATA *a_pstData,
                          IN int32_t   a_bReadOnly)
{
    if (NULL == a_pstData
            || NULL == a_pstData->pszData
            || a_pstData->iSize <= 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    return mprotect(a_pstData->pstHead,
                    a_pstData->iSize + MIB_HEAD_SIZE,
                    a_bReadOnly ? PROT_READ : PROT_RDWR);
}

int32_t tmib_unregister_data_unlocked(INOUT HTMIB     a_hMib,
                                      IN    TMIBDATA *a_pstData)
{
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;
    int32_t    iRet;

    if (NULL == a_hMib || NULL == a_pstData)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstInfo = a_hMib->pstInfo;

    a_pstData->szLib[0] = '\0';
    a_pstData->szMeta[0] = '\0';

    iRet = tmib_find_data_unlocked(a_hMib, a_pstData, 1);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, NULL);
        return iRet;
    }

    pstEntry = pstInfo->entries + a_pstData->iEntry;
    if (pstEntry->bDelete)
    {
        return 0;
    }

    if (!a_hMib->bCreate)   /* enable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
    }

    pstEntry->bDelete = 1;

    if (!a_hMib->bCreate)   /* disable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
    }

    return 0;
}

int32_t tmib_unregister_data(INOUT HTMIB     a_hMib,
                             IN    TMIBDATA *a_pstData)
{
    int32_t iRet;

    if (NULL == a_hMib || NULL == a_pstData || tmib_lock_i(a_hMib) < 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iRet = tmib_unregister_data_unlocked(a_hMib, a_pstData);

    tmib_unlock_i(a_hMib);

    return iRet;
}

int32_t tmib_unregister_domain_unlocked(INOUT HTMIB       a_hMib,
                                        IN    const char *a_pszDomain)
{
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;
    int32_t    i;

    if (NULL == a_hMib || NULL == a_pszDomain)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    pstInfo = a_hMib->pstInfo;

    if (!a_hMib->bCreate)   /* enable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
    }

    for (i = 0; i < pstInfo->iCurDatas; ++i)
    {
        pstEntry = pstInfo->entries + i;
        if (0 == stricmp(pstEntry->szDomain, a_pszDomain))
        {
            pstEntry->bDelete = 1;
        }
    }

    if (!a_hMib->bCreate)   /* disable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
    }

    return 0;
}

int32_t tmib_unregister_domain(INOUT HTMIB       a_hMib,
                               IN    const char *a_pszDomain)
{
    int32_t iRet;

    if (NULL == a_hMib || NULL == a_pszDomain || tmib_lock_i(a_hMib) < 0)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iRet = tmib_unregister_domain_unlocked(a_hMib, a_pszDomain);

    tmib_unlock_i(a_hMib);

    return iRet;
}

int32_t tmib_get_data(IN    HTMIB     a_hMib,
                      INOUT TMIBDATA *a_pstData,
                      IN    int32_t   a_bReadOnly)
{
    LPTMIBINFO  pstInfo;
    LPTMIBENTRY pstEntry;
    int32_t     iPageSize;
    int32_t     i;

    if (NULL == a_hMib || NULL == a_pstData)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iPageSize = tos_get_pagesize();
    pstInfo   = a_hMib->pstInfo;

    for (i = 0; i < pstInfo->iCurDatas; ++i)
    {
        pstEntry = pstInfo->entries + i;

#define STR_CMP(V) (stricmp(pstEntry->V, a_pstData->V))
#define INT_CMP(V) (a_pstData->V && pstEntry->V != a_pstData->V)
#define INT_NEQ(V) \
    (a_pstData->V && pstEntry->V && pstEntry->V != a_pstData->V)
        if (pstEntry->bDelete
                || STR_CMP(szDomain)
                || STR_CMP(szName)
                || INT_CMP(iProcID)
                || INT_NEQ(iVersion)
                || INT_NEQ(iPeriods))
        {
            continue;
        }
#undef INT_NEQ
#undef INT_CMP
#undef STR_CMP

        if (pstEntry->iOff % iPageSize)
        {
            T_ERROR_PUSH(-2, NULL);
            return -TMNG_ERR_ALIGN;
        }

        memcpy(a_pstData->szLib, pstEntry->szLib, sizeof(pstEntry->szLib));
        a_pstData->szLib[sizeof(a_pstData->szLib) - 1] = '\0';

        memcpy(a_pstData->szMeta, pstEntry->szMeta, sizeof(pstEntry->szMeta));
        a_pstData->szMeta[sizeof(a_pstData->szMeta) - 1] = '\0';

        a_pstData->iVersion = pstEntry->iVersion;
        a_pstData->iPeriods = pstEntry->iPeriods;
        a_pstData->iEntry   = i;

        if (pstEntry->bExtern)
        {
            a_pstData->iSize   = pstEntry->iExSize;
            a_pstData->pstHead = (LPTMIBHEAD)tshmat(pstEntry->iExShm, 0);
        }
        else
        {
            a_pstData->iSize   = pstEntry->iSize;
            a_pstData->pstHead = (LPTMIBHEAD)((size_t)pstInfo \
                    + pstEntry->iOff);
        }
        a_pstData->iSize   = a_pstData->iSize - MIB_HEAD_SIZE;
        a_pstData->pszData = (char *)((size_t)a_pstData->pstHead \
                + MIB_HEAD_SIZE);

        if (!a_bReadOnly)
        {
            mprotect((char *)a_pstData->pstHead,
                     a_pstData->iSize + MIB_HEAD_SIZE,
                     PROT_RDWR);
        }

        return 0;
    }

    T_ERROR_PUSH(-3, NULL);
    return -TMNG_ERR_TMIB_GET_DATA;
}

int32_t tmib_validate_data(IN    HTMIB     a_hMib,
                           INOUT TMIBDATA *a_pstData,
                           IN    int32_t   a_bReadOnly)
{
    if (NULL == a_hMib
            || NULL == a_pstData
            || a_pstData->iEntry < 0
            || a_pstData->iEntry >= a_hMib->pstInfo->iCurDatas)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    return a_hMib->pstInfo->entries[a_pstData->iEntry].bDelete
       ? tmib_get_data(a_hMib, a_pstData, a_bReadOnly)
       : 0;
}

int32_t tmib_dump_head(IN HTMIB  a_hMib,
                       IN FILE  *a_fp)
{
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;
    char       szTempStr[40];
    int32_t    i;

    if (NULL == a_hMib)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    if (NULL == a_fp)
    {
        a_fp = stdout;
    }

    pstInfo = a_hMib->pstInfo;
    fprintf(a_fp,
            "\nMib Head Data:\n"
            "\t\t size     = %d\n"
            "\t\t maxdatas = %d\n"
            "\t\t curdatas = %d\n"
            "\t\t headsize = %d\n"
            "\t\t bCreate  = %d\n",
            pstInfo->iSize,
            pstInfo->iMaxDatas,
            pstInfo->iCurDatas,
            pstInfo->iHeadSize,
            a_hMib->bCreate);

    for (i = 0; i < pstInfo->iCurDatas; ++i)
    {
        pstEntry = pstInfo->entries + i;

        if (pstEntry->tUpdateTime)
        {
            FormatTime_i(pstEntry->tUpdateTime, szTempStr, sizeof(szTempStr));
        }
        else
        {
            szTempStr[0] = '\0';
        }

        fprintf(a_fp,
                "Data[%04d]: Domain=\"%s\" Name=\"%s\" "
                "size=%d exsize=%d, exshm=%Zd, "
                "ProcID=%d %s %s\n",
                i, pstEntry->szDomain, pstEntry->szName,
                pstEntry->iSize, pstEntry->iExSize, (size_t)pstEntry->iExShm,
                pstEntry->iProcID, szTempStr,
                pstEntry->bDelete ? "[deleted]" : "");
    }

    return 0;
}

int32_t tmib_dump(IN HTMBDESC  a_hDesc,
                  IN TMIBDATA *a_pstData,
                  IN FILE     *a_fp)
{
    LPTDRMETA pstMeta;
    TDRDATA   stHost;
    int32_t   iRet;

    if (NULL == a_hDesc || NULL == a_pstData)
    {
        T_ERROR_PUSH(-1, NULL);
        return -TMNG_ERR_ARGS;
    }

    iRet = tmb_open_metalib(a_hDesc, a_pstData->szLib, a_pstData->iVersion);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, "tmb_open_metalib");
        return iRet;
    }

    iRet = tmb_meta_by_name(a_hDesc, a_pstData->szMeta, &pstMeta);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, "tmb_meta_by_name");
        return iRet;
    }

    stHost.pszBuff = a_pstData->pszData;
    stHost.iBuff   = a_pstData->iSize;

    if (NULL == a_fp)
    {
        a_fp = stdout;
    }

    iRet = tdr_fprintf(pstMeta, a_fp, &stHost, a_pstData->iVersion);
    if (iRet < 0)
    {
        return -TMNG_ERR_TDR_FPRINTF;
    }
    return 0;
}

int32_t tmib_dump_once(IN const char *a_pszBase,
                       IN TMIBDATA   *a_pstData,
                       IN FILE       *a_fp)
{
    HTMBDESC hDesc;
    int32_t  iRet;

    iRet = tmb_open(&hDesc, a_pszBase, 1);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, "tmb_open");
        return iRet;
    }

    iRet = tmib_dump(hDesc, a_pstData, a_fp);

    tmb_close(&hDesc);

    return iRet;
}

int32_t tmib_dump_all(IN HTMIB       a_hMib,
                      IN const char *a_pszBase,
                      IN FILE       *a_fp)
{
    TMIBDATA   stData;
    HTMBDESC   hDesc;
    TMIBINFO  *pstInfo;
    TMIBENTRY *pstEntry;
    char       szReport[25];
    char       szUpdate[25];
    int32_t    i;

    if (!(a_hMib && a_pszBase && tmb_open(&hDesc, a_pszBase, 1) >= 0))
    {
        T_ERROR_PUSH(-1, "args error or tmb_open fail");
        return -TMNG_ERR_ARGS;
    }

    if (NULL == a_fp)
    {
        a_fp = stdout;
    }

    pstInfo = a_hMib->pstInfo;

    for (i = 0; i < pstInfo->iCurDatas; ++i)
    {
        pstEntry = pstInfo->entries + i;

        fprintf(a_fp,
                "Data %d: Domain=\"%s\" Name=\"%s\" "
                "Lib=\"%s\", Meta=\"%s\", ProcID=%d "
                "size=%d, ext=%d %s\n",
                i, pstEntry->szDomain, pstEntry->szName,
                pstEntry->szLib, pstEntry->szMeta, pstEntry->iProcID,
                pstEntry->iSize, pstEntry->iExSize,
                pstEntry->bDelete ? "[deleted]" : "");

        memcpy(stData.szLib, pstEntry->szLib, sizeof(pstEntry->szLib));
        stData.szLib[sizeof(stData.szLib) - 1] = '\0';

        memcpy(stData.szMeta, pstEntry->szMeta, sizeof(pstEntry->szMeta));
        stData.szMeta[sizeof(stData.szMeta) - 1] = '\0';

        stData.iVersion = pstEntry->iVersion;
        stData.iEntry   = i;
        stData.bExtern  = pstEntry->bExtern;

        if (stData.bExtern)
        {
            stData.iSize = pstEntry->iExSize;
            stData.pstHead = (LPTMIBHEAD)tshmat(pstEntry->iExShm, 0);
        }
        else
        {
            stData.iSize = pstEntry->iSize;
            stData.pstHead = (LPTMIBHEAD)((size_t)pstInfo + pstEntry->iOff);
        }
        stData.iSize = stData.iSize - MIB_HEAD_SIZE;
        stData.pszData = (char *)((size_t)stData.pstHead + MIB_HEAD_SIZE);

        FormatTime_i(stData.pstHead->tReportTime, szReport, sizeof(szReport));
        FormatTime_i(stData.pstHead->tUpdateTime, szUpdate, sizeof(szUpdate));

        fprintf(a_fp,
                "        periods=%d, update at %s, reprot at %s\n",
                stData.pstHead->iPeriods, szUpdate, szReport);
        tmib_dump(hDesc, &stData, a_fp);
    }

    tmb_close(&hDesc);

    return 0;
}

