/**
 *
 * @file     tdr_net.c
 * @brief    TDR元数据网络交换消息编解码模块
 *
 * @author steve jackyai
 * @version 1.0
 * @date 2007-04-28
 *
 *
 * Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved.
 *
 */

#include <assert.h>
#include <wchar.h>


#include "tdr_os.h"
#include "tdr/tdr_net.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr_define_i.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_metalib_manage.h"
#include "tdr_ctypes_info_i.h"
#include "tdr_net_i.h"
#include "tdr_error_i.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif


#define TDR_CALC_META_SIZEINFO_TARGET(pstStackTop, pszNet)			\
{																	\
    if (0 > pstStackTop->iMetaSizeInfoOff)							\
    {																\
        pstStackTop->pszMetaSizeInfoTarget = pszNet;				\
        pszNet -= pstStackTop->iMetaSizeInfoOff;					\
        pstStackTop->pszNetBase = pszNet;							\
    }else															\
    {																\
        pstStackTop->pszNetBase = pszNet;							\
        pstStackTop->pszMetaSizeInfoTarget = pstStackTop->pszNetBase + pstStackTop->iMetaSizeInfoOff ; \
    }																					\
}


#define TDR_META_HAS_VERSION_INDICATOR(pstMeta) (0 < (pstMeta)->stVersionIndicator.iUnitSize)

#define TDR_GET_VERSION_INDICATOR_NET(a_pszNetBase, a_pszNetEnd,pstCurMeta, iCutOffVersion, a_iVersion) \
{																   \
    int64_t lval =0;											\
    char *pszPtr = a_pszNetBase + pstCurMeta->stVersionIndicator.iNOff;	\
    if ((a_pszNetEnd - pszPtr) >= (ptrdiff_t)pstCurMeta->stVersionIndicator.iUnitSize)\
    {																	\
        TDR_GET_INT_NET(lval, pstCurMeta->stVersionIndicator.iUnitSize, pszPtr);	\
        iCutOffVersion = (int)lval;							\
        if (0 == iCutOffVersion)							\
        {													\
            iCutOffVersion = a_iVersion;					\
        }													\
    }else														\
    {															\
        iCutOffVersion = a_iVersion;							\
    }                                                           \
}

#define TDR_PUSH_STACK_ITEM(a_pstStackTop,  a_pstTypeMeta, a_pstEntry, a_iCount, a_idx, a_pszHostStart, a_pszNetStart) \
{																						\
    a_pstStackTop->pstMeta = a_pstTypeMeta;												\
    a_pstStackTop->iCount = a_iCount;													\
    a_pstStackTop->idxEntry = a_idx;													\
    a_pstStackTop->pszHostBase = a_pszHostStart;										\
    a_pstStackTop->pszNetBase = a_pszNetStart;											\
    if (a_pstTypeMeta->stSizeType.iUnitSize > 0)										\
    {																					\
        a_pstStackTop->iMetaSizeInfoUnit = a_pstTypeMeta->stSizeType.iUnitSize;			\
        if (TDR_INVALID_INDEX != a_pstTypeMeta->stSizeType.idxSizeType)					\
        {																				\
            a_pstStackTop->pszMetaSizeInfoTarget = a_pszNetStart;						\
            a_pstStackTop->iMetaSizeInfoOff = 0 - a_pstStackTop->iMetaSizeInfoUnit;		\
            a_pszNetStart += a_pstStackTop->iMetaSizeInfoUnit;							\
        }else																			\
        {																				\
            a_pstStackTop->pszMetaSizeInfoTarget = a_pszNetStart + a_pstTypeMeta->stSizeType.iNOff;		\
            a_pstStackTop->iMetaSizeInfoOff = a_pstTypeMeta->stSizeType.iNOff;			\
        }																				\
    }else if (a_pstEntry->stSizeInfo.iUnitSize > 0)										\
    {																					\
        a_pstStackTop->iMetaSizeInfoUnit = a_pstEntry->stSizeInfo.iUnitSize;			\
        if (TDR_INVALID_INDEX != a_pstEntry->stSizeInfo.idxSizeType)					\
        {																				\
            a_pstStackTop->pszMetaSizeInfoTarget = a_pszNetStart;						\
            a_pszNetStart += a_pstStackTop->iMetaSizeInfoUnit;							\
            a_pstStackTop->iMetaSizeInfoOff = 0 - a_pstStackTop->iMetaSizeInfoUnit;		\
        }else  if (a_pstEntry->stSizeInfo.iNOff >= pstEntry->iNOff)						\
        {																				\
            a_pstStackTop->iMetaSizeInfoOff = a_pstEntry->stSizeInfo.iNOff - a_pstEntry->iNOff;	\
            a_pstStackTop->pszMetaSizeInfoTarget = a_pszNetStart + a_pstStackTop->iMetaSizeInfoOff ; \
        }else																			\
        {																				\
            a_pstStackTop->pszMetaSizeInfoTarget = (a_pstStackTop - 1)->pszNetBase + a_pstEntry->stSizeInfo.iNOff; \
        }																				\
    }else																				\
    {																					\
        a_pstStackTop->iMetaSizeInfoUnit = 0;											\
    }																					\
    a_pstStackTop->pszNetBase = a_pszNetStart;											\
}



