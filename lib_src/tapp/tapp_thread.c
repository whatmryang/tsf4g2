#include "tapp_i.h"
#include "comm/comm.h"
#include "comm/tmempool.h"
#include "assert.h"

int tapp_check_threads(TAPPCTX *a_pstCtx)
{
    if (a_pstCtx->iNthreads > 0)
    {
        tapp_thread_watchdog_check(a_pstCtx);
        // tapp_clear_fini_thread(a_pstCtx);
    }
    return 0;
}

int tapp_get_num_running_thread(TAPPCTX *pstCtx)
{
    LPTMEMPOOL pstPool = gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool;
    int iIdx;
    TAPPTHREADINNERCTX *pstInnerCtx;
    int iCount = 0;

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstInnerCtx = tmempool_get_bypos(pstPool, iIdx);
        if (!pstInnerCtx)
        {
            continue;
        }

        if (pstInnerCtx->iFsm == TAPP_THREAD_STATUS_RUNNING)
        {
            iCount++;
        }
    }
    return iCount;
}

int tapp_thread_exit(TAPPTHREADCTX *pstThreadCtx)
{
    TAPPTHREADINNERCTX *pstInnerCtx;

    TASSERTRET(pstThreadCtx,-1);

    pstInnerCtx = container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);
    pstInnerCtx->iExitMainLoop = 1;
    return 0;
}

int tapp_thread_init_user_rundata(TAPPCTX *pstCtx, const char *pszMetaName,
        TAPPDATA *pstTappData)
{
    intptr_t iLen;

    memset(pstTappData, 0, sizeof(TAPPDATA));
    if (0 == strlen(pszMetaName))
    {
        return 0;
    }

    pstTappData->iMeta = (intptr_t) tdr_get_meta_by_name(
            (LPTDRMETALIB) pstCtx->iLib, pszMetaName);
    if (NULL == (LPTDRMETA) pstTappData->iMeta)
    {
        printf("failed to get meta by name:%s\n", pszMetaName);
        return -1;
    }
    // The name of the rundata. This should be allocated by the user first.
    pstTappData->pszMetaName = strdup(pszMetaName);
    iLen = tdr_get_meta_size((LPTDRMETA) pstTappData->iMeta);

    pstTappData->iLen = iLen;
    pstTappData->pszBuff = calloc(1, iLen);
    if (!pstTappData->pszBuff)
    {
        return -1;
    }

    return 0;
}

// Just free the memory.
int tapp_thread_fini_user_rundata(TAPPDATA *pstTappData)
{
    TASSERTRET(pstTappData, -1);

    free(pstTappData->pszBuff);
    pstTappData->pszBuff = NULL;

    free(pstTappData->pszMetaName);
    pstTappData->pszMetaName = NULL;

    memset(pstTappData, 0, sizeof(TAPPDATA));

    return 0;
}

int tapp_init_thread_rundata(TAPPCTX *pstCtx, TAPPTHREADINNERCTX *pstInner)
{
    memset(pstInner->szRunDataMetaName, 0, sizeof(pstInner->szRunDataMetaName));
    STRNCPY(pstInner->szRunDataMetaName,"tapp_thread_basedata",sizeof(pstInner->szRunDataMetaName));

    pstInner->stInnerRunData.iThreadId = pstInner->stThreadCtx.iThreadIdx;
    pstInner->stMainRunData.pszMetaName = pstInner->szRunDataMetaName;
    pstInner->stMainRunData.iMeta = (intptr_t) tdr_get_meta_by_name(
            (LPTDRMETALIB) (void *) (&g_szMetalib_tapp_basedata_def[0]),
            pstInner->stMainRunData.pszMetaName);
    if (NULL == (LPTDRMETA) pstInner->stMainRunData.iMeta)
    {
        return -1;
    }

    pstInner->stMainRunData.pszBuff = (char *) &pstInner->stToMainRunData;
    pstInner->stMainRunData.iLen = sizeof(pstInner->stToMainRunData);

    return 0;
}

