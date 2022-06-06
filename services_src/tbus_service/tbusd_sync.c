/** @file $RCSfile: tbusd_sync.c,v $
  general description of this module
  $Id: tbusd_sync.c,v 1.27 2009-11-09 05:54:28 jacky Exp $
  @author $Author: jacky $
  @date $Date: 2009-11-09 05:54:28 $
  @version $Revision: 1.27 $
  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
  @note Platform: Linux
  */


#include <assert.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <netinet/tcp.h>
#endif
#include "pal/pal.h"
#include "tbus/tbus.h"
#include "tbusd_misc.h"
#include "tbusd_sync.h"
#include "tbusd_log.h"
#include "tbusd_send.h"
#include "tbus/tbus_channel.h"


extern RELAYENV gs_stRelayEnv;

/** ������������SYN��
  @param[in] a_pstRunEnv relay���������������Ļ���
  @param[in] a_pstRelay ͨ�����ݽṹ
  @retval 0 �ɹ�
  @retval ���� ʧ��
  */
int tbusd_relaycli_send_syn(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay);

/**
  @brief Build a sync package via specified sequence and commamd id

  @param[in,out] a_ptHead -- data buffer needs to be filled built TBUSHEAD
  @param[in] a_ptChannel -- channel information
  @param[in] a_iSeq -- sequence needs to be sent
  @param[in] a_cCmd -- commamd id

  @retval none

  @note
  */
static void tbusd_fill_syn_head ( INOUT TBUSHEAD *a_ptHead, IN const TBUSCHANNEL *a_ptChannel, IN const int a_iSeq, IN const char a_chCmd ) ;

/** �����������յ������ݰ��������Ӱ󶨵��ض�relayͨ����
  @param[in] a_pstRunEnv���������������Ļ���
  @param[in] a_pstNode ���ӹ���ڵ�
  @param[in] a_pstHead �յ������ݰ�ͷ��
  @return �ɹ����ذ�relayͨ����ָ�룬���򷵻�NULL
  */
LPRELAY tbusd_get_relay_by_conn(IN LPRELAYENV a_pstRunEnv, IN LPCONNNODE a_pstNode, IN LPTBUSHEAD a_pstHead);

/** ������conn��relayͨ�������󶨹�ϵ,������relayͨ����status
*/
void tbusd_bind_conn_relay(IN LPCONNNODE a_pstConn, IN LPRELAY a_pstRelay, IN int a_iRelayStatus);

/** �������conn��relayͨ��֮��İ󶨹�ϵ,����relayͨ����status����ΪTBUSD_STATUS_NO_CONNECTION
*/
void tbusd_unbind_conn_relay(IN LPCONNNODE a_pstConn, IN LPRELAY a_pstRelay);

/** ����relayͨ���ϵ��������ݰ���
  @param[in] a_pstRunEnv���������������Ļ���
  @param[in] a_pstRelay relayͨ�����ݽṹָ��
  @param[in] a_pstHead ���յ����������ݰ�
  @return �ɹ����ذ�relayͨ����ָ�룬���򷵻�NULL
  */
int tbusd_proc_handshake_pkg(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN LPTBUSHEAD a_pstHead);

/**����URL��Է���������
  @param[in] a_pszConnectURL ��Է��������ӵ�URL
  @return ��������������
  */
int tbusd_connect_nonblock(IN LPRELAY a_pstRelay);

/** relayͨ����TBUSD_STATUS_SERVER_INIT״̬�յ��������ݰ�
  @param[in] a_pstRunEnv �����������Ļ���
  @param[in] a_pstRelay ����ͨ��
  @param[in] a_iRecvSeq �Է����ܵ������һ�����ݰ������к�
  @param[in] a_bRecvSeqFlag �������ݰ��ı�־λ
  */
int tbusd_relaysvr_recv_syn(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN  int a_iRecvSeq,
        IN unsigned char a_bRecvSeqFlag);

/** relayͨ����TBUSD_STATUS_SERVER_SYN_RECV״̬�յ��������ݰ�SVR_SYN_SCK
  @param[in] a_pstRunEnv �����������Ļ���
  @param[in] a_pstRelay ����ͨ��
  @param[in] a_dwSendSeq �Է��������ݰ��ı�־λ
  */
int tbusd_relaysvr_recv_ack(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN  int a_iSendSeq);

/** ��������ΪTBUS_HEAD_CMD_HANDSHAKE_SVR_SYN_ACK��������Ϣ
 *��Է��ṩ���ܰ������кźͷ������ݰ������к�
 */
int tbusd_relaysvr_send_syn_ack(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay);

/** relayͨ����TBUSD_STATUS_CLIENT_SYN_SEND״̬�յ��������ݰ�SVR_SYN_SCK
  @param[in] a_pstRunEnv �����������Ļ���
  @param[in] a_pstRelay ����ͨ��
  @param[in] a_dwRecvSeq �Է����ܵ������һ�����ݰ������к�
  @param[in] a_bRecvSeqFlag �������ݰ��ı�־λ
  @param[in] a_dwSendSeq �Է��������ݰ��ı�־λ
  */
int tbusd_relaycli_recv_syn_ack(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN  int a_iRecvSeq,
        IN unsigned char a_bRecvSeqFlag, IN  int a_iSendSeq);

/** ��������ΪTBUS_HEAD_CMD_HANDSHAKE_CLI_ACK��������Ϣ
 *��Է��ṩ�������ݰ������к�
 */
int tbusd_relaycli_send_ack(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay);

static void tbusd_free_conn (IN LPRELAYENV  a_pstRunEnv, IN LPCONNNODE a_pstNode);


static int tbusd_validate_peer_ip(IN LPRELAYENV a_pstRunEnv, LPRELAY a_pstRelay, IN LPCONNNODE a_pstNode);

//////////////////////////////////////////////////////////////////////////////////////////////////

void tbusd_fill_syn_head ( INOUT TBUSHEAD *a_ptHead, IN const TBUSCHANNEL *a_ptChannel, IN const int a_iSeq, IN const char a_chCmd )
{
    memset(a_ptHead, 0, sizeof(TBUSHEAD));

    a_ptHead->bVer = TDR_METALIB_TBUSHEAD_VERSION;
    a_ptHead->iDst = a_ptChannel->pstHead->astAddr[a_ptChannel->iRecvSide];
    a_ptHead->iSrc = a_ptChannel->pstHead->astAddr[a_ptChannel->iSendSide];
    a_ptHead->bCmd = a_chCmd;
    a_ptHead->iSeq = a_iSeq;
    a_ptHead->bFlag = TBUS_HEAD_FLAG_SYN;
    return ;
}



int tbusd_relaycli_send_syn(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay)
{
    int iRet = TBUS_SUCCESS;
    TBUSHEAD stHead;
    LPTBUSCHANNEL pstChannel;
    CHANNELVAR *pstVar = NULL ;
    LPCONNNODE pstConn;
    char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
    int iHeadLen = 0;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);
    assert(NULL != a_pstRelay->pConnNode);

    /*���������Ϣͷ��*/
    pstChannel = &a_pstRelay->stChl;
    tbusd_fill_syn_head(&stHead, pstChannel, RELAY_INIT_SEQUENCE_VALUE,
            TBUS_HEAD_CMD_HANDSHAKE_CLI_SYN);

    /*���������Ϣ��, ����ͨ����߽��յ�����Ϣ�����к�*/
    pstVar = TBUS_CHANNEL_VAR_PUSH(pstChannel);
    tlog_trace(g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "dwHead:%u dwGet:%u dwTail:%u dwSize:%u iRescseq:%d",
            pstVar->dwHead, pstVar->dwGet, pstVar->dwTail, pstVar->dwSize, pstVar->iRecvSeq);
    if ( 0 == pstVar->chPStart )
    {
        stHead.stExtHead.stSynHead.bSynFlag = TBUS_SYNFLAG_NO_RECV;
        stHead.iRecvSeq = RELAY_INIT_SEQUENCE_VALUE;
    }else
    {
        stHead.stExtHead.stSynHead.bSynFlag = TBUS_SUNFLAG_CONTINUE_RECV;
        stHead.iRecvSeq = pstVar->iRecvSeq;
    }

    iHeadLen = sizeof(szNet);
    iRet = tbus_encode_head(&stHead, &szNet[0], &iHeadLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,"failed to pack tbus head,iRet %x on the relay(%d)",
                iRet, a_pstRelay->iID);
        return iRet;
    }

    /*ȡͨ�������ӹ�����*/
    pstConn = (LPCONNNODE)a_pstRelay->pConnNode;
    tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,"tbusd_relaycli_send_syn, send CLI_SYN pkg<chPStart %i, iRecvSeq %u> on socket %i",
            pstVar->chPStart, stHead.iRecvSeq, pstConn->iFd);

    /*��������*/
    iRet = tbusd_send_all(pstConn, (const char *)&szNet[0], iHeadLen);
    if (0 != iRet)
    {
        tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,"tbusd_relaycli_send_syn, relay<%d> failed to send syn pkg(len:%d) on socket %d",
                a_pstRelay->iID, iHeadLen, pstConn->iFd) ;
    }

    time(&pstConn->tLastSynSeq);
    return iRet;
}



