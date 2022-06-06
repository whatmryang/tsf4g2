/*
 * tlogmem.c
 *
 *  Created on: 2010-7-7
 *      Author: mickeyxu
 */

#include "tlog_i.h"
#include "tlogmem.h"
#include "tlog/tlog_bill.h"
#include "../tmng/tsm_tdr.h"
#include "tlog_priority.h"
#include "../tloghelp/tlogload_i.h"

#define TLOG_MEM_DUMP_FILE_MIN_SIZE 10000000
#define TMEMLOGHEADMAXSTRING 256

#define TLOGMEMHEADMAGIC 0x1242
#define TLOGMEMHEADVERSION 1

enum tlogMemBlockType
{
    TLOGMEMBLOCK_STRING = 0, TLOGMEMBLOCK_BIN = 1, TLOGMEMBLOCK_PAD = 2
};

typedef enum tlogMemBlockType TLOGMEMBLOCKTYPE;

struct tagTlogMemHead
{
    int iMagic;
    int iVersion;
    int iInited;

    size_t iItemWrited;
    size_t iTotalWritedBytes;

    int iGetBuffCalled;
    size_t iStart;
    size_t iEnd;
    size_t iBuffSize;
    char szBuff[0];
};

typedef struct tagTlogMemHead TLOGMEMHEAD;
typedef struct tagTlogMemHead *LPTLOGMEMHEAD;

struct tagTlogMemBlockHead
{
    size_t iFrameSize;
    struct timeval stTime;
    int iPriority;
    int iLine;
    int iBuffUsed;
    TLOGMEMBLOCKTYPE iType;
    char szFile[TMEMLOGHEADMAXSTRING];
    char szFunc[TMEMLOGHEADMAXSTRING];
    char szBuff[0];
};

struct tagTlogMemBlockTail
{
    size_t iFrameSize;
};

typedef struct tagTlogMemBlockHead TLOGMEMBLOCKHEAD;
typedef struct tagTlogMemBlockTail TLOGMEMBLOCKTAIL;

#define TLOG_MEM_BLOCK_BASE_SIZE    (sizeof(TLOGMEMBLOCKHEAD)+ sizeof(TLOGMEMBLOCKTAIL))

#define TLOG_MEM_ADD_BLOCK(pstMem, iFrameSize) \
{                                                                                                                                               \
    if (((pstMem)->iStart > (pstMem)->iEnd) && (((pstMem)->iEnd + (iFrameSize)) > (pstMem)->iStart)) \
    {                                                                                                                                               \
        (pstMem)->iStart = ((pstMem)->iEnd + (iFrameSize) + 1) % (pstMem)->iBuffSize;               \
    }                                                                                                                                           \
    (pstMem)->iEnd = ((pstMem)->iEnd + (iFrameSize)) % (pstMem)->iBuffSize;                     \
}


// attach到一块已经存在的共享内存上，如果不存在，返回失败
int tlogmem_attach(TLOGMEM *pstMem, TLOGDEVMEM *pstDev)
{
    TsmObjPtrP ptr = NULL;
    TLOGMEMHEAD *pstMemHead;

    assert(pstDev);
    assert(pstMem);
    memset(pstMem, 0, sizeof(*pstMem));
    pstMem->pstDev = pstDev;

    if (tsm_tdr_open(&pstMem->pstTsmHandle))
    {
        return -1;
    }

    if (tsm_tdr_query(pstMem->pstTsmHandle, pstDev->szUri) != 1)
    {
        return -2;
    }

    ptr = tsm_tdr_row_only(pstMem->pstTsmHandle);
    if (!ptr)
    {
        return -3;
    }

    if (pstDev->iBuffSize && pstDev->iBuffSize != tsm_tdr_row_size(ptr))
    {
        return -4;
    }
    else
    {
        pstDev->iBuffSize = tsm_tdr_row_size(ptr);
    }

    if (tsm_tdr_bind_data(pstMem->pstTsmHandle, pstDev->szUri))
    {
        return -5;
    }

    if (tsm_tdr_bind_size(pstMem->pstTsmHandle, pstDev->iBuffSize))
    {
        return -6;//, NULL,-6, "");
    }
    ptr = tsm_tdr_ptr(pstMem->pstTsmHandle, O_NDELAY);
    if (!ptr)
    {
        return -7;
    }
    //, NULL,-7, "");
    pstMem->pvMemHead = tsm_tdr_at(ptr, 0);
    if (!pstMem->pvMemHead)
    {
        return -8;
    }
    pstMemHead = pstMem->pvMemHead;
    return 0;
}

