/** @file $RCSfile: tbusd_send.c,v $
  general description of this module
  $Id: tbusd_send.c,v 1.25 2009-11-09 05:54:28 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-11-09 05:54:28 $
@version $Revision: 1.25 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/



#include "tbusd_log.h"
#include "tbusd_send.h"
#include "tbusd_misc.h"
#include "tbus/tbus_dyemsg.h"
#include "tbus/tbus_channel.h"
#include "tbus/tbus_log.h"
#include "../../lib_src/tdr/tdr_net_i.h"


extern TAPPCTX g_stAppCtx;
extern RELAYENV gs_stRelayEnv;


unsigned int tbus_get_time_delta(IN LPTBUSTIMESTAMPINTERNAL a_pstBegin,
                                 IN LPTBUSTIMESTAMPINTERNAL a_pstEnd);
static int tbusd_update_msg_timestamp(INOUT char *a_pszNet);

/** ��relayͨ���Ϸ�������
@param[in] a_pstRunEnv -- ���������������Ļ���
@param[in] a_pstRelay relayͨ�����ݽṹָ��
@param[out] a_iMaxSendPkg �����Է������ݰ�

@retval 0 -- no package send
@retval >0 number of package sended

@note
*/
int tbusd_send_pkg(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN int a_iMaxSendPkg)
{
	int iRet = TBUS_SUCCESS;
	int	iPkgSended = 0;
	LPCONNNODE pstConn;
	LPTBUSCHANNEL  pstChannel;
	LPTBUSRELAYCONNSTAT pstRelayStat;
	int iHeadSeq = 0;
	int iGetSeq = 0;
	int iAckNum ;
	char *sData = NULL ;
	CHANNELVAR *pstVar = NULL ;
	LPTBUSD pstConf ;



	assert(NULL != a_pstRunEnv);
	assert(NULL != a_pstRelay);
	assert(NULL != a_pstRelay->pConnNode);
	assert(NULL != a_pstRunEnv->pstStat);


	pstConf = a_pstRunEnv->pstConf;
	assert(NULL != pstConf);
	pstConn = a_pstRelay->pConnNode;
	pstChannel = &a_pstRelay->stChl;
	assert(a_pstRelay->iID < (int)a_pstRunEnv->dwRelayCnt);
    assert(a_pstRelay->iID >= 0);
	pstRelayStat = &a_pstRunEnv->pstStat->stRelayConnStatList.astRelayConnStat[a_pstRelay->iID];

	/*�ȷ���ʣ��û�з����������*/
	iRet = tbusd_send_remain_data(pstConn, pstChannel, a_pstRelay) ;
	if ( 0 != iRet )
	{
		tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
			"tbusd_send_pkg, tbusd_send_remain_data failed to send remain data on socket %d of relay %d, ret %d",
			pstConn->iFd, a_pstRelay->iID, iRet ) ;
		return 1;
	}
	if (TBUSD_CONN_HAVE_REMAIN_DATA(pstConn))
	{
		return 1;	/*ͬ�����ݻ�û�з�����,���Ȳ�����ҵ������*/
	}




	/*��Ƹ��ģ����͵�������Ŀ�ɿɷ������Ϊȷ�ϱ��������һ�����ֵ����,������δȷ�ϰ���
	������������*/
	a_iMaxSendPkg = (a_pstRunEnv->iMaxSendPkgPerLoop < a_iMaxSendPkg)? a_iMaxSendPkg : a_pstRunEnv->iMaxSendPkgPerLoop;
	if ((0 != pstConf->iNeedAckPkg) && (pstConf->iControlUnackedPkgNum != 0))
	{
		pstVar = TBUS_CHANNEL_VAR_GET(pstChannel);
		sData = TBUS_CHANNEL_QUEUE_GET(pstChannel);
		TBUS_GET_PKGSEQ_BY_POS(iHeadSeq, pstChannel, pstVar->dwHead);
		TBUS_GET_PKGSEQ_BY_POS(iGetSeq, pstChannel, pstVar->dwGet);
		iAckNum = iGetSeq - iHeadSeq;/*δȷ����Ŀ����Ŀ*/
		if (iAckNum >= TBUS_MAX_SENDED_PKG_NUM_UNACK)
		{
			tlog_info( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
				"unacked Sended pkg number(%d) reach the max limit(%d), so stop to send pkg!!!"
				"unacked pkg seq form %d to %d",
				iAckNum, TBUS_MAX_SENDED_PKG_NUM_UNACK, iHeadSeq, iGetSeq);
			a_iMaxSendPkg = 0;
		}else
		{
			iAckNum = TBUS_MAX_SENDED_PKG_NUM_UNACK - iAckNum; /*����ɷ���δȷ�����ݰ���Ŀ*/
			a_iMaxSendPkg = (a_iMaxSendPkg > iAckNum) ? iAckNum : a_iMaxSendPkg;
		}/*if (iAckNum >= TBUS_MAX_SENDED_PKG_NUM_UNACK)*/
	}/*if (pstConf->iControlUnackedPkgNum != 0)*/




	/*����ҵ������*/
	while(iPkgSended < a_iMaxSendPkg)
	{
		/*ȡ��һ����Ϣ,���ͷ��ͻ�������*/
		iRet = tbusd_send_one_pkg(pstChannel, pstConn, a_pstRelay);
		if (0 != iRet)
		{
			break;
		}

		if (TBUSD_CONN_HAVE_REMAIN_DATA(pstConn))
		{
			break; /*��ǰ���ݰ���û�з������,�򲻷���һ������*/
		}

		iPkgSended++;
	}/*while(iPkgSended < a_iMaxSendPkg)*/
	if (iPkgSended >  (int)pstRelayStat->dwMaxSendCountPerloop)
	{
		pstRelayStat->dwMaxSendCountPerloop = iPkgSended;
	}


	/*����seqͬ������*/
	if (!TBUSD_CONN_HAVE_REMAIN_DATA(pstConn) && (pstConn->bNeedSynSeq))
	{
		iRet = tbusd_send_synseq_pkg(pstConn, a_pstRelay, pstRelayStat);
		if ( 0 != iRet )
		{
			tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
				"tbusd_send_pkg, tbusd_send_synseq_pkg failed to send sysseq pkg on socket %d of relay %d, ret %d",
				pstConn->iFd, a_pstRelay->iID, iRet ) ;
		}
	}/*if (!TBUSD_CONN_HAVE_REMAIN_DATA(pstConn))*/

	return iPkgSended;
}


