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

  @return success��relayͨ����ָ��; failed������NULL
  @retval < 0 -- failed

  @note
*/
LPRELAY trelay_channel_get (IN const TBUSADDR a_iSrcAddr,IN const TBUSADDR a_iDstAddr,
							IN RELAY **a_pptRelay,IN const unsigned int a_iRelayCnt) ;

/** ͨ�����ӹ�������������
@param[in] pstNode relay�����������Ļ���
@param[in] a_ptEvent �¼������ָ��
@return �ɹ�����0��ʧ�ܷ��ظ���
*/
int tbusd_recv(IN LPCONNNODE pstNode, IN struct epoll_event* a_ptEvent);

/** ͨ�����ӹ�����һ���Խ��������е����ݷ��ͳ�ȥ
@param[in] pstNode relay�����������Ļ���
@param[in] a_pszBuff ���淢�����ݵĻ�����
@param[in] a_iBuff Ҫ�������ݵ��ֽ���
@return �ɹ�����0��ʧ�ܷ��ظ���
*/
int tbusd_send_all(IN LPCONNNODE pstNode, IN const char *a_pszBuff, IN int a_iBuff);

/** ��������
*/
int tbusd_send( IN const int a_iSocket, IN const char *a_pszBuff, IN const int a_iBuffLen );

/** ��������Ϣ�������ж���tbusͷ��
@param[in] a_pstHead ����tbusͷ����Ϣ�ṹ��ָ��
@param[in] a_pstNode a_pszNet���建�����׵�ַ
@param[in] a_iNetLen ������Ϣ�ĳ���
@retval 1 �ɹ���ȡ��һ��tbusͷ��
@retval 0 �����е����ݻ�����һ��tbusͷ��
@retval -1 ��ȡtbusͷ��ʧ��
*/
int tbusd_read_tbushead(OUT LPTBUSHEAD a_pstHead, IN const char *a_pszNet, IN int a_iNetLen);

const char *tbusd_get_error_string(int icode);

/*��url���н���ip��ַ�Ͷ˿ڵĺϷ���
@return 0 �ɹ���������ַ�Ͷ˿ڣ���0 ������ַ�Ͷ˿�ʧ��
*/
int tbusd_parse_url_inet_addr(const char *a_pszUrl, TNETADDR *a_pstAddr);

#endif /**< TBUSD_MISC_H */

