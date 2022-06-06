/** @file $RCSfile: tbus_misc.c,v $
  general description of this module
  $Id: tbus_misc.c,v 1.28 2009-09-10 07:42:22 jacky Exp $
  @author $Author: jacky $
  @date $Date: 2009-09-10 07:42:22 $
  @version $Revision: 1.28 $
  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
  @note Platform: Linux
  */


#include <time.h>
#include "tbus/tbus.h"
#include "tbus_misc.h"
#include "tbus_kernel.h"
#include "tbus_log.h"
#include "tbus/tbus_error.h"

extern LPTLOGCATEGORYINST	g_pstBusLogCat;
extern TBUSGLOBAL g_stBusGlobal;
extern unsigned char g_szMetalib_TbusHead[];


int tbus_auto_get_shm ( INOUT void **a_ppvShm, OUT int *a_piCreated, IN const int a_iShmKey,
                       INOUT  int *a_piSize, IN  int a_iFlag, HANDLE *a_pShmID )
{
    HANDLE iShmID = (HANDLE)-1;
    int	iRet = 0 ;
    struct shmid_ds stShmStat ;

    assert(NULL != a_ppvShm);

    *a_piCreated = 0;
    if (a_iFlag & IPC_CREAT)
    {
        a_iFlag |= IPC_EXCL;
        iShmID = shmget ( a_iShmKey, *a_piSize, a_iFlag) ;
        if ((HANDLE)-1 != iShmID)
        {
            *a_piCreated = 1; /*create*/
        }else
        {
            a_iFlag &= ~IPC_CREAT;
            a_iFlag &= ~IPC_EXCL;
            iShmID = shmget ( a_iShmKey, *a_piSize, a_iFlag) ;
        }
    }else
    {
        iShmID = shmget ( a_iShmKey, *a_piSize, a_iFlag) ;
    }/*if (a_iFlag & IPC_CREAT)*/
    if ( (HANDLE)-1 == iShmID )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"shmget failed by key %d, for %s\n", a_iShmKey, strerror(errno));
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED);
    }

    *a_ppvShm =	shmat ( iShmID, NULL, 0 ) ;
    if ((void *)-1 == *a_ppvShm)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"shmat failed by id %d, for %s\n", iShmID, strerror(errno));
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
    }
    if ( 0 == shmctl( iShmID, IPC_STAT, &stShmStat))
    {
        *a_piSize = (int)stShmStat.shm_segsz;
    }
    *a_pShmID = iShmID;
    tbus_log(TLOG_PRIORITY_DEBUG,"CALL: tbus_auto_get_shm(void **, %i, %i, %i)", a_iShmKey, *a_piSize, a_iFlag ) ;

    return iRet ;
}


char *tbus_conv_hexstr ( IN const unsigned char *a_pszStr, IN const int a_iStrLen )
{
    int iLen, iCount;
    static char sBuffer[20480]; /* 20 KB */

    for (iLen = iCount = 0; (iCount < a_iStrLen)&&(iCount <(int)sizeof(sBuffer) - 1); iCount++)
    {
        if (iLen > (int)sizeof(sBuffer) - 5) break;

        if (iCount % 25 == 24) sprintf(sBuffer + iLen, "%.2x\n", a_pszStr[iCount]);
        else sprintf(sBuffer + iLen, "%.2x ", a_pszStr[iCount]);

        iLen += 3;
    }

    return sBuffer;
}



unsigned short tbus_head_checksum ( IN const TBUSHEAD *a_ptHead )
{
    unsigned short shSum = 0 ;
    int i = 0 ;
#define TBUS_CALC_CHECKSUM_SIZE offsetof(TBUSHEAD,bFlag)

    for (i = 0; i < (int)TBUS_CALC_CHECKSUM_SIZE/2; i++)
    {
        shSum ^= *(short *)((char *)a_ptHead + i * 2);
    }

    return shSum;
}

