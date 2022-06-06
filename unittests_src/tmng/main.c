#include "c_unit_test.h"
#include <stdlib.h>


CUT_BEGIN_TEST_HARNESS
    CUT_RUN_SUITE_TESTS(tmb_create);
    CUT_RUN_SUITE_TESTS(tmb_open);
    CUT_RUN_SUITE_TESTS(tmb_lock);
    CUT_RUN_SUITE_TESTS(tmb_append);
    CUT_RUN_SUITE_TESTS(tmb_find);
    CUT_RUN_SUITE_TESTS(tmb_open_metalib);
    CUT_RUN_SUITE_TESTS(tmb_meta_by);
CUT_END_TEST_HARNESS
