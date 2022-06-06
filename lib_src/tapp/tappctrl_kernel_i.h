/*
**  @file $RCSfile: tappctrl_kernel_i.h ,v $
**  kernel for control msg to tapp
**  @author $Author: jackyai $
**  @date $Date: 2010-01-21 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TAPP_CTRL_I_H
#define TAPP_CTRL_I_H

#include "tappctrl_proto.h"
#include "pal/pal.h"
#include "tlog/tlog.h"


#define TAPPCTRL_CHANNEL_QUEUE_SIZE	65536	/*控制通道数据队列的大小*/


// 'size % sizeof(short)' must be equal to 0
#define TAPP_CALC_CHECKSUM(shSum, pBuff, size)  {    \
	int i = 0 ;                                          \
	short *pTemp = (short *)pBuff;                       \
	shSum = 0 ;                                          \
	for (i = 0; i < (int)(size/sizeof(short)); i++)      \
	{                                                    \
		shSum ^= *pTemp++;                               \
	}                                                    \
}

#ifdef __cplusplus
extern "C" 
{
#endif 

/** 根据进程名，进程id和进程所属业务id生成 控制通道所需信息串
@param[out] a_ppszChannelInfo 返回控制通道所需信息串指针
@param[in] a_pszTappName	进程名字符串
@param[in] a_pszTappID	进程id信息
@param[in] a_iBusinessID 业务id
@return 0：成功； 非0：失败
*/
int tappctrl_set_ctrlfile_info(OUT const char **a_ppszChannelInfo, IN const char *a_pszTappName,
							   IN const char *a_pszTappID, IN int a_iBusinessID);

/** 初始化控制消息处理环境 本接口是非线程安全的。
@param[in] a_pszChannelInfo 用于生成控制通道的信息
@param[in] a_iBusinessID	业务ID
@param[in] a_iIsCtrlor 是否是控制端，其值为非零表示控制端，其值为零表示非控制端（也即服务端）
@param[in] a_pstLogcat 日志句柄
@return 0 成功 非0：失败
*/
int tappctrl_init_i(IN const char *a_pszChannelInfo, IN int a_iIsCtrlor, IN void *a_pstLogcat, IN void* a_iMetaLib);

/*
处理环境释放相关资源资源
*/
void tappctrl_fini(void );

/** 从控制通道中收取一个消息，并解包到指定结构中。本接口是非线程安全的。
@param[out] a_pstPkg 保存解包后的控制消息
@return 0 成功收到一个控制消息 非0：失败
*/
int tappctrl_recv_pkg(INOUT  LPCOMMANDINJECTPKG a_pstPkg);

/** 打包控制消息，并发送到控制通道中。本接口是非线程安全的。
@param[in] a_pstPkg 带发送的控制脚本保存解包后的控制消息
@return 0 成功 非0：失败
*/
int tappctrl_send_pkg(INOUT  LPCOMMANDINJECTPKG a_pstPkg);

/** 初始化网络消息头部
@param[in] a_pstHead 消息头部
@param[in] a_iCmd	命令字段
@param[in] a_iVersion 版本
@return 0 成功 非0：失败
*/
int tappctrl_fill_pkghead(OUT	LPCIHEAD a_pstHead, int a_iCmd, int a_iVersion);

/** send request pkg, which contains only stHead
 */
int tapp_controller_send_req_i(short a_nCmdID, int a_iVersion);

/**get protocol meta version
 */
int tapp_controller_get_version(void );

#ifdef __cplusplus
}
#endif 
#endif /*TAPP_CTRL_I_H*/
