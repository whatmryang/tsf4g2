#ifndef _TBUS_GEN_CONF_H_
#define _TBUS_GEN_CONF_H_

#include <map>
#include <string>

#include "tbus_option.h"
#include "tbus_comm_relation_desc.h"

class TbusGenConf
{
    public:
        int process(const TbusOption& option);

    private:
        int genSingleMachineConf(const TbusOption& option);
        int genMultiMachinesConf(const TbusOption& option);

        static int loadCommDesc(LPTDRMETA pstMeta, TDRDATA* pstMemData, const char* pszDescFile);
};

#endif
