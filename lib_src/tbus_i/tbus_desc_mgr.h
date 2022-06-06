#ifndef _TBUS_DESC_MGR_H_
#define _TBUS_DESC_MGR_H_

#include "tdr/tdr.h"
#include "tbus_desc_ex.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 根据 index 取得对应的 meta 的指针
 * @param[in] a_iIdx 输入index, 只能取 TBUS_CONF_META_INDEXES 中有意义的值
 * @retval a_iIdx对应的 meta 的指针
 */
LPTDRMETA tbus_get_conf_meta(int a_iIdx);

#ifdef __cplusplus
}
#endif

#endif