/**��������ͨ�������ֹ���
 *@param[in] a_pstRunEnv tbusd�����Ļ������ݽṹָ��
 *@param[in,out] a_pstNode ����ͨ�������ӹ������ݽṹ
 @return �ɹ�������0�����򷵻ط���ֵ
 */
int tbusd_handshake(IN LPRELAYENV a_pstRunEnv, INOUT LPCONNNODE a_pstNode)
{
    int iRet = TBUS_SUCCESS;
    TBUSHEAD stHead;
    LPRELAY pstRelay;
    int iPkg;
    int iPkgLen;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstNode);

    iPkg = tbusd_read_tbushead(&stHead, a_pstNode->szRecvBuff, a_pstNode->iBuff);
    if (0 == iPkg)
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_TRACE, "tbusd_handshake, no enough data(len:%d) to a handshake pkg in socket %d, so wait data",
                a_pstNode->iBuff, a_pstNode->iFd);
        return TBUS_SUCCESS;
    }else if (0 > iPkg)
    {	/*��ȡ�������ݰ�ʧ�ܣ��ر�����*/
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tbusd_handshake, failed to get handshake pkg  in socket %d, so close socket",
                a_pstNode->iFd);
        a_pstNode->bNeedFree = TBUSD_TRUE;
        return -1;
    }

    /*���յ���Ϣͷ������������������Ϣ�Ļ�����*/
    iPkgLen = stHead.bHeadLen + stHead.iBodyLen;
    if (iPkgLen > a_pstNode->iBuff)
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL, "msg length error, head<%d>, body<%d>,"
                      " total<%d>, iBuff<%d>, socket<%d>",
                      (int)stHead.bHeadLen, (int)stHead.iBodyLen, iPkgLen, (int)a_pstNode->iBuff,
                      a_pstNode->iFd);
        a_pstNode->bNeedFree = TBUSD_TRUE;
        return -1;
    }
    a_pstNode->iBuff -= iPkgLen;
    if (0 < a_pstNode->iBuff)
    {
        memmove(&a_pstNode->szRecvBuff[0], &a_pstNode->szRecvBuff[iPkgLen], a_pstNode->iBuff);
    }

    /*�����յ������ݰ���socket���Ӱ󶨵��ض�relayͨ��*/
    pstRelay = tbusd_get_relay_by_conn(a_pstRunEnv, a_pstNode, &stHead);
    if (NULL == pstRelay)
    {
        a_pstNode->bNeedFree = TBUSD_TRUE;
        return -1;

    }

    iRet = tbusd_proc_handshake_pkg(a_pstRunEnv, pstRelay, &stHead);
    if (0 != iRet)
    {
        a_pstNode->bNeedFree = TBUSD_TRUE;
    }

    return iRet;
}

int tbusd_connect_peer (LPRELAYENV  a_pstRunEnv, IN time_t tCur, IN LPRELAY a_pstRelay)
{
    int iRet = TBUS_SUCCESS;
    CONNNODE *ptNode = NULL ;
    LPTBUSRELAYCONNSTAT pstRelayStat;
    int iSockfd;
    time_t tConncetGap;
    int iMinConncetGap;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRunEnv->pstStat);

    assert(NULL != a_pstRunEnv->pstConf);

    if (TBUSD_STATUS_NO_CONNECTION != a_pstRelay->dwRelayStatus)
    {
        return -1;
    }
    if (a_pstRelay->iID >= (int)a_pstRunEnv->dwRelayCnt)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "relay's id(%d) must be less than relay count(%d)",
                a_pstRelay->iID, a_pstRunEnv->dwRelayCnt) ;
        return -2;
    }
    pstRelayStat = &a_pstRunEnv->pstStat->stRelayConnStatList.astRelayConnStat[a_pstRelay->iID];


    iMinConncetGap = a_pstRunEnv->pstConf->iReconnectGap;
    if (iMinConncetGap >= TBUS_MAX_CONNECT_PEER_GAP)
    {
        iMinConncetGap = TBUS_MAX_CONNECT_PEER_GAP -1;
    }
    tConncetGap = (pstRelayStat->dwReconnectCount * iMinConncetGap) % TBUS_MAX_CONNECT_PEER_GAP;
    if ((tCur - a_pstRelay->tLastConnectPeer) < tConncetGap)
    {
        return 0;
    }

    pstRelayStat->dwReconnectCount++;
    a_pstRelay->tLastConnectPeer = tCur;
    iSockfd = tbusd_connect_nonblock(a_pstRelay);
    if (0 > iSockfd)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,  "tbusd_connect_peer, relay %d connecting failed by URL:%s",
                a_pstRelay->iID, a_pstRelay->stRelayInfo.szMConn) ;
        return -3 ;
    }


    tlog_info( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,"tbusd_connect_peer, connecting to '%s' on socket %i for relay %d",
            a_pstRelay->stRelayInfo.szMConn, iSockfd, a_pstRelay->iID ) ;
/*
 * ��pal epoll windows�汾(IOCPģ��ʵ��)������,
 * windows��������Ȼ�Է������ķ�ʽ��������,
 * ���ǲ�������epoll�жϷ������������Ƿ��Ѿ��ɹ�,
 * ���Զ���windowsϵͳ, û��ָ��epoll�ļ����¼�,
 * �Ӷ�Ҳ��û�а� fd ���� epoll �Ĺ���,
 */
#if defined(_WIN32) || defined(_WIN64)
    ptNode = tbusd_create_conn(a_pstRunEnv, iSockfd, RELAY_FD_CLIENT_MODE, 0);
#else
    ptNode = tbusd_create_conn(a_pstRunEnv, iSockfd, RELAY_FD_CLIENT_MODE, EPOLLIN|EPOLLOUT);
#endif
    if(NULL == ptNode)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,"tbusd_create_conn %d failed for relay(%d) ",
                a_pstRelay->iID,iSockfd) ;
        tnet_close (iSockfd) ;
        return -4;
    }

    /* connection status */
    ptNode->bIsConnected = TBUSD_FALSE;
    tbusd_bind_conn_relay(ptNode, a_pstRelay, TBUSD_STATUS_CONNECTING);

    return iRet ;
}

/**����URL��Է���������
  @param[in] a_pszConnectURL ��Է��������ӵ�URL
  @return ��������������
  */
int tbusd_connect_nonblock(IN LPRELAY a_pstRelay)
{
    int iRet = TBUS_SUCCESS;
    int iSockfd;


    assert(NULL != a_pstRelay);

    iSockfd = tnet_open(a_pstRelay->stRelayInfo.szMConn);
    if (0 > iSockfd)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_connect_nonblock, tnet_open failed by URL: %s, errno:%u, for %s",
                   a_pstRelay->stRelayInfo.szMConn, TBUSD_GET_SOCKET_ERROR_CODE,
                   tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE)) ;
        return -1;
    }

    tnet_set_nonblock(iSockfd, 1);

    iRet = tsocket_start_connect(iSockfd, (struct sockaddr*)&a_pstRelay->stConnectAddr.stIn,
                           sizeof(a_pstRelay->stConnectAddr.stIn));
    if (-1 == iRet)
    {
        tlog_error(g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tsocket_start_connect failed  by URL: %s, error:%u for %s",
                   a_pstRelay->stRelayInfo.szMConn, TBUSD_GET_SOCKET_ERROR_CODE,
                   tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE)) ;

        tnet_close(iSockfd);
        return -1;
    }

    return iSockfd;
}

#if defined(_WIN32) || defined(_WIN64)
/** �������������Ƿ��Ѿ��ɹ�
 * @param[in] a_pstRelay, ·�����ݽṹ
 * @return �ɹ�����0, ���򷵻� !0
 */
