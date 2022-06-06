#include <cassert>

#include "gtest.h"
#include "tdr/tdr.h"

extern unsigned char g_szMetalib_TbusConf[];


#ifdef __cplusplus
extern "C" {
#endif

int tdr_input_probe(INOUT int* a_piMatchedIdx,
                    IN LPTDRMETA* a_pastMetas,
                    IN const char* a_pszFile);

#ifdef __cplusplus
}
#endif

class TdrInputTest : public testing::Test {
    protected:
        static void SetUpTestCase();
        static void TearDownTestCase() ;
        static LPTDRMETA* pstMetas;
};

static const char* aszMetaNames[] = {
    "TbusGCIM",
    "RelayMnger",
    "Tbusd",
    "TbusConf",
};

void TdrInputTest::SetUpTestCase()
{
    LPTDRMETALIB pstLib = (LPTDRMETALIB)g_szMetalib_TbusConf;

    const int iMetaCount = 5;

    pstMetas = new LPTDRMETA[iMetaCount];

    for (int i = 0; i < iMetaCount - 1; i++)
    {
        pstMetas[i] = tdr_get_meta_by_name(pstLib, aszMetaNames[i]);
        assert(NULL != pstMetas[i]);
    }

    pstMetas[iMetaCount - 1] = NULL;
}

void TdrInputTest::TearDownTestCase()
{
    delete [] pstMetas;
}

LPTDRMETA* TdrInputTest::pstMetas = NULL;


TEST_F(TdrInputTest, no_root_input)
{
    int iRet = TDR_SUCCESS;
    int iMathcedIdx = -1;

    iRet = tdr_input_probe(&iMathcedIdx, pstMetas, "no_root.xml");
    EXPECT_TRUE(TDR_ERR_IS_ERROR(iRet));
}

TEST_F(TdrInputTest, no_matched_root_input)
{
    int iRet = TDR_SUCCESS;
    int iMathcedIdx = 0;

    iRet = tdr_input_probe(&iMathcedIdx, pstMetas, "no_matched_root.xml");
    EXPECT_FALSE(TDR_ERR_IS_ERROR(iRet));
    EXPECT_TRUE(0 > iMathcedIdx);
}

TEST_F(TdrInputTest, has_matched_root_input)
{
    int iRet = TDR_SUCCESS;
    int iMathcedIdx = 0;

    iRet = tdr_input_probe(&iMathcedIdx, pstMetas, "has_matched_root.xml");
    EXPECT_FALSE(TDR_ERR_IS_ERROR(iRet));
    EXPECT_TRUE(0 <= iMathcedIdx);

    EXPECT_STREQ(aszMetaNames[iMathcedIdx], tdr_get_meta_name(pstMetas[iMathcedIdx]));
}
