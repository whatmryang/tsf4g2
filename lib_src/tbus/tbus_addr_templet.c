#include <assert.h>
#include <string.h>
#include <errno.h>
#include "pal/pal.h"
#include "tbus/tbus.h"
#include "tbus/tbus_macros.h"
#include "tbus_addr_templet.h"
#include "tbus_log.h"
#include "tbus/tbus_error.h"
#include "tbus_desc.h"
#include "tbus_misc.h"

/** 从通信地址模板串某一节的字符串中分析出此节存储所需的位信息
*/

int tbus_add_addrtemplet_segment(OUT LPTBUSADDRTEMPLET a_pstAddrTemplet, IN char *a_pszSeg);



char *tbus_parse_addr_by_str(IN const char *a_pszAddStr);
int tbus_parse_addrinstance(OUT LPTBUSADDRINSTANCE a_pstAddrInst, IN const char *a_pszAddStr);
int tbus_parse_addrmapping(IN LPTBUSADDRINSTANCE a_pstAddrInst1, IN LPTBUSADDRINSTANCE a_pstAddrInst2);

char *tbus_parse_addr_by_str(IN const char *a_pszAddStr)
{


	static char szAddrBuff[TBUS_MAX_ADDR_STRING_LEN];

    const char* pszTmp = a_pszAddStr;

    int iTmp = 0;
    int iCorrectSeg = 0;

	assert(NULL != a_pszAddStr);

	while(pszTmp != NULL)
	{
        if(*pszTmp == '.')
        {
            if(iCorrectSeg)
            {
                szAddrBuff[iTmp] = '.';
                iTmp++;
            }
            else
            {
                szAddrBuff[iTmp] = '0';
                iTmp++;
                szAddrBuff[iTmp] = '.';
                iTmp++;
            }

            iCorrectSeg = 0;
        }
        else if(isalnum(*pszTmp))
        {
            iCorrectSeg = 1;
            szAddrBuff[iTmp] = *pszTmp;
            iTmp++;
        }

        pszTmp++;
    }

    if(szAddrBuff[iTmp] == '.')
    {
        iTmp++;
        szAddrBuff[iTmp] = '0';
        iTmp++;
    }

    szAddrBuff[iTmp] = '\0';

	return &szAddrBuff[0];
}


int tbus_parse_addrinstance(OUT LPTBUSADDRINSTANCE a_pstAddrInst, IN const char *a_pszAddStr)
{
    const char* pszTmp = a_pszAddStr;

    int iTmp = -1;

	assert(NULL != a_pstAddrInst);
	assert(NULL != a_pszAddStr);
	a_pstAddrInst->iSegNum = 0;

	while(pszTmp != NULL)
	{
        if(*pszTmp == '.')
        {
            if(iTmp >= 0)
            {
               a_pstAddrInst->astSegRange[a_pstAddrInst->iSegNum].iBegin = iTmp;
			   a_pstAddrInst->astSegRange[a_pstAddrInst->iSegNum].iEnd = iTmp;
            }
            else
            {
               //这里可能需要根据模板来限制范围，暂时用一个大数
               a_pstAddrInst->astSegRange[a_pstAddrInst->iSegNum].iBegin = 0;
			   a_pstAddrInst->astSegRange[a_pstAddrInst->iSegNum].iEnd = 1000000;
            }
            a_pstAddrInst->iSegNum++;

            iTmp = -1;
        }
        else if(isalnum(*pszTmp))
        {
            if(iTmp == -1)
            {
                iTmp = (*pszTmp - '0');
            }
            else
            {
                iTmp = iTmp*10 + (*pszTmp - '0');
            }
        }

        pszTmp++;
    }

	return 0;

}

int tbus_parse_addrmapping(IN LPTBUSADDRINSTANCE a_pstAddrInst1, IN LPTBUSADDRINSTANCE a_pstAddrInst2)
{
	int i = 0;
    if(a_pstAddrInst1->iSegNum != a_pstAddrInst2->iSegNum)
    {
        return 1;
    }


    for(i = 0; i < a_pstAddrInst1->iSegNum; i++)
    {
        if(a_pstAddrInst1->astSegRange[i].iBegin < a_pstAddrInst2->astSegRange[i].iBegin
           || a_pstAddrInst1->astSegRange[i].iEnd > a_pstAddrInst2->astSegRange[i].iEnd)
        {
            return 1;
        }
    }

    return 0;
}



