/**
*
* @file     tdr_net_i.h
* @brief    TDR元数据网络交换消息编解码内部使用模块
*
* @author steve jackyai
* @version 1.0
* @date 2007-12-26
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_NET_I_H
#define TDR_NET_I_H

#include "tdr/tdr_ctypes_info.h"
#include "tdr_os.h"
#include "tdr/tdr_net.h"
#include "tdr_metalib_kernel_i.h"


extern int64_t llNumericLimits[];



#define TDR_NTOHNS(d, s, size)			{			      \
    int i;												\
    for(i=0; i<size; i++)										\
    {															\
    *(uint16_t*)d	=	tdr_ntoh16(*(uint16_t*)s);   \
    d	+=	sizeof(uint16_t);			      \
    s	+=	sizeof(uint16_t);			      \
    }															\
}


#define TDR_HTONNS(d, s, size)			{			      \
    int i;								      \
    for(i=0; i<size; i++)						      \
    {																	\
    *(uint16_t*)d	=	tdr_hton16(*(uint16_t*)s);   \
    d	+=	sizeof(uint16_t);			      \
    s	+=	sizeof(uint16_t);		 	      \
    }								      \
    }

#define TDR_NTOHNL(d, s, size)			{			      \
    int i;								      \
    for(i=0; i<size; i++)						      \
    {								      \
    *(uint32_t*)d	=	tdr_ntoh32(*(uint32_t*)s);    \
    d	+=	sizeof(uint32_t);			      \
    s	+=	sizeof(uint32_t);			      \
    }								      \
    }

#define TDR_HTONNL(d, s, size)			{			      \
    int i;								      \
    for(i=0; i<size; i++)						      \
    {								      \
    *(uint32_t*)d	=	tdr_hton32(*(uint32_t*)s);    \
    d	+=	sizeof(uint32_t);			      \
    s	+=	sizeof(uint32_t);			      \
    }								      \
    }


#define TDR_NTOHNQ(d, s, size)			{			      \
    int i;								      \
    for(i=0; i<size; i++)						      \
    {								      \
    *(uint64_t*)d	=	tdr_ntoh64(*(uint64_t*)s);		      \
    d	+=	sizeof(uint64_t);			      \
    s	+=	sizeof(uint64_t);			      \
    }								      \
}

#define TDR_HTONNQ(d, s, size)			{			      \
    int i;								      \
    for(i=0; i<size; i++)						      \
    {								      \
    *(uint64_t*)d	=	tdr_hton64(*(uint64_t*)s);		      \
    d	+=	sizeof(uint64_t);			      \
    s	+=	sizeof(uint64_t);			      \
    }								      \
}




#define TDR_PACK_WSTRING_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pszNetStart, a_pszNetEnd, a_pszHostStart, a_pszHostEnd) \
{																													\
	int i;																				\
	ptrdiff_t iLen;																		\
	ptrdiff_t iSize;																	\
	for (i = 0; i < a_iCount; i++)														\
	{																					\
		if (0 == a_pstEntry->iCustomHUnitSize)											\
		{																				\
			iSize = a_pszHostEnd - a_pszHostStart;										\
		}else																			\
		{																				\
			iSize = (ptrdiff_t)a_pstEntry->iCustomHUnitSize;							\
		}																				\
		iLen = tdr_wcsnlen((wchar_t*)a_pszHostStart, iSize) + 1;						\
		iLen *= sizeof(tdr_wchar_t);													\
		if (iLen > iSize) /* the string has no space for null. */						\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "wstring length error:"                          \
                          " real_len<%d> available_len<%d>"                             \
                          " net_start<%p> net_end<%p> custom_size<%u>",                 \
                          (int)iLen, (int)iSize, a_pszNetStart, a_pszNetEnd,                      \
                          (unsigned)a_pstEntry->iCustomHUnitSize);    \
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);			\
			break;																		\
		}																				\
		if ((a_pszNetEnd - a_pszNetStart ) < (ptrdiff_t)( iLen + a_pstEntry->stSizeInfo.iUnitSize) )		\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"                         \
                          " real_len<%d> sizeinfo_size<%u>"                             \
                          " net_start<%p> net_end<%p>",                                 \
                          (int)iLen, (unsigned)a_pstEntry->stSizeInfo.iUnitSize,                       \
                          a_pszNetStart, a_pszNetEnd);                                  \
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);				\
			break;																		\
		}                                                                               \
        if (iLen > llNumericLimits[a_pstEntry->stSizeInfo.iUnitSize - 1])               \
        {                                                                               \
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "sizeinfo numeric overflow:"                     \
                          " real_value<%d> value_limit<%u>", (int)iLen,                 \
                          (unsigned)llNumericLimits[a_pstEntry->stSizeInfo.iUnitSize-1]);\
            a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_SIZEINFO_OVERFLOW);              \
            break;                                                                      \
        }                                                                               \
		TDR_SET_INT_NET(a_pszNetStart, a_pstEntry->stSizeInfo.iUnitSize, iLen);			\
		a_pszNetStart	+=	a_pstEntry->stSizeInfo.iUnitSize;							\
		TDR_MEMCPY(a_pszNetStart, a_pszHostStart, (size_t)iLen, TDR_MIN_COPY);						\
		a_pszHostStart += (iSize-iLen);													\
	}																					\
}



