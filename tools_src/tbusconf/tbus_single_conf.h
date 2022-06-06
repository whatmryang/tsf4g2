#ifndef _TBUS_SINGLE_CONF_H_
#define _TBUS_SINGLE_CONF_H_

#include <set>
#include <map>
#include <string>

#include "tbus_machine_desc.h"
#include "tbus_comm_relation_desc.h"

class TbusSingleConf
{
    public:
        int init(const SINGLE_MACHINE& stDesc, const std::string& outputDir);

        int outputConf();

        const char* getOutDir()
        {
            return machineDesc.outputDir.c_str();
        }

    private:
        int initName2Id(const SINGLE_MACHINE& stDesc);
        int fillGCIMData(const SINGLE_MACHINE& stDesc);

    private:
        MachineDesc machineDesc;

    private:
        std::set<std::string> processWithTbus;
};

#endif
