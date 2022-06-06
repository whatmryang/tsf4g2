#include "tbus_conf_mgr.c"

#include "gtest.h"

#include "tbus_desc_ex.h"

class TbusConfTest : public testing::Test {
    protected:
        static void SetUpTestCase();
        static void TearDownTestCase() ;
        static LPTBUSCONF pstConf;
        static TBUSCONFINFO stConfInfo;
};

void TbusConfTest::SetUpTestCase()
{
    tbus_init_conf_info(&stConfInfo);

    stConfInfo.pstConf = new TBUSCONF;
    stConfInfo.pszErr  = new char[1024];

    assert(NULL != stConfInfo.pstConf);
}

void TbusConfTest::TearDownTestCase()
{
    delete stConfInfo.pstConf;
    stConfInfo.pstConf = NULL;

    delete [] stConfInfo.pszErr;
    stConfInfo.pszErr = NULL;
}

TBUSCONFINFO TbusConfTest::stConfInfo;


TEST_F(TbusConfTest, bigger_conf)
{
}

TEST_F(TbusConfTest, little_conf)
{
}