int tlogmem_init(TLOGMEM *pstMem, TLOGDEVMEM *pstDev)
{
    int iCreated = 1;
    TsmObjPtrP ptr = NULL;
    TLOGMEMHEAD *pstMemHead;
    assert(pstDev);
    assert(pstMem);
    memset(pstMem, 0, sizeof(*pstMem));

    pstMem->pstDev = pstDev;

    if (tsm_tdr_open(&pstMem->pstTsmHandle))
    {
        return -1;
    }

    if (tsm_tdr_bind_data(pstMem->pstTsmHandle, pstDev->szUri))
    {
        return -2;
    }

    if (tsm_tdr_bind_size(pstMem->pstTsmHandle, pstDev->iBuffSize))
    {
        return -3;
    }

    ptr = tsm_tdr_ptr(pstMem->pstTsmHandle, O_CREAT | O_EXCL);
    if (!ptr)
    {
        return -4;
    }
    if (ptr == (void *) -1)
    {
        ptr = tsm_tdr_ptr(pstMem->pstTsmHandle, 0);//, NULL, 4, "");
        if (!ptr)
        {
            return -5;
        }
        iCreated = 0;
    }

    pstMem->pvMemHead = tsm_tdr_at(ptr, 0);

    if (!pstMem->pvMemHead)
    {
        return -6;
    }

    if (!iCreated)
    {
        tlogmem_dump(pstMem, pstDev->iBuffSize);
    }

    pstMemHead = pstMem->pvMemHead;
    memset(pstMemHead, 0, sizeof(TLOGMEMHEAD));
    pstMemHead->iBuffSize = pstDev->iBuffSize - sizeof(TLOGMEMHEAD);
    pstMemHead->iMagic = TLOGMEMHEADMAGIC;
    pstMemHead->iVersion = TLOGMEMHEADVERSION;
    pstMemHead->iInited = 1;
    return 0;
}

int tlogmem_is_valid_pos(TLOGMEM *pstMem, size_t iOff)
{
    TLOGMEMHEAD *pstMemHead = pstMem->pvMemHead;

    if (iOff >= pstMemHead->iBuffSize)
    {
        return 0;
    }

    if (pstMemHead->iStart == pstMemHead->iEnd)
    {
        return 0;
    }

    if (pstMemHead->iStart < pstMemHead->iEnd)
    {
        if (iOff >= pstMemHead->iStart && iOff < pstMemHead->iEnd)
        {
            return 1;
        }
    }

    if (pstMemHead->iStart > pstMemHead->iEnd)
    {
        if (!(iOff >= pstMemHead->iEnd && iOff < pstMemHead->iStart))
        {
            return 1;
        }
    }

    return 0;
}

int tlogmem_dump_head(LPTLOGBILL pstBill, TLOGMEMHEAD *pstMem)
{
    // 将头部信息导出到文件中。
    char szHead[1024];
    memset(szHead, 0, sizeof(szHead));
    tlog_bill_log(pstBill, "#####TotalPacket(%d) TotalBytes(%d)####\n",
            pstMem->iItemWrited, pstMem->iTotalWritedBytes);
    return 0;
}

