/*
 **  @file $RCSfile: tlog_category.c,v $
 **  general description of this module
 **  $Id: tlog_category.c,v 1.11 2009-03-27 06:17:02 kent Exp $
 **  @author $Author: kent $
 **  @date $Date: 2009-03-27 06:17:02 $
 **  @version $Revision: 1.11 $
 **  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
 **  @note Platform: Linux
 */

#include "tlog_i.h"
#include "tlogmem.h"
#include "comm/comm.h"

#define evt_is_enabled(cat, priority) \
    ( ( TLOG_DEV_NO != (cat)->stDevice.iType ) && ( TLOG_PRIORITY_NULL==(cat)->iPriorityHigh || (priority)>=(cat)->iPriorityHigh ) \
    && ( TLOG_PRIORITY_NULL==(cat)->iPriorityLow || (priority)<=(cat)->iPriorityLow ) )

int tlog_category_log_in(TLOGCATEGORYINNERINST *a_pstCatInnerInst,
        TLOGEVENTBASE *a_pstEvt);

int tlog_category_append(TLOGCATEGORYINNERINST *a_pstCatInnerInst,
        TLOGEVENTBASE *a_pstEvt)
{
    TLOGIOVEC astIOVec[3];
    int iVec;
    int iRet;

    if (NULL == a_pstCatInnerInst)
    {
        return -1;
    }

    if (!evt_is_enabled(a_pstCatInnerInst->pstCat, a_pstEvt->evt_priority))
    {
        return 0;
    }

    if (!a_pstCatInnerInst->iInited)
    {
        if (-1 == tlogany_init(CATINST2DEVANY(a_pstCatInnerInst),
				&a_pstCatInnerInst->pstCat->stDevice))
			return -1;
        a_pstCatInnerInst->iInited = 1;
    }

    iVec = 0;
    if (0 < a_pstEvt->evt_msg_prefix_len)
    {
        astIOVec[iVec].iov_base = (void*) a_pstEvt->evt_msg_prefix_buffer;
        astIOVec[iVec].iov_len = (size_t) a_pstEvt->evt_msg_prefix_len;
        iVec++;
    }

    astIOVec[iVec].iov_base = (void*) a_pstEvt->evt_msg;
    astIOVec[iVec].iov_len = (size_t) a_pstEvt->evt_msg_len;
    iVec++;

    if (0 < a_pstEvt->evt_msg_sufix_len)
    {
        astIOVec[iVec].iov_base = (void*) a_pstEvt->evt_msg_sufix_buffer;
        astIOVec[iVec].iov_len = (size_t) a_pstEvt->evt_msg_sufix_len;
        iVec++;
    }

    iRet = tlogany_writev(CATINST2DEVANY(a_pstCatInnerInst), a_pstEvt->evt_id,
            a_pstEvt->evt_cls, astIOVec, iVec);

    return iRet;
}

int tlog_category_get_buffer(TLOGCATEGORYINNERINST *a_pstCatInnerInst,
        char **ppszBuff)
{
    int iRet;

    if (TLOG_DEV_MEM == a_pstCatInnerInst->pstCat->stDevice.iType
            && a_pstCatInnerInst->pstCat->stDevice.stDevice.stMem.iEnable)
    {
        TLOGANY *pstDev;
        pstDev = CATINST2DEVANY(a_pstCatInnerInst);

        iRet = tlogmem_get_buff(&pstDev->stInst.stAny.stMem, ppszBuff,
                a_pstCatInnerInst->pstCat->iMaxMsgSize);
        if (iRet)
        {
            goto fail;
        }
        return 0;
    }

    // 如果获取出错，就使用当前的buffer
    fail: if (a_pstCatInnerInst->pszBuff)
    {
        *ppszBuff = a_pstCatInnerInst->pszBuff;
        return 0;
    }
    return -1;
}

