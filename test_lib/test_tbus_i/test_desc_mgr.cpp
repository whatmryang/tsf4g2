#include "tbus_desc_mgr.c"

#include "gtest.h"
#include "tdr/tdr.h"


TEST(test_desc_mgr, invalid_index)
{
    int index = -1;
    EXPECT_TRUE(NULL == tbus_get_conf_meta(index));

    index = TBUS_CONF_META_COUNT;
    EXPECT_TRUE(NULL == tbus_get_conf_meta(index));
}

TEST(test_desc_mgr, valid_index)
{
    LPTDRMETA pstMeta = NULL;

    for (int index = 0; index < TBUS_CONF_META_COUNT; index++)
    {
        pstMeta = tbus_get_conf_meta(index);
        EXPECT_TRUE(NULL != pstMeta);
        EXPECT_STREQ(tbus_get_conf_meta_name(index), tdr_get_meta_name(pstMeta));
    }

    /* 以下测试依赖于 tbus_desc_mgr.c 中定义的 gs_MetaNames */
    pstMeta = tbus_get_conf_meta(TBUS_CONF_META_GCIM);
    EXPECT_TRUE(NULL != pstMeta);
    EXPECT_STREQ("TbusGCIM", tdr_get_meta_name(pstMeta));

    pstMeta = tbus_get_conf_meta(TBUS_CONF_META_GRM);
    EXPECT_TRUE(NULL != pstMeta);
    EXPECT_STREQ("RelayMnger", tdr_get_meta_name(pstMeta));

    pstMeta = tbus_get_conf_meta(TBUS_CONF_META_TBUSD);
    EXPECT_TRUE(NULL != pstMeta);
    EXPECT_STREQ("Tbusd", tdr_get_meta_name(pstMeta));

    pstMeta = tbus_get_conf_meta(TBUS_CONF_META_TBUSCONF);
    EXPECT_TRUE(NULL != pstMeta);
    EXPECT_STREQ("TbusConf", tdr_get_meta_name(pstMeta));
}