int tbusd_send_remain_data(INOUT LPCONNNODE a_pstConn,INOUT LPTBUSCHANNEL a_pstChannel, IN LPRELAY a_pstRelay)
{

	int	iRemain = 0;
	char *pszDataToSend;
	int	iSend = 0 ;
	LPSENDDATAMNG pstSendMng ;
    LPTBUSRELAYCONNSTAT pstRelayStat;

	assert(NULL != a_pstConn);
	assert(NULL != a_pstChannel);
    assert(NULL != a_pstRelay);
    assert(NULL != gs_stRelayEnv.pstStat);
    assert(a_pstRelay->iID < (int)gs_stRelayEnv.dwRelayCnt);
    assert(a_pstRelay->iID >= 0);
    pstRelayStat = &gs_stRelayEnv.pstStat->stRelayConnStatList.astRelayConnStat[a_pstRelay->iID];


	if (!TBUSD_CONN_HAVE_REMAIN_DATA(a_pstConn))
	{
		return TBUS_SUCCESS;	/*ͬ�����ݻ�û�з�����,���Ȳ�����ҵ������*/
	}

	pstSendMng = &a_pstConn->stSendMng;
	assert(NULL != pstSendMng->pstSendHead);
	tlog_trace( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
		"tbusd_send_remain_data, pkg<len:%d> seq: %u type: %d sendPos: %d on socket %d",
		pstSendMng->iSendBuff, ntohl(pstSendMng->pstSendHead->iSeq), pstSendMng->enRemainDataType,
		pstSendMng->iSendPos, a_pstConn->iFd);


	/*��������*/
	if (pstSendMng->enRemainDataType == TBUSD_DATA_TYPE_SYN)
	{
		pszDataToSend = pstSendMng->pszSynData + pstSendMng->iSendPos;
	}else
	{
		pszDataToSend = pstSendMng->pszBussiData + pstSendMng->iSendPos;
	}
	iRemain = pstSendMng->iSendBuff - pstSendMng->iSendPos;
	iSend = tnet_send(a_pstConn->iFd, pszDataToSend, iRemain, 0);
	if( iSend<0 )
	{
		tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "tnet_send send data(pkg<len:%d> seq: %u type: %d sendPos: %d ) failed on  socket %i, errno:%u, %s",
			pstSendMng->iSendBuff, ntohl(pstSendMng->pstSendHead->iSeq), pstSendMng->enRemainDataType,
			pstSendMng->iSendPos,
			a_pstConn->iFd, TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE) ) ;

		a_pstConn->bNeedFree = TBUSD_TRUE;
		return -1 ;
	}

	/*��鷢�ͽ��*/
	pstRelayStat->ullSendPkgBytes += iSend;
	if( iSend < iRemain )
	{	/*��������û�з������*/
		pstSendMng->iSendPos += iSend;
		return TBUS_SUCCESS;
	}

	/*ʣ�������Ѿ�������,���޸���Ӧ���ݽṹ*/
	switch(pstSendMng->enRemainDataType)
	{
	case TBUSD_DATA_TYPE_SYN:
		time(&a_pstConn->tLastSynSeq); /*�������һ�η���seqͬ������ʱ��*/
		pstRelayStat->dwSendPkgCount++;
		break;
	case TBUSD_DATA_TYPE_BUSINESS:
		pstRelayStat->dwSendPkgCount++;
		if(pstSendMng->pstSendHead->bFlag & TBUS_HEAD_FLAG_TACE)
		{
			tbus_log_dyedpkg(pstSendMng->pstSendHead, "Send(tbusd)");
		}
		TBUSD_ADJUST_SENDING_CHANNEL_POS(pstSendMng,a_pstChannel);


		tlog_debug( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
			"tbusd_send_remain_data, pkg<len:%d> seq: %u type: %d on socket %d(relay:%d)",
			pstSendMng->iSendBuff, ntohl(pstSendMng->pstSendHead->iSeq), pstSendMng->enRemainDataType,
			a_pstConn->iFd, a_pstRelay->iID);
		break;
	default:
		break;
	}
	pstSendMng->iSendPos = 0;
	pstSendMng->iSendBuff = 0;
	pstSendMng->enRemainDataType = TBUSD_DATA_TYPE_UNKNOWN;

	return TBUS_SUCCESS;
}