#define TDR_PACK_STRING_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pszNetStart, a_pszNetEnd, a_pszHostStart, a_pszHostEnd)\
{																													\
	int i;																				\
	ptrdiff_t iLen;																		\
	ptrdiff_t iSize;																	\
	for (i = 0; i < a_iCount; i++)														\
	{																					\
		if (0 == a_pstEntry->iCustomHUnitSize)											\
		{																				\
			iSize = a_pszHostEnd - a_pszHostStart;										\
		}else																			\
		{																				\
			iSize = (ptrdiff_t)a_pstEntry->iCustomHUnitSize;							\
		}																				\
		iLen = tdr_strnlen(a_pszHostStart, iSize);										\
		if (iLen >= iSize)																\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "string length error:"                           \
                          " real_len<%d> available_len<%d>"                             \
                          " net_start<%p> net_end<%p> custom_size<%u>",                 \
                          (int)iLen, (int)iSize, a_pszNetStart, a_pszNetEnd,            \
                          (unsigned)a_pstEntry->iCustomHUnitSize);                      \
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);			\
			break;																		\
		}																				\
		iLen++;																			\
		if ((a_pszNetEnd - a_pszNetStart) < (ptrdiff_t)(iLen + a_pstEntry->stSizeInfo.iUnitSize) )	\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"                         \
                          " real_len<%d> sizeinfo_size<%u>"                             \
                          " net_start<%p> net_end<%p>",                                 \
                          (int)iLen, (unsigned)a_pstEntry->stSizeInfo.iUnitSize,        \
                          a_pszNetStart, a_pszNetEnd);                                  \
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);				\
			break;																		\
		}                                                                               \
        if (iLen > llNumericLimits[a_pstEntry->stSizeInfo.iUnitSize - 1])               \
        {                                                                               \
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "sizeinfo numeric overflow:"                     \
                          " real_value<%d> value_limit<%u>", (int)iLen,                 \
                          (unsigned)llNumericLimits[a_pstEntry->stSizeInfo.iUnitSize-1]);\
            a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_SIZEINFO_OVERFLOW);              \
            break;                                                                      \
        }                                                                               \
		TDR_SET_INT_NET(a_pszNetStart, a_pstEntry->stSizeInfo.iUnitSize, iLen);			\
		a_pszNetStart	+=	a_pstEntry->stSizeInfo.iUnitSize;						    \
		memcpy(a_pszNetStart, a_pszHostStart, iLen);							  	    \
		a_pszNetStart += iLen;															\
		a_pszHostStart += iSize;														\
	}																					\
}