int tapp_create_thread(TAPPCTX *pstCtx, TAPPTHREADCONF *a_pstConf,
        void *a_pvArg, LPTAPPTHREADCTX *ppstThread)
{
    int iRet;
    int iIdx;

    char szBillCatName[256];
    char szLogCategory[128];
    LPTAPPTHREADCTX pstThreadCtx = NULL;
    LPTAPPTHREADINNERCTX pstInnerCtx;
    LPTLOGCATEGORYINST pstLogCat;
    TAPPTHREADCONF *pstThreadConf;

    LPTMEMPOOL pstPool = gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool;
    LPTAPPTHREADCONF a_pstThreadConf;

    assert(NULL != gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool);

    if (!pstCtx)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"NULL ctx passed to me.");
        return -1;
    }
    if (a_pstConf)
    {
        a_pstThreadConf = a_pstConf;
    }
    else
    {
        a_pstThreadConf = &pstCtx->stThreadConf;
    }

    if (!ppstThread || !pstCtx || !a_pstThreadConf || !ppstThread)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Invalid param");
        return -1;
    }

    iIdx = tmempool_alloc_bypos(
            gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool);
    if (iIdx < 0)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to allocate by pos");
        return -1;
    }

    pstInnerCtx = (LPTAPPTHREADINNERCTX) tmempool_get_bypos(
            gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, iIdx);
    if (!pstInnerCtx)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to get ptr by pos");
        goto free_data;
    }

    memset(pstInnerCtx, 0, sizeof(*pstInnerCtx));

    pstInnerCtx->pstAppCtx = pstCtx;
    pstInnerCtx->stConf = *a_pstThreadConf;
    pstThreadConf = &pstInnerCtx->stConf;
    pstThreadCtx = &pstInnerCtx->stThreadCtx;
    pstInnerCtx->pvData = a_pvArg;
    pstInnerCtx->iFsm = TAPP_THREAD_STATUS_CREATED;

    pstThreadCtx->iThreadIdx = iIdx;
    pstThreadCtx->pstAppConf = &pstCtx->stConfData;

    iRet = tapp_thread_init_user_rundata(pstCtx,
            pstThreadConf->szRunDataCumuMeta, &pstThreadCtx->stRunDataCumu);
    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to init rundata cumu");
        goto free_data;
    }

    iRet = tapp_thread_init_user_rundata(pstCtx,
            pstThreadConf->szRunStatusMeta, &pstThreadCtx->stRunDataStatus);
    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to init rundata status");
        goto free_data;
    }

    iRet = tapp_init_thread_rundata(pstCtx, pstInnerCtx);
    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to init thread rundata");
        goto free_data;
    }

    if (gs_stCtxInternal.pszBillConfFile)
    {
        TLOGCATEGORYINST *pstCat = NULL;
        int iRet;
        memset(szBillCatName, 0, sizeof(szBillCatName));
        snprintf(szBillCatName, sizeof(szBillCatName), "%s_thread_bill%d",
                gs_stCtxInternal.pszBillConfFile, iIdx
                        % tmempool_get_max_items(pstPool));

        pstThreadCtx->pstBillCategory = tlog_get_category(&gs_stTapp.stLogCtx,
                szBillCatName);

        if (!pstThreadCtx->pstBillCategory)
        {
            pstCat = tlog_get_category(&gs_stTapp.stLogCtx,
                    TLOG_DEF_CATEGORY_BILL);
            if (!pstCat)
            {
                goto free_data;
            }

            iRet = tlog_clone_category(pstCat, szBillCatName);
            if (iRet)
            {
                tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to clone bill category");
                goto free_data;
            }

            pstThreadCtx->pstBillCategory = tlog_get_category(
                    &gs_stTapp.stLogCtx, szBillCatName);

            if (!pstThreadCtx->pstBillCategory)
            {
                tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to init thread bill(%s)",szBillCatName);
                goto free_data;
            }
        }
    }

    tapp_get_category("texttrace", &pstLogCat);
    memset(szLogCategory, 0, sizeof(szLogCategory));
    if (strlen(pstInnerCtx->stConf.szThreadName))
    {
        snprintf(szLogCategory, sizeof(szLogCategory), "thread_%s_%d",
                pstInnerCtx->stConf.szThreadName, iIdx
                        % tmempool_get_max_items(pstPool));
    }
    else
    {
        snprintf(szLogCategory, sizeof(szLogCategory), "thread_%d", iIdx
                % tmempool_get_max_items(pstPool));
    }

    tlog_clone_category(pstLogCat, szLogCategory);
    tapp_get_category(szLogCategory, &(pstThreadCtx->pstLogCategory));

    pstInnerCtx->pvRingBuff = tapp_get_thread_channel_buffer(pstCtx, iIdx);
    if (!pstInnerCtx->pvRingBuff)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to get thread channel buffer.");
        goto free_data;
    }

    iRet = tbus_channel_open_mem(&pstInnerCtx->stThreadCtx.pstChannel,
            pstThreadCtx->iThreadIdx, TAPP_MAIN_THREAD_ID,
            (pstInnerCtx->pvRingBuff),
            gs_stCtxInternal.stAppThreadCtxInternal.iBlockSize,
            gs_stCtxInternal.stAppThreadCtxInternal.iNeedInitChannel,
            pstCtx->iChannelSize);
    if (iRet)
    {
        printf("thread2main Tbus channel init error:%s\n", tbus_error_string(
                iRet));
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Thread %d thread2main init failed %s",pstInnerCtx->stThreadCtx.iThreadIdx,tbus_error_string(iRet));
        goto free_data;
    }

    iRet = tbus_channel_open_mem(&pstInnerCtx->pstChannelMainToThread,
            TAPP_MAIN_THREAD_ID, pstThreadCtx->iThreadIdx,
            (pstInnerCtx->pvRingBuff),
            gs_stCtxInternal.stAppThreadCtxInternal.iBlockSize, 0,
            pstCtx->iChannelSize);

    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Thread %d main2thread init failed %s",pstInnerCtx->stThreadCtx.iThreadIdx,tbus_error_string(iRet));
        printf("main2thread Tbus channel init error:%s\n", tbus_error_string(
                iRet));
        goto free_data;
    }

    if (ppstThread)
    {
        *ppstThread = pstThreadCtx;
    }

    iRet = pthread_create(&(pstThreadCtx->pstThread), NULL,
            tapp_thread_function, pstInnerCtx);
    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Thread %d Failed to Start",pstInnerCtx->stThreadCtx.iThreadIdx);
        goto free_data;
    }

    tlog_info(gs_stTapp.pstAppCatText,0,0,"Thread %d Created successfully",pstInnerCtx->stThreadCtx.iThreadIdx);

    return 0;

    free_data:

    if (!pstThreadCtx)
    {
        return -1;
    }

    tapp_thread_fini_user_rundata(&pstThreadCtx->stRunDataCumu);
    tapp_thread_fini_user_rundata(&pstThreadCtx->stRunDataStatus);

    if (pstInnerCtx->pstChannelMainToThread)
    {
        tbus_channel_close(&pstInnerCtx->pstChannelMainToThread);
    }

    if (pstInnerCtx->stThreadCtx.pstChannel)
    {
        tbus_channel_close(&pstInnerCtx->stThreadCtx.pstChannel);
    }

    memset(pstInnerCtx, 0, sizeof(*pstInnerCtx));

    tmempool_free(gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, iIdx);

    return -1;
}

// 删除线程内部数据，并调用fini，必须再线程处于fini状态下调用。
int tapp_delete_thread(TAPPCTX *pstCtx, TAPPTHREADCTX *pstThreadCtx)
{
    LPTMEMPOOL pstPool = gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool;
    TAPPTHREADINNERCTX *pstInnerCtx;
    int iRet;

    if (NULL == pstThreadCtx || NULL == pstCtx)
    {
        return -1;
    }

    pstInnerCtx = container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);

    if (pstInnerCtx->iFsm == TAPP_THREAD_STATUS_FINI)
    {
        //void *pvRes;

        tbus_close_channel(&pstInnerCtx->stThreadCtx.pstChannel);
        tbus_close_channel(&pstInnerCtx->pstChannelMainToThread);

        // 这里会等待线程退出，造成阻塞。
        //pthread_join(pstInnerCtx->stThreadCtx.pstThread, &pvRes);
        // tlog_info(gs_stTapp.pstAppCatText,0,0,"Thread %d terminated with %"PRIdPTR"",pstInnerCtx->stThreadCtx.iThreadIdx,
        //        (intptr_t)pvRes);

        iRet = tmempool_free_byptr(pstPool, pstInnerCtx);
        if (iRet)
        {
            tlog_error(gs_stTapp.pstAppCatText,0,0,"Thread %d Failed to free ctx",pstInnerCtx->stThreadCtx.iThreadIdx);
            return -1;
        }

        tapp_thread_fini_user_rundata(&pstThreadCtx->stRunDataCumu);
        tapp_thread_fini_user_rundata(&pstThreadCtx->stRunDataStatus);
    }
    else
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Trying to fini Thread(%d) which is not in status TAPP_THREAD_STATUS_FINI",pstInnerCtx->stThreadCtx.iThreadIdx);
        return -1;
    }

    return 0;
}

