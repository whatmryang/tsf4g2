#ifndef _TSM_INFO_H_
#define _TSM_INFO_H_
//============================================================================
// @Id:       $Id: tsm_info.h 10522 2010-07-20 02:22:08Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-07-20 10:22:08 +0800 #$
// @Revision: $Revision: 10522 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include "tsm_base.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
typedef struct
{
    int       iBid;                     // 业务 id
    size_t    iSize;                    // 数据尺寸
    char      szLib[DR_NAME_LEN];       // 库名
    int       iLibVer;                  // 库版本号
    int       iLibBuildVer;             // 库建立版本号
    char      szMeta[DR_NAME_LEN];      // Meta名
    int       iMetaVer;                 // Meta版本号
    char      szOther[360];             // 其他参数
} TsmTdrMetaInfo;

#define tsm_tdr_meta_is_null(P) ( \
    0 == (P)->iBid && \
    0 == (P)->iSize && \
    0 == (P)->szLib[0] && \
    0 == (P)->iLibVer && \
    0 == (P)->iLibVer && \
    0 == (P)->szMeta[0] && \
    0 == (P)->iMetaVer && \
    0 == (P)->szOther[0])


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
typedef struct
{
    int       iBid;                     // 业务 id
    size_t    iSize;                    // 数据尺寸
    char      szProgName[DR_NAME_LEN];  // 程序名
    char      szProgId[DR_NAME_LEN];    // 程序ID
    char      szFuncName[DR_NAME_LEN];  // 功能名
    char      szValueName[DR_NAME_LEN]; // 另一个用来区分数据 Uri的名字
    char      szValuePath[360];         // 变量路径
} TsmTdrDataInfo;

#define tsm_tdr_data_is_null(P) ( \
    0 == (P)->iBid && \
    0 == (P)->iSize && \
    0 == (P)->szProgName[0] && \
    0 == (P)->szProgId[0] && \
    0 == (P)->szFuncName[0] && \
    0 == (P)->szValueName[0] && \
    0 == (P)->szValuePath[0])


//----------------------------------------------------------------------------
// THE END
//============================================================================
#endif