/* Show Hex val of a string, if a_iFlag = 1, show printable character as char */
void tbus_print_hexstr ( IN const unsigned char *a_pszStr, IN const int a_iLen, IN const int a_iFlag )
{
    register int iCount = 0 ;

    for (iCount = 0; iCount < a_iLen; iCount++)
    {
        if (iCount % 25 == 0) printf("\n");
        if (a_iFlag && a_pszStr[iCount] > 0x1f) printf("%2c ", a_pszStr[iCount]);
        else printf("%.2x ", a_pszStr[iCount]);
    }
    if ((iCount - 1) % 25) printf("\n");
}




int tbus_addr_aton(IN const char *a_pszAddr, OUT TBUSADDR *a_piAddr)
{
    int iRet;
    LPTBUSSHMGCIMHEAD pstHead;

    //assert(NULL != a_pszAddr);
    //assert(NULL != a_piAddr);
    if ((NULL == a_pszAddr)||(NULL == a_piAddr))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_pszAddr:%p a_piAddr: %p", a_pszAddr, a_piAddr);
        return TBUS_ERR_ARG;
    }

    if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInitStatus )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"global bus module not inited, so cannot assign handle" ) ;
        iRet = TBUS_ERR_INIT ;
        return iRet ;
    }
    assert(NULL != g_stBusGlobal.pstGCIM);

    pstHead = &g_stBusGlobal.pstGCIM->stHead;

    iRet = tbus_addr_aton_by_addrtemplet(&pstHead->stAddrTemplet, a_pszAddr, a_piAddr);

    return iRet;
}

char *tbus_addr_ntoa(IN TBUSADDR a_iAddr)
{
    LPTBUSSHMGCIMHEAD pstHead;

    if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInitStatus )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"global bus module not inited, so cannot assign handle" ) ;
        return "";
    }

    assert(NULL != g_stBusGlobal.pstGCIM);

    pstHead = &g_stBusGlobal.pstGCIM->stHead;
    return tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, a_iAddr);

}



#ifdef TBUS_OPTIMIZE_ENCODING_HEAD