int tlogmem_dump_item(LPTLOGBILL pstBill, TLOGMEMBLOCKHEAD *pstHead)
{
    int iLen;
    struct tm tm;
    char szHead[1024];
#if defined (_WIN32) || defined (_WIN64)
#pragma warning (disable: 4133)
#endif
   if(TLOGMEMBLOCK_PAD ==  pstHead->iType)
    {
        return 0;
    }


    memset(szHead, 0, sizeof(szHead));
    errno = 0;
    localtime_r(&pstHead->stTime.tv_sec, &tm);

    iLen = snprintf(szHead, sizeof(szHead),
            "[%04d%02d%02d %02d:%02d:%02d%06d]:[%s:%d][%s()][%s]", tm.tm_year
                    + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
            tm.tm_sec, (int) pstHead->stTime.tv_usec, pstHead->szFile,
            pstHead->iLine, pstHead->szFunc, tlog_priority_to_string(
                    pstHead->iPriority));
#if defined (_WIN32) || defined (_WIN64)
    if (iLen < 0)
    {
        if (EINVAL == errno)
        {
            return -1;
        }
        else
        {
            iLen = sizeof(szHead);
        }
    }

#else
    if (iLen < 0)
    {
        return -1;
    }

    if (iLen >= sizeof(szHead))
    {
        iLen = sizeof(szHead);
    }
#endif

    tlog_bill_write(pstBill, szHead, iLen);
    // Write the buffer used.
    {
        tlog_bill_write(pstBill, pstHead->szBuff, pstHead->iBuffUsed);
        tlog_bill_write(pstBill, "\n", 1);
    }
    return 0;
}

int tlogmem_dump(TLOGMEM *pstMem, size_t iSize)
{
    char *pszBuff = NULL;
    int iRet;

    LPTLOGBILL pstBill = NULL;
    TLOGMEMHEAD *pstMemHead = pstMem->pvMemHead;
    TLOGMEMBLOCKHEAD *pstHead;
    TLOGMEMBLOCKTAIL *pstTail;
    size_t tiEnd;

    if (pstMemHead->iMagic != TLOGMEMHEADMAGIC || pstMemHead->iVersion
            != TLOGMEMHEADVERSION || iSize < sizeof(TLOGMEM))
    {
        return -1;
    }

    if (pstMemHead->iBuffSize + sizeof(TLOGMEMHEAD) != iSize)
    {
        return -1;
    }

    pszBuff = calloc(1, iSize);
    if (!pszBuff)
    {
        return -1;
    }

    memcpy(pszBuff, pstMemHead, iSize);
    pstMemHead = (TLOGMEMHEAD *) pszBuff;

    if (pstMemHead->iStart == pstMemHead->iEnd)
    {
        return 0;
    }

    if (pstMem->pstDev->dwSizeLimit < TLOG_MEM_DUMP_FILE_MIN_SIZE)
    {
        pstMem->pstDev->dwSizeLimit = TLOG_MEM_DUMP_FILE_MIN_SIZE;
    }

    iRet = tlog_bill_init(&pstBill, pstMem->pstDev->dwSizeLimit, 1,
            pstMem->pstDev->szPattern);
    if (iRet)
    {
        goto error;
    }

    // 写头部信息和统计信息
    tlogmem_dump_head(pstBill, pstMemHead);

    tiEnd = pstMemHead->iEnd;
    if (0 == tiEnd)
    {
        tiEnd = pstMemHead->iBuffSize;
    }

    while (1)
    {
        // 先取尾巴
        if (sizeof(TLOGMEMBLOCKTAIL) > (tiEnd))
        {
            goto error;
        }

        pstTail = (TLOGMEMBLOCKTAIL *) (pstMemHead->szBuff + (tiEnd
                - sizeof(TLOGMEMBLOCKTAIL)));

        if (tiEnd < pstTail->iFrameSize)
        {
            goto error;
        }
        // 取头
        pstHead = (TLOGMEMBLOCKHEAD *) (pstMemHead->szBuff + tiEnd
                - pstTail->iFrameSize);
        // 恢复消息。
        if (!tlogmem_is_valid_pos(pstMem, tiEnd - pstTail->iFrameSize))
        {
            goto error;
        }

        if (pstHead->iFrameSize != pstTail->iFrameSize)
        {
            goto error;
        }

        // 如果有数据，将数据写入文件。
        if (pstHead->iBuffUsed > 0 && (pstHead->iType != TLOGMEMBLOCK_PAD))
        {
            iRet = tlogmem_dump_item(pstBill, pstHead);
            if (iRet)
            {
                goto error;
            }
        }
        // 移动尾指针

        if (tiEnd >= pstHead->iFrameSize)
        {
            tiEnd -= pstHead->iFrameSize;
        }

        if (tiEnd == 0)
        {
            tiEnd = pstMemHead->iBuffSize;
        }
    }

    error: tlog_bill_fini(&pstBill);
    if (pszBuff)
    {
        free(pszBuff);
    }
    return 0;
}

