/**
*
* @file    tlogfile.h  
* @brief   ��־�ļ���¼ģ��
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

#ifndef TLOGFILE_H

#define TLOGFILE_H

#include <stdlib.h>
#include <stdio.h>
#include "tlogio.h"
#include "tlogfiledef.h"

#define TLOGFILE_PATH_LEN	(TLOGFILE_PATTERN_LEN + 64)
#define TLOGFILE_MAX_SIZE   4200000000UL


#ifdef __cplusplus
extern "C"
{
#endif


struct tagTLogDevFileInst
{
	FILE* fp;
	char* pszBuff;		/* if filebuff is set, here is file buffer pointer */
	time_t tLastMsg;
	int fDirty;
	time_t tLastSync;
	int iCurRotate;
	time_t tCreate;	                 /* the time for create file. */
	uint32_t iTail;
	int iRes;
	char szPath[TLOGFILE_PATH_LEN]; // ��ǰд���ļ���·��
};

typedef struct tagTLogDevFileInst	TLOGDEVFILEINST;
typedef struct tagTLogDevFileInst	*LPTLOGDEVFILEINST;

struct tagTLogFile
{
	TLOGDEVFILE* pstDev;
	TLOGDEVFILEINST stInst;
};

typedef struct tagTLogFile			TLOGFILE;
typedef struct tagTLogFile			*LPTLOGFILE;


/**
* д���ʼ��tlog�ļ��ṹ
* @param[in,out] a_pstLogFile tlog�ļ��ṹ
* @param[in] a_pstDev     �ļ�����
*
* @return 0 �ɹ�
*		��0 ʧ��
*/
int tlogfile_init(TLOGFILE* a_pstLogFile, TLOGDEVFILE* a_pstDev);


/**
* ���ļ���д������ƴ�
* @param[in] a_pstLogFile tlog�ļ��ṹ
* @param[in] a_pszBuff     �����ƴ�ָ��
* @param[in] a_iBuff     �����ƴ�����
*
* @return 0 �ɹ�
*		��0 ʧ��
*/
int tlogfile_write(TLOGFILE* a_pstLogFile, const char* a_pszBuff, int a_iBuff);

/**
* �ͷ��ļ��ṹ
* @param[in,out] a_pstLogFile tlog�ļ��ṹ
*
* @return 0 �ɹ�
*		��0 ʧ��
*/
int tlogfile_fini(TLOGFILE* a_pstLogFile);

/**
*��������������д���ļ�
* @param[in,out] a_pstLogFile tlog�ļ��ṹ
*
* @return 0 �ɹ�
*		��0 ʧ��
*/
int tlogfile_sync(TLOGFILE* a_pstLogFile);


/**
*�����������������д���ļ�
* @param[in,out] a_pstLogFile tlog�ļ��ṹ
* @param[in,out] a_pstIOVec ��Ҫд��������������ַ
* @param[in,out] a_iCount ��Ҫд�����������Ԫ�ظ���
* @return 0 �ɹ�
*		��0 ʧ��
*/
int tlogfile_writev(TLOGFILE* a_pstLogFile, const TLOGIOVEC* a_pstIOVec, int a_iCount);


// ���ļ������޸ĳ�bill������
int tlogfile_is_bill(TLOGFILE *a_pstLogFile);

#ifdef __cplusplus
}
#endif

#endif /* TLOGFILE_H */
