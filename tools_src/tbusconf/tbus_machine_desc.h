#ifndef _TBUS_MACHINE_DESC_H_
#define _TBUS_MACHINE_DESC_H_

#include <set>
#include <map>
#include <string>

#include "tbus_desc.h"
#include "tbus_comm_relation_desc.h"

class MachineDesc
{
    public:
        int init();
        int initGCIM();
        int initRelay();
        int output(std::set<std::string>& processWithTbus);

        bool contains(const std::string& processName)
        {
            return (processName2Id.end() != processName2Id.find(processName));
        }

    private:
        int outputGCIM();
        int outputRelay();

        int outputTbusdConf();
        int outputAddrList(std::set<std::string>& processWithTbus);

    public:
        std::string outputDir;
        std::map<std::string, std::string> processName2Id;
        TBUSGCIM stGCIM;

        std::string listenAddr;
        RELAYMNGER stRelay;
};

#endif
