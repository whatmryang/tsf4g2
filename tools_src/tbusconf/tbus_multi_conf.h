#ifndef _TBUS_MULTI_CONF_H_
#define _TBUS_MULTI_CONF_H_

#include <set>
#include <map>
#include <string>

#include "tbus_desc.h"
#include "tbus_machine_desc.h"
#include "tbus_comm_relation_desc.h"

class TbusMultiConf
{
    public:
        TbusMultiConf() { pMachine = NULL; }
        ~TbusMultiConf()
        {
            if (NULL != pMachine)
                delete [] pMachine;
        }

        int init(const MULTI_MACHINES& stDesc, const std::string& outputDir);

        int outputConf();

        const char* getOutDir()
        {
            return outputDir.c_str();
        }

    private:
        int initProcessWithTbus(const MULTI_MACHINES& stDesc);
        int initProcessName2Machine(const MULTI_MACHINES& stDesc);
        int initProcessName2Id(MachineDesc& machine,const MACHINE_DESC& stMachine);

        int initMachineDescList(const MULTI_MACHINES& stDesc);

        int initMachineDesc(MachineDesc& machine,
                            const MACHINE_DESC& stMachine,
                            const COMMUNICATION_RELATION& stCommunication);

        int initMachineGCIM(MachineDesc& machine,
                            const MACHINE_DESC& stMachine,
                            const COMMUNICATION_RELATION& stCommunication);

        int initMachineRelay(MachineDesc& machine,
                             const MACHINE_DESC& stMachine,
                             const COMMUNICATION_RELATION& stCommunication);

    private:
        unsigned int dwMachineCount;
        MachineDesc* pMachine;

    private:
        std::string outputDir;
        std::set<std::string> processWithTbus;
        std::map<std::string, std::string> processName2ListenAddr;
        std::map<std::string, std::string> processName2MachineName;
        std::map<std::string, std::string> processName2Id;
};

#endif