int tbus_encode_head(IN LPTBUSHEAD a_pstHead, IN char *a_pszNet, INOUT int *a_piLen, IN int a_iVersion)
{
    int iRet = TBUS_SUCCESS;

    LPTBUSHEAD pstNetHead;
    int iEncodedNet;
    char *pch = NULL;
    short nCheckSum = 0;

    assert(NULL != a_pstHead);
    assert(NULL != a_pszNet);
    assert(NULL != a_piLen);

    TOS_UNUSED_ARG(a_iVersion);


    /*出于效率考虑，为减少网络空间是否足够的检查的判断，简单粗暴地要求网络缓冲区大小不小于 本地数据结构大小，由于tbus_encode_head
      只在tbus内部调用，我们这个条件比较容易实现*/
    if (*a_piLen < (int)sizeof(TBUSHEAD))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"Net buffer size(%d) is too small, it should be not less than %"PRIdPTR"",
                 *a_piLen, sizeof(TBUSHEAD));
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PACK_HEAD);
    }



    pstNetHead = (LPTBUSHEAD)a_pszNet;
    a_pstHead->bVer = TDR_METALIB_TBUSHEAD_VERSION;
    pstNetHead->bVer = TDR_METALIB_TBUSHEAD_VERSION;
    pstNetHead->iBodyLen = tdr_hton32(a_pstHead->iBodyLen);
    pstNetHead->iDst = tdr_hton32(a_pstHead->iDst);
    pstNetHead->iSrc = tdr_hton32(a_pstHead->iSrc);
    pstNetHead->iSeq = tdr_hton32(a_pstHead->iSeq);
    pstNetHead->bFlag = a_pstHead->bFlag;
    pstNetHead->bCmd = a_pstHead->bCmd;
    pstNetHead->iRecvSeq = tdr_hton32(a_pstHead->iRecvSeq);
    iEncodedNet = TBUS_HEAD_EXTHEAD_DATA_OFFSET;

    pch = a_pszNet + TBUS_HEAD_EXTHEAD_DATA_OFFSET;
    switch(a_pstHead->bCmd)
    {
        case TBUS_HEAD_CMD_TRANSFER_DATA:
            {
                *(unsigned char *)pch = a_pstHead->stExtHead.stDataHead.bRoute;
                pch++;
                *pch = a_pstHead->stExtHead.stDataHead.chIsDyed;
                pch++;
                iEncodedNet += sizeof(char)*2;
                if (0 != a_pstHead->stExtHead.stDataHead.chIsDyed)
                {
                    iEncodedNet += TDR_SIZEOF_INT;
                    *(unsigned int *)pch = (unsigned int)tdr_hton32(a_pstHead->stExtHead.stDataHead.iDyedID);
                    pch += TDR_SIZEOF_INT;
                }/*if (0 != a_pstHead->stExtHead.stDataHead.chIsDyed)*/
                if (0 < a_pstHead->stExtHead.stDataHead.bRoute)
                {
                    int i;
                    if (TBUS_FORWARD_MAX_ROUTE < a_pstHead->stExtHead.stDataHead.bRoute)
                    {
                        tbus_log(TLOG_PRIORITY_ERROR,"route num(%d) should not bigger than the max limit(%d)",
                                 a_pstHead->stExtHead.stDataHead.bRoute, TBUS_FORWARD_MAX_ROUTE);
                        return  TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ROUTE);
                    }
                    for (i = 0; i < a_pstHead->stExtHead.stDataHead.bRoute; i++)
                    {
                        *(unsigned int *)pch = (unsigned int)tdr_hton32(a_pstHead->stExtHead.stDataHead.routeAddr[i]);
                        pch += TDR_SIZEOF_INT;
                    }
                    iEncodedNet += a_pstHead->stExtHead.stDataHead.bRoute * TDR_SIZEOF_INT;
                }/*if (0 != a_pstHead->stExtHead.stDataHead.chIsDyed)*/
            }
            break;
        default:
            *(unsigned char *)pch = a_pstHead->stExtHead.stSynHead.bSynFlag;
            pch++;
            *(unsigned char *)pch = a_pstHead->stExtHead.stSynHead.bReserve;
            pch++;
            iEncodedNet += 2;
    }/*switch(a_pstHead->bCmd)*/

    *(int64_t *)pch = tdr_hton64(a_pstHead->stTimeStamp.llSec);
    pch += TDR_SIZEOF_INT64;
    *(unsigned int *)pch = (unsigned int)tdr_hton32(a_pstHead->stTimeStamp.dwUSec);
    pch += TDR_SIZEOF_INT;
    *(unsigned int *)pch = (unsigned int)tdr_hton32(a_pstHead->stTimeStamp.dwCumm);
    pch += TDR_SIZEOF_INT;
    *(int*)pch = (int)tdr_hton32(a_pstHead->iHeartBeatMsgType);
    pch += TDR_SIZEOF_INT;
    iEncodedNet += (TDR_SIZEOF_INT64 + TDR_SIZEOF_INT + TDR_SIZEOF_INT + TDR_SIZEOF_INT);

    /*设置校验值*/
    a_pstHead->nCheckSum = 0;
    a_pstHead->bHeadLen = iEncodedNet;
    pstNetHead->bHeadLen = iEncodedNet;
    TBUS_CALC_HEAD_CHECKSUM(nCheckSum, a_pstHead);
    a_pstHead->nCheckSum = nCheckSum;
    pstNetHead->nCheckSum = tdr_hton16(nCheckSum);
    *a_piLen = iEncodedNet;


    tbus_log_data(TLOG_PRIORITY_TRACE, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));

    return iRet;
}