/* 1字节、2字节、4字节可以表示的最大整数值
 * 此最大值用于检查设置sizeinfo时，是否会溢出
 */
int64_t llNumericLimits[] = {0xff, 0xffff, 0, 0xffffffff};


int tdr_hton(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstNet, INOUT LPTDRDATA a_pstHost, IN int a_iVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;
    int iChange;
    LPTDRMETAENTRY pstEntry;
    int iArrayRealCount ;
    char *pszNetStart;
    char *pszNetEnd;
    char *pszHostStart;
    char *pszHostEnd;
    int idxSubEntry;

    TDR_CLR_ERR_BUF();
    if ((NULL == a_pstMeta)||(TDR_TYPE_UNION == a_pstMeta->iType)
        ||(NULL == a_pstNet)||(NULL == a_pstHost))
    {
        TDR_ERR_GET_LEN();
        TDR_SET_ERROR(TDR_ERR_BUF, "invalid parameters:"
                      " a_pstMeta<%p>==NULL"
                      " or a_pstMeta->iType==TDR_TYPE_UNION"
                      " or a_pstNext<%p>==NULL"
                      " or a_pstHost<%p>==NULL",
                      a_pstMeta, a_pstNet, a_pstHost);

        return TDR_ERRIMPLE_INVALID_PARAM;
    }
    if ((NULL == a_pstNet->pszBuff)||(0 >= a_pstNet->iBuff)||
        (NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
    {
        TDR_ERR_GET_LEN();
        TDR_SET_ERROR(TDR_ERR_BUF, "invalid parameters:"
                      " a_pstNet->pszBuff<%p>==NULL"
                      " or a_pstNet->iBuff<%u><=0"
                      " or a_pstHost->pszBuff<%p>==NULL"
                      " or a_pstHost->iBuff<%u><=0",
                      a_pstNet->pszBuff, (unsigned)a_pstNet->iBuff,
                      a_pstHost->pszBuff, (unsigned)a_pstHost->iBuff);

        return TDR_ERRIMPLE_INVALID_PARAM;
    }

    if ((0 == a_iVersion) ||(a_iVersion > a_pstMeta->iCurVersion))
    {
        a_iVersion = a_pstMeta->iCurVersion;
    }

    if ((a_pstMeta->iBaseVersion > a_iVersion)
        || (TDR_META_HAS_VERSION_INDICATOR(a_pstMeta)
            && (a_pstMeta->iVersionIndicatorMinVer > a_iVersion)))
    {
        TDR_ERR_GET_LEN();

        a_pstHost->iBuff = 0;
        a_pstNet->iBuff = 0;
        if (a_pstMeta->iBaseVersion > a_iVersion)
        {
            TDR_SET_ERROR(TDR_ERR_BUF, "invalid parameter:"
                          " expect a_iVersion<%d> NOT less than"
                          " base_version<%d> of meta<%s>",
                          a_iVersion, a_pstMeta->iBaseVersion,
                          tdr_get_meta_name(a_pstMeta));
        } else
        {
            TDR_SET_ERROR(TDR_ERR_BUF, "invalid parameter:"
                          " expect a_iVersion<%d> NOT less than"
                          " min_ver<%d> of meta<%s>'s versionindicator",
                          a_iVersion, a_pstMeta->iVersionIndicatorMinVer,
                          tdr_get_meta_name(a_pstMeta));
        }
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    pszNetStart = a_pstNet->pszBuff;
    pszNetEnd = a_pstNet->pszBuff + a_pstNet->iBuff;
    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstCurMeta = a_pstMeta;
    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstStackTop = &stStack[0];
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->pszHostBase = a_pstHost->pszBuff;
    pstStackTop->pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    if (0 < pstCurMeta->stSizeType.iUnitSize)
    {
        pstStackTop->iMetaSizeInfoUnit = pstCurMeta->stSizeType.iUnitSize;
        if( TDR_INVALID_INDEX != pstCurMeta->stSizeType.idxSizeType )
        {
            pstStackTop->pszMetaSizeInfoTarget = pszNetStart;
            pszNetStart += pstStackTop->iMetaSizeInfoUnit;
        }else
        {
            pstStackTop->pszMetaSizeInfoTarget = pszNetStart + pstCurMeta->stSizeType.iNOff;
        }
    }else
    {
        pstStackTop->iMetaSizeInfoUnit = 0;
        pstStackTop->iMetaSizeInfoOff =	0;
        pstStackTop->pszMetaSizeInfoTarget = NULL;
    }
    pstStackTop->pszNetBase = pszNetStart;
    pstStackTop->iChange = 1;
    pstStackTop->iCode = 0;

    TDR_SET_VERSION_INDICATOR(iRet, pszHostStart, pszHostEnd, pstCurMeta, a_iVersion);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        TDR_ERR_GET_LEN();
        TDR_SET_ERROR(TDR_ERR_BUF, "; %s:%d", __FILE__, __LINE__);
        a_pstHost->iBuff = 0;
        a_pstNet->iBuff = 0;
        return iRet;
    }

    iStackItemCount = 1;
    pstStackTop->pstEntry = NULL;


    iChange = 0;
    while (0 < iStackItemCount)
    {
        if (0 != iChange)
        {
            iChange = 0;
            if (0 < pstStackTop->iMetaSizeInfoUnit)
            {
                int64_t lLen = pszNetStart - pstStackTop->pszNetBase;

                if (lLen > llNumericLimits[pstStackTop->iMetaSizeInfoUnit - 1])
                {
                    TDR_ERR_GET_LEN();
                    TDR_SET_ERROR(TDR_ERR_BUF, "sizeinfo numeric overflow:"
                                  " real_value<%d> value_limit<%u>"
                                  " entry<%s> meta<%s> unpacked_entry_count<%d>", (int)lLen,
                                  (unsigned)llNumericLimits[pstStackTop->iMetaSizeInfoUnit - 1],
                                  tdr_get_entry_name(pstStackTop->pstEntry),
                                  tdr_get_meta_name(pstStackTop->pstMeta),
                                  pstStackTop->iCount);
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_SIZEINFO_OVERFLOW);
                    break;
                }
                TDR_SET_INT_NET(pstStackTop->pszMetaSizeInfoTarget,
                                pstStackTop->iMetaSizeInfoUnit,	lLen);

            }
            if (0 < pstStackTop->iCount)
            {
                if (pszNetStart > pszNetEnd)
                {
                    TDR_ERR_GET_LEN();
                    TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"
                                  " pszNetStart<%p> pszNetEnd<%p>"
                                  " entry<%s> meta<%s> unpacked_entry_count<%d>",
                                  pszNetStart, pszNetEnd,
                                  tdr_get_entry_name(pstStackTop->pstEntry),
                                  tdr_get_meta_name(pstStackTop->pstMeta),
                                  pstStackTop->iCount);
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);
                    break;
                }
                if (0 < pstStackTop->iMetaSizeInfoUnit)
                {
                    TDR_CALC_META_SIZEINFO_TARGET(pstStackTop, pszNetStart);
                } else
                {
                    pstStackTop->pszNetBase = pszNetStart;							\
                }
            }

            if (1 < iStackItemCount)
            {
                //统计编码字节数
                assert(NULL != pstStackTop->pstEntry);
                if (TDR_ENTRY_IS_REFER_TYPE(pstStackTop->pstEntry))
                {
                    (pstStackTop-1)->iCode += pstStackTop->pstEntry->iHUnitSize;
                }else
                {
                    (pstStackTop-1)->iCode += pstCurMeta->iHUnitSize;
                }
                pstStackTop->iCode = 0;
            }
        }/*if (0 != iChange)*/


        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;



        if (pstEntry->iVersion > a_iVersion)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*指针不编码*/
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }



        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_iVersion);
        if ((0 > iArrayRealCount)
            || ((pstEntry->iCount < iArrayRealCount) && (0 < pstEntry->iCount)))
        {/*实际数目为负数或比数组最大长度要大，则无效*/
            TDR_ERR_GET_LEN();
            TDR_SET_ERROR(TDR_ERR_BUF, "invalid refer value:"
                          " expect refer_value<%d> in [0, array_count<%d>]"
                          " about entry<%s> of meta<%s>",
                          iArrayRealCount, pstEntry->iCount,
                          tdr_get_entry_name(pstEntry), tdr_get_meta_name(pstCurMeta));
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            pstStackTop->iCode = pstEntry->iHOff;
            break;
        }
        if (0 >= iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
        if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            pszHostStart = *(char **)pszHostStart;
            if (TDR_TYPE_COMPOSITE < pstEntry->iType)
            {
                pszHostEnd = pszHostStart + pstEntry->iNRealSize;
            }else
            {
                LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                pszHostEnd = pszHostStart + pstTypeMeta->iHUnitSize;
            }
        }else
        {
            pszHostEnd = pstStackTop->pszHostEnd;
        }


        /*简单数据类型*/
        if (TDR_TYPE_COMPOSITE < pstEntry->iType)
        {
            TDR_PACK_SIMPLE_TYPE_ENTRY(iRet, pstEntry, iArrayRealCount,
                                       pszNetStart, pszNetEnd, pszHostStart, pszHostEnd);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                TDR_ERR_GET_LEN();
                TDR_SET_ERROR(TDR_ERR_BUF, "; entry<%s> of meta<%s> %s:%d",
                              tdr_get_entry_name(pstEntry), tdr_get_meta_name(pstCurMeta),
                              __FILE__, __LINE__);
                break;
            }
            if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
            {
                pstStackTop->iCode = pstEntry->iHOff + pstEntry->iHOff;
            }else
            {
                pstStackTop->iCode = pszHostStart - pstStackTop->pszHostBase;
            }
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*复合数据类型*/
        if (TDR_STACK_SIZE <=  iStackItemCount)
        {
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
            pstStackTop->iCode = pstEntry->iHOff;
            break;
        }

        if (TDR_TYPE_UNION == pstEntry->iType)
        {
            TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, a_iVersion, pstCurMeta, idxSubEntry);
            if(NULL == pstCurMeta)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }
        }else
        {
            pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
            idxSubEntry = 0;
        }

        if (pstCurMeta->iVersionIndicatorMinVer > a_iVersion)
        {
            TDR_ERR_GET_LEN();
            TDR_SET_ERROR(TDR_ERR_BUF, "invalid cut-off version:"
                          " expect a_iVersion<%d> NOT less than"
                          " min_ver<%d> of meta<%s>'s versionindicator",
                          a_iVersion, pstCurMeta->iVersionIndicatorMinVer,
                          tdr_get_meta_name(pstCurMeta));
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
            break;
        }

        /*递归进行结构体内部进行处理*/
        iStackItemCount++;
        pstStackTop++;
        pstStackTop->iCode = 0;
        TDR_PUSH_STACK_ITEM(pstStackTop, pstCurMeta, pstEntry, iArrayRealCount, idxSubEntry,
                            pszHostStart, pszNetStart);
        pstStackTop->pszHostEnd = pszHostEnd;
        pstStackTop->pstEntry = pstEntry;
        pstStackTop->iChange = 1;
        TDR_SET_VERSION_INDICATOR(iRet, pszHostStart, pszHostEnd, pstCurMeta, a_iVersion);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            TDR_ERR_GET_LEN();
            TDR_SET_ERROR(TDR_ERR_BUF, "; %s:%d", __FILE__, __LINE__);
            break;
        }


    }/*while (0 < iStackItemCount)*/

    a_pstHost->iBuff = (int)stStack[0].iCode;
    if (TDR_ERR_IS_ERROR(iRet))
    {
        for(iChange= 1; iChange < iStackItemCount; iChange++)
        {
            //统计编码字节数
            assert(NULL != stStack[iChange].pstEntry);
            if (TDR_ENTRY_IS_REFER_TYPE(stStack[iChange].pstEntry))
            {
                a_pstHost->iBuff += (int)stStack[iChange].pstEntry->iHUnitSize;
                break;
            }else
            {
                a_pstHost->iBuff += (int)stStack[iChange].iCode;
            }
        }
    }/*if (TDR_ERR_IS_ERROR(iRet))*/


    a_pstNet->iBuff = pszNetStart - a_pstNet->pszBuff;

    return iRet;
}

