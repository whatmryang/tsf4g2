#include "gtest.h"
#include "comm/shtable.h"

int hashCmp(const void* one, const void* two)
{
    return *(int*)one == *(int*)two;
}

unsigned hashCode(const void* data)
{
    return *(unsigned*)data;
}

TEST(shtable, remove_by_ptr)
{
    LPSHTABLE pstTable = NULL;

#define MAX_UNIT 100

    int data[MAX_UNIT];
    void* ptrs[MAX_UNIT];

    pstTable = sht_create(100, MAX_UNIT, 100, NULL);
    EXPECT_TRUE(NULL != pstTable);

    for (int i = 0; i < 100; i++)
    {
        data[i] = i;
        ptrs[i] = sht_insert_unique(pstTable, &data[i], hashCmp, hashCode);
        EXPECT_TRUE(NULL != ptrs[i]);
    }

    for (int i = 0; i < 100; i++)
    {
        int iRet = sht_remove_by_dataptr(pstTable, ptrs[i]);
        EXPECT_TRUE(0 == iRet);
    }

    sht_destroy(&pstTable);
    EXPECT_TRUE(NULL == pstTable);
}