int tbusd_check_nonblock_connect(IN LPRELAYENV  a_pstRunEnv, IN time_t tCur, IN LPRELAY a_pstRelay)
{
    int iRet = TBUS_SUCCESS;
    CONNNODE *ptNode = NULL ;
    epoll_event_t e ;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);

    ptNode = a_pstRelay->pConnNode;
    assert(NULL !=  ptNode);

    if (a_pstRelay->iID >= (int)a_pstRunEnv->dwRelayCnt)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_check_nonblock_connect, relay's id(%d) must be less than relay count(%d)",
                   a_pstRelay->iID, a_pstRunEnv->dwRelayCnt) ;
        return -1;
    }

    if (TBUSD_STATUS_CONNECTING != a_pstRelay->dwRelayStatus)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_check_nonblock_connect, relay %d 's status(%u) is NOT TBUSD_STATUS_CONNECTING)",
                   a_pstRelay->iID, a_pstRelay->dwRelayStatus) ;
        return -1;
    }

#define TBUSD_NONBLOCK_CONNECT_TIMEOUT		10
    if ((tCur - a_pstRelay->tLastConnectPeer) >= TBUSD_NONBLOCK_CONNECT_TIMEOUT)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_check_nonblock_connect, relay %d nonblock-connect to %s timeout",
                   a_pstRelay->iID, a_pstRelay->stRelayInfo.szMConn) ;

        return -1;
    }

    iRet = tsocket_check_connect(ptNode->iFd, 0);
    if (-1 == iRet) /* error */
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_check_nonblock_connect, relay %d tsocket_check_connect failed by URL:%s",
                   a_pstRelay->iID, a_pstRelay->stRelayInfo.szMConn) ;
        return -1;
    } else if (0 != iRet) /* NO error, connecting still in progress */
    {
        tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_check_nonblock_connect, relay %d tsocket_check_connect by URL:%s: still in progress",
                   a_pstRelay->iID, a_pstRelay->stRelayInfo.szMConn) ;

        return 0;
    }

    tlog_info( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
              "tbusd_check_nonblock_connect, connected to '%s' on socket %i for relay %d",
              a_pstRelay->stRelayInfo.szMConn, ptNode->iFd, a_pstRelay->iID ) ;

    /* ���������ӳɹ�, ����EPOLLIN�¼� */
    ptNode->bIsConnected = TBUSD_TRUE;
    memset( &e, 0, sizeof(e) ) ;
    e.events = EPOLLIN;
    e.data.fd = ptNode->idx;
    if(epoll_ctl(a_pstRunEnv->iEpoolFd, EPOLL_CTL_ADD, ptNode->iFd, &e) < 0)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                      "tbusd_check_nonblock_connect, relay %d epoll_ctl add EPOLLIN failed, so close socket %d",
                      a_pstRelay->iID, ptNode->iFd) ;
        return -1;
    }

    ptNode->ulEpollKey = e.ulKey;

    /* ����ͬ���� */
    iRet = tbusd_relaycli_send_syn(a_pstRunEnv, a_pstRelay) ;
    if ( TBUS_SUCCESS != iRet )
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_check_nonblock_connect, tbusd_relaycli_send_syn failed on socket %d of  relay  %d",
                   ptNode->iFd, a_pstRelay->iID) ;

        return -1;
    }

    a_pstRelay->dwRelayStatus = TBUSD_STATUS_CLIENT_SYN_SEND;
    tlog_info( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
              "tbusd_check_nonblock_connect, relay<%d> send CLI_SYN handshake pkg on socket %d,"
              " so its status become TBUSD_STATUS_CLIENT_SYN_SEND from TBUSD_STATUS_CONNECTING ",
              a_pstRelay->iID, ptNode->iFd);

    return 0;
}
#endif


static int tbusd_validate_peer_ip(IN LPRELAYENV a_pstRunEnv, LPRELAY a_pstRelay, IN LPCONNNODE a_pstNode)
{
    int iRet = TBUS_SUCCESS;
    struct sockaddr_in stLoopAddr;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRunEnv->pstConf);
    assert(NULL != a_pstNode);
    assert(NULL != a_pstRelay);

    if (0 == a_pstRunEnv->pstConf->iEnableCheckPeerIP)
    {
        return iRet;
    }
    if ((a_pstRelay->stConnectAddr.stIn.sin_addr.s_addr!= a_pstNode->stAddr.sin_addr.s_addr))
    {
        //����ַ��127.0.0.1�����ڲ��ԣ�������Ҫ���⴦��
        inet_aton("127.0.0.1",&stLoopAddr.sin_addr);
        if ((a_pstRelay->stConnectAddr.stIn.sin_addr.s_addr != stLoopAddr.sin_addr.s_addr) &&
                (stLoopAddr.sin_addr.s_addr != a_pstNode->stAddr.sin_addr.s_addr))
        {
            char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
            char szAddr2[TBUS_MAX_ADDR_STRING_LEN];
            tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "socket<%d> peer address(%s) is different the address(URL:%s,0x%08x) in relay(peer:%s) configure,"
                    " please check the both tbusd's configure loopaddr:%s",
                    a_pstNode->iFd, tinet_ntop(AF_INET, &a_pstNode->stAddr.sin_addr, szAddr1, sizeof(szAddr1)),
                    a_pstRelay->stRelayInfo.szMConn, a_pstRelay->stConnectAddr.stIn.sin_addr.s_addr,
                    tbus_addr_ntop(a_pstRelay->stRelayInfo.dwAddr, szAddr2, sizeof(szAddr2)),inet_ntoa(stLoopAddr.sin_addr)) ;
            iRet = -1;
        }/*if ((pstRelay->stConnectAddr.stIn.sin_addr.s_addr != stLoopAddr.sin_addr.s_addr) &&*/
    }/*if ((0 != a_pstRunEnv->pstConf->iEnableCheckPeerIP) &&*/

    return iRet;
}

int tbusd_connecting_check(IN LPRELAYENV a_pstRunEnv, IN LPCONNNODE a_pstNode, IN struct epoll_event* a_ptEvent )
{
    int error;
#if defined(_WIN32) || defined(_WIN64)
    int len;
    int iLen;
#else
    socklen_t len;
    socklen_t iLen;
#endif
    LPRELAY pstRelay;
    int iRet = TBUS_SUCCESS;


    assert(NULL != a_pstNode);
    assert(NULL != a_ptEvent);

    len = sizeof(error);
    error = 0;
    if( getsockopt(a_pstNode->iFd, SOL_SOCKET, SO_ERROR, (char *)&error, &len) < 0 || error != 0 )
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tbusd_connecting_check, connected failed, so close socket %d",
                a_pstNode->iFd) ;
        return -1;
    }

    /*ȥ��EPOLLOUT�¼��Ĺ�ע,ֻ��עEPOLLIN*/
    a_ptEvent->events =  EPOLLIN;
    if(epoll_ctl(a_pstRunEnv->iEpoolFd, EPOLL_CTL_MOD, a_pstNode->iFd, a_ptEvent) < 0)
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tbusd_connecting_check, epoll_ctl del EPOLLOUT failed, so close socket %d",
                a_pstNode->iFd) ;
        return -1;
    }

    //ȥ�Է���ַ������ip��֤
    a_pstNode->bIsConnected = TBUSD_TRUE;
    iLen = sizeof(a_pstNode->stAddr);
    getpeername(a_pstNode->iFd, (struct sockaddr *)&a_pstNode->stAddr, &iLen);
    pstRelay = a_pstNode->pstBindRelay;
    if (NULL == pstRelay)
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tbusd_connecting_check, conn have not bind any relay, so close socket %d",
                a_pstNode->iFd) ;
        iRet = -2;
    }
    if (TBUS_SUCCESS == iRet)
    {
        iRet = tbusd_validate_peer_ip(a_pstRunEnv, pstRelay, a_pstNode);
        if (TBUS_SUCCESS != iRet)
        {
            tlog_error( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,  "the peer address of conn(socket:%d) is different to relay(ID��%d)'s configure",
                    a_pstNode->iFd, pstRelay->iID);
        }
    }/*if (TBUS_SUCCESS == iRet)*/


    //����ͬ����
    if (TBUS_SUCCESS == iRet)
    {
        tlog_debug( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,"tbusd_connecting_check, connect to peer on socket %d of relay %d, begin handshake ...",
                a_pstNode->iFd, pstRelay->iID) ;
        iRet = tbusd_relaycli_send_syn(a_pstRunEnv, pstRelay) ;
    }

    if ( TBUS_SUCCESS != iRet )
    {
        tlog_error( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,  "tbusd_connecting_check, tbusd_relaycli_send_syn failed on socket %d of  relay  %d",
                a_pstNode->iFd, pstRelay->iID) ;
        a_pstNode->bNeedFree = TBUSD_TRUE;
    }else
    {
        pstRelay->dwRelayStatus = TBUSD_STATUS_CLIENT_SYN_SEND;
        tlog_info( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "tbusd_connecting_check, relay<%d> send CLI_SYN handshake pkg on socket %d, so its status become TBUSD_STATUS_CLIENT_SYN_SEND \
                from TBUSD_STATUS_CONNECTING ", pstRelay->iID, a_pstNode->iFd);
    }


    return iRet;
}

