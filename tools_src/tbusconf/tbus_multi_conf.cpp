#include <set>
#include <map>
#include <string>
#include <iostream>

#include "tbus_tools.h"
#include "tbus_multi_conf.h"
#include "tbus_comm_relation_desc.h"

using std::set;
using std::map;
using std::string;

int TbusMultiConf::init(const MULTI_MACHINES& stDesc, const std::string& outputDir)
{
    this->outputDir = outputDir;
    int iRet = TbusTools::checkDir(this->outputDir);
    if (0 != iRet)
    {
        fprintf(stderr, "Error: failed to create direcotry '%s'\n", this->outputDir.c_str());
        return -1;
    }

    dwMachineCount = stDesc.stMachineList.dwMachineCount;
    pMachine = new (std::nothrow) MachineDesc[dwMachineCount];
    if (NULL == pMachine)
    {
        fprintf(stderr, "Error: no memmory available\n");
        return -1;
    }

    iRet = initProcessWithTbus(stDesc);
    if (0 != iRet)
    {
        return iRet;
    }

    iRet = initProcessName2Machine(stDesc);
    if (0 != iRet)
    {
        return iRet;
    }

    iRet = initMachineDescList(stDesc);
    if (0 != iRet)
    {
        return iRet;
    }

    return 0;
}

int TbusMultiConf::outputConf()
{
    for (unsigned int i = 0; i < dwMachineCount; i++)
    {
        int iRet = pMachine[i].output(processWithTbus);
        if (0 != iRet)
        {
            return -1;
        }
    }

    return 0;
}

int TbusMultiConf::initProcessWithTbus(const MULTI_MACHINES& stDesc)
{
    unsigned int i =0;
    for (i = 0; i < stDesc.stCommunication.dwConnectionCount; i++)
    {
        const CONNECTION_DESC& stConnect = stDesc.stCommunication.astConnection[i];

        processWithTbus.insert(stConnect.szNameOfProcessA);
        processWithTbus.insert(stConnect.szNameOfProcessB);
    }
    return 0;
}

int TbusMultiConf::initProcessName2Id(MachineDesc& machine,const MACHINE_DESC& stMachine)
{
    for (unsigned int i = 0; i < stMachine.stProcessList.dwProcessCount; i++)
    {
        const PROCESS_DESC& process = stMachine.stProcessList.astProcess[i];
        machine.processName2Id[process.szName] = process.szId;
        processName2Id[process.szName] = process.szId;
    }
    return 0;
}

int TbusMultiConf::initMachineGCIM(MachineDesc& machine,
                                   const MACHINE_DESC& stMachine,
                                   const COMMUNICATION_RELATION& stCommunication)
{
    int iRet = machine.initGCIM();
    if (0 != iRet)
    {
        return -1;
    }

    strncpy(machine.stGCIM.szGCIMShmKey, stMachine.szGCIMShmKey, sizeof(machine.stGCIM.szGCIMShmKey));
    strncpy(machine.stGCIM.szAddrTemplet, stMachine.szAddrTemplet, sizeof(machine.stGCIM.szAddrTemplet));
    for (unsigned int i = 0; i < stCommunication.dwConnectionCount; i++)
    {
        const CONNECTION_DESC& stConnect = stCommunication.astConnection[i];
        if (machine.processName2Id.end() != machine.processName2Id.find(stConnect.szNameOfProcessA)
            || machine.processName2Id.end() != machine.processName2Id.find(stConnect.szNameOfProcessB))
        {
            CHANNELCNF& stChannel = machine.stGCIM.astChannels[machine.stGCIM.dwCount];

            stChannel.dwSendSize = stConnect.dwBufferSize;
            stChannel.dwRecvSize = stConnect.dwBufferSize;
            stChannel.dwAddressCount = 2;

            map<string, string>::const_iterator iter;
            iter = processName2Id.find(stConnect.szNameOfProcessA);
            assert(processName2Id.end() != iter);
            const string& idOfProcessA = iter->second;

            iter = processName2Id.find(stConnect.szNameOfProcessB);
            assert(processName2Id.end() != iter);
            const string& idOfProcessB = iter->second;

            strncpy(stChannel.aszAddress[0], idOfProcessA.c_str(), sizeof(stChannel.aszAddress[0]));
            strncpy(stChannel.aszAddress[1], idOfProcessB.c_str(), sizeof(stChannel.aszAddress[1]));
            snprintf(stChannel.szDesc, sizeof(stChannel.szDesc), "%s --- %s",
                     stConnect.szNameOfProcessA, stConnect.szNameOfProcessB);
            machine.stGCIM.dwCount++;
        }
    }

    return 0;
}