TAPP_THREAD_STATUS tapp_get_thread_status(IN TAPPCTX *a_pstCtx,IN TAPPTHREADCTX *a_pstThreadCtx)
{
    LPTAPPTHREADINNERCTX pstInner ;

    if ((NULL == a_pstCtx)||(NULL == a_pstThreadCtx))
    {
        return TAPP_THREAD_STATUS_DEAD;
    }

    pstInner = container(TAPPTHREADINNERCTX,stThreadCtx,a_pstThreadCtx);

    return pstInner->iFsm;
}

// 将线程状态设置为fini.
int tapp_stop_thread(TAPPCTX *pstCtx, TAPPTHREADCTX *pstThreadCtx)
{
    LPTAPPTHREADINNERCTX pstInner =
            container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);

    if (NULL == pstThreadCtx || NULL == pstInner)
    {
        return -1;
    }

    pstInner->iFsm = TAPP_THREAD_STATUS_FINI;
    return 0;
}

int tapp_thread_proc_tick(LPTAPPTHREADINNERCTX pstInner)
{
    if (gs_stCtxInternal.iTick != pstInner->iLastTick)
    {
        if (pstInner->stConf.pfnTick)
        {
            pstInner->stConf.pfnTick(&pstInner->stThreadCtx, pstInner->pvData);
        }
        pstInner->iLastTick = gs_stCtxInternal.iTick;
    }

    return 0;
}

void *tapp_thread_function(void *pvData)
{
    TAPPTHREADINNERCTX *pstInnerCtx = pvData;
    LPTAPPTHREADCTX pstThreadCtx = &pstInnerCtx->stThreadCtx;

    tdr_init_for_thread();
    if (NULL != pstInnerCtx->stConf.pfnInit)
    {
        if (pstInnerCtx->stConf.pfnInit(&pstInnerCtx->stThreadCtx,
                pstInnerCtx->pvData))
        {
            printf("Thread %d init failed \n",
                    pstInnerCtx->stThreadCtx.iThreadIdx);
            pstInnerCtx->iFsm = TAPP_THREAD_STATUS_FINI;
            pthread_exit((void *) -1);
        }
    }

    tlog_info(pstThreadCtx->pstLogCategory,0,0,"Thread %d started \n",pstThreadCtx->iThreadIdx);

    if (NULL == pstInnerCtx->stConf.pfnProc)
    {
        printf("Thread %d has no proc registered \n",
                pstInnerCtx->stThreadCtx.iThreadIdx);

        tdr_fini_for_thread();
        pthread_exit((void *) -1);
    }

    pstInnerCtx->iFsm = TAPP_THREAD_STATUS_RUNNING;

    while (!pstInnerCtx->iExitMainLoop)
    {
        tapp_thread_proc_proc(pstInnerCtx);
        tapp_thread_proc_watchdog(pstInnerCtx);
        tapp_thread_proc_reload(pstInnerCtx);
        tapp_thread_proc_rundata(pstInnerCtx);
        tapp_thread_proc_tick(pstInnerCtx);
        pthread_testcancel();
    }

    if (pstInnerCtx->stConf.pfnFini)
    {
        pstInnerCtx->stConf.pfnFini(&pstInnerCtx->stThreadCtx,
                pstInnerCtx->pvData);
    }

    tlog_info(pstInnerCtx->stThreadCtx.pstLogCategory,0,0,"Thread %d terminated",pstInnerCtx->stThreadCtx.iThreadIdx);
    pstInnerCtx->iFsm = TAPP_THREAD_STATUS_FINI;

    tdr_fini_for_thread();
    return 0;
}

int tapp_thread_proc_watchdog(TAPPTHREADINNERCTX *pstInnerCtx)
{
    pstInnerCtx->iWatchDogCounter = 0;
    return 0;
}

int tapp_thread_watchdog_check(TAPPCTX *pstCtx)
{
    LPTMEMPOOL pstPool = gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool;
    int iIdx;
    TAPPTHREADINNERCTX *pstInnerCtx;

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstInnerCtx = tmempool_get_bypos(pstPool, iIdx);
        if (!pstInnerCtx)
        {
            continue;
        }

        if (pstInnerCtx->iWatchDogEnable && (pstInnerCtx->iFsm
                == TAPP_THREAD_STATUS_RUNNING))
        {
            pstInnerCtx->iWatchDogCounter++;
            if (pstInnerCtx->iWatchDogCounter > 10)
            {
                tlog_fatal(gs_stTapp.pstAppCatData,0, 0,"Thread %d has gone %d ticks without clearing watchdog\n",
                        pstInnerCtx->stThreadCtx.iThreadIdx,pstInnerCtx->iWatchDogCounter);
            }
        }
    }
    return 0;
}

int tapp_get_thread_list(TAPPCTX *pstCtx, LPTAPPTHREADCTX *ppstThread,
        int *piCount)
{
    int i;
    int iIdx;
    LPTAPPTHREADINNERCTX pstInner;

    if (!ppstThread)
    {
        return -1;
    }

    if (!piCount)
    {
        return -1;
    }

    i = 0;

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        if (i < *piCount)
        {
            pstInner
                    = tmempool_get_bypos(
                            gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool,
                            iIdx);
            if (!pstInner)
            {
                continue;
            }

            ppstThread[i] = &(pstInner->stThreadCtx);
        }
        else
        {
            break;
        }
        i++;
    }

    *piCount = i;
    return 0;
}

int tapp_get_thread_num(TAPPCTX *pstCtx)
{
    return tmempool_get_used_items(
            gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool);
}

int tapp_thread_set_exit_flag(TAPPCTX *pstCtx)
{
    int iIdx;
    LPTAPPTHREADINNERCTX pstIternal;

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstIternal = tmempool_get_bypos(
                gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, iIdx);
        if (!pstIternal)
        {
            continue;
        }
        pstIternal->iExitMainLoop = 1;
    }

    return 0;
}

int tapp_wait_thread_exit(TAPPCTX *pstCtx)
{
    int iRet;
    int iTimeOut;

    if (pstCtx->iNthreads <= 0)
    {
        return 0;
    }

    iRet = tapp_thread_set_exit_flag(pstCtx);

    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to set exit flag");
        return -1;
    }

    iTimeOut = 0;
    while (!tapp_are_threads_in_stats(pstCtx, TAPP_THREAD_STATUS_FINI))
    {
        usleep(10000);
        iTimeOut++;
        if (!iTimeOut % 50)
        {
            fprintf(stderr, ".");
        }
    }

    return 0;
}