/* TODO: 保证所有对此函数的调用都检查headlen + bodylen <= a_iLen */
int tbus_decode_head(OUT LPTBUSHEAD a_pstHead, IN char *a_pszNet, IN int a_iLen, IN int a_iVersion)
{
    int iRet = TBUS_SUCCESS;

    LPTBUSHEAD pstNetHead;
    int iExpectedNet;
    char *pch = NULL;
    short nCheckSum = 0;

    assert(NULL != a_pstHead);
    assert(NULL != a_pszNet);

    TOS_UNUSED_ARG(a_iVersion);



    /*目前已经都是使用head版本 都不低于1,因此 以下的实现认为网络编码信息中的头结构与内存格式的数据
      结构是一致的，即不考虑版本兼容问题，如果版本有变化，则需修改*/
    if (a_iLen < (int)TBUS_HEAD_EXTHEAD_DATA_OFFSET)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"net Head info length(%d) too small， for at least it should be %"PRIdPTR"",
                 a_iLen, TBUS_HEAD_EXTHEAD_DATA_OFFSET);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
    }
    pstNetHead = (LPTBUSHEAD)a_pszNet;
    a_pstHead->bVer = pstNetHead->bVer;
    if (TBUS_MIN_HEAD_VERSION > a_pstHead->bVer)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid head version(%d) ,it should not less than min version %d",
                 a_pstHead->bVer, TBUS_MIN_HEAD_VERSION);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
    }
    a_pstHead->bHeadLen = pstNetHead->bHeadLen;
    a_pstHead->nCheckSum = tdr_ntoh16(pstNetHead->nCheckSum);
    a_pstHead->iBodyLen = tdr_ntoh32(pstNetHead->iBodyLen);
    a_pstHead->iDst = tdr_ntoh32(pstNetHead->iDst);
    a_pstHead->iSrc = tdr_ntoh32(pstNetHead->iSrc);
    a_pstHead->iSeq = tdr_ntoh32(pstNetHead->iSeq);
    a_pstHead->bFlag = pstNetHead->bFlag;
    a_pstHead->bCmd = pstNetHead->bCmd;
    a_pstHead->iRecvSeq = tdr_ntoh32(pstNetHead->iRecvSeq);

    pch = a_pszNet + TBUS_HEAD_EXTHEAD_DATA_OFFSET;
    switch(a_pstHead->bCmd)
    {
        case TBUS_HEAD_CMD_TRANSFER_DATA:
            {
                iExpectedNet = TBUS_HEAD_EXTHEAD_DATA_OFFSET + 2;
                if (a_iLen < iExpectedNet)
                {
                    tbus_log(TLOG_PRIORITY_ERROR,"net Head info length(%d) too small， it  at least  should be %d",
                             a_iLen, iExpectedNet);
                    return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
                }
                a_pstHead->stExtHead.stDataHead.bRoute = *(unsigned char *)pch;
                pch++;
                a_pstHead->stExtHead.stDataHead.chIsDyed = *pch;
                pch++;
                if (0 != a_pstHead->stExtHead.stDataHead.chIsDyed)
                {
                    iExpectedNet += TDR_SIZEOF_INT;
                    if (a_iLen < iExpectedNet )
                    {
                        tbus_log(TLOG_PRIORITY_ERROR,"net Head info length(%d) too small， it  at least  should be %d",
                                 a_iLen, iExpectedNet);
                        tbus_log_data(TLOG_PRIORITY_ERROR, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
                        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
                    }
                    a_pstHead->stExtHead.stDataHead.iDyedID = tdr_ntoh32(*(unsigned int *)pch);
                    pch += TDR_SIZEOF_INT;
                }/*if (0 != a_pstHead->stExtHead.stDataHead.chIsDyed)*/
                if (0 < a_pstHead->stExtHead.stDataHead.bRoute)
                {
                    int i;
                    iExpectedNet += a_pstHead->stExtHead.stDataHead.bRoute * TDR_SIZEOF_INT;
                    if (a_iLen < iExpectedNet)
                    {
                        tbus_log(TLOG_PRIORITY_ERROR,"net Head info length(%d) too small， it  at least  should be %d",
                                 a_iLen, iExpectedNet);
                        tbus_log_data(TLOG_PRIORITY_ERROR, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
                        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
                    }
                    if (TBUS_FORWARD_MAX_ROUTE < a_pstHead->stExtHead.stDataHead.bRoute)
                    {
                        tbus_log(TLOG_PRIORITY_ERROR,"route num(%d) should not bigger than the max limit(%d)",
                                 a_pstHead->stExtHead.stDataHead.bRoute, TBUS_FORWARD_MAX_ROUTE);
                        return  TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ROUTE);
                    }
                    for (i = 0; i < a_pstHead->stExtHead.stDataHead.bRoute; i++)
                    {
                        a_pstHead->stExtHead.stDataHead.routeAddr[i] = tdr_ntoh32(*(unsigned int *)pch);
                        pch += TDR_SIZEOF_INT;
                    }
                }/*if (0 != a_pstHead->stExtHead.stDataHead.bRoute)*/
            }
            break;
        default:
            iExpectedNet = TBUS_HEAD_EXTHEAD_DATA_OFFSET + sizeof(char)*2;
            if (a_iLen < iExpectedNet)
            {
                tbus_log(TLOG_PRIORITY_ERROR,"net Head info length(%d) too small， it  at least  should be %d",
                         a_iLen, iExpectedNet);
                return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
            }
            a_pstHead->stExtHead.stSynHead.bSynFlag = *(unsigned char*)pch;
            pch++;
            a_pstHead->stExtHead.stSynHead.bReserve = *(unsigned char*)pch;
            pch++;
    }/*switch(a_pstHead->bCmd)*/


    /*检验hash值*/
    TBUS_CALC_HEAD_CHECKSUM(nCheckSum, a_pstHead);
    if (0 != nCheckSum)
    {
        tbus_log(TLOG_PRIORITY_FATAL,"failed to check headsum,headlen:%d msglen:%d src:0x%x dst:0x%x",
                 a_pstHead->bHeadLen, a_pstHead->iBodyLen, a_pstHead->iSrc, a_pstHead->iDst);
        tlog_log_hex(g_pstBusLogCat, TLOG_PRIORITY_FATAL, 0, 0, (char*)a_pstHead, sizeof(*a_pstHead));
        tbus_log_data(TLOG_PRIORITY_FATAL, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));

        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_FAILED_CHECK_HEADSUM);
    }

    /* a_pstHead->bVer必然是个非负数 */
    if (TBUS_TIMESTAP_VERSION  > a_pstHead->bVer)
    {
        //低于TBUS_TIMESTAP_VERSION版本没有时间戳 和iHeartBeatMsgType，设置缺省值
        a_pstHead->bFlag &= ~TBUS_HEAD_FLAG_WITH_TIMESTAMP;
        a_pstHead->iHeartBeatMsgType = 0;
    }else
    {
        /*stTimeStamp.llSec stTimeStamp.dwUSec stTimeStamp.dwCumm iHeartBeatMsgType*/
        iExpectedNet = TDR_SIZEOF_INT64 + TDR_SIZEOF_INT * 3;
        if (a_iLen < iExpectedNet)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"net Head info length(%d) too small for timestamp and heartbeat， it  at least  should be %d",
                     a_iLen, iExpectedNet);
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
        }
        if (!(TBUS_HEAD_FLAG_WITH_TIMESTAMP & a_pstHead->bFlag))
        {
            pch += TDR_SIZEOF_INT64 + TDR_SIZEOF_INT + TDR_SIZEOF_INT;
        }else
        {
            a_pstHead->stTimeStamp.llSec = tdr_ntoh64(*(int64_t*)pch);
            pch += TDR_SIZEOF_INT64;
            a_pstHead->stTimeStamp.dwUSec = (unsigned int)tdr_ntoh32(*(unsigned int*)pch);
            pch += TDR_SIZEOF_INT;
            a_pstHead->stTimeStamp.dwCumm = (unsigned int)tdr_ntoh32(*(unsigned int*)pch);
            pch += TDR_SIZEOF_INT;
        }
        a_pstHead->iHeartBeatMsgType = (int)tdr_ntoh32(*(unsigned int*)pch);
        pch += TDR_SIZEOF_INT;
    }

    tbus_log_data(TLOG_PRIORITY_TRACE, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
    return iRet;
}
#else

