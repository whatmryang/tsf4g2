#ifndef _TBUS_GEN_DESC_H_
#define _TBUS_GEN_DESC_H_

#include "tbus_option.h"
#include "tdr/tdr.h"

class TbusGenDesc
{
    public:
        int process(const TbusOption& option);

    private:
        int genSingleMachineDesc(const TbusOption& option);
        int genMultiMachinesDesc(const TbusOption& option);
        int genMachinesDesc(const TbusOption& option, LPTDRMETA pstMeta);
};

#endif
