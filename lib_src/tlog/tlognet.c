/*
 **  @file $RCSfile: tlognet.c,v $
 **  general description of this module
 **  $Id: tlognet.c,v 1.4 2009-03-27 06:17:02 kent Exp $
 **  @author $Author: kent $
 **  @date $Date: 2009-03-27 06:17:02 $
 **  @version $Revision: 1.4 $
 **  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
 **  @note Platform: Linux
 */

#include "pal/pal.h"
#include "tlog/tlogerr.h"
#include "tlognet.h"

#define TLOG_NET_MIN_BUFF_SIZE 10240000

int tlognet_connect_i(TLOGNET* a_pstLogNet, int a_iTimeout);

int tlognet_connect_i(TLOGNET* a_pstLogNet, int a_iTimeout)
{
    int iRet = 0;

    if (!a_pstLogNet->pstDev->szUrl[0])
        return -1;

    a_pstLogNet->stInst.s
            = tnet_connect(a_pstLogNet->pstDev->szUrl, a_iTimeout);

    if (-1 == a_pstLogNet->stInst.s)
    {
        return -1;
    }

    iRet = tnet_set_nonblock(a_pstLogNet->stInst.s, 1);
    iRet = tsocket_get_type(a_pstLogNet->stInst.s, &a_pstLogNet->stInst.iType);

    if (a_pstLogNet->pstDev->iSendBuff > 0)
        iRet = tsocket_set_sendbuff(a_pstLogNet->stInst.s,
                a_pstLogNet->pstDev->iSendBuff);

    return iRet;
}

int tlognet_init(TLOGNET* a_pstLogNet, TLOGDEVNET* a_pstDev)
{
    int iRet;

    assert(a_pstLogNet && a_pstDev);

    memset(a_pstLogNet, 0, sizeof(*a_pstLogNet));

    a_pstLogNet->stInst.s = -1;
    a_pstLogNet->pstDev = a_pstDev;

    if (a_pstDev->iBuffSize < TLOG_NET_MIN_BUFF_SIZE)
    {
        a_pstDev->iBuffSize = TLOG_NET_MIN_BUFF_SIZE;
    }

    a_pstLogNet->stInst.pszBuff = calloc(1, a_pstDev->iBuffSize);
    if (!a_pstLogNet->stInst.pszBuff)
    {
        return -1;
    }

    iRet = tlognet_connect_i(a_pstLogNet, a_pstDev->iConnTimeout);

    if (iRet < 0)
    {
        printf("connect net url %s fail\n", a_pstLogNet->pstDev->szUrl);
        tlognet_fini(a_pstLogNet);
        return TLOG_ERR_MAKE_ERROR(TLOG_ERROR_NET_CONN);
    }
    else
        return 0;
}

int tlognet_fini(TLOGNET* a_pstLogNet)
{
    assert(a_pstLogNet);

    if (NULL != a_pstLogNet->stInst.pszBuff)
    {
        free(a_pstLogNet->stInst.pszBuff);
        a_pstLogNet->stInst.pszBuff = NULL;
    }    
    if (-1 != a_pstLogNet->stInst.s)
    {
        tnet_close(a_pstLogNet->stInst.s);
        a_pstLogNet->stInst.s = -1;            
    }

    return 0;
}

int tlognet_writev(TLOGNET* a_pstLogNet, const TLOGIOVEC* a_pstIOVec,
        int a_iCount)
{
    int iRecv;
    int i;
    int iSize;
    int iRet;

    char *pszBuff = NULL;
    size_t iBuff = 0;

    iSize = 0;
    iRet = 0;

    if (-1 == a_pstLogNet->stInst.s)
    {
        if (a_pstLogNet->pstDev->iAutoReconnect && (0
                == a_pstLogNet->pstDev->iMaxRetry
                || a_pstLogNet->stInst.iCurRetry
                        < a_pstLogNet->pstDev->iMaxRetry))
        {
            a_pstLogNet->stInst.iCurRetry++;
            tlognet_connect_i(a_pstLogNet, a_pstLogNet->pstDev->iConnTimeout);
        }

        if (-1 == a_pstLogNet->stInst.s)
            return -1;
    }

    // 只有一个缓冲直接发送
    if (a_iCount == 1)
    {
        pszBuff = (char*) a_pstIOVec[0].iov_base;
        iBuff = a_pstIOVec[0].iov_len;

    }
    else if (a_iCount > 1)
    {
        // 计算缓冲大小
        size_t iLen = 0;
        for (i = 0; i < a_iCount; i++)
        {
            // I have to caculate the buff size
            if (iLen + a_pstIOVec[i].iov_len < a_pstLogNet->pstDev->iBuffSize)
            {
                memcpy(a_pstLogNet->stInst.pszBuff + iLen,
                        a_pstIOVec[i].iov_base, a_pstIOVec[i].iov_len);
                iLen += a_pstIOVec[i].iov_len;
            }
            else
            {
                if( a_pstLogNet->pstDev->iBuffSize < iLen)
                    break;

                memcpy(a_pstLogNet->stInst.pszBuff + iLen,
                        a_pstIOVec[i].iov_base, a_pstLogNet->pstDev->iBuffSize - iLen);
                iLen = a_pstLogNet->pstDev->iBuffSize;
                break;
            }

        }

        pszBuff = a_pstLogNet->stInst.pszBuff;
        iBuff = iLen;
    }

    iRecv = tnet_sendall(a_pstLogNet->stInst.s, pszBuff, iBuff,
            a_pstLogNet->pstDev->iSendTimeout);

    if (iRecv == iBuff)
    {
        iSize += iRecv;
    }
    else
    {
        iRet = -1;
    }

    if (-1 == iRet && SOCK_STREAM == a_pstLogNet->stInst.iType)
    {
        if (errno != ENOTCONN)
        {
            tnet_close(a_pstLogNet->stInst.s);
            a_pstLogNet->stInst.s = -1;
        }
    }

    return (iRet < 0) ? -1 : iSize;
}

int tlognet_write(TLOGNET* a_pstLogNet, const char* a_pszBuff, int a_iBuff)
{
    TLOGIOVEC stIOVec;

    stIOVec.iov_base = (void*) a_pszBuff;
    stIOVec.iov_len = a_iBuff;

    return tlognet_writev(a_pstLogNet, &stIOVec, 1);
}

