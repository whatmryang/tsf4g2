/** @file $RCSfile: tbusd_sync.h,v $
  Invoked function header file for tsf4g-tbus relay module
  $Id: tbusd_sync.h,v 1.8 2009-11-09 05:54:28 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-11-09 05:54:28 $
@version $Revision: 1.8 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef TBUSD_SYNC_H
#define TBUSD_SYNC_H

#include "tbusd_misc.h"
#include "tbusd_desc.h"
#include "tbusd.h"

#define RELAY_INIT_SEQUENCE_VALUE	0		/**<�������ݵĳ�ʼseq*/
#define TBUS_MAX_CONNECT_PEER_GAP	30		/*relayͨ��û�н���ʱ�����Է������ʱ����,��λΪ��*/

#define TBUSD_PKG_NUM_RECVED_PERACKSEQ                 	10 	/* ���յ��˺궨��ָ�������ݰ��������л�ȷ�ϰ� */
#define TBUSD_PKG_BYTES_RECVED_PERACKSEQ                20480 	 	/* ���յ��˺궨��ָ�������ݳ��Ⱥ������л�ȷ�ϰ� */

/*�����̵�δȷ�����ݰ��������Ŀ��������͵�δȷ�����ݰ������˷�ֵ������ͣ��������*/
#define TBUS_MAX_SENDED_PKG_NUM_UNACK	300


/*�������������������ӹ�����
@param[in] a_pstRunEnv relay���������������Ļ���
@param[in] a_iFd ����������
@param[in] a_iFdMode ��������������
@param[in] a_iEpEvent ��ע��epool�¼�
*retval succ ���ӹ���ڵ��ָ��

*retval failed ����NULL
*/
LPCONNNODE tbusd_create_conn(IN LPRELAYENV  a_pstRunEnv, IN int a_iFd, IN int a_iFdMode, IN int a_iEpEvent);

/** �ر�ָ�����ӹ�����
@param[in] a_pstRunEnv relay���������������Ļ���
@param[in] a_pstNode ���ӹ�������ָ��
@param[in] a_pstEvent ���¼���Ӧ��epoll�¼�
*/
void tbusd_destroy_conn(IN LPRELAYENV  a_pstRunEnv, IN LPCONNNODE a_pstNode, IN TPOLL_EVENT* a_pstEvent);


/** ���relayͨ�� �����ĳ��ͨ����δ�������ӣ���������Է��������ӣ�������
*SYN�������ݰ�
*/
int tbusd_connect_peer(IN LPRELAYENV  a_pstRunEnv, IN time_t tCur, IN LPRELAY a_pstRelay);

/**��������ͨ�������ֹ���
*@param[in] a_pstRunEnv tbusd�����Ļ������ݽṹָ��
*@param[in,out] a_pstNode ����ͨ�������ӹ������ݽṹ
@return �ɹ�������0�����򷵻ط���ֵ
*/
int tbusd_handshake(IN LPRELAYENV a_pstRunEnv, INOUT LPCONNNODE a_pstNode);





/** �ж������Ƿ��Ѿ�����
*/
int tbusd_connecting_check(IN LPRELAYENV a_pstRunEnv, IN LPCONNNODE a_pstNode, IN struct epoll_event* a_ptEvent ) ;

#if defined(_WIN32) || defined(_WIN64)
/** �������������Ƿ��Ѿ��ɹ�
 * @param[in] a_pstRunEnv �����������Ļ���
 * @param[in] a_pstRelay, ·�����ݽṹ
 * @return �ɹ�����0, ���򷵻� !0
 */
int tbusd_check_nonblock_connect(IN LPRELAYENV  a_pstRunEnv, IN time_t tCur, IN LPRELAY a_pstRelay);
#endif



/** �������к�ͬ�����ݰ�
@param[in] a_pstConn -- �������ݰ�
@param[in] a_pstRelay relayͨ�����ݽṹָ��
@param[out] a_pstRelayStat ͳ�����ݽṹ

@retval 0 -- success
@retval !0 -- failed
*/
int tbusd_send_synseq_pkg(IN LPCONNNODE a_pstConn,IN LPRELAY a_pstRelay, LPTBUSRELAYCONNSTAT a_pstRelayStat);

/**
@brief Verify sync sequence, the sequence a_iAuthSeq should be in [HeadSeq, GetSeq)

@param[in] a_ptChannel -- channel information
@param[in] a_iRecvSeq -- received sequence needs to be verified

@retval 0 -- success, verified ok
@retval !0 -- failed

@note
*/
int tbusd_syn_seq ( IN const TBUSCHANNEL *a_ptChannel, IN const int a_iRecvSeq ) ;





/** ��ȡ��������ѭ���������ݰ��������Ŀ
*/
int tbusd_get_pkglimit_perrecv(IN LPRELAYENV a_pstRunEnv);

#endif /**< TBUSD_SYNC_H */

