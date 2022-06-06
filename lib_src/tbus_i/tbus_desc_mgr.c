#include "tdr/tdr.h"
#include "tbus_desc_mgr.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned char g_szMetalib_TbusConf[];
#ifdef __cplusplus
}
#endif

static LPTDRMETALIB gs_MetaLib = (LPTDRMETALIB)g_szMetalib_TbusConf;


/* gs_MetaNames 中的名称必须与 tbus_desc*.xml 中的 meta 名
 * 顺序必须与 macrosgroup TBUS_CONF_META_INDEXES 对应
 * 个数必须等于　TBUS_CONF_META_COUNT 的值
 */
static const char* gs_MetaNames[] = {
    "TbusGCIM",
    "RelayMnger",
    "Tbusd",
    "TbusConf",
};

/* gs_Metas 的元素个数必须等于　TBUS_CONF_META_COUNT 的值 */
static LPTDRMETA gs_Metas[] = {
    NULL,
    NULL,
    NULL,
    NULL,
};


/* 仅供 gtest 单元测试使用 */
const char* tbus_get_conf_meta_name(int a_iIdx)
{
    if (0 > a_iIdx && a_iIdx >= TBUS_CONF_META_COUNT)
    {
        return NULL;
    }

    return gs_MetaNames[a_iIdx];
}

LPTDRMETA tbus_get_conf_meta(int a_iIdx)
{
    if (0 > a_iIdx && a_iIdx >= TBUS_CONF_META_COUNT)
    {
        return NULL;
    }

    if (NULL == gs_Metas[0])
    {
        int i = 0;
        for (i = 0; i < TBUS_CONF_META_COUNT; i++)
        {
            gs_Metas[i] = tdr_get_meta_by_name(gs_MetaLib, gs_MetaNames[i]);
        }
    }

    return gs_Metas[a_iIdx];
}