int tbus_encode_head(IN LPTBUSHEAD a_pstHead, IN char *a_pszNet, INOUT int *a_piLen, IN int a_iVersion)
{
    int iRet = TBUS_SUCCESS;
    TDRDATA stHost;
    TDRDATA stNet;
    LPTBUSHEAD pstHead;
    short nCheckSum =0;

    assert(NULL != a_pstHead);
    assert(NULL != a_pszNet);
    assert(NULL != a_piLen);

    if (0 == a_iVersion)
    {
        a_iVersion = TDR_METALIB_TBUSHEAD_VERSION;
    }
    if (NULL == g_stBusGlobal.pstHeadMeta)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"meta of tbus head is null");
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GET_HEAD_META);
    }

    /*设置校验 初值*/
    a_pstHead->nCheckSum = 0;

    /* 清除时间戳标志位 */
    a_pstHead->bFlag &= ~TBUS_HEAD_FLAG_WITH_TIMESTAMP;

    /*编码*/
    a_pstHead->bVer = (unsigned char)a_iVersion;
    stNet.iBuff = *a_piLen;
    stNet.pszBuff = a_pszNet;
    stHost.iBuff = sizeof(TBUSHEAD);
    stHost.pszBuff = (char *)a_pstHead;
    iRet = tdr_hton(g_stBusGlobal.pstHeadMeta, &stNet, &stHost, a_iVersion);
    *a_piLen = stNet.iBuff;
    if (TDR_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to pach tbus head, iRet %x, for %s", iRet, tdr_error_string(iRet));
        tbus_log_data(TLOG_PRIORITY_ERROR, g_stBusGlobal.pstHeadMeta, stHost.pszBuff, stHost.iBuff);
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PACK_HEAD);
    }

    /*设置校验值*/
    a_pstHead->bHeadLen = stNet.iBuff;
    TBUS_CALC_HEAD_CHECKSUM(nCheckSum, a_pstHead);
    a_pstHead->nCheckSum = nCheckSum;
    pstHead = (LPTBUSHEAD)a_pszNet;
    pstHead->nCheckSum = tdr_hton16(nCheckSum);

    tbus_log_data(TLOG_PRIORITY_TRACE, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));

    return iRet;
}

