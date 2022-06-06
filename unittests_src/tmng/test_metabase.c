#include "pal/pal.h"
#include "tmng/tmng.h"

#include "tmng_errno.h"
#include "tmng/tmetabase.h"

#include "c_unit_test.h"


//============================================================================
// setup & teardown: create
//----------------------------------------------------------------------------
const char *t_share_memory_name = "33221";

int setup_create(void);
int teardown_create(void);


HTMBDESC g_hDescCreate;


int setup_create(void)
{
    int32_t iRet;

    iRet = tmb_create(&g_hDescCreate, t_share_memory_name, 2, 1000000);

    CUT_ASSERT_EQUAL(0, iRet);
    CUT_ASSERT_NOT_EQUAL(NULL, g_hDescCreate);

    return 0;
}

int teardown_create(void)
{
    int32_t iRet;

    CUT_ASSERT_NOT_EQUAL(NULL, g_hDescCreate);

    iRet = tmb_destroy(t_share_memory_name);
    g_hDescCreate = NULL;

    CUT_ASSERT_EQUAL(0, iRet);

    return 0;
}


//============================================================================
// test suite: tmb_open
//----------------------------------------------------------------------------
int tmb_open__args_1(void);
int tmb_open__args_2(void);
int tmb_open__ok(void);

int tmb_close__args_1(void);
int tmb_close__args_2(void);


CUT_BEGIN_SUITE_F(tmb_open, &setup_create, &teardown_create)
    CUT_RUN_TEST(tmb_open__args_1);
    CUT_RUN_TEST(tmb_open__args_2);
    CUT_RUN_TEST(tmb_open__ok);
    CUT_RUN_TEST(tmb_close__args_1);
    CUT_RUN_TEST(tmb_close__args_2);
CUT_END_SUITE_F


//============================================================================
// test suite: tmb_open functions
//----------------------------------------------------------------------------
int tmb_open__args_1(void)
{
    int32_t iRet;

    iRet = tmb_open(NULL, t_share_memory_name, 0);

    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);

    return 0;
}

int tmb_open__args_2(void)
{
    HTMBDESC pDesc = (HTMBDESC)-1;
    int32_t  iRet;

    iRet = tmb_open(&pDesc, NULL, 0);

    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);
    CUT_ASSERT_EQUAL(NULL, pDesc);

    return 0;
}

int tmb_open__ok(void)
{
    HTMBDESC pDesc = (HTMBDESC)-1;
    int32_t  iRet;

    iRet = tmb_open(&pDesc, t_share_memory_name, 0);

    CUT_ASSERT_EQUAL(0, iRet);
    CUT_ASSERT_NOT_EQUAL(NULL, pDesc);

    iRet = tmb_close(&pDesc);

    CUT_ASSERT_EQUAL(0, iRet);
    CUT_ASSERT_EQUAL(NULL, pDesc);

    return 0;
}

int tmb_close__args_1(void)
{
    int32_t iRet;

    iRet = tmb_close(NULL);

    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);

    return 0;
}

int tmb_close__args_2(void)
{
    HTMBDESC pDesc = (HTMBDESC)NULL;
    int32_t  iRet;

    iRet = tmb_close(&pDesc);

    CUT_ASSERT_EQUAL(iRet, -TMNG_ERR_ARGS);

    return 0;
}


//============================================================================
// setup & teardown: open
//----------------------------------------------------------------------------
int setup_open(void);
int teardown_open(void);

HTMBDESC g_hDescOpen;


int setup_open(void)
{
    int32_t iRet;

    setup_create();

    iRet = tmb_open(&g_hDescOpen, t_share_memory_name, 0);

    CUT_ASSERT_EQUAL(0, iRet);
    CUT_ASSERT_NOT_EQUAL(NULL, g_hDescOpen);

    return 0;
}

int teardown_open(void)
{
    int32_t iRet;

    CUT_ASSERT_NOT_EQUAL(NULL, g_hDescOpen);

    iRet = tmb_close(&g_hDescOpen);

    CUT_ASSERT_EQUAL(NULL, g_hDescOpen);
    CUT_ASSERT_EQUAL(0, iRet);

    teardown_create();

    return 0;
}


//============================================================================
// test suite: tmb_lock
//----------------------------------------------------------------------------
int tmb_lock__args_1(void);
int tmb_lock__args_2(void);
int tmb_lock__ok(void);
int tmb_unlock__args_1(void);
int tmb_unlock__args_2(void);


CUT_BEGIN_SUITE_F(tmb_lock, &setup_open, &teardown_open)
    CUT_RUN_TEST(tmb_lock__args_1);
    CUT_RUN_TEST(tmb_lock__args_2);