#define TDR_PACK_INTEGER_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pszNetStart, a_pszNetEnd, a_pszHostStart, a_pszHostEnd)\
{																						\
	if((a_pszHostStart + a_pstEntry->iHUnitSize * a_iCount) > a_pszHostEnd)				\
	{																					\
        TDR_ERR_GET_LEN();                                                              \
        TDR_SET_ERROR(TDR_ERR_BUF, "host-buffer NOT enough:"                            \
                      " host_start<%p> host_end<%p> unit_size<%u> count<%d>",           \
                      a_pszHostStart, a_pszHostEnd, (unsigned)a_pstEntry->iHUnitSize, a_iCount);  \
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);					\
		break;																			\
	}																					\
	if ((a_pszNetStart + a_pstEntry->iNUnitSize * a_iCount)>a_pszNetEnd )				\
	{																					\
        TDR_ERR_GET_LEN();                                                              \
        TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"                             \
                      " net_start<%p> net_end<%p> unit_size<%u> count<%d>",             \
                      a_pszNetStart, a_pszNetEnd, (unsigned)a_pstEntry->iNUnitSize, a_iCount);    \
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);					\
		break;																			\
	}																				\
	switch( a_pstEntry->iNUnitSize )													\
	{																				\
	case 4:																			\
		TDR_HTONNL(a_pszNetStart, a_pszHostStart, a_iCount);						\
		break;																		\
	case 2:																			\
		TDR_HTONNS(a_pszNetStart, a_pszHostStart, a_iCount);						\
		break;																		\
	case 1:																			\
		TDR_MEMCPY(a_pszNetStart, a_pszHostStart, a_iCount, TDR_MIN_COPY);			\
		break;																		\
	default:	/* must be 8 bytes. */												\
		TDR_HTONNQ(a_pszNetStart, a_pszHostStart, a_iCount);						\
	break;																			\
	}																				\
}



#define TDR_UNPACK_WSTRING_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pszNetStart, a_pszNetEnd, a_pszHostStart, a_pszHostEnd) \
{																						\
	int i;																				\
	int64_t iLen;																		\
	size_t iSize;																		\
	for (i = 0; i < a_iCount; i++)														\
	{																					\
		if (0 == a_pstEntry->iCustomHUnitSize)											\
		{																				\
			iSize = a_pszHostEnd - a_pszHostStart;										\
		}else																			\
		{																				\
			iSize = a_pstEntry->iCustomHUnitSize;										\
		}																				\
		if( a_pszNetStart + a_pstEntry->stSizeInfo.iUnitSize >= pszNetEnd )				\
		{	/* net buffer too small. */													\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"                         \
                          " expect end<%p> > (start<%p> + sizeinfo<%u>)",               \
                          a_pszNetEnd, a_pszNetStart, (unsigned)a_pstEntry->stSizeInfo.iUnitSize);\
			a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);			\
			break;																		\
		}																				\
		TDR_GET_INT_NET(iLen, a_pstEntry->stSizeInfo.iUnitSize, a_pszNetStart);			\
		if (iLen < 2)	/* wstring length can not be less than 1. */					\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer wstring length error:"               \
                          " expect length<%d> >= min_wstring_len<2>"                    \
                          ", start<%p>, sizeinfo<%u>)",                                 \
                          (int)iLen, a_pszNetStart, (unsigned)a_pstEntry->stSizeInfo.iUnitSize);\
			a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);			\
			break;																		\
		}																				\
		if( a_pszNetEnd - a_pszNetStart < (ptrdiff_t)(iLen + a_pstEntry->stSizeInfo.iUnitSize))\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"                         \
                          " expect (end<%p> - start<%p>) >="                            \
                          " (length<%d> + sizeinfo<%u>)",                               \
                          a_pszNetEnd, a_pszNetStart, (int)iLen, (unsigned)a_pstEntry->stSizeInfo.iUnitSize);\
			a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);			\
			break;																		\
		}																				\
		if (iLen > iSize) /* the wstring has no space for null. */						\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "has NO sapce for wstring's null:"               \
                          " length<%d> buffer_size<%d>"                                 \
                          ", start<%p> end<%p> custom_size<%u>)",                       \
                          (int)iLen, (int)iSize, a_pszNetStart, a_pszNetEnd,            \
                          (unsigned)a_pstEntry->stSizeInfo.iUnitSize);                  \
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);			\
			break;																		\
		}																				\
		if(L'\0' != *(tdr_wchar_t *)&a_pszNetStart[a_pstEntry->stSizeInfo.iUnitSize+iLen-2] )		\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "wstring from net-buffer format error:"          \
                          " expect last_char_of_wstring<%hu> is null_wchar<0>"          \
                          ", net_start<%p> length<%d> sizeinfo<%u>",                    \
                          *(tdr_wchar_t*)&a_pszNetStart[a_pstEntry->stSizeInfo.iUnitSize+iLen-2],\
                          a_pszNetStart, (int)iLen, (unsigned)a_pstEntry->stSizeInfo.iUnitSize);\
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);			\
			break;																		\
		}																				\
		a_pszNetStart +=	a_pstEntry->stSizeInfo.iUnitSize;							\
		TDR_MEMCPY(a_pszHostStart, a_pszNetStart, (size_t)iLen, TDR_MIN_COPY);			\
		a_pszHostStart += (iSize-iLen);													\
	}/*for (i = 0; i < a_iCount; i++)*/													\
}

#define TDR_UNPACK_INTEGER_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pszNetStart, a_pszNetEnd, a_pszHostStart, a_pszHostEnd) \
{																													\
	if((a_pszHostStart + a_pstEntry->iHUnitSize * a_iCount) > a_pszHostEnd)						\
	{																					\
        TDR_ERR_GET_LEN();                                                              \
        TDR_SET_ERROR(TDR_ERR_BUF, "host-buffer NOT enough:"                            \
                      " host_start<%p> host_end<%p> unit_size<%u> count<%d>",           \
                      a_pszHostStart, a_pszHostEnd, (unsigned)a_pstEntry->iHUnitSize, a_iCount);  \
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);					\
		break;																			\
	}																					\
	if ((a_pszNetStart + a_pstEntry->iNUnitSize * a_iCount)>a_pszNetEnd )				\
	{																					\
        TDR_ERR_GET_LEN();                                                              \
        TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"                             \
                      " net_start<%p> net_end<%p> unit_size<%u> count<%d>",             \
                      a_pszNetStart, a_pszNetEnd, (unsigned)a_pstEntry->iNUnitSize, a_iCount);    \
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);					\
		break;																			\
	}																					\
	switch( a_pstEntry->iNUnitSize )													\
	{																				\
	case 4:																			\
		TDR_NTOHNL(a_pszHostStart, a_pszNetStart, a_iCount);						\
	break;																		\
	case 2:																			\
		TDR_NTOHNS(a_pszHostStart, a_pszNetStart, a_iCount);						\
		break;																		\
	case 1:																			\
		TDR_MEMCPY(a_pszHostStart, a_pszNetStart, a_iCount, TDR_MIN_COPY);			\
		break;																		\
	default:	/* must be 8 bytes. */												\
		TDR_NTOHNQ(a_pszHostStart, a_pszNetStart, a_iCount);						\
		break;																		\
	}/*switch( a_pstEntry->iNUnitSize )*/										\
}

#define TDR_COPY_COMPOSIZE_ENTRY_DATA(a_iRet, a_pstEntry, a_iCount, a_pszDstHostStart, a_pszDstHostEnd, a_pszSrcHostStart, a_pszSrcHostEnd); \
{																																	\
	int i;																															\
	int iSize;																														\
	if (0 == a_pstEntry->iCustomHUnitSize)											\
	{																				\
		iSize = a_pszSrcHostEnd - a_pszSrcHostStart;										\
	}else																			\
	{																				\
		iSize = a_pstEntry->iCustomHUnitSize;										\
	}																				\
	if((a_pszSrcHostStart + iSize * a_iCount) > a_pszSrcHostEnd)						\
	{																					\
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);					\
		break;																			\
	}																					\
	if ((a_pszDstHostStart + iSize * a_iCount)>a_pszDstHostEnd )				\
	{																					\
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);					\
		break;																			\
	}																					\
	for (i =0; i < a_iCount; i++)														\
	{																					\
		TDR_MEMCPY(a_pszDstHostStart, a_pszSrcHostStart, iSize, TDR_MIN_COPY);			\
	}																					\
}