int tbus_parse_addrtemplet(OUT LPTBUSADDRTEMPLET a_pstAddrTemplet, IN const char *a_pszAddTemplet)
{
	char *pchDot = NULL;
	char *pszAddrTempletDup = NULL;
	int iRet ;
	int iBits;
	int i;
	char szAddrTemplet[TBUS_MAX_ADDR_TEMPLET_STRING_LEN];

	assert(NULL != a_pstAddrTemplet);
	assert(NULL != a_pszAddTemplet);

	a_pstAddrTemplet->iSegNum = 0;
	STRNCPY(szAddrTemplet, a_pszAddTemplet, sizeof(szAddrTemplet));
	tbus_log(TLOG_PRIORITY_DEBUG,"AddrTemplet: %s", szAddrTemplet);
	pszAddrTempletDup = &szAddrTemplet[0];


	/*从模板串向前解析点分法的每一节的信息*/
	do
	{
		char *pszSeg = NULL;
		pchDot = strrchr(pszAddrTempletDup, TBUS_SPLIT_SEGMENT_CHARACTER);
		if (NULL != pchDot)
		{
			pszSeg = pchDot + 1;
			*pchDot = '\0';
		}else
		{
			pszSeg = pszAddrTempletDup;
		}
		iRet = tbus_add_addrtemplet_segment(a_pstAddrTemplet, pszSeg);
		if (TBUS_ERR_IS_ERROR(iRet))
		{
			break;
		}
	} while(NULL != pchDot);


	iBits = 0;
	for (i = 0; i< a_pstAddrTemplet->iSegNum; i++)
	{
		iBits += a_pstAddrTemplet->astSeg[i].bBitNum;
	}
	if ((0 >= iBits)|| (iBits > TBUS_ADDR_TEMPLET_MAX_BITS_NUM))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid address templet %s, total bits num must be in the scope[1,%d]",
			a_pszAddTemplet, TBUS_ADDR_TEMPLET_MAX_BITS_NUM);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PARSE_ADDR_TEMPLET);
	}

	return iRet ;
}

int tbus_add_addrtemplet_segment(OUT LPTBUSADDRTEMPLET a_pstAddrTemplet, IN char *a_pszSeg)
{
	char *pchDesc;
	LPTBUSHANDLEEG pstSeg;
	int iBitsNum;
	int i;

	assert(NULL != a_pstAddrTemplet);
	assert(NULL != a_pszSeg);

	if (a_pstAddrTemplet->iSegNum >= (int)(sizeof(a_pstAddrTemplet->astSeg)/sizeof(a_pstAddrTemplet->astSeg[0])))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"address segment num reach max segments<%d> limits, cannot add any  segment",
			a_pstAddrTemplet->iSegNum);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PARSE_ADDR_TEMPLET);
	}

	pstSeg = &a_pstAddrTemplet->astSeg[a_pstAddrTemplet->iSegNum];
	pchDesc = strchr(a_pszSeg, TBUS_SPLIT_SEGMENT_DESCRITION_CHARACTER);
	if (NULL == pchDesc)
	{
		pchDesc = a_pszSeg;
	}else
	{
		pchDesc++;
	}

	iBitsNum = strtol(pchDesc, NULL, 10);
	if ((0 >= iBitsNum) || (iBitsNum > TBUS_ADDR_TEMPLET_MAX_BITS_NUM))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid address segment bits num<%s>, its value must be 1-%d",
			pchDesc, TBUS_ADDR_TEMPLET_MAX_BITS_NUM);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PARSE_ADDR_TEMPLET);
	}

	pstSeg->bBitNum = (unsigned char)iBitsNum;

	pstSeg->dwMaxVal = 1;
	for (i = 0; i < iBitsNum -1; i++)
	{
		pstSeg->dwMaxVal <<=1;
		pstSeg->dwMaxVal += 1;
	}


	pstSeg->bStartBit = 0;
	for (i = 0; i < a_pstAddrTemplet->iSegNum; i++)
	{
		pstSeg->bStartBit += a_pstAddrTemplet->astSeg[i].bBitNum;
	}


	a_pstAddrTemplet->iSegNum++;
	tbus_log(TLOG_PRIORITY_FATAL,"address segment[%d]: maxval:%u bitsnum:%d startbits:%d",
		a_pstAddrTemplet->iSegNum, pstSeg->dwMaxVal, pstSeg->bBitNum, pstSeg->bStartBit);

	return TBUS_SUCCESS;
}