int tbusd_send_one_pkg( IN const TBUSCHANNEL *a_pstChannel, IN LPCONNNODE a_pstConn,
					   IN LPRELAY a_pstRelay)
{
	int iPkgLen = 0,
		iDataLen = 0;
	char *sData = NULL ;
	CHANNELVAR *pstVar = NULL ;
	CHANNELVAR *pstPushVar = NULL ;
	int iRet = 0;
	LPTBUSHEAD pstHead;


    assert(NULL != a_pstChannel);
    assert(NULL != a_pstConn);
    assert(NULL != a_pstRelay);


	pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);
	sData = TBUS_CHANNEL_QUEUE_GET(a_pstChannel);
	if (pstVar->dwGet == pstVar->dwTail)
	{
		return TBUS_ERR_CHANNEL_EMPTY;
	}


	/*����ͷָ�뵽����ĩβ�Ƿ��㹻����һ����Ϣ*/
	TBUS_CHECK_QUEUE_HEAD_VAR(sData, pstVar->dwSize, pstVar->dwGet);
	iDataLen = (int)(pstVar->dwTail - pstVar->dwGet);
	if (iDataLen < 0)
	{
		iDataLen += pstVar->dwSize;
	}
	if (0 >= iDataLen)
	{   	/*�ٴμ�����ݶ������Ƿ�������*/
		return TBUS_ERR_CHANNEL_EMPTY;
	}

	//get head ����Ч���ϵĿ��ǣ����ﲻ����tbus_decode_headȡ��
	//TBushead������ֱ��ʹ������ת���������ڴ��е�����ǿ������ת��
	if ((int)TBUS_HEAD_MIN_SIZE > iDataLen)
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,
			"failed to Get tbus head, dwGet:%u dwTail:%u dataLen:%d is less than tbushead min length(%d), its impossible!",
			pstVar->dwGet, pstVar->dwTail, iDataLen, (int)TBUS_HEAD_MIN_SIZE);
		pstVar->dwGet = pstVar->dwHead = pstVar->dwTail;
		return -1;
	}
	pstHead = (LPTBUSHEAD)(sData +pstVar->dwGet);


	/*������ݳ���������������ݳ����Ƿ���Ч*/
	iPkgLen = pstHead->bHeadLen + ntohl(pstHead->iBodyLen);
	if ((0 >= iPkgLen) || (iPkgLen > (int)pstVar->dwSize))
	{
		tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
			"invalid  length(%d) of pkg sending, its should not bigger than the size %d of channel queue or less than 1, impossible!",
			iPkgLen, pstVar->dwSize);
		pstVar->dwGet = pstVar->dwHead = pstVar->dwTail;
		return  -1;
	}
	if (iDataLen < iPkgLen)
	{
		tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
			"tbusd_send_one_pkg, data len %d in TBUSCHANNEL is less than pkg len:%d, impossible!",
			iDataLen, iPkgLen);
		pstVar->dwGet = pstVar->dwHead = pstVar->dwTail;
		return -1;
	}


	/*�Ӵ��������ݰ����к�*/
	pstPushVar = TBUS_CHANNEL_VAR_PUSH(a_pstChannel);
	if ((a_pstConn->bNeedSynSeq) ||((pstPushVar->iRecvSeq - a_pstConn->iLastAckSeq) > 0))
	{
		pstHead->bFlag |= TBUS_HEAD_FLAG_WITH_ACK;
		pstHead->iRecvSeq = htonl(pstPushVar->iRecvSeq);
		a_pstConn->iLastAckSeq = pstPushVar->iRecvSeq;
		a_pstConn->bNeedSynSeq = TBUSD_FALSE;
        time(&a_pstConn->tLastSynSeq);
	}

    if (TBUS_TIMESTAP_VERSION <= pstHead->bVer && (TBUS_HEAD_FLAG_WITH_TIMESTAMP & pstHead->bFlag))
    {
        iRet = tbusd_update_msg_timestamp((char*)pstHead);
        if (0 != iRet)
        {
            return iRet;
        }
    }

	/*��������*/
	a_pstConn->stSendMng.pstSendHead = pstHead;
	a_pstConn->stSendMng.enRemainDataType = TBUSD_DATA_TYPE_BUSINESS;
	a_pstConn->stSendMng.iSendBuff = iPkgLen;
	a_pstConn->stSendMng.iSendPos = 0;
	a_pstConn->stSendMng.pszBussiData = sData +pstVar->dwGet;
	iRet = tbusd_send_remain_data(a_pstConn, (LPTBUSCHANNEL)a_pstChannel, a_pstRelay) ;
	if ( 0 != iRet )
	{
		LPRELAY pstRelay = a_pstConn->pstBindRelay;
		assert(NULL != pstRelay);
		tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
			"tbusd_send_remain_data failed to send remain data on socket %d of relay %d, ret %d",
			a_pstConn->iFd, pstRelay->iID, iRet ) ;
	}

	return iRet;
}





