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
    int       iBid;                     // ҵ�� id
    size_t    iSize;                    // ���ݳߴ�
    char      szLib[DR_NAME_LEN];       // ����
    int       iLibVer;                  // ��汾��
    int       iLibBuildVer;             // �⽨���汾��
    char      szMeta[DR_NAME_LEN];      // Meta��
    int       iMetaVer;                 // Meta�汾��
    char      szOther[360];             // ��������
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
    int       iBid;                     // ҵ�� id
    size_t    iSize;                    // ���ݳߴ�
    char      szProgName[DR_NAME_LEN];  // ������
    char      szProgId[DR_NAME_LEN];    // ����ID
    char      szFuncName[DR_NAME_LEN];  // ������
    char      szValueName[DR_NAME_LEN]; // ��һ�������������� Uri������
    char      szValuePath[360];         // ����·��
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