int tapp_send_to_thread(TAPPCTX *pstCtx, LPTAPPTHREADCTX pstThread, char *buff,
        size_t iBuff)
{
    LPTAPPTHREADINNERCTX pstInternal;
    int iRet = 0;

    if (!pstCtx || !pstThread || !buff || 0 == iBuff)
        return -1;

    pstInternal = container(TAPPTHREADINNERCTX,stThreadCtx,pstThread);

    iRet = tbus_channel_send(pstInternal->pstChannelMainToThread, buff, iBuff,
            0);
    if (iRet)
    {
        tlog_warn(gs_stTapp.pstAppCatText,0,0,"Failed to forward to channel Thread id:%d,%s",pstThread->iThreadIdx,
                tbus_error_string(iRet));
        return -1;
    }
    else
    {
        tlog_debug(gs_stTapp.pstAppCatText,0,0,"Forward succ Thread id:%d,len:%"PRIdPTR"",pstThread->iThreadIdx,iBuff);
    }

    ((TAPP_BASEDATA*) (gs_stCtxInternal.stBaseData.pszBuff))->lDispatchToThreadNum++;

    return 0;
}

int tapp_sendv_to_thread(IN TAPPCTX *pstTappCtx,
                                   IN LPTAPPTHREADCTX pstThread,
                                   IN const struct iovec *a_ptVector,  IN const int a_iVecCnt)
{
    LPTAPPTHREADINNERCTX pstInternal;
    int iRet = 0;

    if ((NULL == pstTappCtx) || (NULL == pstThread) || (NULL == a_ptVector)|| (0 >= a_iVecCnt) )
        return -1;

    pstInternal = container(TAPPTHREADINNERCTX,stThreadCtx,pstThread);

    iRet = tbus_channel_sendv(pstInternal->pstChannelMainToThread, a_ptVector, a_iVecCnt, 0);
    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to call tbus_channel_sendv to send data to channel Thread id:%d,%s",pstThread->iThreadIdx,
            tbus_error_string(iRet));
        return -1;
    }
    else
    {
        tlog_debug(gs_stTapp.pstAppCatText,0,0,"Forward succ Thread id:%d",pstThread->iThreadIdx);
    }

    ((TAPP_BASEDATA*) (gs_stCtxInternal.stBaseData.pszBuff))->lDispatchToThreadNum++;

    return 0;
}

int tapp_forward_to_thread(TAPPCTX *pstCtx, LPTAPPTHREADCTX pstThread,
        char *buff, size_t iBuff)
{
    int iRet = 0;
    struct iovec vec[1];

    if (!pstCtx || !pstThread || !buff || 0 == iBuff)
        return -1;

    vec[0].iov_base = buff;
    vec[0].iov_len = iBuff;
    iRet = tapp_forwardv_to_thread(pstCtx, pstThread, &vec[0],1);

    return 0;
}

int tapp_forwardv_to_thread(IN TAPPCTX *a_pstCtx,
                                      IN TAPPTHREADCTX *a_pstThreadCtx,
                                      IN const struct iovec *a_ptVector,  IN const int a_iVecCnt)
{
    LPTAPPTHREADINNERCTX pstInternal;
    char szTbusHead[1024];
    int iTbusHeadLen;
    int iRet = 0;

    if ((NULL == a_pstCtx) || (NULL == a_pstThreadCtx) || (NULL == a_ptVector) || (0 >= a_iVecCnt))
        return -1;

    pstInternal = container(TAPPTHREADINNERCTX,stThreadCtx,a_pstThreadCtx);

    iTbusHeadLen = sizeof(szTbusHead);
    iRet = tbus_get_bus_head(a_pstCtx->iBus, &szTbusHead[0], &iTbusHeadLen);
    if (0 != iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to get tbus head Thread id:%d,%s",a_pstThreadCtx->iThreadIdx,
            tbus_error_string(iRet));
        return -1;
    }

    iRet = tbus_channel_set_bus_head(pstInternal->pstChannelMainToThread,
        &szTbusHead[0], iTbusHeadLen);
    if (0 != iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to set tbus head Thread id:%d,%s",a_pstThreadCtx->iThreadIdx,
            tbus_error_string(iRet));
        return -1;
    }

    iRet = tbus_channel_forwardv(pstInternal->pstChannelMainToThread, a_ptVector, a_iVecCnt,0);
    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to forward to channel Thread id:%d,%s",a_pstThreadCtx->iThreadIdx,
            tbus_error_string(iRet));
        return -1;
    }
    else
    {
        tlog_debug(gs_stTapp.pstAppCatText,0,0,"Forward succ Thread id:%d",a_pstThreadCtx->iThreadIdx);
    }

    ((TAPP_BASEDATA*) (gs_stCtxInternal.stBaseData.pszBuff))->lDispatchToThreadNum++;

    return 0;
}

int tapp_recv_from_thread(TAPPCTX *pstCtx, LPTAPPTHREADCTX pstThread,
        char *buff, size_t *iBuff)
{
    int iRet =0;
    size_t iSize =0;
    char *szPkg = NULL;


    if (!pstCtx || !buff || (NULL == iBuff))
    {
        return -1;
    }

    iRet = tapp_peek_from_thread(pstCtx,pstThread, &szPkg, &iSize);
    if (0 != iRet)
    {
        return iRet;
    }

    if (iSize > *iBuff)
    {
        tlog_fatal(gs_stTapp.pstAppCatText,0,0,
                   "the recv buff to small(%d), cannot recv the msg(len:%d)",
                   (int)*iBuff,(int)iSize);
        iRet = -3;
    }else
    {
        memcpy(buff,szPkg,iSize);
        *iBuff = iSize;
    }
    if (0 != iRet)
    {
        return iRet;
    }

    return tapp_delete_from_thread(pstCtx, pstThread);
}

//从单个通道联系接收的数据包数，但接收数据包超过此数时，切换到下个通道去接收
#define TAPP_MAX_PKG_NUM_RECV_CONTINUE  10