#define TDR_COPY_STRING_ENTRY_DATA(a_iRet, a_pstEntry, a_iCount, a_pszDstHostStart, a_pszDstHostEnd, a_pszSrcHostStart, a_pszSrcHostEnd);\
{																																	\
	int i;																															\
	size_t iSize;																														\
	if (0 == a_pstEntry->iCustomHUnitSize)											\
	{																				\
		iSize = a_pszSrcHostEnd - a_pszSrcHostStart;										\
	}else																			\
	{																				\
		iSize = a_pstEntry->iCustomHUnitSize;										\
	}																				\
	if((a_pszSrcHostStart + iSize * a_iCount) > a_pszSrcHostEnd)						\
	{																					\
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);					\
		break;																			\
	}																					\
	if ((a_pszDstHostStart + iSize * a_iCount)>a_pszDstHostEnd )				\
	{																					\
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);					\
		break;																			\
	}																					\
	for (i =0; i < a_iCount; i++)														\
	{																					\
		TDR_MEMCPY(a_pszDstHostStart, a_pszSrcHostStart, (size_t)iSize, TDR_MIN_COPY);			\
	}																					\
}

#define TDR_COPY_INTEGER_ENTRY_DATA(a_iRet, a_pstEntry, a_iCount, a_pszDstHostStart, a_pszDstHostEnd, a_pszSrcHostStart, a_pszSrcHostEnd);\
{																														\
	int i;																												\
	if((a_pszSrcHostStart + a_pstEntry->iHUnitSize * a_iCount) > a_pszSrcHostEnd)						\
	{																					\
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);					\
		break;																			\
	}																					\
	if ((a_pszDstHostStart + a_pstEntry->iNUnitSize * a_iCount)>a_pszDstHostEnd )				\
	{																					\
		a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);					\
		break;																			\
	}																					\
	switch( a_pstEntry->iNUnitSize )													\
	{																				\
	case 4:																			\
		for(i=0; i<a_iCount; i++)						      \
		{												 \
			*(uint32_t*)a_pszDstHostStart	=	*(uint32_t*)a_pszSrcHostStart;    \
			a_pszDstHostStart	+=	sizeof(uint32_t);			      \
			a_pszSrcHostStart	+=	sizeof(uint32_t);			      \
		}												 \
		break;																		\
	case 2:																			\
		for(i=0; i<a_iCount; i++)						      \
		{												 \
			*(uint16_t*)a_pszDstHostStart	=	*(uint16_t*)a_pszSrcHostStart;    \
			a_pszDstHostStart	+=	sizeof(uint16_t);			      \
			a_pszSrcHostStart	+=	sizeof(uint16_t);			      \
		}												 \
		break;																		\
	case 1:																			\
		TDR_MEMCPY(a_pszDstHostStart, a_pszSrcHostStart, (size_t)a_iCount, TDR_MIN_COPY);			\
		break;																		\
	default:	/* must be 8 bytes. */												\
		for(i=0; i<a_iCount; i++)						      \
		{												 \
			*(uint64_t*)a_pszDstHostStart	=	*(uint64_t*)a_pszSrcHostStart;    \
			a_pszDstHostStart	+=	sizeof(uint64_t);			      \
			a_pszSrcHostStart	+=	sizeof(uint64_t);			      \
		}												 \
		break;																		\
	}/*switch( a_pstEntry->iNUnitSize )*/										\
}