int TbusMultiConf::initMachineRelay(MachineDesc& machine,
                                    const MACHINE_DESC& stMachine,
                                    const COMMUNICATION_RELATION& stCommunication)
{
    int iRet = machine.initRelay();
    if (0 != iRet)
    {
        return -1;
    }

    strncpy(machine.stRelay.szRelayShmKey, stMachine.szRelayShmKey, sizeof(machine.stRelay.szRelayShmKey));
    strncpy(machine.stRelay.szAddrTemplet, stMachine.szAddrTemplet, sizeof(machine.stRelay.szAddrTemplet));
    set<string> uniquePeerName;
    for (unsigned int i = 0; i < stCommunication.dwConnectionCount; i++)
    {
        const CONNECTION_DESC& stConnect = stCommunication.astConnection[i];
        const char* pszPeerA = stConnect.szNameOfProcessA;
        const char* pszPeerB = stConnect.szNameOfProcessB;

        if ((machine.contains(pszPeerA) && !machine.contains(pszPeerB))
            || (machine.contains(pszPeerB) && !machine.contains(pszPeerA)))
        {
            set<string>::size_type uniquePeerCount = uniquePeerName.size();

            SHMRELAY& relay = machine.stRelay.astRelays[machine.stRelay.dwUsedCnt];
            map<string, string>::const_iterator iter;
            const string* remoteMachineName = NULL;
            if (machine.contains(pszPeerA))
            {
                uniquePeerName.insert(pszPeerB);
                strncpy(relay.szAddr, processName2Id[pszPeerB].c_str(), sizeof(relay.szAddr));
                iter = processName2ListenAddr.find(pszPeerB);
                remoteMachineName = &processName2MachineName.find(pszPeerB)->second;
            } else
            {
                uniquePeerName.insert(pszPeerA);
                strncpy(relay.szAddr, processName2Id[pszPeerA].c_str(), sizeof(relay.szAddr));
                iter = processName2ListenAddr.find(pszPeerA);
                remoteMachineName = &processName2MachineName.find(pszPeerA)->second;
            }
            assert(processName2ListenAddr.end() != iter);
            if (uniquePeerCount != uniquePeerName.size())
            {
                strncpy(relay.szMConn, iter->second.c_str(), sizeof(relay.szMConn));

                snprintf(relay.szDesc, sizeof(relay.szDesc), "%s[%s] TO %s[%s]",
                         stMachine.szMachineName, stMachine.szIP,
                         remoteMachineName->c_str(), iter->second.c_str());

                machine.stRelay.dwUsedCnt++;
            }
        }
    }

    return 0;
}

int TbusMultiConf::initMachineDesc(MachineDesc& machine,
                                   const MACHINE_DESC& stMachine,
                                   const COMMUNICATION_RELATION& stCommunication)
{
    machine.outputDir = outputDir + string(stMachine.szMachineName) + "_" + string(stMachine.szIP);

    char strPort[6];
    snprintf(strPort, sizeof(strPort), "%d", stMachine.wListenPort);
    strPort[sizeof(strPort)-1] = '\0';
    machine.listenAddr = string("tcp://") + stMachine.szIP + ":" + strPort + "?reuse=1";

    int iRet = initMachineGCIM(machine, stMachine, stCommunication);
    if (0 != iRet)
    {
        return -1;
    }

    iRet = initMachineRelay(machine, stMachine, stCommunication);
    if (0 != iRet)
    {
        return -1;
    }

    return 0;
}

int TbusMultiConf::initMachineDescList(const MULTI_MACHINES& stDesc)
{
    for (unsigned int i = 0; i < dwMachineCount; i++)
    {
        const MACHINE_DESC& stMachine = stDesc.stMachineList.astMachine[i];

        int iRet = initProcessName2Id(pMachine[i], stMachine);
        if (0 != iRet)
        {
            return -1;
        }
    }

    for (unsigned int i = 0; i < dwMachineCount; i++)
    {
        const MACHINE_DESC& stMachine = stDesc.stMachineList.astMachine[i];

        int iRet = initMachineDesc(pMachine[i], stMachine, stDesc.stCommunication);
        if (0 != iRet)
        {
            return -1;
        }
    }

    return 0;
}

int TbusMultiConf::initProcessName2Machine(const MULTI_MACHINES& stDesc)
{
    for (unsigned int i = 0; i < dwMachineCount; i++)
    {
        const MACHINE_DESC& stMachine = stDesc.stMachineList.astMachine[i];
        const PROCESS_ARRAY& processList = stMachine.stProcessList;

        char strPort[6];
        snprintf(strPort, sizeof(strPort), "%d", stMachine.wListenPort);
        strPort[sizeof(strPort)-1] = '\0';
        const string listenAddr = string("tcp://") + stMachine.szIP + ":" + strPort;

        for (unsigned int j = 0; j < processList.dwProcessCount; j++)
        {
            const string& szProcessName = processList.astProcess[j].szName;
            if (processName2ListenAddr.end() == processName2ListenAddr.find(szProcessName))
            {
                processName2ListenAddr[szProcessName] = listenAddr;
            }
            if (processName2MachineName.end() == processName2MachineName.find(szProcessName))
            {
                processName2MachineName[szProcessName] = stMachine.szMachineName;
            }
        }
    }

    return 0;
}