int tapp_peek_from_thread(IN TAPPCTX *a_pstCtx,
                                    IN LPTAPPTHREADCTX a_pstThreadCtx,
                                    INOUT char **a_ppszBuff,
                                    OUT size_t *a_iBuff)
{
    LPTAPPTHREADINNERCTX pstInternal = NULL;
    int iRet = 0;


    if ((NULL == a_pstCtx) || (NULL == a_ppszBuff) || (NULL == a_iBuff))
    {
        return -1;
    }

    if (NULL != a_pstThreadCtx)
    {
        pstInternal = container(TAPPTHREADINNERCTX,stThreadCtx,a_pstThreadCtx);
        assert(NULL != pstInternal->pstChannelMainToThread);
        iRet = tbus_channel_peek_msg(pstInternal->pstChannelMainToThread, (const char **)a_ppszBuff, a_iBuff,0);
    }else
    {
        //遍历所有线程通道 收取数据
        int i ;
        int iThreadNum = tmempool_get_max_items(gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool);
        if (0 >= iThreadNum)
        {
            return -3;
        }
        if ((0 > gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx) || (gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx >= iThreadNum))
        {
            gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx = 0;
            gs_stCtxInternal.stAppThreadCtxInternal.iChannelPkgNumRecved = 0;
        }
        if (TAPP_MAX_PKG_NUM_RECV_CONTINUE <= gs_stCtxInternal.stAppThreadCtxInternal.iChannelPkgNumRecved)
        {
            gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx = (gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx + 1) % iThreadNum;
            gs_stCtxInternal.stAppThreadCtxInternal.iChannelPkgNumRecved = 0;
        }
        for (i = 0; i < iThreadNum; i++)
        {
            pstInternal = tmempool_get_bypos(gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool,
                gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx);
            if ((NULL == pstInternal) || (pstInternal->iFsm != TAPP_THREAD_STATUS_RUNNING))
            {//尝试下一个通道
                gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx = (gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx + 1) % iThreadNum;
                gs_stCtxInternal.stAppThreadCtxInternal.iChannelPkgNumRecved = 0;
                continue;
            }
            iRet = tbus_channel_peek_msg(pstInternal->pstChannelMainToThread, (const char **)a_ppszBuff, a_iBuff,0);
            if (0 == iRet)
            {
                gs_stCtxInternal.stAppThreadCtxInternal.iChannelPkgNumRecved++;
                break;//已经收到了数据包
            }else
            {
                //尝试下一个通道
                gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx = (gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx + 1) % iThreadNum;
                gs_stCtxInternal.stAppThreadCtxInternal.iChannelPkgNumRecved = 0;
            }
        }/*for (i = 0; i < iThreadNum; i++)*/
    }/*if (NULL != a_pstThreadCtx)*/

    if (iRet != 0)
    {
        if (iRet != TBUS_ERR_CHANNEL_EMPTY)
        {
            tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to recv from tbus channel. %s",tbus_error_string(iRet));
            return -1;
        }
        else
        {
            return -2;
        }
    }else
    {
        //记录路由信息
        gs_stCtxInternal.stAppThreadCtxInternal.iTbusHeadLen
            = sizeof(gs_stCtxInternal.stAppThreadCtxInternal.szTbusHead);
        iRet = tbus_channel_get_bus_head(pstInternal->pstChannelMainToThread,
            gs_stCtxInternal.stAppThreadCtxInternal.szTbusHead,
            &gs_stCtxInternal.stAppThreadCtxInternal.iTbusHeadLen);
        if (0 != iRet)
        {
            tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to get tbus channel head. %s",tbus_error_string(iRet));
        }
        ((TAPP_BASEDATA*) (gs_stCtxInternal.stBaseData.pszBuff))->lRecvFromThreadNum++;
    }

    return 0;
}

int tapp_delete_from_thread(IN TAPPCTX *a_pstCtx, IN LPTAPPTHREADCTX a_pstThreadCtx)
{
    LPTAPPTHREADINNERCTX pstInternal = NULL;
    int iRet = 0;


    if (NULL == a_pstCtx)
    {
        return -1;
    }

    //取出线程相关通道
    if (NULL != a_pstThreadCtx)
    {
        pstInternal = container(TAPPTHREADINNERCTX,stThreadCtx,a_pstThreadCtx);
    }else
    {
        assert(NULL != gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool);
        pstInternal = tmempool_get_bypos(gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool,
            gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx);
    }
    if ((NULL == pstInternal) || (pstInternal->iFsm != TAPP_THREAD_STATUS_RUNNING))
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"prepare to delete msg, but the thread(by id:%d) is not runnig",
            ((NULL == a_pstThreadCtx)?gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx:a_pstThreadCtx->iThreadIdx));
        return -2;
    }

    //删除消息
    assert(NULL != pstInternal->pstChannelMainToThread);
    iRet = tbus_channel_delete_msg(pstInternal->pstChannelMainToThread);

    return iRet;
}


int tapp_thread_recv(TAPPTHREADCTX *pstThreadCtx, char *buff, size_t *iBuff)
{
    int iRet;
    size_t iSize;
    LPTAPPTHREADINNERCTX pstInnerCtx =
            container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);

    if (!pstThreadCtx || !buff || !iBuff || 0 == *iBuff)
    {
        return -1;
    }

    iSize = *iBuff;
    iRet = tbus_channel_recv(pstThreadCtx->pstChannel, buff, &iSize, 0);
    *iBuff = iSize;
    if (iRet)
    {
        if (iRet == (TBUS_ERR_CHANNEL_EMPTY))
        {
            return -2;
        }
        else
        {
            tlog_error(pstThreadCtx->pstLogCategory,0,0,"recv failed %s",tbus_error_string(iRet));
            return -1;
        }
    }
    else
    {
        tlog_debug(pstThreadCtx->pstLogCategory,0,0,"recv succ len:%"PRIdPTR"",iSize);
        pstInnerCtx->stInnerRunData.lTotalRecvPkg++;
    }

    return 0;
}

int tapp_thread_peek(IN TAPPTHREADCTX *pstThreadCtx,
                               INOUT char **a_ppBuff, OUT size_t *a_iBuff)
{
    int iRet;
    LPTAPPTHREADINNERCTX pstInnerCtx;

    if ((NULL == pstThreadCtx) || (NULL == a_ppBuff)|| (NULL == a_iBuff))
    {
        return -1;
    }

    pstInnerCtx = container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);
    assert(NULL != pstThreadCtx->pstChannel);
    iRet = tbus_channel_peek_msg(pstThreadCtx->pstChannel, (const char **)a_ppBuff, a_iBuff, 0);
    if (iRet)
    {
        if (iRet == (TBUS_ERR_CHANNEL_EMPTY))
        {
            return -2;
        }
        else
        {
            tlog_error(pstThreadCtx->pstLogCategory,0,0,"recv failed %s",tbus_error_string(iRet));
            return -1;
        }
    }
    else
    {
        tlog_debug(pstThreadCtx->pstLogCategory,0,0,"recv succ len:%"PRIdPTR"",*a_iBuff);
        pstInnerCtx->stInnerRunData.lTotalRecvPkg++;
    }

    return 0;
}