LPCONNNODE tbusd_create_conn(IN LPRELAYENV  a_pstRunEnv, IN int a_iFd, IN int a_iFdMode, IN int a_iEpEvent)
{
    CONNNODE *ptNode = NULL ;
    int iFlag;
    int iRet = 0;
    int iNodeIndx;
    LPTMEMPOOL pstMemPool;
    LPTBUSD pstConf;
    char* pszTempBuff = NULL;
    int iTemBuffSize = 0;

    assert(0 < a_iFd);
    assert((RELAY_FD_LISTEN_MODE <= a_iFdMode) && (a_iFdMode <= RELAY_FD_CLIENT_MODE));
    assert(NULL != a_pstRunEnv);
    pstMemPool = a_pstRunEnv->ptMemPool;
    pstConf = a_pstRunEnv->pstConf;
    assert(NULL != pstMemPool);
    assert(NULL != pstConf);


    /* set socket option */
    iRet = tnet_set_nonblock(a_iFd, 1 ) ;
    if ( 0 > iRet )
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tbusd_connection_accept, tnet_set_nonblock() failed %i, errno:%u, for %s",
                iRet, TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE) ) ;
        return NULL ;
    }

    iFlag = 1 ;
    setsockopt (a_iFd, SOL_SOCKET, TCP_NODELAY, (void *)&iFlag, sizeof(int) ) ;
    if (0 < pstConf->iSendBuff)
    {
        tnet_set_sendbuff(a_iFd, pstConf->iSendBuff);
    }
    if (0 < pstConf->iRecvBuff)
    {
        tnet_set_recvbuff(a_iFd, pstConf->iRecvBuff);
    }
    if ((RELAY_FD_CLIENT_MODE == a_iFdMode) || (RELAY_FD_SERVER_MODE == a_iFdMode))
    {
        /*���ڴ������ݵ�relay���ӣ������ִ��ѡ���*/
        iFlag = 1;
        setsockopt(a_iFd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&iFlag, sizeof(iFlag));
    }


    /*Get node for sockt connection*/
    iNodeIndx =	tmempool_alloc (pstMemPool) ;
    if ( 0 > iNodeIndx )
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tbusd_create_conn, tmempool_alloc failed, for fd<%i>", a_iFd) ;
        return NULL;
    }
    tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_DEBU, " tmempool_alloc %0x node to socket %i", iNodeIndx, a_iFd) ;
    ptNode = (CONNNODE *) tmempool_get ( pstMemPool, iNodeIndx ) ;
    pszTempBuff = ptNode->szRecvBuff;
    iTemBuffSize = ptNode->iRecvBuffSize;
    memset (ptNode, 0, sizeof(CONNNODE)) ;
    ptNode->szRecvBuff = pszTempBuff;
    ptNode->iRecvBuffSize = iTemBuffSize;
    ptNode->dwType = a_iFdMode ;
    ptNode->iFd = a_iFd ;
    ptNode->pstBindRelay = NULL ;
    ptNode->tCreate = time(NULL);
    ptNode->idx = iNodeIndx;
    ptNode->bIsConnected = TBUSD_TRUE;

    if (NULL == ptNode->szRecvBuff)
    {
        ptNode->szRecvBuff = (char *)malloc(a_pstRunEnv->pstConf->iPkgMaxSize);
        if ((NULL ==  ptNode->szRecvBuff))
        {
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "failed to malloc memory for conn node recv/send buff(Size:%d) of fd<%i>",
                          a_pstRunEnv->pstConf->iPkgMaxSize, a_iFd) ;
            iRet = -1;
        }else
        {
            ptNode->iRecvBuffSize = a_pstRunEnv->pstConf->iPkgMaxSize;
        }
    }

    /* ���뵽 epoll �й��� */
    if (0 == iRet && 0 != a_iEpEvent)
    {
        epoll_event_t e ;

        memset( &e, 0, sizeof(e) ) ;
        e.events = a_iEpEvent;
        e.data.fd = iNodeIndx;
        iRet = epoll_ctl (a_pstRunEnv->iEpoolFd, EPOLL_CTL_ADD, a_iFd, &e ) ;
        if ( 0 != iRet )
        {
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "epoll_ctl add sockfd %d failed,ret: %u, for %s, Ret:%d",
                    a_iFd, TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE) ,iRet ) ;
        }else
        {
#if defined(_WIN32) || defined(_WIN64)
            ptNode->ulEpollKey = e.ulKey;
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_DEBU, "EPOL add succ sockfd %d epollkey:%u nodeindex:%u",
                    a_iFd, ptNode->ulEpollKey, e.data.fd) ;
#endif
        }

    }

    if (0 != iRet)
    {
        tbusd_free_conn(a_pstRunEnv, ptNode);
        ptNode = NULL;
    }


    return ptNode;
}

static void tbusd_free_conn (IN LPRELAYENV  a_pstRunEnv, IN LPCONNNODE a_pstNode)
{
    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRunEnv->ptMemPool);

    if (NULL == a_pstNode)
    {
        return;
    }

    tnet_close (a_pstNode->iFd) ;

    /*��������ӽڵ��ĳ��relayͨ���󶨣����޸Ĵ�relayͨ����״̬*/
    if (NULL != a_pstNode->pstBindRelay)
    {
        LPRELAY pstRelay = a_pstNode->pstBindRelay;

        tlog_error (g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_destroy_conn, free memnode<%d> of socket<fd: %d mode: %d> of relay %d",
                   a_pstNode->idx, a_pstNode->iFd, a_pstNode->dwType, pstRelay->iID);
        tbusd_unbind_conn_relay(a_pstNode, pstRelay);
    }

    tmempool_free ( a_pstRunEnv->ptMemPool, a_pstNode->idx) ;
}

void tbusd_destroy_conn (IN LPRELAYENV  a_pstRunEnv, IN LPCONNNODE a_pstNode, IN TPOLL_EVENT* a_pstEvent)
{
    struct epoll_event  stEvent;
    int iRet = TBUS_SUCCESS;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRunEnv->ptMemPool);

    if (NULL == a_pstNode)
    {
        return;
    }

    if (NULL == a_pstEvent)
    {
        memset(&stEvent, 0, sizeof(stEvent));
        a_pstEvent = &stEvent;
    }
#if defined(_WIN32) || defined(_WIN64)
    a_pstEvent->ulKey = a_pstNode->ulEpollKey;
#endif
    iRet = epoll_ctl( a_pstRunEnv->iEpoolFd, EPOLL_CTL_DEL, a_pstNode->iFd, a_pstEvent) ;
    if ( 0 != iRet)
    {
#if defined(_WIN32) || defined(_WIN64)
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "tbusd_destroy_conn closed fd %i,errno:%u for %s, key:%u, iRet:%d",
                a_pstNode->iFd, TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE),
                a_pstEvent->ulKey, iRet);
#else
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "tbusd_destroy_conn closed fd %i,errno:%u for %s, iRet:%d",
                a_pstNode->iFd, TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE),
                iRet)  ;
#endif
    }

    tbusd_free_conn(a_pstRunEnv, a_pstNode);
}