int tdr_ntoh(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, INOUT LPTDRDATA a_pstNet, IN int a_iVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;
    int iChange;
    char *pszNetStart;
    char *pszNetEnd;
    char *pszHostStart;
    char *pszHostEnd;
    LPTDRMETAENTRY pstEntry;
    int iArrayRealCount ;
    int64_t iLen;
    int idxSubEntry;


    if ((NULL == a_pstMeta)||(TDR_TYPE_UNION == a_pstMeta->iType)
        ||(NULL == a_pstNet)||(NULL == a_pstHost))
    {
        TDR_ERR_GET_LEN();
        TDR_SET_ERROR(TDR_ERR_BUF, "invalid parameters:"
                      " a_pstMeta<%p>==NULL"
                      " or a_pstMeta->iType==TDR_TYPE_UNION"
                      " or a_pstNext<%p>==NULL"
                      " or a_pstHost<%p>==NULL",
                      a_pstMeta, a_pstNet, a_pstHost);
        return TDR_ERRIMPLE_INVALID_PARAM;
    }
    if ((NULL == a_pstNet->pszBuff)||(0 >= a_pstNet->iBuff)||
        (NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
    {
        TDR_ERR_GET_LEN();
        TDR_SET_ERROR(TDR_ERR_BUF, "invalid parameters:"
                      " a_pstNet->pszBuff<%p>==NULL"
                      " or a_pstNet->iBuff<%u><=0"
                      " or a_pstHost->pszBuff<%p>==NULL"
                      " or a_pstHost->iBuff<%u><=0",
                      a_pstNet->pszBuff, (unsigned)a_pstNet->iBuff,
                      a_pstHost->pszBuff, (unsigned)a_pstHost->iBuff);
        return TDR_ERRIMPLE_INVALID_PARAM;
    }

    if ((0 == a_iVersion) ||(a_iVersion > a_pstMeta->iCurVersion))
    {
        a_iVersion = a_pstMeta->iCurVersion;
    }

    if ((a_pstMeta->iBaseVersion > a_iVersion) || (a_iVersion> a_pstMeta->iCurVersion))
    {
        TDR_ERR_GET_LEN();
        TDR_SET_ERROR(TDR_ERR_BUF, "invalid parameter:"
                      " expect a_iVersion<%d> in [base_ver<%d>, cur_ver<%d>] of meta<%s>",
                      a_iVersion, a_pstMeta->iBaseVersion, a_pstMeta->iCurVersion,
                      tdr_get_meta_name(a_pstMeta));
        a_pstHost->iBuff = 0;
        a_pstNet->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    /* 保证a_iVersion的值不小于meta允许的版本指示器的最小值，从而保证版本指示器本身不被剪裁掉 */
    if (TDR_META_HAS_VERSION_INDICATOR(a_pstMeta)
        && (a_pstMeta->iVersionIndicatorMinVer > a_iVersion))
    {
        TDR_ERR_GET_LEN();
        TDR_SET_ERROR(TDR_ERR_BUF, "invalid parameter:"
                      " expect a_iVersion<%d> NOT less than"
                      " min_ver<%d> of meta<%s>'s versionindicator",
                      a_iVersion, a_pstMeta->iVersionIndicatorMinVer,
                      tdr_get_meta_name(a_pstMeta));
        a_pstHost->iBuff = 0;
        a_pstNet->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    pszNetStart = a_pstNet->pszBuff;
    pszNetEnd = a_pstNet->pszBuff + a_pstNet->iBuff;
    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;

    pstCurMeta = a_pstMeta;
    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstStackTop = &stStack[0];
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->pszNetBase = pszNetStart;
    pstStackTop->pszHostBase = a_pstHost->pszBuff;
    pstStackTop->pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    if (0 < pstCurMeta->stSizeType.iUnitSize)
    {
        pstStackTop->iMetaSizeInfoUnit = pstCurMeta->stSizeType.iUnitSize;
        if( TDR_INVALID_INDEX != pstCurMeta->stSizeType.idxSizeType )
        {
            pstStackTop->pszMetaSizeInfoTarget = pszNetStart;
            pszNetStart += pstStackTop->iMetaSizeInfoUnit;
        }else
        {
            pstStackTop->pszMetaSizeInfoTarget = pszNetStart + pstCurMeta->stSizeType.iNOff;
        }
    }else
    {
        pstStackTop->iMetaSizeInfoUnit = 0;
        pstStackTop->iMetaSizeInfoOff =	0;
        pstStackTop->pszMetaSizeInfoTarget = NULL;
    }

    if (TDR_META_HAS_VERSION_INDICATOR(pstCurMeta))
    {
        TDR_GET_VERSION_INDICATOR_NET(pszNetStart, pszNetEnd, pstCurMeta,
                                      pstStackTop->iCutOffVersion, a_iVersion);
        if ((pstCurMeta->iBaseVersion > pstStackTop->iCutOffVersion)
            || (pstStackTop->iCutOffVersion > pstCurMeta->iCurVersion)
            || (pstStackTop->iCutOffVersion < pstCurMeta->iVersionIndicatorMinVer))
        {
            TDR_ERR_GET_LEN();
            TDR_SET_ERROR(TDR_ERR_BUF, "invalid versionindicator value:"
                          " expect indicator<%d> in [base_ver<%d>, cur_ver<%d>]"
                          " and indicator<%d> NOT less than min_ver<%d>"
                          " of meta<%s>'s versionindicator, a_iVersion<%d>",
                          pstStackTop->iCutOffVersion, pstCurMeta->iBaseVersion,
                          pstCurMeta->iCurVersion, pstStackTop->iCutOffVersion,
                          pstCurMeta->iVersionIndicatorMinVer,
                          tdr_get_meta_name(pstCurMeta), a_iVersion);
            a_pstHost->iBuff = 0;
            a_pstNet->iBuff = 0;
            return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_INDICATOR_VALUE);
        }
    }else
    {
        pstStackTop->iCutOffVersion = a_iVersion;
    }

    iStackItemCount = 1;
    pstStackTop->iCode = 0;
    pstStackTop->iChange = 1;
    pstStackTop->pstEntry = NULL;


    iChange = 0;
    while (0 < iStackItemCount)
    {

        if (0 != iChange)
        {
            iChange	= 0;

            if (0 < pstStackTop->iMetaSizeInfoUnit)
            {
                TDR_GET_INT_NET(iLen, pstStackTop->iMetaSizeInfoUnit,
                                pstStackTop->pszMetaSizeInfoTarget);
                pszNetStart	= pstStackTop->pszNetBase + iLen;
                if ( pszNetStart > pszNetEnd )
                {
                    TDR_ERR_GET_LEN();
                    TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"
                                  " pszNetStart<%p> pszNetEnd<%p> sizeinfo<%d>"
                                  " entry<%s> meta<%s> unpacked_entry_count<%d>",
                                  pszNetStart, pszNetEnd, (int)iLen,
                                  tdr_get_entry_name(pstStackTop->pstEntry),
                                  tdr_get_meta_name(pstStackTop->pstMeta),
                                  pstStackTop->iCount);
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);
                    break;
                }
                //TDR_CALC_META_SIZEINFO_TARGET(pstStackTop, pszNetStart);
            }


            if (0 < pstStackTop->iCount)
            {
                if (0 < pstStackTop->iMetaSizeInfoUnit)
                {
                    TDR_CALC_META_SIZEINFO_TARGET(pstStackTop, pszNetStart);
                } else
                {
                    pstStackTop->pszNetBase =  pszNetStart;
                }
            }

            if (1 < iStackItemCount)
            {
                //统计编码字节数
                assert(NULL != pstStackTop->pstEntry);
                if (TDR_ENTRY_IS_REFER_TYPE(pstStackTop->pstEntry))
                {
                    (pstStackTop-1)->iCode += pstStackTop->pstEntry->iHUnitSize;
                }else
                {
                    (pstStackTop-1)->iCode += pstCurMeta->iHUnitSize;
                }
                pstStackTop->iCode = 0;
            }
        }/*if (0 != iChange)*/

        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;



        /*指针不编码*/
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry,
                                 pstStackTop->pszHostBase, pstStackTop->iCutOffVersion);
        if ((0 > iArrayRealCount)
            || ((pstEntry->iCount < iArrayRealCount) && (0 < pstEntry->iCount)))
        {/*实际数目为负数或比数组最大长度要大，则无效*/
            TDR_ERR_GET_LEN();
            TDR_SET_ERROR(TDR_ERR_BUF, "invalid refer value:"
                          " expect refer_value<%d> in [0, array_count<%d>]"
                          " about entry<%s> of meta<%s>",
                          iArrayRealCount, pstEntry->iCount,
                          tdr_get_entry_name(pstEntry), tdr_get_meta_name(pstCurMeta));
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            pstStackTop->iCode = pstEntry->iHOff;
            break;
        }
        if (0 == iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }


        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
        if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            pszHostStart = *(char **)pszHostStart;
            if (TDR_TYPE_COMPOSITE < pstEntry->iType)
            {
                pszHostEnd = pszHostStart + pstEntry->iNRealSize;
            }else
            {
                LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                pszHostEnd = pszHostStart + pstTypeMeta->iHUnitSize;
            }
        }else
        {
            pszHostEnd = pstStackTop->pszHostEnd;
        }


        if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
        {
            TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd,
                                  pstLib, pstEntry, iArrayRealCount);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                pstStackTop->iCode = pstEntry->iHOff;
                break;
            }
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*简单数据类型*/
        if (TDR_TYPE_COMPOSITE < pstEntry->iType)
        {
            TDR_UNPACK_SIMPLE_TYPE_ENTRY(iRet, pstEntry, iArrayRealCount,
                                         pszNetStart, pszNetEnd, pszHostStart, pszHostEnd);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                TDR_ERR_GET_LEN();
                TDR_SET_ERROR(TDR_ERR_BUF, "; entry<%s> of meta<%s> %s:%d",
                              tdr_get_entry_name(pstEntry), tdr_get_meta_name(pstCurMeta),
                              __FILE__, __LINE__);
                break;
            }
            if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
            {
                pstStackTop->iCode = pstEntry->iHOff + pstEntry->iHUnitSize;
            }else
            {
                pstStackTop->iCode = pszHostStart - pstStackTop->pszHostBase; }
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*复合数据类型*/
        if (TDR_STACK_SIZE <=  iStackItemCount)
        {
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
            pstStackTop->iCode = pstEntry->iHOff;
            break;
        }

        if (TDR_TYPE_UNION == pstEntry->iType)
        {
            TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, pstStackTop->iCutOffVersion, pstCurMeta, idxSubEntry);
            if(NULL == pstCurMeta)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }
        }else
        {
            pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
            idxSubEntry = 0;
        }

        /*递归进行结构体内部进行处理*/
        iStackItemCount++;
        pstStackTop++;
        pstStackTop->iCode = 0;
        TDR_PUSH_STACK_ITEM(pstStackTop, pstCurMeta, pstEntry, iArrayRealCount, idxSubEntry,
                            pszHostStart, pszNetStart);
        pstStackTop->pszHostEnd = pszHostEnd;
        pstStackTop->pstEntry = pstEntry;
        pstStackTop->iChange = 1;
        if (TDR_META_HAS_VERSION_INDICATOR(pstCurMeta))
        {
            TDR_GET_VERSION_INDICATOR_NET(pszNetStart, pszNetEnd, pstCurMeta,
                                          pstStackTop->iCutOffVersion, (pstStackTop-1)->iCutOffVersion);
        }else
        {
            pstStackTop->iCutOffVersion = (pstStackTop-1)->iCutOffVersion;
        }

        /* 到此处已经无法判断是传入的剪裁版本不正确，还是网络数据包的版本指示器的值不正确 */
        if ((pstCurMeta->iBaseVersion > pstStackTop->iCutOffVersion)
            || (pstStackTop->iCutOffVersion < pstCurMeta->iVersionIndicatorMinVer))
        {
            TDR_ERR_GET_LEN();
            TDR_SET_ERROR(TDR_ERR_BUF, "invalid cut-off version:"
                          " expect cut-off version<%d> NOT less than"
                          " max(base_ver<%d>, min_ver<%d>) about meta<%s>",
                          pstStackTop->iCutOffVersion, pstCurMeta->iBaseVersion,
                          a_pstMeta->iVersionIndicatorMinVer, tdr_get_meta_name(a_pstMeta));
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
            break;
        }

    }/*while (0 < iStackItemCount)*/


    a_pstHost->iBuff = (int)stStack[0].iCode;
    if (TDR_ERR_IS_ERROR(iRet))
    {
        for(iChange= 1; iChange < iStackItemCount; iChange++)
        {
            //统计编码字节数
            assert(NULL != stStack[iChange].pstEntry);
            if (TDR_ENTRY_IS_REFER_TYPE(stStack[iChange].pstEntry))
            {
                a_pstHost->iBuff += (int)stStack[iChange].pstEntry->iHUnitSize;
                break;
            }else
            {
                a_pstHost->iBuff += (int)stStack[iChange].iCode;
            }
        }
    }/*if (TDR_ERR_IS_ERROR(iRet))*/

    a_pstNet->iBuff = pszNetStart - a_pstNet->pszBuff;

    return iRet;
}