/**
* @brief delete the first msg in the channel of main thread to thread,
* @param [in] pstThreadCtx the thread ctx.
* @retval 0 successful
* @retval <0 failed
*/
int tapp_thread_delete(IN TAPPTHREADCTX *pstThreadCtx)
{
    int iRet;

    if (NULL == pstThreadCtx)
    {
        return -1;
    }

    assert(NULL != pstThreadCtx->pstChannel);
    iRet = tbus_channel_delete_msg(pstThreadCtx->pstChannel);

    return iRet;
}


int tapp_thread_send(TAPPTHREADCTX *pstThreadCtx, char *buff, size_t iBuff)
{
    int iRet;
    LPTAPPTHREADINNERCTX pstInnerCtx =
            container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);

    if (!pstThreadCtx || !buff || iBuff == 0)
    {
        return -1;
    }

    iRet = tbus_channel_send(pstThreadCtx->pstChannel, buff, iBuff, 0);
    if (iRet)
    {
        tlog_error(pstThreadCtx->pstLogCategory,0,0,"send failed %s",tbus_error_string(iRet));
        pstInnerCtx->stInnerRunData.lTotalSendFailPkg++;
    }
    else
    {
        tlog_debug(pstThreadCtx->pstLogCategory,0,0,"send data success len %"PRIdPTR"",iBuff);
        pstInnerCtx->stInnerRunData.lTotalSendPkg++;
    }
    return iRet;
}

int tapp_thread_sendv(IN TAPPTHREADCTX *pstThreadCtx,
                                IN const struct iovec *a_ptVector,  IN const int a_iVecCnt)
{
    int iRet;
    LPTAPPTHREADINNERCTX pstInnerCtx =
        container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);

    if (!pstThreadCtx || (NULL == a_ptVector) || (0 >= a_iVecCnt))
    {
        return -1;
    }

    pstInnerCtx = container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);
    assert(NULL != pstThreadCtx->pstChannel);
    iRet = tbus_channel_sendv(pstThreadCtx->pstChannel, a_ptVector,a_iVecCnt, 0);
    if (iRet)
    {
        tlog_error(pstThreadCtx->pstLogCategory,0,0,"send failed %s",tbus_error_string(iRet));
        pstInnerCtx->stInnerRunData.lTotalSendFailPkg++;
    }
    else
    {
        tlog_debug(pstThreadCtx->pstLogCategory,0,0,"send data success ");
        pstInnerCtx->stInnerRunData.lTotalSendPkg++;
    }
    return iRet;
}

int tapp_thread_backward(TAPPTHREADCTX *pstThreadCtx, char *buff, size_t iBuff)
{
    int iRet;
    LPTAPPTHREADINNERCTX pstInnerCtx =
            container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);

    if (!pstThreadCtx || !buff || 0 == iBuff)
    {
        return -1;
    }

    iRet = tbus_channel_backward(pstThreadCtx->pstChannel, buff, iBuff, 0);
    if (iRet)
    {
        tlog_error(pstThreadCtx->pstLogCategory,0,0,"Thread %d failed to backward msg %s",pstThreadCtx->iThreadIdx,tbus_error_string(iRet));
        pstInnerCtx->stInnerRunData.lTotalSendFailPkg++;
        return -1;
    }

    tlog_debug(pstThreadCtx->pstLogCategory,0,0,"Thread %d Backward msg succ len:%"PRIdPTR"",pstThreadCtx->iThreadIdx,iBuff);
    pstInnerCtx->stInnerRunData.lTotalSendPkg++;
    return 0;
}

int tapp_thread_backwardv(IN TAPPTHREADCTX *pstThreadCtx,
                                    IN const struct iovec *a_ptVector,  IN const int a_iVecCnt)
{
    int iRet;
    LPTAPPTHREADINNERCTX pstInnerCtx =
        container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);

    if (!pstThreadCtx || (NULL ==a_ptVector) || (0 >= a_iVecCnt))
    {
        return -1;
    }

    iRet = tbus_channel_backwardv(pstThreadCtx->pstChannel, a_ptVector, a_iVecCnt, 0);
    if (iRet)
    {
        tlog_error(pstThreadCtx->pstLogCategory,0,0,"Thread %d failed to backward msg %s",pstThreadCtx->iThreadIdx,tbus_error_string(iRet));
        pstInnerCtx->stInnerRunData.lTotalSendFailPkg++;
        return -1;
    }

    tlog_debug(pstThreadCtx->pstLogCategory,0,0,"Thread %d Backward msg succ ",pstThreadCtx->iThreadIdx);
    pstInnerCtx->stInnerRunData.lTotalSendPkg++;
    return 0;
}


// 检查线程状态，并调用相应的状态改变回调函数
int tapp_clear_fini_thread(TAPPCTX *pstCtx, void *pvArgMain)
{
    LPTMEMPOOL pstPool = gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool;
    int iIdx;
    TAPPTHREADINNERCTX *pstInnerCtx;

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstInnerCtx = tmempool_get_bypos(pstPool, iIdx);
        if (!pstInnerCtx)
        {
            continue;
        }

        if (pstInnerCtx->iFsm == TAPP_THREAD_STATUS_FINI)
        {
            if (pstInnerCtx->stConf.pfnCleanup)
            {
                tlog_info(gs_stTapp.pstAppCatText,0,0,"Calling thread cleanup func");
                pstInnerCtx->stConf.pfnCleanup(pstCtx, &pstInnerCtx->stThreadCtx, pvArgMain,
                        pstInnerCtx->pvData);
            }
            tlog_info(gs_stTapp.pstAppCatText,0,0,"Finishing %d thread\n", iIdx);
        }
    }
    return 0;
}

int tapp_is_thread_running(TAPPCTX* pstCtx, TAPPTHREADCTX *pstThreadCtx)
{
    TAPPTHREADINNERCTX *pstInnerCtx =
            container(TAPPTHREADINNERCTX,stThreadCtx,pstThreadCtx);
    if (NULL == pstThreadCtx)
    {
        return -1;
    }

    return pstInnerCtx->iFsm == TAPP_THREAD_STATUS_RUNNING;
}

void *tapp_get_thread_channel_buffer(TAPPCTX* pstCtx, int iThreadIdx)
{
    return (char *) (gs_stCtxInternal.stAppThreadCtxInternal.pvChannelBuff)
            +\
 (iThreadIdx % tmempool_get_max_items(
                    gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool))\

                    * gs_stCtxInternal.stAppThreadCtxInternal.iBlockSize;
}