LPRELAY tbusd_get_relay_by_conn(IN LPRELAYENV a_pstRunEnv, IN LPCONNNODE a_pstNode, IN LPTBUSHEAD a_pstHead)
{
    LPRELAY pstRelay = NULL;
    char szSrcAddr[32] = { 0 } ;
    char szDstAddr[32] = { 0 } ;
    LPTBUSADDRTEMPLET pstAddrTemplet ;
    int iRet = TBUS_SUCCESS;


    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstNode);
    assert(NULL != a_pstHead);


    /*����������Ѿ���������ͨ������ֱ��ʹ�ô�ͨ��*/
    if (NULL != a_pstNode->pstBindRelay)
    {
        pstRelay = a_pstNode->pstBindRelay;
        tlog_debug( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "tbusd_get_relay_by_conn<fd:%d type:%d>, conn have binded on relay<%d> , use it.",
                a_pstNode->iFd, a_pstNode->dwType, pstRelay->iID) ;
        return pstRelay;
    }else if (RELAY_FD_SERVER_MODE != a_pstNode->dwType)
    {
        /*������ӻ�û�а�ͨ������˵������ֻ���ǶԷ��������ģ���Ϊ�Լ������򿪵�����ֻ�ܷ�����tbusd_connect_peer��
         *���˺������ܱ�֤���������ض�ͨ���󶨵�*/
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,
                "tbusd_get_relay_by_conn, socket<%d> have not band relay, but its type<%d> isnot SERVER_MODE, impossible",
                a_pstNode->iFd, a_pstNode->dwType);
        return NULL;
    }

    /*�Է��򿪵����ӻ�û�к��κ�ͨ�������󶨹�ϵ���������Ϣ�е�Դ��Ŀ�ĵ�ַȥ��ͨ����
     *����ȴ���Ϣͷ��ȡ��Դ��Ŀ�ĵ�ַ
     */
    pstAddrTemplet = &a_pstRunEnv->pstShmGCIM->stHead.stAddrTemplet;
    STRNCPY (szSrcAddr, tbus_addr_nota_by_addrtemplet(pstAddrTemplet, a_pstHead->iSrc),
            sizeof(szSrcAddr)) ;
    STRNCPY (szDstAddr, tbus_addr_nota_by_addrtemplet(pstAddrTemplet, a_pstHead->iDst),
            sizeof(szDstAddr)) ;
    tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_DEBU, "tbusd_get_relay_by_conn, get pkg:src addr %s, dst addr %s on socket %d",
            szSrcAddr, szDstAddr, a_pstNode->iFd) ;


    /*������Ϣ��Դ��Ŀ�ĵ�ַȷ�������ݰ�����Ӧ�ð󶨵��Ǹ�relayͨ����*/
    pstRelay = trelay_channel_get(a_pstHead->iSrc, a_pstHead->iDst, a_pstRunEnv->pastTbusRelay,
            a_pstRunEnv->dwRelayCnt) ;
    if (NULL == pstRelay)
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tbusd_get_relay_by_conn, socket<%d> failed to bind relay by pkg<src:%s, dst:%s>",
                a_pstNode->iFd, szSrcAddr, szDstAddr) ;
        return NULL;
    }

    /*���ӶԶ˵ĵ�ַ�ͱ���relay���õĵ�ַ�Ƿ����������򲻽��ܴ�����*/
    iRet = tbusd_validate_peer_ip(a_pstRunEnv, pstRelay, a_pstNode);
    if (TBUS_SUCCESS != iRet)
    {
        tlog_error( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID, "the peer address of conn(socket:%d) is different to relay(ID��%d)'s configure",
                a_pstNode->iFd, pstRelay->iID);
        return NULL;
    }



    /*������ͨ��֮ǰû�а�����*/
    if (NULL == pstRelay->pConnNode)
    {
        tbusd_bind_conn_relay(a_pstNode, pstRelay, TBUSD_STATUS_SERVER_INIT);
        tlog_error( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "tbusd_get_relay_by_conn, relay<%d> bind conn <socket:%d type:%d>, so relay status become SERVER_INIT",
                pstRelay->iID, a_pstNode->iFd, a_pstNode->dwType) ;
    }else if (pstRelay->pConnNode != a_pstNode)
    {
        /*relay ͨ��֮ǰ�Ѿ���������,��������һ������Ҫ�󶨵���relayͨ���ϣ��󶨳�ͻ����������
         *��ֻ��ѡ������һ����
         * 1.�����ӱ����ǶԷ������������ӣ���Ϊ�Լ�������ֻ����tbusd_connect_peer
         *��������tbusd_connect_peer�лᱣ֤relay��conn���Լ������򿪵�����һ��
         * 2.֮ǰ�Լ��򿪵����Ӵ��ڡ������С�״̬
         * 3.���ڶ��ڶԷ�������������,��󶨵�ַ���tbusdһ�ˣ��Ͽ��Լ������ӣ����Է�������
         *��Ϊrelayͨ��������; ���󶨵�ַС��һ�ˣ��Ͽ��Է������ӣ������Լ���������Ϊrelayͨ��������

*/
        LPCONNNODE pstTmpNode = pstRelay->pConnNode;

        tlog_error( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "tbusd_get_relay_by_conn, connection conflict!! relay<ID: %d status:%d> "
                "have binded conn<socket:%d, type:%d >, new conn<socket:%d, type:%d >",
                pstRelay->iID, pstRelay->dwRelayStatus,
                pstTmpNode->iFd, pstTmpNode->dwType,
                a_pstNode->iFd, a_pstNode->dwType) ;


        if ((!pstTmpNode->bIsConnected) ||(a_pstHead->iSrc > a_pstHead->iDst))
        {
            tlog_error( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,  \
                    "tbusd_get_relay_by_conn, old conn(sock:%d is connected(%d)) or relay bind addr<%s> bigger than peer addr <%s>, so replace old conn(sock:%d) \
                    with new conn(sock:%d)",pstTmpNode->iFd, pstTmpNode->bIsConnected, szSrcAddr, szDstAddr, pstTmpNode->iFd, a_pstNode->iFd);
            tbusd_unbind_conn_relay(pstTmpNode, pstRelay);	/*ȡ���ϵ�conn��relayͨ���İ�*/
            tbusd_bind_conn_relay(a_pstNode, pstRelay, TBUSD_STATUS_SERVER_INIT);	/*���µ�����conn��relayͨ����*/
            pstTmpNode->bNeedFree = TBUSD_TRUE;;  /*�ϵ����ӽ��Ͽ�*/
            tbusd_destroy_conn(a_pstRunEnv, pstTmpNode, NULL);
        }else
        {
            tlog_error( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,  \
                    "tbusd_get_relay_by_conn, relay bind addr<%s> less than peer addr <%s>, so close new  conn (sock:%d)\
                    and use old conn(sock:%d)", szSrcAddr, szDstAddr,  a_pstNode->iFd,pstTmpNode->iFd );
            pstRelay = NULL;
            a_pstNode->bNeedFree = TBUSD_TRUE;
        }/*if (iBindAddr > iPeerAddr)*/

    }/*if (NULL == pstRelay->pConnNode)*/


    return pstRelay ;
}


/** ������conn��relayͨ�������󶨹�ϵ,������relayͨ����status
*/
void tbusd_bind_conn_relay(IN LPCONNNODE a_pstConn, IN LPRELAY a_pstRelay, IN int a_iRelayStatus)
{
    assert(NULL != a_pstConn);
    assert(NULL != a_pstRelay);

    a_pstConn->pstBindRelay = a_pstRelay;
    a_pstRelay->pConnNode = a_pstConn;
    a_pstRelay->dwRelayStatus = a_iRelayStatus;
    time(&a_pstRelay->tLastHeartbeat);
}

/** �������conn��relayͨ��֮��İ󶨹�ϵ,����relayͨ����status����ΪTBUSD_STATUS_NO_CONNECTION
*/
void tbusd_unbind_conn_relay(IN LPCONNNODE a_pstConn, IN LPRELAY a_pstRelay)
{
    assert(NULL != a_pstConn);
    assert(NULL != a_pstRelay);

    a_pstConn->pstBindRelay = NULL;
    a_pstRelay->pConnNode = NULL;
    a_pstRelay->dwRelayStatus = TBUSD_STATUS_NO_CONNECTION;
}