// write中不需要做任何动作
int tlogmem_writev(TLOGMEM *pstMem, char *pszBuff, size_t iBuff)
{
    // Write binary message.
    return 0;
}

// 分配可写空间
int tlogmem_alloca_buffer(TLOGMEM *pstMemHandle, size_t iSize)
{
    TLOGMEMHEAD *pstHead = NULL;
    pstHead = pstMemHandle->pvMemHead;
    if (pstHead->iEnd > pstHead->iStart)
    {
        if (pstHead->iEnd + iSize > pstHead->iBuffSize)
        {
            return -1;
        }
        return 0;
    }

    if (pstHead->iStart - pstHead->iEnd > iSize)
    {
        return 0;
    }

    if (pstHead->iStart == pstHead->iEnd)
    {
        return 0;
    }

    pstHead->iStart = pstHead->iEnd + iSize;

    if (pstHead->iStart > pstHead->iBuffSize)
    {
        return -1;
    }
    return 0;
}

int tlogmem_get_buff(TLOGMEM *pstMemHandle, char **ppszBuff, size_t iBuff)
{
    size_t iEstSize = iBuff + TLOG_MEM_BLOCK_BASE_SIZE;
    TLOGMEMBLOCKHEAD *pstBlockHead = NULL;
    TLOGMEMHEAD*pstMem;
    pstMem = pstMemHandle->pvMemHead;

    if (!ppszBuff || !pstMem->iInited)
    {
        return -1;
    }

    if (iEstSize > (size_t) pstMem->iBuffSize)
    {
        return -1;
    }
    assert(pstMem->iEnd < pstMem->iBuffSize);

    // 在缓冲区中创建一个空消息，填充消息尾部
    if (pstMem->iEnd + iEstSize >= pstMem->iBuffSize)
    {
        TLOGMEMBLOCKHEAD *pstHead = NULL;
        TLOGMEMBLOCKTAIL *pstTail = NULL;
        size_t iFree = pstMem->iBuffSize -pstMem->iEnd ;

         if (iFree <  TLOG_MEM_BLOCK_BASE_SIZE)
        {
            pstMem->iEnd = pstMem->iStart = 0;
            return -1;
        }

        pstHead = (TLOGMEMBLOCKHEAD*) (pstMem->szBuff + pstMem->iEnd);
        pstHead->iFrameSize = iFree;
        pstHead->iType = TLOGMEMBLOCK_PAD;
        pstTail = (TLOGMEMBLOCKTAIL*) (pstMem->szBuff + pstMem->iBuffSize
                - sizeof(TLOGMEMBLOCKTAIL));
        pstTail->iFrameSize = iFree;

        TLOG_MEM_ADD_BLOCK(pstMem, iFree);
    }

    pstBlockHead = (TLOGMEMBLOCKHEAD *) &pstMem->szBuff[pstMem->iEnd];
    *ppszBuff =  pstBlockHead->szBuff ;

    return 0;
}

