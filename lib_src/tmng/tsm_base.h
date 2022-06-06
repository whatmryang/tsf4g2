#ifndef _TSM_BASE_H_
#define _TSM_BASE_H_
//============================================================================
// @Id:       $Id: tsm_base.h 12726 2010-08-30 09:56:22Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-08-30 17:56:22 +0800 #$
// @Revision: $Revision: 12726 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include "pal/ttypes.h"

#include "tsingleton.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
#define DR_NAME_LEN             32
#define URI_MAX_LEN             1024

#define TSM_TDR_C_INDEX_KEY     0x18790314
#define TSM_TDR_C_INDEX_KEY_STR "410583828"

#define TSM_TDR_VERSION         1


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
typedef struct
{
    int    iIndexKey;

    int    iLibHash;
    int    iLibCap;
    int    iUriHash;
    int    iUriCap;

    int    iHashCap;

    size_t iOffsetLib;
    size_t iOffsetUri;
    size_t iOffsetHash;

    size_t iIndexSize;
} TsmBase;

TSINGLETON_H(TsmBase, I);


//----------------------------------------------------------------------------
//  系统中的两种 URI
//----------------------------------------------------------------------------
typedef enum
{
    TSM_URI_META = 1,
    TSM_URI_DATA = 2
} TsmUriType;

//----------------------------------------------------------------------------
//  保存节点位置的结构
//----------------------------------------------------------------------------
typedef struct
{
    int iType;
    int iRow;
    int iCol;
    int iIndex;
} TsmBaseVar;

//----------------------------------------------------------------------------
//  测试索引的类型
//      a_pstHash   哈希结点信息结构指针
//      a_iIndex    hash_find 或者 hash_next返回的索引
//  成功返回 0      同时填充结构体中其他信息
//  失败返回
//      -1          结构指针为空
//      -2          索引越界
//----------------------------------------------------------------------------
int tsm_base_index(TsmBaseVar *a_pstVar);


//----------------------------------------------------------------------------
// THE END
//============================================================================
#endif
