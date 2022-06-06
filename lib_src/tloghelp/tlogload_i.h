#ifndef TLOG_LOAD_I_H_
#define TLOG_LOAD_I_H_

#include "tloghelp/tlogload.h"
#include "tlog/tlog.h"
#include "../lib_src/tlog/tlog_i.h"

#ifdef __cplusplus
extern "C"
{
#endif

LPTLOGCONF tlog_conf_alloc(void);

/**��mib�л�ȡtlog����
 *@param[in]	     a_pMib  ָ��
 *@param[in]      a_pszDomain    mib��
 *@param[in]      a_pszName     ������
 *@param[in]      a_iProcID   ����ID


 *@retval  NOT NULL  �ɹ� ����tlogconfָ�룬��������free�ͷţ�
 NULL  ʧ��
 */
LPTLOGCONF tlog_get_cfg_from_mib(void* a_pMib, const char *a_pszDomain,
        const char *a_pszName, int a_iProcID);

/**��ʼ��mempool�ڴ棬�γɿ�������
 *@param[in,out]	 a_pstConf  tlogconfָ�룬����ʼ����tlogconf
 *@param[in]      a_pszPath  �����ļ�·��

 *@retval  0  �ɹ�
 -1  ʧ��
 */
int tlog_init_cfg_from_file(LPTLOGCONF a_pstConf, const char *a_pszPath);

/**��ʼ��Ĭ��tlog����
 *@param[in,out]	 a_pstConf  tlogconfָ�룬����ʼ����tlogconf
 *@param[in]      a_pszPath  �ļ������basename��������׺��

 *@retval  0  �ɹ�
 ��0  ʧ��
 *@note ֻ�᷵��0����������a_pszPath�д��󣬽��ᵼ���޷�д�ļ�����
 */
int tlog_init_cfg_default(LPTLOGCONF a_pstConf, const char* a_pszPath);

LPTLOGCTX tlog_init_default(const char *a_pstzPath);
#ifdef __cplusplus
}
#endif

#endif

