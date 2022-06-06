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


#define TAPPCTRL_CHANNEL_QUEUE_SIZE	65536	/*����ͨ�����ݶ��еĴ�С*/


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

/** ���ݽ�����������id�ͽ�������ҵ��id���� ����ͨ��������Ϣ��
@param[out] a_ppszChannelInfo ���ؿ���ͨ��������Ϣ��ָ��
@param[in] a_pszTappName	�������ַ���
@param[in] a_pszTappID	����id��Ϣ
@param[in] a_iBusinessID ҵ��id
@return 0���ɹ��� ��0��ʧ��
*/
int tappctrl_set_ctrlfile_info(OUT const char **a_ppszChannelInfo, IN const char *a_pszTappName,
							   IN const char *a_pszTappID, IN int a_iBusinessID);

/** ��ʼ��������Ϣ������ ���ӿ��Ƿ��̰߳�ȫ�ġ�
@param[in] a_pszChannelInfo �������ɿ���ͨ������Ϣ
@param[in] a_iBusinessID	ҵ��ID
@param[in] a_iIsCtrlor �Ƿ��ǿ��ƶˣ���ֵΪ�����ʾ���ƶˣ���ֵΪ���ʾ�ǿ��ƶˣ�Ҳ������ˣ�
@param[in] a_pstLogcat ��־���
@return 0 �ɹ� ��0��ʧ��
*/
int tappctrl_init_i(IN const char *a_pszChannelInfo, IN int a_iIsCtrlor, IN void *a_pstLogcat, IN void* a_iMetaLib);

/*
�������ͷ������Դ��Դ
*/
void tappctrl_fini(void );

/** �ӿ���ͨ������ȡһ����Ϣ���������ָ���ṹ�С����ӿ��Ƿ��̰߳�ȫ�ġ�
@param[out] a_pstPkg ��������Ŀ�����Ϣ
@return 0 �ɹ��յ�һ��������Ϣ ��0��ʧ��
*/
int tappctrl_recv_pkg(INOUT  LPCOMMANDINJECTPKG a_pstPkg);

/** ���������Ϣ�������͵�����ͨ���С����ӿ��Ƿ��̰߳�ȫ�ġ�
@param[in] a_pstPkg �����͵Ŀ��ƽű���������Ŀ�����Ϣ
@return 0 �ɹ� ��0��ʧ��
*/
int tappctrl_send_pkg(INOUT  LPCOMMANDINJECTPKG a_pstPkg);

/** ��ʼ��������Ϣͷ��
@param[in] a_pstHead ��Ϣͷ��
@param[in] a_iCmd	�����ֶ�
@param[in] a_iVersion �汾
@return 0 �ɹ� ��0��ʧ��
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
