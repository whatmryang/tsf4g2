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

#define RELAY_INIT_SEQUENCE_VALUE	0		/**<接收数据的初始seq*/
#define TBUS_MAX_CONNECT_PEER_GAP	30		/*relay通道没有建立时重连对方的最大时间间隔,单位为秒*/

#define TBUSD_PKG_NUM_RECVED_PERACKSEQ                 	10 	/* 接收到此宏定义指定的数据包后发送序列化确认包 */
#define TBUSD_PKG_BYTES_RECVED_PERACKSEQ                20480 	 	/* 接收到此宏定义指定的数据长度后发送序列化确认包 */

/*能容忍的未确认数据包的最大数目，如果发送的未确认数据包超过此阀值，则暂停发送数据*/
#define TBUS_MAX_SENDED_PKG_NUM_UNACK	300


/*将连接描述符加入连接管理器
@param[in] a_pstRunEnv relay服务器运行上下文环境
@param[in] a_iFd 连接描述符
@param[in] a_iFdMode 连接描述的类型
@param[in] a_iEpEvent 关注的epool事件
*retval succ 连接管理节点的指针

*retval failed 返回NULL
*/
LPCONNNODE tbusd_create_conn(IN LPRELAYENV  a_pstRunEnv, IN int a_iFd, IN int a_iFdMode, IN int a_iEpEvent);

/** 关闭指定连接管理器
@param[in] a_pstRunEnv relay服务器运行上下文环境
@param[in] a_pstNode 连接管理器的指针
@param[in] a_pstEvent 此事件对应的epoll事件
*/
void tbusd_destroy_conn(IN LPRELAYENV  a_pstRunEnv, IN LPCONNNODE a_pstNode, IN TPOLL_EVENT* a_pstEvent);


/** 检查relay通道 ，如果某个通道尚未建立连接，就主动与对方建立连接，并发送
*SYN握手数据包
*/
int tbusd_connect_peer(IN LPRELAYENV  a_pstRunEnv, IN time_t tCur, IN LPRELAY a_pstRelay);

/**处理连接通道的握手过程
*@param[in] a_pstRunEnv tbusd上下文环境数据结构指针
*@param[in,out] a_pstNode 连接通道上连接管理数据结构
@return 成功处理返回0，否则返回非零值
*/
int tbusd_handshake(IN LPRELAYENV a_pstRunEnv, INOUT LPCONNNODE a_pstNode);





/** 判断连接是否已经建立
*/
int tbusd_connecting_check(IN LPRELAYENV a_pstRunEnv, IN LPCONNNODE a_pstNode, IN struct epoll_event* a_ptEvent ) ;

#if defined(_WIN32) || defined(_WIN64)
/** 检测非阻塞连接是否已经成功
 * @param[in] a_pstRunEnv 服务器上下文环境
 * @param[in] a_pstRelay, 路由数据结构
 * @return 成功返回0, 否则返回 !0
 */
int tbusd_check_nonblock_connect(IN LPRELAYENV  a_pstRunEnv, IN time_t tCur, IN LPRELAY a_pstRelay);
#endif



/** 发送序列号同步数据包
@param[in] a_pstConn -- 连接数据包
@param[in] a_pstRelay relay通道数据结构指针
@param[out] a_pstRelayStat 统计数据结构

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





/** 获取单个处理循环接受数据包的最大数目
*/
int tbusd_get_pkglimit_perrecv(IN LPRELAYENV a_pstRunEnv);

#endif /**< TBUSD_SYNC_H */