int tdr_pack_union_entry_i(IN LPTDRMETA a_pstMeta, IN int a_idxEntry, INOUT LPTDRDATA a_pstNetData, IN LPTDRDATA a_pstHostData, IN int a_iVersion)
{
    LPTDRMETAENTRY pstSelEntry;
    char *pch;
    size_t iLeftLen;
    int j;
    int iRet = TDR_SUCCESS;
    char *pszHostEnd;
    char *pszNetEnd;
    char *pszMetaBase;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstNetData);
    assert(NULL != a_pstNetData->pszBuff);
    assert(0 < a_pstNetData->iBuff);
    assert(NULL != a_pstHostData);
    assert(NULL != a_pstHostData->pszBuff);
    assert(0 < a_pstHostData->iBuff);
    assert(TDR_TYPE_UNION == a_pstMeta->iType);
    assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));


    pstSelEntry = a_pstMeta->stEntries + a_idxEntry;
    pch = a_pstNetData->pszBuff;
    iLeftLen = a_pstNetData->iBuff;
    pszNetEnd = a_pstNetData->pszBuff + a_pstNetData->iBuff;
    pszHostEnd = a_pstHostData->pszBuff + a_pstHostData->iBuff;
    pszMetaBase = a_pstHostData->pszBuff ;
    if (TDR_TYPE_STRUCT == pstSelEntry->iType)
    {
        TDRDATA stNetInfo;
        TDRDATA stHostInfo;
        LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
        LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstSelEntry->ptrMeta);
        for (j = 0; j < pstSelEntry->iCount; j++)
        {
            stNetInfo.iBuff = iLeftLen;
            stNetInfo.pszBuff = pch;
            stHostInfo.pszBuff = pszMetaBase + j * pstSelEntry->iHUnitSize;
            stHostInfo.iBuff = pszHostEnd - stHostInfo.pszBuff;
            iRet = tdr_hton(pstType, &stNetInfo, &stHostInfo, a_iVersion);
            if (0 != iRet)
            {
                break;
            }
            iLeftLen -= stNetInfo.iBuff;
            pch += stNetInfo.iBuff;
        }
    }else
    {
        TDR_PACK_SIMPLE_TYPE_ENTRY(iRet, pstSelEntry, pstSelEntry->iCount, pch, pszNetEnd, pszMetaBase, pszHostEnd);
    }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

    a_pstNetData->iBuff = pch - a_pstNetData->pszBuff;

    return iRet;
}