int tlog_category_logv_va_list(TLOGCATEGORYINST *a_pstCat, TLOGEVENT *a_pstEvt,
        const char* a_pszFmt, va_list a_Ap)
{
    char *pszMessage = NULL;
    int iMessageLen;
    int iRet;
    TLOGCATEGORYINNERINST *pstCatInnerInst = NULL;
    if (NULL == a_pstCat || NULL == a_pstEvt || NULL == a_pszFmt)
    {
        return -1;
    }

    pstCatInnerInst = container(TLOGCATEGORYINNERINST, stCatInst, a_pstCat);
    if (a_pstEvt->evt_is_msg_binary)
    {
        return -1;
    }

    // We need to get the buffer here.
    iRet = tlog_category_get_buffer(pstCatInnerInst, &a_pstEvt->evt_msg);
    a_pstEvt->evt_msg_len = pstCatInnerInst->pstCat->iMaxMsgSize;
    if (iRet)
    {
        return -1;
    }

    pszMessage = (char*) a_pstEvt->evt_msg;
    assert(NULL != pszMessage);
    iMessageLen = a_pstEvt->evt_msg_len;
    assert(0 < iMessageLen);

    errno = 0;
    a_pstEvt->evt_msg_len = vsnprintf(pszMessage, iMessageLen, a_pszFmt, a_Ap);
    a_pstEvt->evt_msg = pszMessage;

#if defined (_WIN32) || defined (_WIN64)
    if (a_pstEvt->evt_msg_len < 0)
    {
        if (EINVAL == errno)
        {
            return -1;
        }
        else
        {
            a_pstEvt->evt_msg_len = iMessageLen;
        }
    }

    if (a_pstEvt->evt_msg_len == iMessageLen)
    {
        a_pstEvt->evt_msg_len = iMessageLen - 1;
        *(pszMessage + a_pstEvt->evt_msg_len - 1) = '\0';
    }
#else
    if (a_pstEvt->evt_msg_len < 0)
    {
        return -1;
    }

    if (a_pstEvt->evt_msg_len >= iMessageLen)
    {
        a_pstEvt->evt_msg_len = iMessageLen - 1;
        *(pszMessage + a_pstEvt->evt_msg_len - 1) = '\0';
    }
#endif

    iRet = tlog_category_log(&pstCatInnerInst->stCatInst, a_pstEvt);
    return iRet;
}

#if defined (_WIN32) || defined (_WIN64)
#if _MSC_VER < 1400

#define TLOG_INIT_PRIORITY(_pri) \
        va_list ap;\
        TLOGLOCINFO locinfo = TLOG_LOC_INFO_INITIALIZER(NULL);\
        TLOGEVENT stEvt;\
        int iRet = 0;\
\
        if(!(tlog_category_is_priority_enabled(a_pstCatInst, _pri) && tlog_category_can_write(a_pstCatInst, _pri,id, cls)))\
        {\
            return 0;\
        }\
\
        stEvt.evt_priority = _pri;\
        stEvt.evt_id = id;\
        stEvt.evt_cls = cls;\
        stEvt.evt_is_msg_binary = 0;\
        stEvt.evt_loc = &locinfo;\
\
        if(NULL == a_pszFmt)\
        {\
            return -1;\
        }\
        va_start(ap, a_pszFmt);\
        iRet = tlog_category_logv_va_list(a_pstCatInst, &stEvt, a_pszFmt, ap);\
        va_end(ap);\
        return iRet;\

int tlog_log(TLOGCATEGORYINST *a_pstCatInst, int priority, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(priority)
}

int tlog_fatal(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_FATAL)
}

int tlog_alert(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_ALERT)
}

int tlog_error(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_ERROR)
}

int tlog_notice(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_NOTICE)
}

int tlog_debug(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_DEBUG)
}

int tlog_trace(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_TRACE)
}

int tlog_warn(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_WARN)
}

int tlog_crit(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_CRIT)
}