int tbusd_proc_send(IN LPRELAYENV a_pstRunEnv, IN int a_iMaxSendPkg)
{

	unsigned int i = 0;
	RELAY *ptRelay = NULL ;
	LPCONNNODE pstConn;
	int iSendPkg = 0;

	assert(NULL != a_pstRunEnv);

	for ( i=0; i< a_pstRunEnv->dwRelayCnt; i++ )
	{
		ptRelay = a_pstRunEnv->pastTbusRelay[i] ;
		if (!TBUSD_RELAY_IS_ENABLE(ptRelay))
		{
			continue ;
		}
		if ( TBUSD_STATUS_ESTABLISHED != ptRelay->dwRelayStatus )
		{
			tlog_trace(g_ptRelayLog,ptRelay->iID, TBUSD_LOGCLS_RELAYID,
				"tbusd_proc_send, relay %d staus is %i should not send data", i, ptRelay->dwRelayStatus ) ;
			continue ;
		}

		iSendPkg += tbusd_send_pkg (a_pstRunEnv, ptRelay, a_iMaxSendPkg);
		pstConn = (LPCONNNODE)ptRelay->pConnNode;
		assert(NULL != pstConn);
		if (pstConn->bNeedFree)
		{
			tlog_error( g_ptRelayLog,ptRelay->iID, TBUSD_LOGCLS_RELAYID,
				"tbusd_proc_send, relay<%d> send pkg failed, so close socket %d",
				ptRelay->iID, pstConn->iFd) ;
			tbusd_destroy_conn(a_pstRunEnv, pstConn, NULL);
			continue ;
		}
	}/*for ( i=0; i< a_pstRunEnv->stRelayGlobal.dwRelayCnt; i++ )*/

	return iSendPkg ;
}

