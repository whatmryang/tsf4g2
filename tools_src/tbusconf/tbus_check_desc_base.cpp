#include <set>
#include <cstdio>
#include <string>
#include <cstring>

#include "tbus_tools.h"
#include "tbus_check_desc_base.h"

using std::set;
using std::string;

int CheckDescBase::checkProcessListUnique(const PROCESS_ARRAY& stProcessList)
{
    int iRet = 0;

    set<string> uniqueNameSet;
    set<string> uniqueIdSet;

    int iCheckIDRet = checkProcessIDUnique(stProcessList, uniqueIdSet);
    if (0 != iCheckIDRet)
    {
        iRet = iCheckIDRet;
    }

    int iCheckNameRet = checkProcessNameUnique(stProcessList, uniqueNameSet);
    if (0 != iCheckNameRet)
    {
        iRet = iCheckNameRet;
    }

    return iRet;
}

int CheckDescBase::checkCommunication(const COMMUNICATION_RELATION& stCommunication)
{
    int iRet = 0;

    set<string> uniqueSet;

    for (unsigned int i = 0; i < stCommunication.dwConnectionCount; i++)
    {
        string processA(stCommunication.astConnection[i].szNameOfProcessA);
        string processB(stCommunication.astConnection[i].szNameOfProcessB);

        if (processA == processB)
        {
            fprintf(stderr, "Error: connection '%s <--> %s' in communication"
                    " description file connects the same process '%s'\n",
                    stCommunication.astConnection[i].szNameOfProcessA,
                    stCommunication.astConnection[i].szNameOfProcessB,
                    stCommunication.astConnection[i].szNameOfProcessA);
            iRet = -1;
            continue;
        }

        if (processA > processB)
        {
            swap(processA, processB);
        }

        set<string>::size_type iUniqueCount = uniqueSet.size();
        uniqueSet.insert(processA + processB);
        if (uniqueSet.size() == iUniqueCount)
        {
            fprintf(stderr, "Error: duplicated connection '%s <--> %s' in communication description file\n",
                    stCommunication.astConnection[i].szNameOfProcessA,
                    stCommunication.astConnection[i].szNameOfProcessB);
            iRet = -1;
        }
    }

    return iRet;
}

int CheckDescBase::checkProcessIDUnique(const PROCESS_ARRAY& stProcessList, set<string>& uniqueSet)
{
    int iRet = 0;

    for (unsigned int i = 0; i < stProcessList.dwProcessCount; i++)
    {
        set<string>::size_type iUniqueCount = uniqueSet.size();

        uniqueSet.insert(stProcessList.astProcess[i].szId);
        if (uniqueSet.size() == iUniqueCount)
        {
            fprintf(stderr, "Error: more than one process with the same id=\"%s\"\n",
                    stProcessList.astProcess[i].szId);
            iRet = -1;
        }
    }

    return iRet;
}

int CheckDescBase::checkProcessNameUnique(const PROCESS_ARRAY& stProcessList, set<string>& uniqueSet)
{
    int iRet = 0;

    for (unsigned int i = 0; i < stProcessList.dwProcessCount; i++)
    {
        set<string>::size_type iUniqueCount = uniqueSet.size();

        uniqueSet.insert(stProcessList.astProcess[i].szName);
        if (uniqueSet.size() == iUniqueCount)
        {
            fprintf(stderr, "Error: more than one process with the same name=\"%s\"\n",
                    stProcessList.astProcess[i].szName);
            iRet = -1;
        }
    }

    return iRet;
}

int CheckDescBase::checkProcessListIntegrity(const PROCESS_ARRAY& stProcessList)
{
    int iRet = 0;

    for (unsigned int i = 0; i < stProcessList.dwProcessCount; i++)
    {
        if (0 == strlen(stProcessList.astProcess[i].szName))
        {
            fprintf(stderr, "Error: process name=\"\" found, but process name can NOT be empty\n");
            iRet = -1;
        }

        if (0 == strlen(stProcessList.astProcess[i].szId))
        {
            fprintf(stderr, "Error: process id=\"\" found, but process id can NOT be empty\n");
            iRet = -1;
        }
    }

    return iRet;
}

int CheckDescBase::checkConnectIntegrity(const COMMUNICATION_RELATION& stConnectList)
{
    int iRet = 0;

    for (unsigned int i = 0; i < stConnectList.dwConnectionCount; i++)
    {
        if (0 == strlen(stConnectList.astConnection[i].szNameOfProcessA))
        {
            fprintf(stderr, "Error: nameOfProcessA=\"\" found, but process name can NOT be empty\n");
            iRet = -1;
        }

        if (0 == strlen(stConnectList.astConnection[i].szNameOfProcessB))
        {
            fprintf(stderr, "Error: nameOfProcessB=\"\" found, but process name can NOT be empty\n");
            iRet = -1;
        }

        if (0 == stConnectList.astConnection[i].dwBufferSize)
        {
            fprintf(stderr, "Error: one bufferSize in communication description file is 0\n");
            iRet = -1;
        }
    }

    return iRet;
}

int CheckDescBase::checkCommRefer(const std::set<std::string>& processName,
                                  const COMMUNICATION_RELATION& stCommunication)
{
    int iRet = 0;
    for (unsigned int i = 0; i < stCommunication.dwConnectionCount; i++)
    {
        const string& processA = stCommunication.astConnection[i].szNameOfProcessA;
        const string& processB = stCommunication.astConnection[i].szNameOfProcessB;
        if (processName.end() == processName.find(processA))
        {
            fprintf(stderr, "Error: nameOfProcessA=\"%s\" found, but NO process with name=\"%s\"\n",
                    processA.c_str(), processA.c_str());
            iRet = -1;
        }
        if (processName.end() == processName.find(processB))
        {
            fprintf(stderr, "Error: nameOfProcessB=\"%s\" found, but NO process with name=\"%s\"\n",
                    processB.c_str(), processB.c_str());
            iRet = -1;
        }
    }
    return iRet;
}
