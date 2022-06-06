#include "pal/pal.h"
#include "tmng/tmng.h"

#include "tmng_errno.h"
#include "tmng/tmetabase.h"

//#include "tdr/tdr_metalib_kernel_i.h"
#include "c_unit_test.h"


//============================================================================
// tmb_create ≤‚ ‘
//----------------------------------------------------------------------------
int tmb_create__args_1(void);
int tmb_create__args_2(void);
int tmb_create__args_3(void);
int tmb_create__args_4(void);

int tmb_destroy__args(void);
int tmb_destroy__delete(void);


CUT_BEGIN_SUITE(tmb_create)
    CUT_RUN_TEST(tmb_create__args_1);
    CUT_RUN_TEST(tmb_create__args_2);
    CUT_RUN_TEST(tmb_create__args_3);
    CUT_RUN_TEST(tmb_create__args_4);
    CUT_RUN_TEST(tmb_destroy__args);
    CUT_RUN_TEST(tmb_destroy__delete);
CUT_END_SUITE


//============================================================================
// tmb_create ≤‚ ‘∫Ø ˝
//----------------------------------------------------------------------------
int tmb_create__args_1(void)
{
    int32_t iRet = tmb_create(NULL, "hello", 100, 1000000);

    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);

    return 0;
}

int tmb_create__args_2(void)
{
    HTMBDESC p_Handle = (HTMBDESC)-1;
    int32_t iRet = tmb_create(&p_Handle, NULL, 100, 1000000);

    CUT_ASSERT_EQUAL(NULL, p_Handle);
    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);

    return 0;
}

int tmb_create__args_3(void)
{
    HTMBDESC p_Handle = (HTMBDESC)-1;
    int32_t iRet = tmb_create(&p_Handle, "hello", 0, 1000000);

    CUT_ASSERT_EQUAL(NULL, p_Handle);
    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);

    return 0;
}

int tmb_create__args_4(void)
{
    HTMBDESC p_Handle = (HTMBDESC)-1;
    int32_t iRet = tmb_create(&p_Handle, "hello", 100, 400);

    CUT_ASSERT_EQUAL(NULL, p_Handle);
    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);

    return 0;
}

int tmb_destroy__args(void)
{
    int32_t iRet = tmb_destroy(NULL);

    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);

    return 0;
}

int tmb_destroy__delete(void)
{
    int32_t iRet = tmb_destroy("Hello");

    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_SHMDEL);

    return 0;
}


//============================================================================
//
//----------------------------------------------------------------------------
HTMBDESC hDesc = NULL;



//void setup(void);
//void teardown(void);
//============================================================================
// main
//----------------------------------------------------------------------------
//CUT_BEGIN_TEST_HARNESS

//----------------------------------------------------------------------------
// THE END
//============================================================================
