#include <map>
#include <string>
#include <fstream>
#include <cassert>
#include <cstring>

#include "tdr/tdr.h"
#include "tbus_tools.h"
#include "tbus_single_conf.h"
#include "tbus_comm_relation_desc.h"

using std::set;
using std::map;
using std::endl;
using std::string;
using std::ofstream;

int TbusSingleConf::init(const SINGLE_MACHINE& stDesc, const std::string& outputDir)
{
    this->machineDesc.outputDir = outputDir;

    int iRet = initName2Id(stDesc);
    if (0 != iRet)
    {
        return -1;
    }

    iRet = machineDesc.initGCIM();
    if (0 != iRet)
    {
        return -1;
    }

    iRet = fillGCIMData(stDesc);
    if (0 != iRet)
    {
        return -1;
    }

    return 0;
}

int TbusSingleConf::initName2Id(const SINGLE_MACHINE& stDesc)
{
    const PROCESS_ARRAY& stList = stDesc.stProcessList;
    for (unsigned int i = 0; i < stList.dwProcessCount; i++)
    {
        machineDesc.processName2Id[stList.astProcess[i].szName] = stList.astProcess[i].szId;
    }

    return 0;
}

int TbusSingleConf::fillGCIMData(const SINGLE_MACHINE& stDesc)
{
    strncpy(machineDesc.stGCIM.szGCIMShmKey, stDesc.szGCIMShmKey, sizeof(machineDesc.stGCIM.szGCIMShmKey));
    strncpy(machineDesc.stGCIM.szAddrTemplet, stDesc.szAddrTemplet, sizeof(machineDesc.stGCIM.szAddrTemplet));

    unsigned int i =0;
    for (i = 0; i < stDesc.stCommunication.dwConnectionCount; i++)
    {
        CHANNELCNF& stChannel  = machineDesc.stGCIM.astChannels[i];
        const CONNECTION_DESC& stConnect = stDesc.stCommunication.astConnection[i];

        stChannel.dwSendSize = stConnect.dwBufferSize;
        stChannel.dwRecvSize = stConnect.dwBufferSize;
        stChannel.dwAddressCount = 2;

        processWithTbus.insert(stConnect.szNameOfProcessA);
        processWithTbus.insert(stConnect.szNameOfProcessB);

        map<string, string>::const_iterator iter;
        iter = machineDesc.processName2Id.find(stConnect.szNameOfProcessA);
        assert(machineDesc.processName2Id.end() != iter);
        const string& idOfProcessA = iter->second;

        iter = machineDesc.processName2Id.find(stConnect.szNameOfProcessB);
        assert(machineDesc.processName2Id.end() != iter);
        const string& idOfProcessB = iter->second;

        strncpy(stChannel.aszAddress[0], idOfProcessA.c_str(), sizeof(stChannel.aszAddress[0]));
        strncpy(stChannel.aszAddress[1], idOfProcessB.c_str(), sizeof(stChannel.aszAddress[1]));
        strncpy(stChannel.szDesc, "tbusconf generated channel", sizeof(stChannel.szDesc));
        snprintf(stChannel.szDesc, sizeof(stChannel.szDesc), "%s --- %s",
                 stConnect.szNameOfProcessA, stConnect.szNameOfProcessB);
    }

    machineDesc.stGCIM.dwCount = i;

    return 0;
}

int TbusSingleConf::outputConf()
{
    int iRet = machineDesc.output(processWithTbus);
    if (0 != iRet)
    {
        return iRet;
    }

    return 0;
}