int tlog_info(TLOGCATEGORYINST *a_pstCatInst, int id, int cls,
        const char* a_pszFmt, ...)
{
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_INFO)
}

int tlog_bill(TLOGCATEGORYINST *a_pstCatInst,const char *a_pszFmt,...)
{
    int id = 0;
    int cls = 0;
    TLOG_INIT_PRIORITY(TLOG_PRIORITY_ERROR);
}

#endif
#endif

int tlog_category_logv(TLOGCATEGORYINST *a_pstCatInst, TLOGEVENT *a_pstEvt,
        const char* a_pszFmt, ...)
{
    va_list ap;
    int iRet;
    va_start(ap, a_pszFmt);
    iRet = tlog_category_logv_va_list(a_pstCatInst, a_pstEvt, a_pszFmt, ap);
    va_end(ap);
    return iRet;
}

int tlog_category_log_in(TLOGCATEGORYINNERINST *a_pstCatInnerInst,
        TLOGEVENTBASE *a_pstEvt)
{
    TLOGCATEGORYINNERINST *pstCatInnerInst = NULL;
    struct tagTLogCtx* pstLogCtx = NULL;
    int iRet = 0;

    if (!a_pstCatInnerInst)
        return -1;

    if (!evt_is_enabled(a_pstCatInnerInst->pstCat, a_pstEvt->evt_priority))
    {
        return 0;
    }

    if (!tlogfilter_match_vec(&(a_pstCatInnerInst->pstCat->stFilter),
            a_pstEvt->evt_id, a_pstEvt->evt_cls))
    {
        return 0;
    }

    // 取时间
    if (a_pstCatInnerInst->iFlag & TLOGCAT_FALG_NONE_USE_GETTIMEOFDAY)
    {
        a_pstEvt->evt_timestamp = a_pstCatInnerInst->stCurr;
    }
    else
    {
        gettimeofday(&a_pstEvt->evt_timestamp, NULL);
    }
    a_pstEvt->evt_msg_sufix_len = 0;
    a_pstEvt->evt_msg_prefix_len = 0;

    // 记录内存日志
    if (TLOG_DEV_MEM == a_pstCatInnerInst->pstCat->stDevice.iType
            && a_pstCatInnerInst->pstCat->stDevice.stDevice.stMem.iEnable)
    {
        TLOGANY *pstDev;
        pstDev = CATINST2DEVANY(a_pstCatInnerInst);
        tlogmem_make_frame(&pstDev->stInst.stAny.stMem, a_pstEvt,
                a_pstEvt->evt_msg_len);
        return 0;
    }

    a_pstEvt->evt_category = a_pstCatInnerInst->pstCat->szName;
    pstLogCtx = CATINST2LOGCTX(a_pstCatInnerInst);
    a_pstEvt->evt_hostName = pstLogCtx->szHostName;
    a_pstEvt->evt_moduleName = pstLogCtx->szModuleName;

    //消息格式化
    if (a_pstEvt->evt_is_msg_binary)
    {
        TLOGBINHEAD stHead;
        a_pstCatInnerInst->iSeq++;
        TLOGBIN_INIT_HEAD(&stHead, a_pstCatInnerInst->iSeq,
                a_pstEvt->evt_id, a_pstEvt->evt_cls,
                a_pstEvt->evt_type, a_pstEvt->evt_version,
                a_pstEvt->evt_msg_len, &a_pstEvt->evt_timestamp );

        //对消息头部进行编码，并将编码信息写到 evt的前缀信息区中
        a_pstEvt->evt_msg_prefix_len = tlogbin_hton_head(
                a_pstEvt->evt_msg_prefix_buffer,
                (int) sizeof(a_pstEvt->evt_msg_prefix_buffer), &stHead);
        if (0 > a_pstEvt->evt_msg_prefix_len)
         {
              a_pstEvt->evt_msg_prefix_len = 0;
         }
    }else
    {
        if ('\0' != a_pstCatInnerInst->pstCat->szFormat[0])
        {
            tlog_layout_format(a_pstEvt, a_pstCatInnerInst->pstCat->szFormat);
        }
    }/*if (a_pstEvt->evt_is_msg_binary)*/

    for (pstCatInnerInst = a_pstCatInnerInst; pstCatInnerInst; pstCatInnerInst
            = pstCatInnerInst->pstParent)
    {
#ifdef TSF4G_TLOG_THREAD_SAFE
        iRet = pthread_mutex_lock(&pstCatInnerInst->stLogMutex);
        if (0 != iRet)
        {
            return -2;
        }
#endif
        
        iRet = tlog_category_append(pstCatInnerInst, a_pstEvt);

#ifdef TSF4G_TLOG_THREAD_SAFE
        pthread_mutex_unlock(&pstCatInnerInst->stLogMutex);
#endif

        if (!pstCatInnerInst->pstCat->iLevelDispatch)
        {
            break;
        }
    }

    return iRet;
}