int tbusd_proc_handshake_pkg(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN LPTBUSHEAD a_pstHead)
{
    int iRet = TBUS_SUCCESS;
    int iRecvSeq;
    int iSendSeq;
    LPTBUSCHANNEL pstChannel;

    assert(NULL != a_pstHead);
    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);

    /*����������ݰ�����ȡ��������ݰ�*/
    switch(a_pstHead->bCmd)
    {
        case TBUS_HEAD_CMD_HANDSHAKE_CLI_SYN:
        case TBUS_HEAD_CMD_HANDSHAKE_SVR_SYN_ACK:
        case TBUS_HEAD_CMD_HANDSHAKE_CLI_ACK:
            break;
        default:
            tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "tbusd_proc_handshake_pkg, the pkg is not handshake pkg<flag: 0x%x cmd:%d>",
                    a_pstHead->bFlag, a_pstHead->bCmd) ;
            return -1;
    }

    iRecvSeq = a_pstHead->iRecvSeq;
    iSendSeq = a_pstHead->iSeq;
    tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
            "tbusd_proc_handshake_pkg, pkg<flag:%d cmd:%d>, recvseq:%u Synflag: 0x%x seq<%u> relay<%d> status:%d",
            a_pstHead->bFlag, a_pstHead->bCmd, iRecvSeq, a_pstHead->stExtHead.stSynHead.bSynFlag,
            iSendSeq, a_pstRelay->iID, a_pstRelay->dwRelayStatus);

    /*����relayĿǰ��״̬���д���*/
    pstChannel = &a_pstRelay->stChl;
    switch(a_pstRelay->dwRelayStatus)
    {
        case TBUSD_STATUS_SERVER_INIT:
            {
                if (TBUS_HEAD_CMD_HANDSHAKE_CLI_SYN != a_pstHead->bCmd)
                {
                    tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                            "relay<%d> status is TBUSD_STATUS_SERVER_INIT(%d), only recv cmd=CLI_SYN handshake pkg, but pkgcmd=%d",
                            a_pstRelay->iID, TBUS_HEAD_CMD_HANDSHAKE_CLI_SYN, a_pstHead->bCmd);
                    iRet = TBUS_ERROR;
                }else
                {
                    iRet = tbusd_relaysvr_recv_syn(a_pstRunEnv, a_pstRelay, iRecvSeq,
                            a_pstHead->stExtHead.stSynHead.bSynFlag);
                }
            }
            break;
        case TBUSD_STATUS_SERVER_SYN_RECV:
            {
                if (TBUS_HEAD_CMD_HANDSHAKE_CLI_ACK != a_pstHead->bCmd)
                {
                    tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                            "relay<%d> status is TBUSD_STATUS_SERVER_SYN_RECV(%d), only recv cmd=CLI_ACK handshake pkg, but pkgcmd=%d",
                            a_pstRelay->iID, TBUS_HEAD_CMD_HANDSHAKE_CLI_ACK, a_pstHead->bCmd);
                    iRet = TBUS_ERROR;
                }else
                {
                    iRet = tbusd_relaysvr_recv_ack(a_pstRunEnv, a_pstRelay, iSendSeq);
                }
            }
            break;
        case TBUSD_STATUS_CLIENT_SYN_SEND:
            {
                if (TBUS_HEAD_CMD_HANDSHAKE_SVR_SYN_ACK != a_pstHead->bCmd)
                {
                    tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                            "relay<%d> status is TBUSD_STATUS_CLIENT_SYN_SEND(%d), only recv cmd=SVR_SYN_ACK handshake pkg, but pkgcmd=%d",
                            a_pstRelay->iID, TBUS_HEAD_CMD_HANDSHAKE_CLI_ACK, a_pstHead->bCmd);
                    iRet = TBUS_ERROR;
                }else
                {
                    iRet = tbusd_relaycli_recv_syn_ack(a_pstRunEnv, a_pstRelay, iRecvSeq,
                            a_pstHead->stExtHead.stSynHead.bSynFlag, iSendSeq);
                }
            }
            break;
        default:
            tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                    "relay<%d> status is %d, cannot proc handshake pkg", a_pstRelay->iID, a_pstRelay->dwRelayStatus);
            iRet = TBUS_ERROR;
    }

    return iRet;
}

/** relayͨ����TBUSD_STATUS_SERVER_INIT״̬�յ��������ݰ�
  @param[in] a_pstRunEnv �����������Ļ���
  @param[in] a_pstRelay ����ͨ��
  @param[in] a_dwRecvSeq �Է����ܵ������һ�����ݰ������к�
  @param[in] a_bRecvSeqFlag �������ݰ��ı�־λ
  */
int tbusd_relaysvr_recv_syn(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN  int a_iRecvSeq,
        IN unsigned char a_bRecvSeqFlag)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSCHANNEL pstChannel;
    CHANNELVAR *pstVar = NULL ;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);
    assert(TBUSD_STATUS_SERVER_INIT == a_pstRelay->dwRelayStatus);

    pstChannel = &a_pstRelay->stChl;
    pstVar = TBUS_CHANNEL_VAR_GET(pstChannel);
    tlog_trace(g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,"dwHead:%u dwGet:%u dwTail:%u dwSize:%u iseq:%d",
            pstVar->dwHead, pstVar->dwGet, pstVar->dwTail, pstVar->dwSize, pstVar->iSeq);
    switch(a_bRecvSeqFlag)
    {
        case TBUS_SUNFLAG_CONTINUE_RECV:
            {
                if (0 != pstVar->chGStart)
                {
                    /*�Ѿ���Է����͹����ݣ��Է�Ҳ���յ������ݣ����ƶ����Ͷ��е�ͷָ��*/
                    tbusd_syn_seq(pstChannel, (int)a_iRecvSeq);
                    pstVar->dwGet = pstVar->dwHead;
                }
                iRet = tbusd_relaysvr_send_syn_ack(a_pstRunEnv, a_pstRelay);
            }
            break;
        case TBUS_SYNFLAG_NO_RECV:
            {/*�Է�֮ǰû���յ�������*/
                pstVar->dwGet = pstVar->dwHead;
                iRet = tbusd_relaysvr_send_syn_ack(a_pstRunEnv, a_pstRelay);
            }
            break;
        default:
            tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                    "relay<%d> status is TBUSD_STATUS_SERVER_INIT, invalid RecvSeqFlag %d",
                    a_pstRelay->iID, a_bRecvSeqFlag);
            iRet = TBUS_ERROR;
    }

    if (0 == iRet)
    {
        a_pstRelay->dwRelayStatus = TBUSD_STATUS_SERVER_SYN_RECV;
        tlog_info( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "relay<%d> recv syn and send syn_ack successfully, so its status become TBUSD_STATUS_SERVER_SYN_RECV \
                from TBUSD_STATUS_SERVER_INIT ", a_pstRelay->iID);
    }

    return iRet;
}

/** ��������ΪTBUS_HEAD_CMD_HANDSHAKE_SVR_SYN_ACK��������Ϣ
 *��Է��ṩ���ܰ������кźͷ������ݰ������к�
 */
int tbusd_relaysvr_send_syn_ack(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay)
{
    int iRet = TBUS_SUCCESS;
    TBUSHEAD stHead;
    LPTBUSCHANNEL pstChannel;
    CHANNELVAR *pstVar = NULL ;

    LPCONNNODE pstConn;
    int iSendSeq;

    char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
    int iHeadLen = 0;


    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);
    assert(NULL != a_pstRelay->pConnNode);

    /*���������Ϣͷ��*/
    pstChannel = &a_pstRelay->stChl;
    pstVar = TBUS_CHANNEL_VAR_GET(pstChannel);
    tlog_trace(g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,"dwHead:%u dwGet:%u dwTail:%u dwSize:%u iRescseq:%d",
            pstVar->dwHead, pstVar->dwGet, pstVar->dwTail, pstVar->dwSize, pstVar->iRecvSeq);
    TBUS_GET_PKGSEQ_BY_POS(iSendSeq, pstChannel,	pstVar->dwGet);
    tbusd_fill_syn_head(&stHead, pstChannel, iSendSeq,
            TBUS_HEAD_CMD_HANDSHAKE_SVR_SYN_ACK);


    /*���������Ϣ��,���ý��ܰ����к�*/
    pstVar = TBUS_CHANNEL_VAR_PUSH(pstChannel);
    if ( 0 == pstVar->chPStart )
    {
        stHead.stExtHead.stSynHead.bSynFlag = TBUS_SYNFLAG_NO_RECV;
        stHead.iRecvSeq = RELAY_INIT_SEQUENCE_VALUE;
    }else
    {
        stHead.stExtHead.stSynHead.bSynFlag = TBUS_SUNFLAG_CONTINUE_RECV;
        stHead.iRecvSeq = pstVar->iRecvSeq;
    }

    iHeadLen = sizeof(szNet);
    iRet = tbus_encode_head(&stHead, &szNet[0], &iHeadLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,  "failed to pack tbus head,iRet %x on relay %d",
                iRet, a_pstRelay->iID);
        return iRet;
    }

    /*ȡͨ�������ӹ�����*/
    pstConn = (LPCONNNODE)a_pstRelay->pConnNode;
    tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
            "tbusd_relaysvr_send_syn_ack, send SVR_SYN_ACK pkg<chPStart %i, iRecvSeq %u dwSendSeq %u > \
            on socket %i", pstVar->chPStart, stHead.iRecvSeq, iSendSeq, pstConn->iFd) ;


    /*��������*/
    iRet = tbusd_send_all(pstConn, &szNet[0], iHeadLen);
    if (0 != iRet)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "tbusd_relaysvr_send_syn_ack, relay<%d> failed to send pkg",
                a_pstRelay->iID) ;
    }

    return iRet;
}

