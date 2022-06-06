/**
*
* @file     tlogerr.c 
* @brief    tlog��������
* 
* @author kent
* @version 1.0
* @date 2007-11-28 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include "tdr/tdr_error.h"
#include "tlog/tlogerr.h"


static const char *s_errmsg[] = {
	"û�д���",
	"��־�ļ��򿪳���",
	"����������ʧ��",
	"forward ��ϵ��ѭ��",	
};

const char * tlog_error_string(int iErrorCode)
{
	const char *pchStr = NULL;

	if (!TERR_IS_ERROR(iErrorCode))
	{
		return "û�д���";
	}
	
	switch(TERR_GET_MOD(iErrorCode))
	{
	case TMODID_DR:
		pchStr = "tdr mod err";
		break;
	case TMODID_LOG:
		{
			int iErrno = TERR_GET_ERRNO(iErrorCode);
			iErrno -= TLOG_ERR_START_ERRNO;
			if (iErrno < 0 || iErrno >= TLOG_ERROR_COUNT)
			{
				pchStr =  "δ֪����";
			}
			else
			{
				pchStr = s_errmsg[iErrno];
			}
		};
		break;
	default:
		pchStr =  "δ֪����";
	}
	
	return pchStr;
}