int tbus_decode_head(OUT LPTBUSHEAD a_pstHead, IN char *a_pszNet, IN int a_iLen, IN int a_iVersion)
{
    int iRet = TBUS_SUCCESS;
    TDRDATA stHost;
    TDRDATA stNet;


    assert(NULL != a_pstHead);
    assert(NULL != a_pszNet);


    if (0 == a_iVersion)
    {
        a_iVersion = TDR_METALIB_TBUSHEAD_VERSION;
    }
    if (NULL == g_stBusGlobal.pstHeadMeta)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"meta of tbus head is null");
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GET_HEAD_META);
    }


    stNet.iBuff = a_iLen;
    stNet.pszBuff = a_pszNet;
    stHost.iBuff = sizeof(TBUSHEAD);
    stHost.pszBuff = (char *)a_pstHead;
    iRet = tdr_ntoh(g_stBusGlobal.pstHeadMeta, &stHost, &stNet, a_iVersion);
    if (TDR_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to unpach tbus head(len:%"PRIdPTR"), iRet %x, for %s",
                 stNet.iBuff, iRet, tdr_error_string(iRet));
        tbus_log_data(TLOG_PRIORITY_ERROR, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
    }

    if (0 == iRet)
    {
        /*检验hash值*/
        short nCheckSum =0;
        TBUS_CALC_HEAD_CHECKSUM(nCheckSum, a_pstHead);
        if (0 != nCheckSum)
        {
            tbus_log(TLOG_PRIORITY_FATAL,"failed to check headsum,headlen:%d msglen:%d src:0x%x dst:0x%x",
                     a_pstHead->bHeadLen, a_pstHead->iBodyLen, a_pstHead->iSrc, a_pstHead->iDst);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_FAILED_CHECK_HEADSUM);
        }
    }/*if (0 == iRet)*/

    if (0 == iRet)
    {
        /* a_pstHead->bVer必然是个非负数 */
        if (TBUS_TIMESTAP_VERSION > a_pstHead->bVer || !(TBUS_HEAD_FLAG_WITH_TIMESTAMP & a_pstHead->bFlag))
        {
            a_pstHead->bFlag &= ~TBUS_HEAD_FLAG_WITH_TIMESTAMP;
        }
    }

    tbus_log_data(TLOG_PRIORITY_TRACE, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
    return iRet;
}
#endif /*#ifdef TBUS_OPTIMIZE_ENCODING_HEAD*/