#define TDR_COPY_ENTRY_DATA(a_iRet, a_pstEntry, a_iCount, a_pszDstHostStart, a_pszDstHostEnd, a_pszSrcHostStart, a_pszSrcHostEnd) \
{																														\
	switch(a_pstEntry->iType)																							\
	{																													\
	case TDR_TYPE_UNION:																								\
	case TDR_TYPE_STRUCT:																								\
		TDR_COPY_COMPOSIZE_ENTRY_DATA(a_iRet, a_pstEntry, a_iCount, a_pszDstHostStart, a_pszDstHostEnd, a_pszSrcHostStart, a_pszSrcHostEnd); \
		break;																											\
	case TDR_TYPE_STRING:																							\
	case TDR_TYPE_WSTRING:																							\
		TDR_COPY_STRING_ENTRY_DATA(a_iRet, a_pstEntry, a_iCount, a_pszDstHostStart, a_pszDstHostEnd, a_pszSrcHostStart, a_pszSrcHostEnd);\
		break;																										\
	default:																										\
		TDR_COPY_INTEGER_ENTRY_DATA(a_iRet, a_pstEntry, a_iCount, a_pszDstHostStart, a_pszDstHostEnd, a_pszSrcHostStart, a_pszSrcHostEnd);\
		break;																										\
	}																												\
}