int tapp_def_thread_init(TAPPCTX* pstCtx, void* pvArg)
{
    int iRet;
    size_t iSize;

    LPTAPPTHREADINTERNELDATA pstInternal =
            &gs_stCtxInternal.stAppThreadCtxInternal;

    if (pstCtx->iNthreads == 0)
    {
        return 0;
    }
    else if (pstCtx->iNthreads < 0)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Invalid thread num %d"
                ,pstCtx->iNthreads);
        return -1;
    }

    TASSERT(pstCtx->iNthreads > 0);

    tapp_init_thread_conf(pstCtx);

    iRet = tmempool_new(&(pstInternal->pstThreadPool), pstCtx->iNthreads,
            sizeof(TAPPTHREADINNERCTX));
    if (iRet)
    {

        tlog_error(gs_stTapp.pstAppCatText,0,0,
                "Failed to allocate inner ctx of Thread" );
        return -1;
    }

    iSize = tbus_channel_calc_mem_size(pstCtx->iChannelSize);
    gs_stCtxInternal.stAppThreadCtxInternal.iBlockSize = iSize;
    iSize *= pstCtx->iNthreads;

    if (gs_stCtxInternal.iUseShmChannel)
    {
        int iRet;
        char szName[1024];
        int iCreated = 0;

        tlog_trace(gs_stTapp.pstAppCatText,0,0,"Initing tapp thread using shm channels.");
        // Use business id here.
        memset(szName, 0, sizeof(szName));
        if (pstCtx->pszId && pstCtx->pszGCIMKey)
        {
            snprintf(szName, sizeof(szName) - 1,
                    "threadChannelShm_%d_%s_%s_%s", pstCtx->iId,
                    pstCtx->argv[0], pstCtx->pszId, pstCtx->pszGCIMKey);
        }
        else if (pstCtx->pszId)
        {
            snprintf(szName, sizeof(szName) - 1, "threadChannelShm_%d_%s_%s",
                    pstCtx->iId, pstCtx->argv[0], pstCtx->pszId);
        }
        else if (pstCtx->pszGCIMKey)
        {
            snprintf(szName, sizeof(szName) - 1, "threadChannelShm_%d_%s_%s",
                    pstCtx->iId, pstCtx->argv[0], pstCtx->pszGCIMKey);
        }
        else
        {
            snprintf(szName, sizeof(szName) - 1, "threadChannelShm_%d_%s",
                    pstCtx->iId, pstCtx->argv[0]);
        }

        if (gs_stCtxInternal.iResetShm)
        {
            tcommshm_remove(szName);
        }

        iRet = tcommshm_init(&gs_stCtxInternal.stAppThreadCtxInternal.pstShm,
                szName, iSize, &iCreated);
        if (iRet)
        {
            tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to init the thread channel shm");
            return -1;
        }

        iRet = tcommshm_get(gs_stCtxInternal.stAppThreadCtxInternal.pstShm,
                &gs_stCtxInternal.stAppThreadCtxInternal.pvChannelBuff, NULL);

        if (iRet)
        {
            tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to get the thread channel shm");
            return -1;
        }

        gs_stCtxInternal.stAppThreadCtxInternal.iNeedFree = 0;
        gs_stCtxInternal.stAppThreadCtxInternal.iNeedInitChannel
                = gs_stCtxInternal.iResetShm;

    }
    else
    {
        gs_stCtxInternal.stAppThreadCtxInternal.pvChannelBuff
                = calloc(1, iSize);
        TASSERT(gs_stCtxInternal.stAppThreadCtxInternal.pvChannelBuff);
        if (!gs_stCtxInternal.stAppThreadCtxInternal.pvChannelBuff)
        {
            tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to allocate thread bus buffer");
            return -1;
        }

        gs_stCtxInternal.stAppThreadCtxInternal.iNeedFree = 1;
        gs_stCtxInternal.stAppThreadCtxInternal.iNeedInitChannel = 1;
    }

    tlog_info(gs_stTapp.pstAppCatText,0,0,"Tapp thread init done.");
    return 0;
}

int tapp_def_thread_fini(TAPPCTX* pstCtx, void* pvArg)
{
    if (gs_stCtxInternal.stAppThreadCtxInternal.iNeedFree)
    {
        free(gs_stCtxInternal.stAppThreadCtxInternal.pvChannelBuff);
    }

    return 0;
}

int tapp_init_thread_conf(TAPPCTX *a_pstCtx)
{
    if (a_pstCtx->iChannelSize < 1024)
    {
        a_pstCtx->iChannelSize = 1024000;
    }
    return 0;
}

int tapp_thread_proc_proc(TAPPTHREADINNERCTX *pstInnerCtx)
{
    int iRet;
    if (TAPP_THREAD_STATUS_RUNNING == pstInnerCtx->iFsm)
    {
        if (NULL != pstInnerCtx->stConf.pfnProc)
        {
            struct timeval stEnd;
            struct timeval stStart;
            struct timeval stSub;
            float fCost;

            gettimeofday(&stStart, NULL);
            iRet = pstInnerCtx->stConf.pfnProc(&pstInnerCtx->stThreadCtx,
                    pstInnerCtx->pvData);
            gettimeofday(&stEnd, NULL);

            TV_DIFF(stSub, stEnd, stStart);
            fCost = (float) (stSub.tv_sec * 1000 + stSub.tv_usec / 1000.0);

            pstInnerCtx->stInnerRunData.lProcTotalNum++;
            pstInnerCtx->stInnerRunData.fProcMaxTime
                    = tmax(fCost,pstInnerCtx->stInnerRunData.fProcMaxTime);
            pstInnerCtx->stInnerRunData.fProcMinTime
                    = tmin(fCost,pstInnerCtx->stInnerRunData.fProcMinTime);
            pstInnerCtx->stInnerRunData.fProcTotalTime += fCost;

            if (iRet)
            {
                pstInnerCtx->iIdleCount++;
            }

            if (pstInnerCtx->iIdleCount > pstInnerCtx->pstAppCtx->iIdleCount)
            {
                pstInnerCtx->iIdleCount = 0;
                pstInnerCtx->stInnerRunData.lIdleTotalNum++;
                usleep(1000 * pstInnerCtx->pstAppCtx->iIdleSleep);
            }
        }
    }
    else
    {
    	pstInnerCtx->iExitMainLoop = 1;
    }
    return 0;
}