int tlog_category_priority_check_chain(TLOGCATEGORYINNERINST *a_pstCatInst,
        int a_priority)
{
    TLOGCATEGORYINNERINST *pstCatInst;
    int iRet = 0;

    for (pstCatInst = a_pstCatInst; pstCatInst; pstCatInst
            = pstCatInst->pstForward)
    {
        if (evt_is_enabled(pstCatInst->pstCat, a_priority))
        {
            iRet = 1;
            break;
        }
    }

    return iRet;
}

int tlog_category_filter_check_chain(TLOGCATEGORYINNERINST *a_pstCatInst,
        int a_iId, int a_iCls)
{
    TLOGCATEGORYINNERINST *pstCatInst;
    int iRet = 0;

    for (pstCatInst = a_pstCatInst; pstCatInst; pstCatInst
            = pstCatInst->pstForward)
    {
        if (tlogfilter_match_vec(&(pstCatInst->pstCat->stFilter), a_iId, a_iCls))
        {
            iRet = 1;
            break;
        }
    }

    return iRet;
}

int tlog_category_can_write(TLOGCATEGORYINST * a_pstCatInst, int a_priority,
        int a_iId, int a_iCls)
{
    TLOGCATEGORYINNERINST *pstCatInnerInst;
    TLOGCATEGORYINNERINST *pstInnerCatInst;
    int iRet = 0;

    if (NULL == a_pstCatInst)
    {
        return 0;
    }

    pstCatInnerInst = container(TLOGCATEGORYINNERINST, stCatInst, a_pstCatInst);
    assert(NULL != pstCatInnerInst);

    for (pstInnerCatInst = pstCatInnerInst; pstInnerCatInst; pstInnerCatInst
            = pstInnerCatInst->pstForward)
    {
        if (evt_is_enabled(pstInnerCatInst->pstCat, a_priority) && ((0
                >= pstInnerCatInst->pstCat->stFilter.iCount)
                || (tlogfilter_match_vec(&(pstInnerCatInst->pstCat->stFilter),
                        a_iId, a_iCls))))
        {
            iRet = 1;
            break;
        }
    }

    return iRet;
}

