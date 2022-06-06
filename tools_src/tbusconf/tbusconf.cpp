#include "tbus_option.h"
#include "tbus_gen_desc.h"
#include "tbus_gen_conf.h"

#include <cstdio>
#include <cassert>

int tbus_gen_desc_file(const TbusOption& option);
int tbus_gen_tbus_conf(const TbusOption& option);

int main(int argc, char *argv[])
{
    int iRet = 0;

    TbusOption option;
    iRet = option.parse(argc, argv);
    if (0 != iRet || option.bNeedExit)
    {
        return 0;
    }

    assert(TbusOption::TBUS_OPER_UNDEFINE != option.eOperID);

    switch (option.eOperID)
    {
        case TbusOption::TBUS_OPER_GEN_DESC:
            iRet = tbus_gen_desc_file(option);
            break;
        case TbusOption::TBUS_OPER_GEN_CONF:
            iRet = tbus_gen_tbus_conf(option);
            break;
        default:
            break;
    }

    return 0;
}

int tbus_gen_desc_file(const TbusOption& option)
{
    TbusGenDesc genDesc;

    return genDesc.process(option);
}

int tbus_gen_tbus_conf(const TbusOption& option)
{
    TbusGenConf genConf;

    return genConf.process(option);
}