int tapp_thread_rundata_log(TAPPTHREADINNERCTX *pstInner,
        TAPPDATA *a_pstTappData)
{
    int iRet = 0;

    char szMsg[4096];
    TDRDATA stOut;
    TDRDATA stHost;

    if (0 == a_pstTappData->iMeta || 0 == a_pstTappData->iLen || NULL
            == a_pstTappData->pszBuff)
        return -1;

    stOut.iBuff = sizeof(szMsg);
    stOut.pszBuff = szMsg;
    stHost.iBuff = a_pstTappData->iLen;
    stHost.pszBuff = a_pstTappData->pszBuff;

    iRet = tdr_sprintf((LPTDRMETA) (a_pstTappData->iMeta), &stOut, &stHost, 0);
    if (0 > iRet)
        return -1;

    tlog_info(pstInner->stThreadCtx.pstLogCategory, 0, 0, "%s", szMsg);

    return 0;
}

int tapp_thread_proc_rundata(TAPPTHREADINNERCTX *pstInner)
{
    if (pstInner->iNeedThreadRunData)
    {
        pstInner->stToMainRunData = pstInner->stInnerRunData;
        pstInner->stInnerRunData.fProcMaxTime = 0.0;
        pstInner->stInnerRunData.fProcMinTime = 1000000000.0;
        pstInner->iNeedThreadRunData = 0;

        tapp_thread_rundata_log(pstInner, &pstInner->stThreadCtx.stRunDataCumu);
        tapp_thread_rundata_log(pstInner,
                &pstInner->stThreadCtx.stRunDataStatus);

    }
    return 0;
}

int tapp_thread_proc_reload(TAPPTHREADINNERCTX *pstInnerCtx)
{
    if (1 == pstInnerCtx->iNeedReload)
    {
        if (pstInnerCtx->stConf.pfnReload)
        {
            pstInnerCtx->stConf.pfnReload(&pstInnerCtx->stThreadCtx,
                    pstInnerCtx->pvData);
        }
        pstInnerCtx->iNeedReload = 0;
    }
    return 0;
}

int tapp_set_thread_status(TAPPCTX *pstCtx, int iStatus)
{
    int iIdx;
    LPTAPPTHREADINNERCTX pstIternal;

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstIternal = tmempool_get(
                gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, iIdx);

        if (!pstIternal)
        {
            continue;
        }
        pstIternal->iFsm = iStatus;
    }
    return 0;
}

int tapp_are_threads_in_stats(TAPPCTX *pstCtx, int iStatus)
{
    int iIdx;
    LPTAPPTHREADINNERCTX pstIternal;

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstIternal = tmempool_get_bypos(
                gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, iIdx);

        if (!pstIternal)
        {
            continue;
        }

        if (iStatus != pstIternal->iFsm)
        {
            return 0;
        }
    }
    return 1;
}

int tapp_have_threads_in_stats(TAPPCTX *pstCtx, int iStatus)
{
    int iIdx;
    LPTAPPTHREADINNERCTX pstIternal;
    assert(pstCtx);
    if (pstCtx->iNthreads <= 0)
    {
        return 0;
    }

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstIternal = tmempool_get_bypos(
                gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, iIdx);

        if (!pstIternal)
        {
            continue;
        }

        if (iStatus == pstIternal->iFsm)
        {
            return 1;
        }
    }
    return 0;
}

int tapp_reload_thread(TAPPCTX *pstCtx)
{
    int iIdx;
    LPTAPPTHREADINNERCTX pstIternal;

    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstIternal = tmempool_get_bypos(
                gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, iIdx);

        if (!pstIternal)
        {
            continue;
        }
        pstIternal->iNeedReload = 1;
    }
    return 0;
}

int tapp_backward_from_thread(IN TAPPCTX *a_pstCtx, char *a_buff, size_t iBuff)
{
    int iRet;
    int iSrc = 0;
    int iDst = 0;

    if (!a_pstCtx || !a_buff || 0 == iBuff)
    {
        return -1;
    }

    iRet = tbus_set_bus_head(a_pstCtx->iBus,
            gs_stCtxInternal.stAppThreadCtxInternal.szTbusHead,
            gs_stCtxInternal.stAppThreadCtxInternal.iTbusHeadLen);
    if (0 != iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to set tbus head. %s",tbus_error_string(iRet));
        return -1;
    }

    iRet = tbus_backward(a_pstCtx->iBus, &iSrc, &iDst, a_buff, iBuff, 0);
    if (iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText,0,0,
                "Failed to backward msg%s",
                tbus_error_string(iRet));
        return -1;
    }

    return 0;
}

TSF4G_API int tapp_backwardv_from_thread(IN TAPPCTX *a_pstCtx,
										 IN const struct iovec *a_ptVector,  IN const int a_iVecCnt)
{
	int iRet;
	int iSrc = 0;
	int iDst = 0;

	if ((NULL == a_pstCtx) || (NULL == a_ptVector) || (0 >= a_iVecCnt))
	{
		return -1;
	}

	iRet = tbus_set_bus_head(a_pstCtx->iBus,
		gs_stCtxInternal.stAppThreadCtxInternal.szTbusHead,
		gs_stCtxInternal.stAppThreadCtxInternal.iTbusHeadLen);
	if (0 != iRet)
	{
		tlog_error(gs_stTapp.pstAppCatText,0,0,"Failed to set tbus head. %s",tbus_error_string(iRet));
		return iRet;
	}

	iRet = tbus_backwardv(a_pstCtx->iBus, &iSrc, &iDst, a_ptVector, a_iVecCnt, 0);
	if (iRet)
	{
		tlog_error(gs_stTapp.pstAppCatText,0,0,
			"Failed to tbus_backwardv msg%s",
			tbus_error_string(iRet));
	}

	return iRet;
}

int tapp_send_thread_rundata_req(TAPPCTX *pstCtx)
{
    int iIdx;
    LPTAPPTHREADINNERCTX pstIternal;
    for (iIdx = 0; iIdx < pstCtx->iNthreads; iIdx++)
    {
        pstIternal = tmempool_get_bypos(
                gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, iIdx);

        if (!pstIternal)
        {
            continue;
        }
        pstIternal->iNeedThreadRunData = 1;
    }
    return 0;
}

int tapp_collect_thread_rundata(TAPPCTX *pstCtx)
{
    int i;
    LPTAPPTHREADINNERCTX pstIternal;

    for (i = 0; i < pstCtx->iNthreads; i++)
    {
        pstIternal = tmempool_get_bypos(
                gs_stCtxInternal.stAppThreadCtxInternal.pstThreadPool, i);

        if (!pstIternal)
        {
            continue;
        }
        if (!pstIternal->iNeedThreadRunData)
        {
            tapp_rundata_log(&pstIternal->stMainRunData);
            pstIternal->iNeedThreadRunData = 1;
        }
    }
    return 0;
}

