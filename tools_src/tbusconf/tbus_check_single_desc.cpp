#include <set>
#include <cstdio>
#include <string>
#include <cstring>

#include "tbus_check_single_desc.h"

using std::set;
using std::string;

int CheckSingleDesc::check(const SINGLE_MACHINE& stDesc)
{
    int iRet = 0;

    if (0 == iRet)
    {
        iRet = checkIntegrity(stDesc);
    }

    if (0 == iRet)
    {
        iRet = checkUnique(stDesc);
    }

    if (0 == iRet)
    {
        iRet = checkReferrence(stDesc);
    }

    if (0 != iRet)
    {
        fprintf(stderr, "Error: invalid communication relation description file format\n");
    }

    return iRet;
}

int CheckSingleDesc::checkIntegrity(const SINGLE_MACHINE& stDesc)
{
    int iRet = 0;

    if (0 == strlen(stDesc.szGCIMShmKey))
    {
        fprintf(stderr, "Error: GCIMShmKey=\"\" found, but GCIMShmKey can NOT be empty\n");
        iRet = -1;
    }

    if (0 == strlen(stDesc.szAddrTemplet))
    {
        fprintf(stderr, "Error: szAddrTemplet=\"\" found, but szAddrTemplet can NOT be empty\n");
        iRet = -1;
    }

    int iCheckProcessRet= checkProcessListIntegrity(stDesc.stProcessList);
    if (0 != iCheckProcessRet)
    {
        iRet = -1;
    }

    int iCheckConnectRet = checkConnectIntegrity(stDesc.stCommunication);
    if (0 != iCheckConnectRet)
    {
        iRet = -1;
    }

    return iRet;
}

int CheckSingleDesc::checkReferrence(const SINGLE_MACHINE& stDesc)
{
    set<string> processName;
    for (unsigned int i = 0; i < stDesc.stProcessList.dwProcessCount; i++)
    {
        processName.insert(stDesc.stProcessList.astProcess[i].szName);
    }

    return checkCommRefer(processName, stDesc.stCommunication);
}

int CheckSingleDesc::checkUnique(const SINGLE_MACHINE& stDesc)
{
    int iRet = 0;

    int iCheckProcessRet = checkProcessListUnique(stDesc.stProcessList);
    if (0 != iCheckProcessRet)
    {
        iRet = -1;
    }

    int iCheckCommRet = checkCommunication(stDesc.stCommunication);
    if (0 != iCheckCommRet)
    {
        iRet = -1;
    }

    return iRet;
}
