/** @file $RCSfile: tbusd_misc.h,v $
  Invoked function header file for tsf4g-tbus relay module
  $Id: tbusd_misc.h,v 1.5 2009-05-14 06:58:50 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-05-14 06:58:50 $
@version $Revision: 1.5 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef TBUSD_MISC_H
#define TBUSD_MISC_H

#include "pal/pal.h"
#include "tbus/tbus_macros.h"
#include "tbus/tbus_misc.h"
#include "tbus/tbus_kernel.h"
#include "tbusd_sync.h"
#include "tbusd_send.h"
#include "tbusd_recv.h"

#include "tbusd_desc.h"

#if defined(_WIN32) || defined(_WIN64)
#define TBUSD_GET_LAST_ERROR_CODE GetLastError()
#define TBUSD_GET_SOCKET_ERROR_CODE WSAGetLastError()
#else
#define TBUSD_GET_LAST_ERROR_CODE errno
#define TBUSD_GET_SOCKET_ERROR_CODE errno
#endif

/**
  @brief Locate local channel information index via specified source and destination address

  @param[in] a_iSrcAddr -- source address
  @param[in] a_iDstAddr -- destination address
  @param[in] a_pptRelay -- local channel information set
  @param[in] a_iRelayCnt -- local channel information set count

  @return success：relay通道的指针; failed：返回NULL
  @retval < 0 -- failed

  @note
*/
LPRELAY trelay_channel_get (IN const TBUSADDR a_iSrcAddr,IN const TBUSADDR a_iDstAddr,
							IN RELAY **a_pptRelay,IN const unsigned int a_iRelayCnt) ;

/** 通过连接管理器接受数据
@param[in] pstNode relay服务器上下文环境
@param[in] a_ptEvent 事件对象的指针
@return 成功返回0，失败返回负数
*/
int tbusd_recv(IN LPCONNNODE pstNode, IN struct epoll_event* a_ptEvent);

/** 通过连接管理器一次性将缓冲区中的数据发送出去
@param[in] pstNode relay服务器上下文环境
@param[in] a_pszBuff 保存发送数据的缓冲区
@param[in] a_iBuff 要发送数据的字节数
@return 成功返回0，失败返回负数
*/
int tbusd_send_all(IN LPCONNNODE pstNode, IN const char *a_pszBuff, IN int a_iBuff);

/** 发送数据
*/
int tbusd_send( IN const int a_iSocket, IN const char *a_pszBuff, IN const int a_iBuffLen );

/** 从网络消息缓冲区中读出tbus头部
@param[in] a_pstHead 保存tbus头部信息结构的指针
@param[in] a_pstNode a_pszNet缓冲缓冲区首地址
@param[in] a_iNetLen 网络消息的长度
@retval 1 成功读取到一个tbus头部
@retval 0 链接中的数据还不够一个tbus头部
@retval -1 读取tbus头部失败
*/
int tbusd_read_tbushead(OUT LPTBUSHEAD a_pstHead, IN const char *a_pszNet, IN int a_iNetLen);

const char *tbusd_get_error_string(int icode);

/*从url串中解析ip地址和端口的合法性
@return 0 成功解析出地址和端口，非0 解析地址和端口失败
*/
int tbusd_parse_url_inet_addr(const char *a_pszUrl, TNETADDR *a_pstAddr);

#endif /**< TBUSD_MISC_H */