#define TDR_UNPACK_STRING_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pszNetStart, a_pszNetEnd, a_pszHostStart, a_pszHostEnd) \
{																														\
	int i;																				\
	int64_t iLen = 0;																	\
	ptrdiff_t iSize;																	\
	for (i = 0; i < a_iCount; i++)														\
	{																					\
		if (0 == a_pstEntry->iCustomHUnitSize)											\
		{																				\
			iSize = a_pszHostEnd - a_pszHostStart;										\
		}else																			\
		{																				\
			iSize = (ptrdiff_t)a_pstEntry->iCustomHUnitSize;										\
		}																					\
		if( a_pszNetStart + a_pstEntry->stSizeInfo.iUnitSize >= a_pszNetEnd )			\
		{	/* net buffer too small. */													\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"                         \
                          " expect end<%p> > (start<%p> + sizeinfo<%u>)",                \
                          a_pszNetEnd, a_pszNetStart, (unsigned)a_pstEntry->stSizeInfo.iUnitSize);\
			a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);			\
			break;																		\
		}																				\
		TDR_GET_INT_NET(iLen, a_pstEntry->stSizeInfo.iUnitSize, a_pszNetStart);				\
		if (iLen < 1)	/* string length can not be less than 1. */						\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer string length error:"                         \
                          " expect length<%d> >= min_string_len<1>"                                    \
                          ", start<%p>, sizeinfo<%u>)",                                   \
                          (int)iLen, a_pszNetStart, (unsigned)a_pstEntry->stSizeInfo.iUnitSize);       \
			a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);			\
			break;																		\
		}																				\
		if( (a_pszNetEnd - a_pszNetStart )< (ptrdiff_t)(iLen + a_pstEntry->stSizeInfo.iUnitSize)  )		\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "net-buffer NOT enough:"                         \
                          " expect (end<%p> - start<%p>) >="                            \
                          " (length<%d> + sizeinfo<%u>)",                               \
                          a_pszNetEnd, a_pszNetStart, (int)iLen, (unsigned)a_pstEntry->stSizeInfo.iUnitSize);\
			a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);			\
			break;																		\
		}																				\
		if (iLen > iSize) /* the string has no space for null. */						\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "has NO sapce for string's null:"                \
                          " length<%d> buffer_size<%d>"                                 \
                          ", start<%p> end<%p> custom_size<%u>)",                       \
                          (int)iLen, (int)iSize, a_pszNetStart, a_pszNetEnd,                      \
                          (unsigned)a_pstEntry->stSizeInfo.iUnitSize);                            \
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);			\
			break;																		\
		}																				\
		if('\0' != a_pszNetStart[a_pstEntry->stSizeInfo.iUnitSize+iLen-1] )				\
		{																				\
            TDR_ERR_GET_LEN();                                                          \
            TDR_SET_ERROR(TDR_ERR_BUF, "string from net-buffer format error:"           \
                          " expect last_char_of_string<%d> is null_char<0>"             \
                          ", net_start<%p> length<%d> sizeinfo<%u>",                    \
                          (int)a_pszNetStart[a_pstEntry->stSizeInfo.iUnitSize+iLen-1],  \
                          a_pszNetStart, (int)iLen, (unsigned)a_pstEntry->stSizeInfo.iUnitSize);       \
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);			\
			break;																		\
		}																				\
		a_pszNetStart +=	a_pstEntry->stSizeInfo.iUnitSize;								\
		TDR_MEMCPY(a_pszHostStart, a_pszNetStart, (size_t)iLen, TDR_MIN_COPY);									\
		a_pszHostStart += (iSize - iLen);														\
	}/*for (i = 0; i < iArrayRealCount; i++)*/											\
}