int tlog_category_log_tdr(TLOGCATEGORYINST *a_pstCat, TLOGEVENT *a_pstEvt,
        void * a_pmeta, void *a_buff, intptr_t a_bufflen, int a_version)
{
    TDRDATA _stHost;
    TDRDATA _stOut;
    int iRet;
    TLOGCATEGORYINNERINST *pstInnerCatInst = NULL;

    if (NULL == a_pstCat || NULL == a_pstEvt || NULL == a_pmeta || NULL
            == a_buff)
    {
        return -1;
    }

    pstInnerCatInst = container(TLOGCATEGORYINNERINST, stCatInst, a_pstCat);

    iRet = tlog_category_get_buffer(pstInnerCatInst, &a_pstEvt->evt_msg);
    a_pstEvt->evt_msg_len = pstInnerCatInst->pstCat->iMaxMsgSize;
    if (iRet)
    {
        return -1;
    }

#if 0
    _iBuff = sizeof(_szBuff);
    if( pstInnerCatInst->pstCat->iMaxMsgSize<=(int) _iBuff )
    {
        _stOut.pszBuff = &_szBuff[0];
    }
    else
    {
        _iBuff = pstInnerCatInst->pstCat->iMaxMsgSize;
        _stOut.pszBuff = (char *)alloca(_iBuff);
        if( !_stOut.pszBuff )
        {
            _stOut.pszBuff = &_szBuff[0];
            _iBuff = sizeof(_szBuff);
        }
    }
    _stOut.iBuff = _iBuff;
#endif
    _stOut.pszBuff = (char *) a_pstEvt->evt_msg;
    _stOut.iBuff = a_pstEvt->evt_msg_len;
    _stHost.iBuff = a_bufflen;
    _stHost.pszBuff = (char *) a_buff;
    iRet = tdr_sprintf(a_pmeta, &_stOut, &_stHost, a_version);
    //if(iRet)
    //{
    //	tlog_error(a_pstCat,0,0,"tlog_log_dr(%s)",tdr_error_string(iRet));
    //}


    if (_stOut.iBuff < a_pstEvt->evt_msg_len)
    {
        _stOut.pszBuff[_stOut.iBuff] = '\0';
    }
    else
    {
        _stOut.pszBuff[a_pstEvt->evt_msg_len - 1] = '\0';
    }

    //a_pstEvt->evt_msg = &_stOut.pszBuff[0];
    a_pstEvt->evt_msg_len = _stOut.iBuff;

    return tlog_category_log(a_pstCat, a_pstEvt);

}

int tlog_category_log(TLOGCATEGORYINST *a_pstCatInst, TLOGEVENT *a_pstEvt)
{
    TLOGCATEGORYINNERINST *pstCatInst = NULL;
    TLOGCATEGORYINNERINST *pstCatInnerInst = NULL;
    int iRet = 0;
    TLOGEVENTBASE stEvt;

    if (NULL == a_pstCatInst || NULL == a_pstEvt)
    {
        return -1;
    }

    pstCatInnerInst = container(TLOGCATEGORYINNERINST, stCatInst, a_pstCatInst);

    stEvt.evt_priority = a_pstEvt->evt_priority;
    stEvt.evt_cls = a_pstEvt->evt_cls;
    stEvt.evt_id = a_pstEvt->evt_id;
    stEvt.evt_is_msg_binary = a_pstEvt->evt_is_msg_binary; // Why??
    stEvt.evt_type = a_pstEvt->evt_type;
    stEvt.evt_version = a_pstEvt->evt_version;
    stEvt.evt_loc = a_pstEvt->evt_loc;
    stEvt.evt_msg = a_pstEvt->evt_msg;
    stEvt.evt_msg_len = a_pstEvt->evt_msg_len;

    for (pstCatInst = pstCatInnerInst; pstCatInst; pstCatInst
            = pstCatInst->pstForward)
    {
        tlog_category_log_in(pstCatInst, &stEvt);
    }

    return iRet;
}

static char HEX_value[16] =
{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
        'F' };

char* TlogBin2Hex(unsigned char * pbin, int binlen, char* phex, int *hexlen)
{
    unsigned char low, hi;
    int j = 0, i = 0;
    if (*hexlen < (binlen * 2 + 1))
    {
        return NULL;
    }
    while (i <= binlen)
    {
        low = pbin[i] & 0X0F;
        hi = pbin[i] >> 4;
        phex[j++] = HEX_value[hi];
        phex[j++] = HEX_value[low];
        i++;
    }
    *hexlen = binlen * 2;
    phex[*hexlen] = 0;
    return phex;
}