int tdr_unpack_union_entry_i(IN LPTDRMETA a_pstMeta, IN int a_idxEntry, IN LPTDRDATA a_pstHostData, INOUT LPTDRDATA a_pstNetData, IN int a_iVersion)
{
    LPTDRMETAENTRY pstSelEntry;
    char *pch;
    size_t iLeftLen;
    char *pszMetaBase;
    int j;
    int iRet = TDR_SUCCESS;
    char *pszHostEnd;
    char *pszNetEnd;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstNetData);
    assert(NULL != a_pstNetData->pszBuff);
    assert(0 < a_pstNetData->iBuff);
    assert(NULL != a_pstHostData);
    assert(NULL != a_pstHostData->pszBuff);
    assert(0 < a_pstHostData->iBuff);
    assert(TDR_TYPE_UNION == a_pstMeta->iType);
    assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));


    pstSelEntry = a_pstMeta->stEntries + a_idxEntry;
    pch = a_pstNetData->pszBuff;
    iLeftLen = a_pstNetData->iBuff;
    pszNetEnd = a_pstNetData->pszBuff + a_pstNetData->iBuff;
    pszHostEnd = a_pstHostData->pszBuff + a_pstHostData->iBuff;
    pszMetaBase = a_pstHostData->pszBuff ;
    if (TDR_TYPE_STRUCT == pstSelEntry->iType)
    {
        TDRDATA stNetInfo;
        TDRDATA stHostInfo;
        LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
        LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstSelEntry->ptrMeta);
        for (j = 0; j < pstSelEntry->iCount; j++)
        {
            stNetInfo.iBuff = iLeftLen;
            stNetInfo.pszBuff = pch;
            stHostInfo.pszBuff = pszMetaBase + j * pstSelEntry->iHUnitSize;;
            stHostInfo.iBuff = pszHostEnd - stHostInfo.pszBuff;
            iRet = tdr_ntoh(pstType, &stHostInfo, &stNetInfo, a_iVersion);
            if (0 != iRet)
            {
                break;
            }
            iLeftLen -= stNetInfo.iBuff;
            pch += stNetInfo.iBuff;
        }
    }else
    {
        TDR_UNPACK_SIMPLE_TYPE_ENTRY(iRet, pstSelEntry, pstSelEntry->iCount, pch, pszNetEnd, pszMetaBase, pszHostEnd);
    }
    a_pstNetData->iBuff = pch - a_pstNetData->pszBuff;

    return iRet;
}