#define TDR_PACK_SIMPLE_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pstNetStart, a_pstNetEnd, a_pstHostStart, a_pstHostEnd) \
{																										\
	switch(a_pstEntry->iType)																			\
	{																									\
	case TDR_TYPE_STRING:																				\
		TDR_PACK_STRING_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pstNetStart, a_pstNetEnd, a_pstHostStart, a_pstHostEnd);	\
		break; 																	\
	case TDR_TYPE_WSTRING:																					\
		TDR_PACK_WSTRING_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pstNetStart, a_pstNetEnd, a_pstHostStart, a_pstHostEnd);	\
		break;																\
	default:																							\
		{																								\
			TDR_PACK_INTEGER_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pstNetStart, a_pstNetEnd, a_pstHostStart, a_pstHostEnd);	\
		}																								\
		break; /*default:*/																				\
	}/*switch(pstEntry->iType)*/																		\
}

#define TDR_UNPACK_SIMPLE_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pstNetStart, a_pstNetEnd, a_pstHostStart, a_pstHostEnd) \
{																										\
	switch(a_pstEntry->iType)																			\
	{																									\
	case TDR_TYPE_STRING:																				\
		TDR_UNPACK_STRING_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pstNetStart, a_pstNetEnd, a_pstHostStart, a_pstHostEnd);	\
		break; /*case TDR_TYPE_STRING:*/																	\
	case TDR_TYPE_WSTRING:																					\
		TDR_UNPACK_WSTRING_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pstNetStart, a_pstNetEnd, a_pstHostStart, a_pstHostEnd);	\
		break; /*case TDR_TYPE_WSTRING:*/																\
	default:																							\
		TDR_UNPACK_INTEGER_TYPE_ENTRY(a_iRet, a_pstEntry, a_iCount, a_pstNetStart, a_pstNetEnd, a_pstHostStart, a_pstHostEnd);	\
		break; /*default:*/																				\
	}/*switch(pstEntry->iType)*/																		\
}

/**打包一个union数据成员
*/
#ifdef __cplusplus
extern "C"
{
#endif
     int tdr_pack_union_entry_i(IN LPTDRMETA a_pstMeta, IN int a_idxEntry, INOUT LPTDRDATA a_pstNetData, IN LPTDRDATA a_pstHostData, IN int a_iVersion);

/**解包一个union数据成员
*/
int tdr_unpack_union_entry_i(IN LPTDRMETA a_pstMeta, IN int a_idxEntry, INOUT LPTDRDATA a_pstHostData, INOUT LPTDRDATA a_pstNetData,  IN int a_iVersion);

#ifdef __cplusplus
}
#endif
#endif /*TDR_NET_I_H*/