// 这个是暴露在外面的接口
int tlog_category_log_hex(TLOGCATEGORYINST *a_pstCatInst, TLOGEVENT *a_pstEvt)
{
    TLOGCATEGORYINNERINST *pstCatInst;
    int iRet = 0;
    TLOGEVENTBASE stEvt;
    char *pszBuff;
    size_t iBuff;
    TLOGCATEGORYINNERINST *pstCatInnerInst;

    if (NULL == a_pstCatInst || NULL == a_pstEvt)
    {
        return -1;
    }
    pstCatInnerInst = container(TLOGCATEGORYINNERINST, stCatInst, a_pstCatInst);

    stEvt.evt_priority = a_pstEvt->evt_priority;
    stEvt.evt_cls = a_pstEvt->evt_cls;
    stEvt.evt_id = a_pstEvt->evt_id;
    stEvt.evt_is_msg_binary = a_pstEvt->evt_is_msg_binary;
    stEvt.evt_type = a_pstEvt->evt_type;
    stEvt.evt_version = a_pstEvt->evt_version;
    stEvt.evt_loc = a_pstEvt->evt_loc;

    pszBuff = a_pstEvt->evt_msg;
    iBuff = a_pstEvt->evt_msg_len;

    iRet = tlog_category_get_buffer(pstCatInnerInst, &a_pstEvt->evt_msg);
    a_pstEvt->evt_msg_len = pstCatInnerInst->pstCat->iMaxMsgSize;
    if (iRet)
    {
        return -1;
    }

    if (iBuff * 2 > a_pstEvt->evt_msg_len)
    {
        iBuff = (a_pstEvt->evt_msg_len - 2) / 2;
    }

    TlogBin2Hex((unsigned char *) pszBuff, iBuff, (char *) a_pstEvt->evt_msg,
            &a_pstEvt->evt_msg_len);

    stEvt.evt_msg = a_pstEvt->evt_msg;
    stEvt.evt_msg_len = a_pstEvt->evt_msg_len;

    for (pstCatInst = pstCatInnerInst; pstCatInst; pstCatInst
            = pstCatInst->pstForward)
    {
        iRet = tlog_category_log_in(pstCatInst, &stEvt);
    }

    return iRet;
}

// 设置时间为当前时间,应该设置全局的时间变量
int tlog_category_set_time_ptr(TLOGCATEGORYINST *a_pstCatInst,
        struct timeval *pstCurr)
{
    TLOGCATEGORYINNERINST *pstInnerCat;
    if (NULL == a_pstCatInst)
    {
        return -1;
    }
    pstInnerCat = container(TLOGCATEGORYINNERINST, stCatInst, a_pstCatInst);
    pstInnerCat->stCurr = *pstCurr;
    pstInnerCat->iFlag |= TLOGCAT_FALG_NONE_USE_GETTIMEOFDAY;
    return 0;
}

int tlog_category_set_priority(TLOGCATEGORYINST *a_pstCatInst,
        int iPriorityHigh, int iPriorityLow)
{
    TLOGCATEGORYINNERINST *pstInnerCat;
    if (NULL == a_pstCatInst)
    {
        return -1;
    }

    pstInnerCat = container(TLOGCATEGORYINNERINST, stCatInst, a_pstCatInst);

    pstInnerCat->pstCat->iPriorityHigh = iPriorityHigh;
    pstInnerCat->pstCat->iPriorityLow = iPriorityLow;
    return 0;
}

int tlog_category_set_format(TLOGCATEGORYINST *a_pstCatInst,
        const char *pszPattern)
{
    TLOGCATEGORYINNERINST *pstInnerCat;
    if (NULL == a_pstCatInst || NULL == pszPattern)
    {
        return -1;
    }

    pstInnerCat = container(TLOGCATEGORYINNERINST, stCatInst, a_pstCatInst);

    if (pstInnerCat->pstCat)
    {
        STRNCPY(pstInnerCat->pstCat->szFormat,pszPattern,sizeof(pstInnerCat->pstCat->szFormat));
    }
    return 0;
}
