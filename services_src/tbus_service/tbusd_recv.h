/** @file $RCSfile: tbusd_recv.h,v $
  Invoked function header file for tsf4g-tbus relay module
  $Id: tbusd_recv.h,v 1.4 2008-10-20 09:52:17 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008-10-20 09:52:17 $
@version $Revision: 1.4 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef TBUSD_RECV_H
#define TBUSD_RECV_H

#include "tapp/tapp.h"
#include "tbusd.h"

/** 接收各relay通道上的数据包,并根据接收包的情况控制发送数据包的数目
@param[in] a_pstRunEnv 服务器运行上下文环境
@param[out] a_piMaxSendPkg 最大可以发送数据包
@return 处理成功返回0, 否则返回非零值
*/
int tbusd_proc_recv(IN TAPPCTX *a_pstAppCtx, IN LPRELAYENV a_pstRunEnv, OUT int *a_piMaxSendPkg);


/**从链接收数据缓冲区中接收数据包
*@param[in] a_pstRunEnv 服务器运行上下文
*@param[in] a_pstConn 链接数据结构指针
*@param[in] a_pstRelay relay通道数据结构
*return 成功返回0，失败返回非零值
*/
int tbusd_recv_pkg(IN LPRELAYENV a_pstRunEnv, IN LPCONNNODE a_pstConn, IN LPRELAY a_pstRelay);



/**
  @brief Processed received data

  @param[in] a_pstRelay -- relay通道
  @param[in] a_pstConn -- connection of relay
  @param[in] a_pstHead -- tbus head of data needs to be processed
  @param[in] a_pszPkg  buffer of data
  @param[in] a_iPkgLen len of data
  @retval 0 -- success, processed data ok
  @retval !0 -- failed

  @note
*/
int tbusd_proc_pkg_recved (IN LPRELAY a_pstRelay, IN LPTBUSRELAYCONNSTAT a_pstRelayStat, 
						   IN LPTBUSHEAD a_pstHead, IN const char *a_pszPkg, IN int a_iPkgLen) ;


/**
  @brief Stored received data into local TBUSCHANNEL and updated TBUSCHANNELVAR information.
   Set TBUSCHANNELVAR.chPStart = 1, indicated local TBUSCHANNEL has began to receive data from remote relay

  @param[in] a_ptTBUSCHANNEL -- TBUSCHANNEL information
  @param[in] a_ptBusPkg -- data needs to be stored
  @param[in] a_pstHead -- tbus head of data needs to be processed
  @param[in] a_pszPkg  buffer of data
  @param[in] a_iPkgLen len of data
  @retval 0 -- success, stored data ok
  @retval !0 -- failed

  @note
*/
int tbusd_push_pkg ( IN const TBUSCHANNEL *a_ptTBUSCHANNEL, IN LPTBUSHEAD a_pstHead, IN const char *a_pszPkg, IN int a_iPkgLen) ;






#endif /**< TBUSD_RECV_H */

