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

/**从mib中获取tlog配置
 *@param[in]	     a_pMib  指针
 *@param[in]      a_pszDomain    mib域
 *@param[in]      a_pszName     配置名
 *@param[in]      a_iProcID   进程ID


 *@retval  NOT NULL  成功 返回tlogconf指针，（不能用free释放）
 NULL  失败
 */
LPTLOGCONF tlog_get_cfg_from_mib(void* a_pMib, const char *a_pszDomain,
        const char *a_pszName, int a_iProcID);

/**初始化mempool内存，形成空闲链表
 *@param[in,out]	 a_pstConf  tlogconf指针，被初始化的tlogconf
 *@param[in]      a_pszPath  配置文件路径

 *@retval  0  成功
 -1  失败
 */
int tlog_init_cfg_from_file(LPTLOGCONF a_pstConf, const char *a_pszPath);

/**初始化默认tlog配置
 *@param[in,out]	 a_pstConf  tlogconf指针，被初始化的tlogconf
 *@param[in]      a_pszPath  文件输出的basename，不带后缀。

 *@retval  0  成功
 非0  失败
 *@note 只会返回0，如果传入的a_pszPath有错误，将会导致无法写文件错误。
 */
int tlog_init_cfg_default(LPTLOGCONF a_pstConf, const char* a_pszPath);

LPTLOGCTX tlog_init_default(const char *a_pstzPath);
#ifdef __cplusplus
}
#endif

#endif