// 需要判断剩余空间是否足够放下一条消息，如果不够，需要扩展大小
int tlogmem_make_frame(TLOGMEM *pstMemHandle, TLOGEVENTBASE *pstEvt,
        size_t iUsed)
{
    size_t iFrameSize;
    TLOGMEMBLOCKHEAD *pstBlockHead = NULL;
    TLOGMEMBLOCKTAIL *pstBlockTail = NULL;
    TLOGMEMHEAD*pstMem = pstMemHandle->pvMemHead;

    if (!pstMem || !pstMem->iInited)
    {
        return -1;
    }

    if(pstEvt->evt_is_msg_binary)
     {
            char *pszBuff = NULL;
            int iRet = tlogmem_get_buff(pstMemHandle, &pszBuff, pstEvt->evt_msg_len);
            if (iRet != 0)
            {
                return -1;
            }else
            {
                memcpy(pszBuff, pstEvt->evt_msg,pstEvt->evt_msg_len);
                iUsed = pstEvt->evt_msg_len;
            }
     }/*if(pstEvt->evt_is_msg_binary)*/



    iFrameSize = TLOG_MEM_BLOCK_BASE_SIZE + iUsed;

    // 如果剩下的空间不够放下一条消息，就扩张当前的消息
    if ((pstMem->iEnd + iFrameSize) > pstMem->iBuffSize)
    {
        return -1;
    }
    if (pstMem->iEnd + iFrameSize + TLOG_MEM_BLOCK_BASE_SIZE > pstMem->iBuffSize)
    {
        iFrameSize = pstMem->iBuffSize - pstMem->iEnd;
    }

    // 修改头部信息
    pstBlockHead = (TLOGMEMBLOCKHEAD *) &pstMem->szBuff[pstMem->iEnd];
    pstBlockHead->iFrameSize = iFrameSize;
    pstBlockHead->iBuffUsed = iUsed;
    pstBlockHead->iPriority = pstEvt->evt_priority;
    pstBlockHead->iLine = pstEvt->evt_loc->loc_line;
    pstBlockHead->stTime = pstEvt->evt_timestamp;
    STRNCPY(pstBlockHead->szFile,pstEvt->evt_loc->loc_file,sizeof(pstBlockHead->szFile));
    STRNCPY(pstBlockHead->szFunc,pstEvt->evt_loc->loc_function,sizeof(pstBlockHead->szFunc));
    if (pstEvt->evt_is_msg_binary)
    {
        pstBlockHead->iType = TLOGMEMBLOCK_BIN;
    }else
    {
        pstBlockHead->iType = TLOGMEMBLOCK_STRING;
    }

    // 修改尾部信息
    pstBlockTail = (TLOGMEMBLOCKTAIL *) (((size_t) pstBlockHead) + iFrameSize
            - sizeof(TLOGMEMBLOCKTAIL));
    pstBlockTail->iFrameSize = iFrameSize;

    // 修改全局状态信息和统计量
    TLOG_MEM_ADD_BLOCK(pstMem, iFrameSize);

    pstMem->iItemWrited++;
    pstMem->iTotalWritedBytes += iFrameSize;


    return 0;
}

int tmemlog_dump_memdev(TLOGDEVMEM *pstDev)
{
    TLOGMEM stMem;
    int iRet;
    iRet = tlogmem_attach(&stMem, pstDev);
    if (iRet)
    {
        printf("无法附加到共享内存URI(%s),iRet(%d)\n", pstDev->szUri, iRet);
        return -1;
    }

    tlogmem_dump(&stMem, pstDev->iBuffSize);
    return 0;
}

int tmemlog_dump_tlog_conf(const char *pszConfFileName)
{
    TLOGCONF stLogConf;
    int iRet;
    int i;
    iRet = tlog_init_cfg_from_file(&stLogConf, pszConfFileName);
    if (iRet)
    {
        printf("从文件（%s）初始化日志配置失败，请检查文件是否正确", pszConfFileName);
        return -1;
    }

    for (i = 0; i < stLogConf.iCount; i++)
    {
        if (stLogConf.astCategoryList[i].stDevice.iType == TLOG_DEV_MEM)
        {
            tmemlog_dump_memdev(
                    &stLogConf.astCategoryList[i].stDevice.stDevice.stMem);
        }
    }

    return 0;
}

int tlogmem_fini(TLOGMEM *pstMem)
{
    tsm_tdr_dt(pstMem->pvMemHead);
    tsm_tdr_close(&pstMem->pstTsmHandle);
    return 0;
}