int tdr_get_meta_max_net_size(IN LPTDRMETA a_pstMeta, IN int a_iVersion, OUT size_t* a_ptMaxNetSize)
{
    int iRet = TDR_SUCCESS;

    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;
    int iChange;
    LPTDRMETAENTRY pstEntry;
    int idxSubEntry;

    if ((NULL == a_pstMeta) || (NULL == a_ptMaxNetSize))
    {
        return TDR_ERRIMPLE_INVALID_PARAM;
    }

    if ((0 == a_iVersion) || (a_iVersion > a_pstMeta->iCurVersion))
    {
        a_iVersion = a_pstMeta->iCurVersion;
    }

    if ((a_pstMeta->iBaseVersion > a_iVersion)
        || (TDR_META_HAS_VERSION_INDICATOR(a_pstMeta) && (a_pstMeta->iVersionIndicatorMinVer > a_iVersion)))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }


    pstCurMeta = a_pstMeta;
    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstStackTop = &stStack[0];
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iCode = 0;// has processed entries's total max size
    if (0 < pstCurMeta->stSizeType.iUnitSize)
    {
        pstStackTop->iMetaSizeInfoUnit = pstCurMeta->stSizeType.iUnitSize;
    }else
    {
        pstStackTop->iMetaSizeInfoUnit = 0;
    }
    pstStackTop->iChange = 1;

    iStackItemCount = 1;
    pstStackTop->pstEntry = NULL;


    iChange = 0;
    while (0 < iStackItemCount)
    {
        if (0 != iChange)
        {
            iChange = 0;
            if (0 < pstStackTop->pstMeta->stSizeType.iUnitSize)
            {
                if( TDR_INVALID_INDEX != pstCurMeta->stSizeType.idxSizeType )
                {
                    pstStackTop->iCode += pstStackTop->pstMeta->stSizeType.iUnitSize;
                }

            }
            pstStackTop->iCode *= pstStackTop->iCount;
            if (1 < iStackItemCount)
            {
                assert(NULL != pstStackTop->pstEntry);
                if (TDR_TYPE_UNION == (pstStackTop-1)->pstMeta->iType)
                {
                    /* 对于 union 而言，iCode 记录了其所有entry空间的最大值 */
                    if (pstStackTop->iCode >= (pstStackTop-1)->iCode)
                    {
                        (pstStackTop-1)->iCode = pstStackTop->iCode;
                    }
                    pstStackTop->iCode = 0;

                }
                else
                {
                    (pstStackTop-1)->iCode += pstStackTop->iCode;
                    pstStackTop->iCode = 0;
                }

            }

            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_META_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;

        if (pstEntry->iVersion > a_iVersion)
        {
            TDR_META_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*指针不处理*/
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
        {
            TDR_META_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*简单数据类型*/
        if (TDR_TYPE_COMPOSITE < pstEntry->iType)
        {
            if (TDR_TYPE_STRUCT == pstCurMeta->iType)
            {
                if (0 < pstEntry->iCustomHUnitSize)
                {
                    pstStackTop->iCode += pstEntry->iCustomHUnitSize * pstEntry->iCount;
                }
                else
                {
                    pstStackTop->iCode += pstEntry->iHUnitSize * pstEntry->iCount;
                }
                if (TDR_TYPE_STRING == pstEntry->iType || TDR_TYPE_WSTRING == pstEntry->iType)
                {
                    if (0 < pstEntry->stSizeInfo.iUnitSize)
                    {
                        if( TDR_INVALID_INDEX != pstEntry->stSizeInfo.idxSizeType )
                        {
                            pstStackTop->iCode += pstEntry->stSizeInfo.iUnitSize * pstEntry->iCount;
                        }

                    }
                }
            }
            else if (TDR_TYPE_UNION == pstCurMeta->iType)
            {
                int iCurMaxUnionEntry = 0;
                if (0 < pstEntry->iCustomHUnitSize)
                {
                    iCurMaxUnionEntry += pstEntry->iCustomHUnitSize * pstEntry->iCount;
                }
                else
                {
                    iCurMaxUnionEntry += pstEntry->iHUnitSize * pstEntry->iCount;
                }
                if (TDR_TYPE_STRING == pstEntry->iType || TDR_TYPE_WSTRING == pstEntry->iType)
                {
                    if (0 < pstEntry->stSizeInfo.iUnitSize)
                    {
                        if( TDR_INVALID_INDEX != pstEntry->stSizeInfo.idxSizeType )
                        {
                            iCurMaxUnionEntry += pstEntry->stSizeInfo.iUnitSize * pstEntry->iCount;
                        }

                    }
                }

                if (iCurMaxUnionEntry >= pstStackTop->iCode)
                {
                    pstStackTop->iCode = iCurMaxUnionEntry;
                }
            }
            TDR_META_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*复合数据类型*/
        if (TDR_STACK_SIZE <=  iStackItemCount)
        {
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
            break;
        }

        pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
        idxSubEntry = 0;

        /*递归进行结构体内部进行处理*/
        iStackItemCount++;
        pstStackTop++;
        pstStackTop->pstMeta = pstCurMeta;
        pstStackTop->iCount = pstEntry->iCount;
        pstStackTop->idxEntry = idxSubEntry;
        pstStackTop->pstEntry = pstEntry;
        pstStackTop->iChange = 1;
        pstStackTop->iCode = 0;
    }

    *a_ptMaxNetSize = (size_t)stStack[0].iCode;

    return iRet;
}