//    CUT_RUN_TEST(tmb_lock__ok);
    CUT_RUN_TEST(tmb_unlock__args_1);
    CUT_RUN_TEST(tmb_unlock__args_2);
CUT_END_SUITE_F


//============================================================================
// test suite: tmb_lock function
//----------------------------------------------------------------------------
int tmb_lock__args_1(void)
{
    int32_t iRet;

    iRet = tmb_lock(NULL);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_lock__args_2(void)
{
    HTMBDESC hDesc = NULL;
    char     szBuffer[sizeof(*hDesc)];
    int32_t  iRet;

    hDesc = (HTMBDESC)szBuffer;
    hDesc->iLock = (HANDLE)-1;

    iRet = tmb_lock(hDesc);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_lock__ok(void)
{
    int32_t iRet;

    iRet = tmb_lock(g_hDescOpen);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_unlock(g_hDescOpen);

    CUT_ASSERT_EQUAL(0, iRet);

    return 0;
}

int tmb_unlock__args_1(void)
{
    int32_t iRet;

    iRet = tmb_unlock(NULL);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_unlock__args_2(void)
{
    HTMBDESC hDesc = NULL;
    char     szBuffer[sizeof(*hDesc)];
    int32_t  iRet;

    hDesc = (HTMBDESC)szBuffer;
    hDesc->iLock = (HANDLE)-1;

    iRet = tmb_unlock(hDesc);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}


//============================================================================
// test suite: tmb_append
//----------------------------------------------------------------------------
int tmb_append_unlocked__args_1(void);
int tmb_append_unlocked__args_2(void);
int tmb_append_unlocked__maxlibs(void);
int tmb_append_unlocked__size(void);
int tmb_append__args_1(void);
int tmb_append__args_2(void);
int tmb_append__args_3(void);
int tmb_append__load_metalib(void);
int tmb_append__ok(void);


CUT_BEGIN_SUITE_F(tmb_append, &setup_open, &teardown_open)
    CUT_RUN_TEST(tmb_append_unlocked__args_1);
    CUT_RUN_TEST(tmb_append_unlocked__args_2);
    CUT_RUN_TEST(tmb_append_unlocked__maxlibs);
    CUT_RUN_TEST(tmb_append_unlocked__size);
    CUT_RUN_TEST(tmb_append__args_1);
    CUT_RUN_TEST(tmb_append__args_2);
    CUT_RUN_TEST(tmb_append__args_3);
    CUT_RUN_TEST(tmb_append__load_metalib);
    CUT_RUN_TEST(tmb_append__ok);
CUT_END_SUITE_F


//============================================================================
// test suite: tmb_append function
//----------------------------------------------------------------------------
int tmb_append_unlocked__args_1(void)
{
    int32_t iRet;

    iRet = tmb_append_unlocked(NULL, (LPTDRMETALIB)"hello");

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_append_unlocked__args_2(void)
{
    int32_t iRet;

    iRet = tmb_append_unlocked(g_hDescOpen, NULL);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_append_unlocked__maxlibs(void)
{
    int32_t iRet;

    iRet = tmb_append(g_hDescOpen, "pet1.tdr");

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_append(g_hDescOpen, "pet2.tdr");

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_append(g_hDescOpen, "pet3.tdr");

    CUT_ASSERT_EQUAL(-TMNG_ERR_FULL, iRet);

    return 0;
}

int tmb_append_unlocked__size(void)
{
    int32_t iRet;

    iRet = tmb_append(g_hDescOpen, "pet1_bad.tdr");

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_append(g_hDescOpen, "pet1.tdr");

    CUT_ASSERT_EQUAL(-TMNG_ERR_SIZE, iRet);

    return 0;
}

int tmb_append__args_1(void)
{
    int32_t iRet;

    iRet = tmb_append(NULL, "hello");

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_append__args_2(void)
{
    int32_t iRet;

    iRet = tmb_append(g_hDescOpen, NULL);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_append__args_3(void)
{
    int32_t iRet;

    iRet = tmb_append(g_hDescOpen, "");

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_append__load_metalib(void)
{
    int32_t iRet;

    iRet = tmb_append(g_hDescOpen, "xxxxx.tdr");

    CUT_ASSERT_EQUAL(-TMNG_ERR_TDR_LOAD_METALIB, iRet);

    return 0;
}

int tmb_append__ok(void)
{
    int32_t iRet;

    iRet = tmb_append(g_hDescOpen, "pet1.tdr");

    CUT_ASSERT_EQUAL(0, iRet);

    return 0;
}


//============================================================================
// setup & teardown: data
//----------------------------------------------------------------------------
int setup_data(void);
int teardown_data(void);


int setup_data(void)
{
    int32_t iRet;

    setup_open();

    iRet = tmb_append(g_hDescOpen, "pet1.tdr");

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_append(g_hDescOpen, "pet2.tdr");

    CUT_ASSERT_EQUAL(0, iRet);

    return 0;
}

int teardown_data(void)
{
    teardown_open();

    return 0;
}

//============================================================================
// test suite: tmb_find
//----------------------------------------------------------------------------
int tmb_find_unlocked__args_1(void);
int tmb_find_unlocked__args_2(void);
int tmb_find_unlocked__not_found(void);
int tmb_find_unlocked__ok(void);


CUT_BEGIN_SUITE_F(tmb_find, &setup_data, &teardown_data)
    CUT_RUN_TEST(tmb_find_unlocked__args_1);
    CUT_RUN_TEST(tmb_find_unlocked__args_2);
    CUT_RUN_TEST(tmb_find_unlocked__not_found);
    CUT_RUN_TEST(tmb_find_unlocked__ok);
CUT_END_SUITE_F


//============================================================================
// test suite: tmb_find function
//----------------------------------------------------------------------------
LPTDRMETALIB tmb_find_unlocked(HTMBDESC, const char *, int32_t);

int tmb_find_unlocked__args_1(void)
{
    LPTDRMETALIB pRet;

    pRet = tmb_find_unlocked(NULL, "hello", 0);

    CUT_ASSERT_EQUAL(NULL, pRet);

    return 0;
}

int tmb_find_unlocked__args_2(void)
{
    LPTDRMETALIB pRet;

    pRet = tmb_find_unlocked(g_hDescOpen, NULL, 0);

    CUT_ASSERT_EQUAL(NULL, pRet);

    return 0;
}

int tmb_find_unlocked__not_found(void)
{
    LPTDRMETALIB pRet;

    pRet = tmb_find_unlocked(g_hDescOpen, "pet0", 0);

    CUT_ASSERT_EQUAL(NULL, pRet);

    return 0;
}

int tmb_find_unlocked__ok(void)
{
    LPTDRMETALIB pRet;

    pRet = tmb_find_unlocked(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_NOT_EQUAL(NULL, pRet);

    return 0;
}


//============================================================================
// test suite: tmb_open_metalib
//----------------------------------------------------------------------------
int tmb_open_metalib__args_1(void);
int tmb_open_metalib__args_2(void);
int tmb_open_metalib__not_found(void);
int tmb_open_metalib__ok(void);

CUT_BEGIN_SUITE_F(tmb_open_metalib, &setup_data, &teardown_data)
    CUT_RUN_TEST(tmb_open_metalib__args_1);
    CUT_RUN_TEST(tmb_open_metalib__args_2);
    CUT_RUN_TEST(tmb_open_metalib__not_found);
    CUT_RUN_TEST(tmb_open_metalib__ok);
CUT_END_SUITE_F

//============================================================================
// test suite: tmb_open_metalib function
//----------------------------------------------------------------------------
int tmb_open_metalib__args_1(void)
{
    int32_t iRet;

    iRet = tmb_open_metalib(NULL, "hello", 0);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_open_metalib__args_2(void)
{
    int32_t iRet;

    iRet = tmb_open_metalib(g_hDescOpen, NULL, 0);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_open_metalib__not_found(void)
{
    int32_t iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet0", 0);

    CUT_ASSERT_EQUAL(-TMNG_ERR_TMB_FIND_UNLOCKED, iRet);

    return 0;
}

int tmb_open_metalib__ok(void)
{
    int32_t iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    return 0;
}


//============================================================================
// test suite: tmb_meta_by
//----------------------------------------------------------------------------
int tmb_meta_by_id__args_1(void);
int tmb_meta_by_id__args_2(void);
int tmb_meta_by_id__args_3(void);
int tmb_meta_by_id__not_found(void);
int tmb_meta_by_name__args_1(void);
int tmb_meta_by_name__args_2(void);
int tmb_meta_by_name__args_3(void);
int tmb_meta_by_name__args_4(void);
int tmb_meta_by_name__not_found(void);
int tmb_meta_by_name__ok(void);

CUT_BEGIN_SUITE_F(tmb_meta_by, &setup_data, &teardown_data)
    CUT_RUN_TEST(tmb_meta_by_id__args_1);
    CUT_RUN_TEST(tmb_meta_by_id__args_2);
    CUT_RUN_TEST(tmb_meta_by_id__args_3);
    CUT_RUN_TEST(tmb_meta_by_id__not_found);
    CUT_RUN_TEST(tmb_meta_by_name__args_1);
    CUT_RUN_TEST(tmb_meta_by_name__args_2);
    CUT_RUN_TEST(tmb_meta_by_name__args_3);
    CUT_RUN_TEST(tmb_meta_by_name__args_4);
    CUT_RUN_TEST(tmb_meta_by_name__not_found);
    CUT_RUN_TEST(tmb_open_metalib__ok);
CUT_END_SUITE_F

//============================================================================
// test suite: tmb_meta_by function
//----------------------------------------------------------------------------
int tmb_meta_by_id__args_1(void)
{
    LPTDRMETA pstMeta = (LPTDRMETA)-1;
    int32_t   iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_meta_by_id(NULL, 1, &pstMeta);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_meta_by_id__args_2(void)
{
    LPTDRMETA pstMeta = (LPTDRMETA)-1;
    int32_t   iRet    = tmb_meta_by_id(g_hDescOpen, 1, &pstMeta);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_meta_by_id__args_3(void)
{
    int32_t iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_meta_by_id(g_hDescOpen, 0, NULL);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_meta_by_id__not_found(void)
{
    LPTDRMETA pstMeta = NULL;
    int32_t   iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_meta_by_id(g_hDescOpen, 100, &pstMeta);

    CUT_ASSERT_EQUAL(-TMNG_ERR_NULLMETA, iRet);

    return 0;
}

int tmb_meta_by_name__args_1(void)
{
    LPTDRMETA pstMeta = (LPTDRMETA)-1;
    int32_t   iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_meta_by_name(NULL, "hello", &pstMeta);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_meta_by_name__args_2(void)
{
    LPTDRMETA pstMeta = (LPTDRMETA)-1;
    int32_t   iRet;

    iRet = tmb_meta_by_name(g_hDescOpen, "ArmAttr", &pstMeta);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_meta_by_name__args_3(void)
{
    LPTDRMETA pstMeta = (LPTDRMETA)-1;
    int32_t   iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_meta_by_name(g_hDescOpen, NULL, &pstMeta);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_meta_by_name__args_4(void)
{
    int32_t iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_meta_by_name(g_hDescOpen, "ArmAttr", NULL);

    CUT_ASSERT_EQUAL(-TMNG_ERR_ARGS, iRet);

    return 0;
}

int tmb_meta_by_name__not_found(void)
{
    LPTDRMETA pstMeta = (LPTDRMETA)-1;
    int32_t   iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_meta_by_name(g_hDescOpen, "hello_ArmAttr", &pstMeta);

    CUT_ASSERT_EQUAL(-TMNG_ERR_NULLMETA, iRet);
    CUT_ASSERT_EQUAL(NULL, pstMeta);

    return 0;
}

int tmb_meta_by_name__ok(void)
{
    LPTDRMETA pstMeta = (LPTDRMETA)-1;
    int32_t   iRet;

    iRet = tmb_open_metalib(g_hDescOpen, "pet1", 0);

    CUT_ASSERT_EQUAL(0, iRet);

    iRet = tmb_meta_by_name(g_hDescOpen, "ArmAttr", &pstMeta);

    CUT_ASSERT_EQUAL(0, iRet);
    CUT_ASSERT_NOT_EQUAL(NULL, pstMeta);

    return 0;
}


/*
//============================================================================
// test suite: tmb_dump
//----------------------------------------------------------------------------
int tmb_append_unlocked__args_1(void);
int tmb_append_unlocked__args_2(void);
int tmb_append_unlocked__maxlibs(void);
int tmb_append_unlocked__size(void);
int tmb_append_unlocked__ok(void);
int tmb_append__args_1(void);
int tmb_append__args_2(void);
int tmb_append__args_3(void);
int tmb_append__ok(void);


CUT_BEGIN_SUITE_F(tmb_append, &setup_open, &teardown_open)
    CUT_RUN_TEST(tmb_append_unlocked__args_1);
    CUT_RUN_TEST(tmb_append_unlocked__args_2);
    CUT_RUN_TEST(tmb_append_unlocked__maxlibs);
    CUT_RUN_TEST(tmb_append_unlocked__size);
    CUT_RUN_TEST(tmb_append_unlocked__ok);
    CUT_RUN_TEST(tmb_append__args_1);
    CUT_RUN_TEST(tmb_append__args_2);
    CUT_RUN_TEST(tmb_append__args_3);
    CUT_RUN_TEST(tmb_append__ok);
CUT_END_SUITE_F

//============================================================================
// test suite: tmb_dump function
//----------------------------------------------------------------------------

*/
//----------------------------------------------------------------------------
// THE END
//============================================================================
