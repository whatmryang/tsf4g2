/**
*
* @file     tdr_iostream_i.h  
* @brief    TSF4G-IO�������ģ��
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-23 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_IOSTREAM_I_H
#define TDR_IOSTREAM_I_H

#include <stdio.h>

typedef enum tagIOStreamType
{
	TDR_IOSTREAM_UNKOWN = 0,
	TDR_IOSTREAM_STRBUF = 1,  /**<������IO��*/
	TDR_IOSTREAM_FILE = 2,		/**<�ļ���*/
}TDRIOSTREAMTYPE;

typedef struct  
{
	char *pszBuff;  /**<�������׵�ַ*/
	size_t iBuffLen;	/**<�������Ĵ�С*/
}TDRBUFFIOSTREAMPARAM;

typedef struct  
{
	FILE *a_fp;  /**<�ļ����*/	
}TDRFILEIOSTREAMPARAM;

struct  tagTDRIOStream
{
	TDRIOSTREAMTYPE emIOStreamType;
	union 
	{
		TDRBUFFIOSTREAMPARAM stBuffIOParam;
		TDRFILEIOSTREAMPARAM stFileIOParam;
	}stParam;
#define pszTDRIOBuff stParam.stBuffIOParam.pszBuff
#define iTDRIOBuffLen stParam.stBuffIOParam.iBuffLen
#define fpTDRIO		stParam.stFileIOParam.a_fp
};
typedef struct tagTDRIOStream TDRIOSTREAM;
typedef struct tagTDRIOStream *LPTDRIOSTREAM;

/**����Ϣ�����IO����
*@param[in] a_pstIOStream ������Ϣ��IO��
*@param[in]	a_pszFormat �����ʽ
*return �ɹ�����0�������ش������
*/
int tdr_iostream_write(LPTDRIOSTREAM a_pstIOStream, const char *a_pszFormat, ...);

#endif /*TDR_IOSTREAM_I_H*/