/** relayͨ����TBUSD_STATUS_CLIENT_SYN_SEND״̬�յ��������ݰ�SVR_SYN_SCK
  @param[in] a_pstRunEnv �����������Ļ���
  @param[in] a_pstRelay ����ͨ��
  @param[in] a_dwRecvSeq �Է����ܵ������һ�����ݰ������к�
  @param[in] a_bRecvSeqFlag �������ݰ��ı�־λ
  @param[in] a_dwSendSeq �Է��������ݰ��ı�־λ
  */
int tbusd_relaycli_recv_syn_ack(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN  int a_iRecvSeq,
        IN unsigned char a_bRecvSeqFlag, IN  int a_iSendSeq)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSCHANNEL pstChannel;
    CHANNELVAR *pstVar = NULL ;


    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);
    assert(TBUSD_STATUS_CLIENT_SYN_SEND == a_pstRelay->dwRelayStatus);


    pstChannel = &a_pstRelay->stChl;

    /*����ack���ٴθ��½������ݰ����к�*/
    pstVar = TBUS_CHANNEL_VAR_PUSH(pstChannel);
    if (pstVar->iRecvSeq != (a_iSendSeq -1))
    {
        tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "tbusd_relaycli_recv_syn_ack��recvseq<%d> of relay<%d>, sendseq<%u> of peer, so change recvseq to %u ",
                pstVar->iRecvSeq, a_pstRelay->iID, a_iSendSeq, a_iSendSeq -1);
        pstVar->iRecvSeq = a_iSendSeq -1;
    }

    pstVar = TBUS_CHANNEL_VAR_GET(pstChannel);
    switch(a_bRecvSeqFlag)
    {
        case TBUS_SUNFLAG_CONTINUE_RECV:
            {
                if (0 != pstVar->chGStart)
                {
                    /*�Ѿ���Է����͹����ݣ��Է�Ҳ���յ������ݣ����ƶ����Ͷ��е�ͷָ��*/
                    tbusd_syn_seq(pstChannel, (int)a_iRecvSeq);
                    pstVar->dwGet = pstVar->dwHead;
                }
                iRet = tbusd_relaycli_send_ack(a_pstRunEnv, a_pstRelay);
            }
            break;
        case TBUS_SYNFLAG_NO_RECV:
            {/*�Է�֮ǰû���յ�������*/
                pstVar->dwGet = pstVar->dwHead;
                iRet = tbusd_relaycli_send_ack(a_pstRunEnv, a_pstRelay);
            }
            break;
        default:
            tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                    "tbusd_relaycli_recv_syn_ack, relay<%d> status is TBUS_HEAD_CMD_HANDSHAKE_SVR_SYN_ACK, invalid RecvSeqFlag %d",
                    a_pstRelay->iID, a_bRecvSeqFlag);
            iRet = TBUS_ERROR;
    }

    if (0 == iRet)
    {
        LPCONNNODE pstConn = a_pstRelay->pConnNode;
        assert(NULL != pstConn);
        pstConn->iLastAckSeq = pstVar->iRecvSeq;
        a_pstRelay->dwRelayStatus = TBUSD_STATUS_ESTABLISHED;
        time(&a_pstRelay->tLastHeartbeat);
        tlog_info( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
			"relay(%d) recv syn_ack successfully, so its status become TBUSD_STATUS_ESTABLISHED \
                from TBUSD_STATUS_CLIENT_SYN_SEND ", a_pstRelay->iID);
    }

    return iRet;
}

/** ��������ΪTBUS_HEAD_CMD_HANDSHAKE_CLI_ACK��������Ϣ
 *��Է��ṩ�������ݰ������к�
 */
int tbusd_relaycli_send_ack(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay)
{
    int iRet = TBUS_SUCCESS;
    TBUSHEAD stHead;
    LPTBUSCHANNEL pstChannel;
    CHANNELVAR *pstVar = NULL ;
    LPCONNNODE pstConn;
    int iSendSeq;
    char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
    int iHeadLen = 0;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);
    assert(NULL != a_pstRelay->pConnNode);

    /*���������Ϣͷ��, ���÷������ݰ����к�*/
    pstChannel = &a_pstRelay->stChl;
    pstVar = TBUS_CHANNEL_VAR_GET(pstChannel);
    tlog_trace(g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,"dwHead:%u dwGet:%u dwTail:%u dwSize:%u iRescseq:%d",
            pstVar->dwHead, pstVar->dwGet, pstVar->dwTail, pstVar->dwSize, pstVar->iRecvSeq);
    TBUS_GET_PKGSEQ_BY_POS(iSendSeq, pstChannel,pstVar->dwGet);
    tbusd_fill_syn_head(&stHead, pstChannel, iSendSeq,
            TBUS_HEAD_CMD_HANDSHAKE_CLI_ACK);


    /*�����ܰ����к�����ΪĿǰ�Ľ������к�*/
    pstVar = TBUS_CHANNEL_VAR_PUSH(pstChannel);
    stHead.iRecvSeq = pstVar->iRecvSeq;
    stHead.stExtHead.stSynHead.bSynFlag = TBUS_SUNFLAG_CONTINUE_RECV;

    iHeadLen = sizeof(szNet);
    iRet = tbus_encode_head(&stHead, &szNet[0], &iHeadLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "failed to pack tbus head,iRet %x", iRet);
        return iRet;
    }


    pstConn = (LPCONNNODE)a_pstRelay->pConnNode;
    tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
            "tbusd_relaycli_send_ack, send CLI_ACK pkg<iRecvSeq %u dwSendSeq %u > \
            on socket %i", pstVar->iRecvSeq, iSendSeq, pstConn->iFd) ;


    /*��������*/
    iRet = tbusd_send_all(pstConn, &szNet[0], iHeadLen);
    if (0 != iRet)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "tbusd_relaycli_send_ack, relay<%d> failed to send pkg",
                a_pstRelay->iID) ;
    }

    return iRet;
}

/** relayͨ����TBUSD_STATUS_SERVER_SYN_RECV״̬�յ��������ݰ�SVR_SYN_SCK
  @param[in] a_pstRunEnv �����������Ļ���
  @param[in] a_pstRelay ����ͨ��
  @param[in] a_dwSendSeq �Է��������ݰ��ı�־λ
  */
int tbusd_relaysvr_recv_ack(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN  int a_iSendSeq)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSCHANNEL pstChannel;
    CHANNELVAR *pstVar = NULL ;
    LPCONNNODE pstConn;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);
    assert(TBUSD_STATUS_SERVER_SYN_RECV == a_pstRelay->dwRelayStatus);


    pstChannel = &a_pstRelay->stChl;

    /*����ack���ٴθ��½������ݰ����к�*/
    pstVar = TBUS_CHANNEL_VAR_PUSH(pstChannel);
    if (pstVar->iRecvSeq  != (a_iSendSeq -1))
    {
        tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "tbusd_relaycli_recv_syn_ack��recvseq<%d> of relay<%d>, sendseq<%u> of peer, so change recvseq to %d ",
                pstVar->iRecvSeq, a_pstRelay->iID, a_iSendSeq, a_iSendSeq -1);
        pstVar->iRecvSeq = a_iSendSeq -1;
    }

    a_pstRelay->dwRelayStatus = TBUSD_STATUS_ESTABLISHED;
    pstConn = a_pstRelay->pConnNode;
    assert(NULL != pstConn);
    pstConn->iLastAckSeq = pstVar->iRecvSeq;
    tlog_info( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
            "tbusd_relaysvr_recv_ack, relay<%d> recv cli_ack successfully, so its status become TBUSD_STATUS_ESTABLISHED \
            from TBUSD_STATUS_SERVER_SYN_RECV ", a_pstRelay->iID);


    return iRet;
}