void tbus_dump_addrtemplet(IN LPTBUSADDRTEMPLET a_pstAddrTemplet, IN FILE *a_fpDump)
{
	int i;
	assert(NULL != a_pstAddrTemplet);

	if (NULL == a_fpDump)
	{
		a_fpDump = stdout;
	}

	fprintf(a_fpDump, "addr segments num: %d  content:\n", a_pstAddrTemplet->iSegNum);
	for (i = a_pstAddrTemplet->iSegNum - 1; i >= 0; i--)
	{
		LPTBUSHANDLEEG pstSeg = &a_pstAddrTemplet->astSeg[i];
		fprintf(a_fpDump, "%d(%d/%d).",
			pstSeg->dwMaxVal,
			(int)pstSeg->bBitNum, (int)pstSeg->bStartBit);
	}
	fprintf(a_fpDump,"\n");
}



/**根据地址模板设置将内部通信地址 转换成点分十进制表示的地址串
*@param[in] a_pstTemplet 通信地址模板结构的指针
*@param[in] a_iAddr	通信地址
*@return 成点分十进制表示的地址串
*@note 返回的成点分十进制表示的地址串保存在一个静态缓冲区中，后一次调用会覆盖前一次调用时获取的信息
*/
char *tbus_addr_nota_by_addrtemplet(IN LPTBUSADDRTEMPLET a_pstTemplet,IN TBUSADDR a_iAddr)
{
	static char szAddrBuff[TBUS_MAX_ADDR_STRING_LEN];
	char *pch;
	int iWrite;
	int iLeft;
	int i;

	assert(NULL != a_pstTemplet);

	if (0 >= a_pstTemplet->iSegNum)
	{
		return "";
	}

	a_iAddr = tdr_ntoh32(a_iAddr);
	pch  = &szAddrBuff[0];
	iLeft = sizeof(szAddrBuff);
	for (i = a_pstTemplet->iSegNum - 1; i >= 0; i--)
	{
		LPTBUSHANDLEEG pstSeg = &a_pstTemplet->astSeg[i];
		iWrite = snprintf(pch, iLeft, "%d.", ((a_iAddr>>(int)pstSeg->bStartBit) & pstSeg->dwMaxVal));
		if ((0 > iWrite) || (iWrite >= iLeft))
		{
			break;
		}
		pch += iWrite;
		iLeft -= iWrite;
	}

	/*去掉最后的'.'*/
	pch--;
	*pch = '\0';

	return &szAddrBuff[0];
}