int tbusd_update_msg_timestamp(INOUT char *a_pszNet)
{
    int iRet = 0;
    char chIsDyed = 0;
    char *pchNet = NULL;
    unsigned char bRoute = 0;
    LPTBUSHEAD pstHead = NULL;
    unsigned int dwTimeDelta = 0;
    unsigned int *pdwNetCumm = NULL;
    TBUSTIMESTAMPINTERNAL stTimeStamp;
    TBUSTIMESTAMPINTERNAL stTimeInAppCtx;

    assert(NULL != a_pszNet);

    pstHead = (LPTBUSHEAD)a_pszNet;

    assert(TBUS_TIMESTAP_VERSION <= pstHead->bVer && (TBUS_HEAD_FLAG_WITH_TIMESTAMP & pstHead->bFlag));

    pchNet = a_pszNet + TBUS_HEAD_EXTHEAD_DATA_OFFSET;

    switch (pstHead->bCmd)
    {
        case TBUS_HEAD_CMD_TRANSFER_DATA:
            {
                bRoute = *(unsigned char *)pchNet;
                pchNet++;
                chIsDyed = *pchNet;
                pchNet++;
                if (0 != chIsDyed)
                {
                    pchNet += TDR_SIZEOF_INT;
                }
                if (0 < bRoute)
                {
                    pchNet += (bRoute * TDR_SIZEOF_INT);
                }
            }
            break;
        default:
            pchNet += 2;
            break;
    }

    stTimeStamp.llSec = tdr_ntoh64(*(long long*)pchNet);
    pchNet += sizeof(long long);
    stTimeStamp.dwUSec = (unsigned int)ntohl(*(unsigned int*)pchNet);
    pchNet += sizeof(unsigned int);
    pdwNetCumm = (unsigned int*)pchNet;
    stTimeStamp.dwCumm = (unsigned int)htonl(*(unsigned int*)pchNet);
    pchNet += sizeof(unsigned int);

    stTimeInAppCtx.llSec = g_stAppCtx.stCurr.tv_sec;
    stTimeInAppCtx.dwUSec = (unsigned int)g_stAppCtx.stCurr.tv_usec;

    TBUS_CALC_TIME_DELTA(dwTimeDelta, stTimeInAppCtx, stTimeStamp);
    stTimeStamp.dwCumm += dwTimeDelta;

    *pdwNetCumm = (unsigned int)htonl(stTimeStamp.dwCumm);

    return iRet;
}
