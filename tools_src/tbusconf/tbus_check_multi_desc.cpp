#include <cstdio>

#include "tbus_tools.h"
#include "tbus_check_multi_desc.h"
#include "tbus_comm_relation_desc.h"

int CheckMultiDesc::check(const MULTI_MACHINES& stDesc)
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

int CheckMultiDesc::checkReferrence(const MULTI_MACHINES& stDesc)
{
    set<string> processName;
    for (unsigned int i = 0; i < stDesc.stMachineList.dwMachineCount; i++)
    {
        const PROCESS_ARRAY& stProcessList = stDesc.stMachineList.astMachine[i].stProcessList;
        for (unsigned int j = 0; j < stProcessList.dwProcessCount; j++)
        {
            processName.insert(stProcessList.astProcess[j].szName);
        }
    }

    return checkCommRefer(processName, stDesc.stCommunication);
}

int CheckMultiDesc::checkMachineListUnique(const MACHINE_ARRAY& stList)
{
    int iRet = 0;

    set<string> uniqueMachineSet;
    for (unsigned int i = 0; i < stList.dwMachineCount; i++)
    {
        set<string>::size_type iUniqueCount = uniqueMachineSet.size();
        uniqueMachineSet.insert(stList.astMachine[i].szIP);
        if (uniqueMachineSet.size() == iUniqueCount)
        {
            fprintf(stderr, "Error: more than one machine with the same IP '%s'\n",
                    stList.astMachine[i].szIP);
            iRet = -1;
        }
    }

    set<string> uniqueIdSet;
    set<string> uniqueNameSet;
    for (unsigned int i = 0; i < stList.dwMachineCount; i++)
    {
        int iCheckRet = checkMachineUnique(stList.astMachine[i], uniqueNameSet, uniqueIdSet);
        if (0 != iCheckRet)
        {
            iRet = -1;
        }
    }

    return iRet;
}

int CheckMultiDesc::checkMachineUnique(const MACHINE_DESC& stMachine,
                                       set<string>& uniqueNameSet, set<string>& uniqueIdSet)
{
    int iRet = 0;

    int iCheckNameRet = checkProcessNameUnique(stMachine.stProcessList, uniqueNameSet);
    if (0 != iCheckNameRet)
    {
        iRet = -1;
    }

    int iCheckIdRet = checkProcessIDUnique(stMachine.stProcessList, uniqueIdSet);
    if (0 != iCheckIdRet)
    {
        iRet = -1;
    }

    return iRet;
}

int CheckMultiDesc::checkMachineListIntegrity(const MACHINE_ARRAY& stList)
{
    int iRet = 0;

    for (unsigned int i = 0; i < stList.dwMachineCount; i++)
    {
        int iCheckRet = checkMachineIntegrity(stList.astMachine[i]);
        if (0 != iCheckRet)
        {
            iRet = -1;
        }
    }

    return iRet;
}

int CheckMultiDesc::checkMachineIntegrity(const MACHINE_DESC& stMachine)
{
    int iRet = 0;

    if (0 == strlen(stMachine.szIP))
    {
        fprintf(stderr, "Error: machine IP=\"\" found, but machine IP name can NOT be empty\n");
        iRet = -1;
    }

    if (0 != TbusTools::checkIP(stMachine.szIP))
    {
        fprintf(stderr, "Error: machine IP=\"%s\" found, but \"%s\" is NOT a valid IP\n",
                stMachine.szIP, stMachine.szIP);
        iRet = -1;
    }

    if (0 == strlen(stMachine.szRelayShmKey))
    {
        fprintf(stderr, "Error: relayShmKey=\"\" found, but relayShmKey can NOT be empty\n");
        iRet = -1;
    }

    if (0 == strlen(stMachine.szGCIMShmKey))
    {
        fprintf(stderr, "Error: GCIMShmKey=\"\" found, but GCIMShmKey can NOT be empty\n");
        iRet = -1;
    }

    if (0 == strlen(stMachine.szAddrTemplet))
    {
        fprintf(stderr, "Error: szAddrTemplet=\"\" found, but szAddrTemplet can NOT be empty\n");
        iRet = -1;
    }

    if (1024 >= stMachine.wListenPort)
    {
        fprintf(stderr, "Error: some listenPort in comunication description file is under 1024. unsafe!\n");
        iRet = -1;
    }

    int iCheckProcessRet = checkProcessListIntegrity(stMachine.stProcessList);
    if (0 != iCheckProcessRet)
    {
        iRet = -1;
    }

    return iRet;
}

int CheckMultiDesc::checkIntegrity(const MULTI_MACHINES& stDesc)
{
    int iRet = 0;

    int iCheckMachineRet = checkMachineListIntegrity(stDesc.stMachineList);
    if (0 != iCheckMachineRet)
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

int CheckMultiDesc::checkUnique(const MULTI_MACHINES& stDesc)
{
    int iRet = 0;

    int iCheckMachineRet = checkMachineListUnique(stDesc.stMachineList);
    if (0 != iCheckMachineRet)
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