char *tbus_addr_ntop(IN TBUSADDR a_iAddr, char *a_pszDstBuff, unsigned int a_dwBuffSize )
{
	char *pch;
	int iWrite;
	int iLeft;
	int i;

	if ((NULL == a_pszDstBuff)||(0 >= a_dwBuffSize))
	{
		return "";
	}

    *a_pszDstBuff = '\0';
	if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInitStatus )
	{
		struct in_addr addr;

		memset(&addr, 0, sizeof(addr));
		addr.s_addr = a_iAddr;
		*a_pszDstBuff = '\0';
		tinet_ntop(AF_INET, &addr, a_pszDstBuff, a_dwBuffSize);
	}else
	{
		LPTBUSADDRTEMPLET a_pstTemplet;
		assert(NULL != g_stBusGlobal.pstGCIM);
		a_pstTemplet = &g_stBusGlobal.pstGCIM->stHead.stAddrTemplet;
		a_iAddr = tdr_ntoh32(a_iAddr);
		pch  = a_pszDstBuff;
		iLeft = (int)a_dwBuffSize;
		for (i = a_pstTemplet->iSegNum - 1; i >= 0; i--)
		{
			LPTBUSHANDLEEG pstSeg = &a_pstTemplet->astSeg[i];
			iWrite = snprintf(pch, iLeft, "%d.", ((a_iAddr>>(int)pstSeg->bStartBit) & pstSeg->dwMaxVal));
			if ((0 > iWrite) || (iWrite >= iLeft))
			{
				break;
			}
			pch += iWrite;
			iLeft -= iWrite;
		}

		/*去掉最后的'.'*/
		pch--;
		*pch = '\0';
	}/*if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInitStatus )*/

	return a_pszDstBuff;
}
/**根据通信地址模板将点分十进制表示的地址串转换成内部表示通信地址
*@param[in] a_pstTemplet 通信地址模板结构的指针
*@param[in] a_pszAddr 点分十进制表示的地址串缓冲去指针
*@param[out] a_piAddr 保存通信地址的指针
*@retval	0 成功
*@retval <0	失败
*/
int tbus_addr_aton_by_addrtemplet(IN LPTBUSADDRTEMPLET a_pstTemplet, IN const char *a_pszAddr, OUT LPTBUSADDR a_piAddr)
{
	int iAdd;
	long long llSegVal;
	char szAddrBuff[TBUS_MAX_ADDR_STRING_LEN];
	int i;
	char *pszSeg = NULL;
	int iRet = TBUS_SUCCESS;

	assert(NULL != a_pstTemplet);
	assert(NULL != a_pszAddr);
	assert(NULL != a_pszAddr);

	STRNCPY(szAddrBuff, a_pszAddr, sizeof(szAddrBuff));
	tbus_trim_str(szAddrBuff);
	tbus_log(TLOG_PRIORITY_TRACE,"addr string:%s", szAddrBuff);

	if (strcasecmp(szAddrBuff,"0") == 0)
	{
		*a_piAddr = 0; /*"0" 表示0.0.0.0..这个特殊地址*/
		return 0;
	}

	pszSeg = &szAddrBuff[0];
	iAdd =0 ;
	for (i = a_pstTemplet->iSegNum - 1; i >= 0; i-- )
	{
		LPTBUSHANDLEEG pstSeg = &a_pstTemplet->astSeg[i];
		char *pchDot = strchr(pszSeg, TBUS_SPLIT_SEGMENT_CHARACTER);
		if (NULL != pchDot)
		{
			*pchDot = '\0';
		}
		if ('\0' == *pszSeg)
		{
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
			tbus_log(TLOG_PRIORITY_ERROR,
                     "invalid addr string<%s> do not match addr templet,"
                     " for addr segment num is %d",
                     a_pszAddr, a_pstTemplet->iSegNum);
			break;
		}
		llSegVal = strtoll(pszSeg, NULL, 10);
		if ((0 > llSegVal) || (llSegVal > pstSeg->dwMaxVal))
		{
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
			tbus_log(TLOG_PRIORITY_ERROR,
                     "%dth segment value scope is 0-%u,"
                     " but get value is %lld do not match addr templet,"
                     " for addr segment num is %d",
                     i, pstSeg->dwMaxVal, llSegVal, a_pstTemplet->iSegNum);
			break;
		}

		iAdd += llSegVal<<pstSeg->bStartBit;
		if (NULL != pchDot)
		{
			pszSeg = pchDot + 1;
		}else
		{
			*pszSeg = '\0';
		}
	}/*for (i = a_pstTemplet->iSegNum - 1; i >= 0; i-- )*/

	/*如果模板都匹配完毕，地处串还有剩余*/
	if ((0 > i) &&
		(0 < strnlen(pszSeg, sizeof(szAddrBuff) - (pszSeg - &szAddrBuff[0]))))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid addr string<%s> do not match addr templet, for addr segment num is %d",
			a_pszAddr, a_pstTemplet->iSegNum);

		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
	}

	if (0 == iRet)
	{
		*a_piAddr = tdr_hton32(iAdd);
		tbus_log(TLOG_PRIORITY_DEBUG,"convert addr string <%s> to addr %x",
			a_pszAddr, iAdd);
	}


	return iRet;
}