#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE
int tbus_get_pkghead(OUT LPTBUSHEAD a_pstHead, const char* a_pszQueue, unsigned int dwSize,
                     INOUT unsigned int *a_pdwHead, unsigned int dwTail)
{
    int iDataLen = 0;
    int iHeadLen = 0;
    int iHeadVer;
    int iTailLen;
    char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
    int iRet = TBUS_SUCCESS;
    unsigned int dwHead;

    assert(NULL != a_pszQueue);
    assert(NULL!= a_pstHead);
    assert(0 < dwSize);
    assert(NULL != a_pdwHead);

    dwHead = *a_pdwHead;
    if (dwHead == dwTail)
    {
        return TBUS_ERR_CHANNEL_EMPTY;
    }
    iDataLen = (int)(dwTail - dwHead);
    if (iDataLen < 0)
    {
        iDataLen += dwSize;
    }
    if (iDataLen < TBUS_MIN_NET_LEN_TO_GET_HEADLEN)
    {
        /*first two byte stored vesion and headlen of tbus head*/
        tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) is less than min length(%d) to get tbus head len",
                 iDataLen, TBUS_MIN_NET_LEN_TO_GET_HEADLEN);
        return TBUS_ERR_CHANNEL_CONFUSE;
    }

    iHeadVer = (int)(unsigned char)a_pszQueue[dwHead];
    iHeadLen = (int)(unsigned char)a_pszQueue[(dwHead+TBUS_HEAD_LEN_NET_OFFSET) % dwSize];
    if (0 >= iHeadLen)
    {
        tbus_log(TLOG_PRIORITY_FATAL,"invalid tbus head len (%d), version of head(%d) datalen %d",
                 iHeadLen, iHeadVer, iDataLen);
        return TBUS_ERR_CHANNEL_CONFUSE;
    }

    if (iDataLen < iHeadLen)
    {
        tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) is less than the length(%d) of tbus head len",
                 iDataLen, iHeadLen);
        return TBUS_ERR_CHANNEL_CONFUSE;
    }
    iTailLen = dwSize - dwHead;
    if (iTailLen < iHeadLen)
    {
        memcpy(&szNet[0], a_pszQueue + dwHead, iTailLen);
        memcpy(&szNet[iTailLen], a_pszQueue, iHeadLen - iTailLen);
    } else
    {
        memcpy(&szNet[0], a_pszQueue + dwHead, iHeadLen);
    }
    iRet = tbus_decode_head(a_pstHead, &szNet[0], iHeadLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to unpack tbus head,iRet %x", iRet);
    }

    return iRet;
}
#else /*TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/
int tbus_get_pkghead(OUT LPTBUSHEAD a_pstHead, const char* a_pszQueue, unsigned int dwSize,
                     INOUT unsigned int *a_pdwHead, unsigned int dwTail)
{
    int iDataLen = 0;

    int iRet = TBUS_SUCCESS;


    assert(NULL != a_pszQueue);
    assert(NULL!= a_pstHead);
    assert(0 < dwSize);
    assert(NULL != a_pdwHead);


    if (*a_pdwHead == dwTail)
    {
        return TBUS_ERR_CHANNEL_EMPTY;
    }

    /*检查头指针是否需要移动*/
    TBUS_CHECK_QUEUE_HEAD_VAR(a_pszQueue, dwSize, *a_pdwHead);
    iDataLen = dwTail - *a_pdwHead;
    if (0 > iDataLen)
    {
        iDataLen += dwSize;
    }
    if (0 >= iDataLen)
    {
        return TBUS_ERR_CHANNEL_EMPTY;
    }

    iRet = tbus_decode_head(a_pstHead, (char *)(a_pszQueue +*a_pdwHead), iDataLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to unpack tbus head,iRet %x", iRet);
    }

    return iRet;
}
#endif /*TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/

int tbus_init_headmeta(void)
{
    LPTDRMETALIB pstLib;

    if (NULL != g_stBusGlobal.pstHeadMeta)
    {
        return TBUS_SUCCESS;
    }

    /*get meta of tbus head*/
    pstLib = (LPTDRMETALIB)(void*)g_szMetalib_TbusHead;
    g_stBusGlobal.pstHeadMeta = tdr_get_meta_by_name(pstLib, tdr_get_metalib_name(pstLib));
    if (NULL == g_stBusGlobal.pstHeadMeta)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get meta of tbus head by name %s", tdr_get_metalib_name(pstLib));
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GET_HEAD_META);

    }

    return TBUS_SUCCESS;
}