int tbusd_syn_seq( IN const TBUSCHANNEL *a_ptChannel, IN const  int a_iRecvSeq )
{
    CHANNELVAR *pstVar = NULL ;
    char *sData = NULL ;
    int iPkgLen;
    int	iHeadSeq = 0; /*��δȷ�ϵ����ݰ������к�*/
    int	iGetSeq = 0 ; /*��һ����δ���͵����ݰ������������͵����ݰ������*/
    LPTBUSHEAD pstHead;
    int iRet = 0;

    assert(NULL != a_ptChannel);
    assert(NULL !=  gs_stRelayEnv.pstConf);
    if (0 == gs_stRelayEnv.pstConf->iNeedAckPkg)
    {
        return TBUS_SUCCESS;
    }

    pstVar = TBUS_CHANNEL_VAR_GET(a_ptChannel);
    if (pstVar->dwHead == pstVar->dwGet)
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_DEBU,
                "tbusd_syn_seq, in send queue, headpos is equal to getpos, neednot ack seq:%u", a_iRecvSeq);
        return TBUS_SUCCESS;
    }

    /*ȡ��һ����δ���͵����ݰ������к�*/
    sData = TBUS_CHANNEL_QUEUE_GET(a_ptChannel);
    TBUS_GET_PKGSEQ_BY_POS(iGetSeq, a_ptChannel,pstVar->dwGet);


    do
    {
        TBUS_CHECK_QUEUE_HEAD_VAR(sData, pstVar->dwSize, pstVar->dwHead);
        if (pstVar->dwHead == pstVar->dwTail)
        {
            break;
        }
        pstHead = (LPTBUSHEAD)(sData + pstVar->dwHead);
        iPkgLen = pstHead->bHeadLen + ntohl(pstHead->iBodyLen);
        if ((0 > iPkgLen) || (iPkgLen > (int)pstVar->dwSize))
        {
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,
                    "invalid pkglen: %d, dwHead:%u dwGet:%u dwTail:%u dwSize:%u, so clear ",
                    iPkgLen, pstVar->dwHead, pstVar->dwGet, pstVar->dwTail, pstVar->dwSize);
            pstVar->dwGet = pstVar->dwHead = pstVar->dwTail;
            break;
        }/*if (0 != iRet)*/
        iHeadSeq = ntohl(pstHead->iSeq);

        if (a_iRecvSeq - iHeadSeq < 0 || a_iRecvSeq - iGetSeq >= 0)
        {
            tlog_warn(g_ptRelayLog, 0,0,
                    "tbusd_syn_seq, invalid ackseq %u of channel(%d<-->%d), for headseq %u and getseq %u(getseq-headseq=:%d)",
                    (unsigned int)a_iRecvSeq, a_ptChannel->pstHead->astAddr[0],
                    a_ptChannel->pstHead->astAddr[1], (unsigned int)iHeadSeq, (unsigned int)iGetSeq, iGetSeq-iHeadSeq);
            iRet = TBUS_ERR_CHANNEL_CONFUSE;
            break;
        }

        TBUS_MOVETO_NEXT_PKG(iRet, iPkgLen, a_ptChannel->pstHead, pstVar->dwSize, pstVar->dwHead,
                pstVar->dwTail);
        if (0 != iRet)
        {
            pstVar->dwGet = pstVar->dwHead = pstVar->dwTail;
            tlog_log(g_ptRelayLog, TBUSD_PRIORITY_FATAL,0,0,
                    "move to next pkg  failed, pkglen is %d ", iPkgLen);
            iRet =  TBUS_ERR_CHANNEL_CONFUSE;
            break;
        }
    }while ((a_iRecvSeq - iHeadSeq) > 0);


    return iRet;
}

/** �������к�ͬ�����ݰ�
  @param[in] a_pstConn -- �������ݰ�
  @param[in] a_pstChannel relayͨ�����ݽṹָ��
  @param[out] a_pstRelayStat ͳ�����ݽṹ

  @retval 0 -- success
  @retval !0 -- failed
  */
int tbusd_send_synseq_pkg(IN LPCONNNODE a_pstConn, IN LPRELAY a_pstRelay, LPTBUSRELAYCONNSTAT a_pstRelayStat)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSHEAD pstHead;
    LPTBUSCHANNEL pstChannel;
    LPCHANNELVAR pstVar;
    int iHeadLen;
    char *pch;

    if (TBUSD_CONN_HAVE_REMAIN_DATA(a_pstConn))
    {
        return TBUS_SUCCESS;	/*ʣ�����ݻ�û�з�����,���Ȳ�����ͬ������*/
    }
    if (!a_pstConn->bNeedSynSeq)
    {
        return TBUS_SUCCESS; /*���û��������Ҫ����ͬ�����кŵı�־,�򲻼������д���*/
    }

    /*���ͬ�����ݰ�*/
    pstChannel = &a_pstRelay->stChl;
    pstVar = TBUS_CHANNEL_VAR_PUSH(pstChannel);



    a_pstConn->stSendMng.pstSendHead = &gs_stRelayEnv.stSendHead;
    pstHead = a_pstConn->stSendMng.pstSendHead;
    tbusd_fill_syn_head(pstHead, pstChannel, pstVar->iRecvSeq, TBUS_HEAD_CMD_ACKSEQ);
    pstHead->iRecvSeq = pstVar->iRecvSeq;

    iHeadLen = sizeof(a_pstConn->stSendMng.pszSynData);
    pch = a_pstConn->stSendMng.pszSynData;
    iRet =  tbus_encode_head(pstHead, pch, &iHeadLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "failed to encode syn head pkg  on socket %d of relay %d, ret %x",
                a_pstConn->iFd, a_pstRelay->iID, iRet ) ;
        return iRet;
    }

    /*���ñ�־λ,���ʹ���Ϣ*/
    tlog_debug( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
            "send ackseq %u pkg on socket %d of relay %d",
            pstVar->iRecvSeq, a_pstConn->iFd, a_pstRelay->iID) ;
    a_pstConn->stSendMng.enRemainDataType = TBUSD_DATA_TYPE_SYN;
    a_pstConn->stSendMng.iSendPos = 0;
    a_pstConn->stSendMng.iSendBuff = iHeadLen;
    iRet = tbusd_send_remain_data(a_pstConn, pstChannel, a_pstRelay) ;
    if ( 0 != iRet )
    {
        tlog_error( g_ptRelayLog, a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                "tbusd_send_remain_data failed to send remain data on socket %d of relay %d, ret %d",
                a_pstConn->iFd, a_pstRelay->iID, iRet ) ;
        return iRet;
    }
    a_pstRelayStat->dwSendSynPkgCount++;

    a_pstConn->bNeedSynSeq = TBUSD_FALSE;
    a_pstConn->iLastAckSeq = pstVar->iRecvSeq;

    return iRet;
}



/** ��ȡ��������ѭ���������ݰ��������Ŀ
*/
int tbusd_get_pkglimit_perrecv(IN LPRELAYENV a_pstRunEnv)
{
    int	i = 0;
    RELAY *ptRelay = NULL ;
    int iCount = 0;
    LPTBUSD pstConf;
    int iPkgCount;


    assert(NULL != a_pstRunEnv);
    pstConf = a_pstRunEnv->pstConf;
    assert(NULL != pstConf);


    for ( i=0; i<(signed int)a_pstRunEnv->dwRelayCnt; i++ )
    {
        ptRelay = a_pstRunEnv->pastTbusRelay[i] ;
        if (!TBUSD_RELAY_IS_ENABLE(ptRelay))
        {
            continue ;
        }
        if (TBUSD_STATUS_ESTABLISHED == ptRelay->dwRelayStatus)
        {
            iCount++ ;
        }
    }/*for ( i=0; i<(signed int)pstGlobalRelay->dwRelayCnt; i++ )*/

    if (0 == iCount)
    {
        iPkgCount = pstConf->iMaxRecvPerloop;
    }else
    {
        iPkgCount = pstConf->iMaxRecvPerloop / iCount;
    }


    return iPkgCount;
}





