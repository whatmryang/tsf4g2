#include "ut_os.h"
#include "pal/pal.h"
#include <CUnit/CUnit.h>


void ut_getexe()
{
    char szPath[256];
    int iRet;
    pid_t pid = getpid();

    iRet = tos_get_exe(pid, szPath, sizeof(szPath));
    CU_ASSERT(0 == iRet);
    printf("pid:%d, cwd:%s\n", (int)pid,szPath);
}