void tbus_trim_str( char *a_strInput )
{
    char *pb;
    char *pe;
    size_t iTempLength;


    assert(NULL != a_strInput);
    iTempLength = strlen(a_strInput);
    if( iTempLength == 0 )
    {
        return;
    }

    pb = a_strInput;

    while (((*pb == ' ') || (*pb == '\t') || (*pb == '\n') || (*pb == '\r')) && (*pb != 0))
    {
        pb ++;
    }

    pe = &a_strInput[iTempLength-1];
    while ((pe >= pb) && ((*pe == ' ') || (*pe == '\t') || (*pe == '\n') || (*pe == '\r')))
    {
        pe --;
    }

    *(pe+1) = '\0';

    if (pb != a_strInput)
    {
        strcpy( a_strInput, pb );
    }

}

/**
  @brief 获取通道两端队列上剩余消息的字节总数
  @param[in] a_hHandle tbus句柄
  @param[in] a_iLocalAddr 此tbus通道相对于本端的地址
  @param[in] a_iPeerAddr 此tbus通道相对于对端的地址
  @param[in] a_piInFlux 获取输入通道(a_iLocalAddr <-- a_iPeerAddr)剩余消息字节数的指针
  @param[in] a_piOutFlux 获取输出通道(a_iLocalAddr --> a_iPeerAddr)剩余消息字节数的指针
  @retval 0 -- success
  @retval !0 -- failed
  @note
  */
int tbus_get_channel_flux(IN int a_hHandle, IN TBUSADDR a_iLocalAddr, IN TBUSADDR a_iPeerAddr,
                          OUT int *a_piInFlux, OUT int *a_piOutFlux)
{
    int iRet = 0;
    TBUSHANDLE *pstHandle = NULL ;
    int iChannelNum;
    int i;

    if ((NULL == a_piInFlux)||(NULL == a_piOutFlux))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_piInFlux: %p a_piInFlux: %p",
                 a_piInFlux, a_piOutFlux);
        return TBUS_ERR_ARG;
    }
    pstHandle = TBUS_GET_HANDLE(a_hHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_hHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    iChannelNum = pstHandle->dwChannelCnt;
    for (i = 0; i < iChannelNum; i++)
    {
        TBUSCHANNEL *pstChannel = pstHandle->pastChannelSet[i];
        LPCHANNELHEAD pstHead = pstChannel->pstHead;
        if ((a_iLocalAddr == pstHead->astAddr[pstChannel->iRecvSide]) &&
            (a_iPeerAddr == pstHead->astAddr[pstChannel->iSendSide]))
        {
            LPCHANNELVAR pstGet = &pstHead->astQueueVar[pstChannel->iRecvSide];
            LPCHANNELVAR pstPut = &pstHead->astQueueVar[pstChannel->iSendSide];
            *a_piInFlux = pstGet->dwTail - pstGet->dwHead;
            if (0 > *a_piInFlux)
            {
                *a_piInFlux += pstGet->dwSize;
            }
            *a_piOutFlux = pstPut->dwTail - pstPut->dwHead;
            if (0 > *a_piOutFlux)
            {
                *a_piOutFlux += pstPut->dwSize;
            }
            return 0;
        }
    }/*for (i = 0; i < iChannelNum; i++)*/
    if (i >= iChannelNum)
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);
    }

    return iRet;
}

LPTBUSPROCESSSTATE tbus_get_process_state_i(IN LPTBUSHANDLE a_pstHandle, IN TBUSADDR a_iPeerAddr)
{
    LPTBUSPROCESSSTATE pstPeerState = NULL;
    int i = 0;

    assert(NULL != a_pstHandle);
    assert(TBUS_ADDR_ALL != a_iPeerAddr);
    assert(TBUS_ADDR_DEFAULT != a_iPeerAddr);

    for (i = 0; i < (int)a_pstHandle->dwPeerCnt; i++)
    {
        if (a_pstHandle->astPeerState[i].iPeerAddr == a_iPeerAddr)
        {
            pstPeerState = &a_pstHandle->astPeerState[i];
            break;
        }
    }

    return pstPeerState;
}
