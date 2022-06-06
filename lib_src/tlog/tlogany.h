/**
 *
 * @file    tlogany.h
 * @brief   ��־�豸��¼ģ��
 *
 * @author steve
 * @version 1.0
 * @date 2007-04-05
 *
 *
 * Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
 * All rights reserved.
 *
 */

#ifndef TLOGANY_H
#define TLOGANY_H

#include "tlog_i.h"
#include "tlogio.h"
#include "tlogmem.h"
#include "tlogfiledef.h"
#include "tlogmemdef.h"
#include "tlognetdef.h"
#include "tlogvecdef.h"
#include "tloganydef.h"

#ifdef __cplusplus
extern "C"
{
#endif

union tagTLogDevSelectorInst
{
    TLOGFILE stFile;
    TLOGNET stNet;
    TLOGVEC stVec;
    TLOGMEM stMem;
};

typedef union tagTLogDevSelectorInst TLOGDEVSELECTORINST;
typedef union tagTLogDevSelectorInst *LPTLOGDEVSELECTORINST;

struct tagTLogDevAnyInst
{
    int iType;
    TLOGDEVSELECTORINST stAny;

};

typedef struct tagTLogDevAnyInst TLOGDEVANYINST;
typedef struct tagTLogDevAnyInst *LPTLOGDEVANYINST;

struct tagTLogAny
{
    TLOGDEVANY* pstDev;
    //TLOGDEVANY stDev;

    //int iNeedReload;
    //TLOGDEVANY stTlogAnyNewConf;
    TLOGDEVANYINST stInst;
};

typedef struct tagTLogAny TLOGANY;
typedef struct tagTLogAny *LPTLOGANY;

/**
 * д���ʼ��tlog�ļ��ṹ
 * @param[in,out] a_pstLogAny tlogAny�ṹ
 * @param[in] a_pstDev     �ļ�����
 *
 * @return 0 �ɹ�
 *		��0 ʧ��
 */
int tlogany_init(TLOGANY* a_pstLogAny, TLOGDEVANY* a_pstDev);

/**
 * ���tlog�豸�ṹ
 * @param[in,out] a_pstLogAny tlogAny�ṹ
 *
 * @return 0 �ɹ�
 *		��0 ʧ��
 */
int tlogany_fini(TLOGANY* a_pstLogAny);

/**
 * ���tlog�豸�ṹ
 * @param[in,out] a_pstLogAny tlogAny�ṹ
 * @param[in] a_iID ��־��ID��ͨ��ΪUIN
 * @param[in] a_iCls ��־��CLS���û��Լ����壬���Թ���
 * @param[in] a_pszBuff ��־�ַ�������
 * @param[in] a_iBuff ��־�ַ������ݳ���
 *
 * @return 0 �ɹ�
 *		��0 ʧ��
 */
int tlogany_write(TLOGANY* a_pstLogAny, int a_iID, int a_iCls,
        const char* a_pszBuff, int a_iBuff);

/**
 * ���tlog�豸�ṹ
 * @param[in,out] a_pstLogAny tlogAny�ṹ
 * @param[in] a_iID ��־��ID��ͨ��ΪUIN
 * @param[in] a_iCls ��־��CLS���û��Լ����壬���Թ���
 * @param[in] a_pstIOVec ��־�ַ�������
 * @param[in] a_iCount ��־�ַ������鳤��
 *
 * @return 0 �ɹ�
 *		��0 ʧ��
 */
int tlogany_writev(TLOGANY* a_pstLogAny, int iId, int iCls,
        const TLOGIOVEC* a_pstIOVec, int a_iCount);

#ifdef __cplusplus
}
#endif

#endif /* TLOGANY_H */

