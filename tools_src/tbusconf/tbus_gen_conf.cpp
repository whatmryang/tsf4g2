#include <set>
#include <map>
#include <string>
#include <fstream>
#include <cassert>

#include "tdr/tdr.h"
#include "tbus_tools.h"
#include "tbus_gen_conf.h"
#include "tbus_multi_conf.h"
#include "tbus_single_conf.h"
#include "tbus_check_multi_desc.h"
#include "tbus_check_single_desc.h"
#include "tbus_comm_relation_desc.h"

using std::set;
using std::map;
using std::endl;
using std::string;
using std::ofstream;

int TbusGenConf::process(const TbusOption& option)
{
    assert(TbusOption::TBUS_COM_TYPE_UNDEFINE != option.eComType);

    int iRet = 0;
    switch (option.eComType)
    {
        case TbusOption::TBUS_COM_TYPE_SINGLE:
            iRet = genSingleMachineConf(option);
            break;
        case TbusOption::TBUS_COM_TYPE_MULTI:
            iRet = genMultiMachinesConf(option);
            break;
        default:
            iRet = -1;
    }

    return iRet;
}

int TbusGenConf::genSingleMachineConf(const TbusOption& option)
{
    LPTDRMETA pstMeta = TbusTools::getSingleMeta();
    if (NULL == pstMeta)
    {
        return -1;
    }

    SINGLE_MACHINE* pstCommDesc = new(std::nothrow) SINGLE_MACHINE;
    if (NULL == pstCommDesc)
    {
        fprintf(stderr, "Error: failed to new SINGLE_MACHINE\n");
        return -1;
    }

    TDRDATA stMemData;
    stMemData.pszBuff = (char*)pstCommDesc;
    stMemData.iBuff   = sizeof(*pstCommDesc);

    int iRet = loadCommDesc(pstMeta, &stMemData, option.pszDescFile);
    if (0 == iRet)
    {
        iRet = CheckSingleDesc::check(*pstCommDesc);
    }

    if (0 == iRet)
    {
        string outputDir;
        if (NULL != option.pszOutDir)
        {
            outputDir = option.pszOutDir;
        } else
        {
            outputDir = "localhost";
        }

        TbusSingleConf* pSingleConf = new (std::nothrow) TbusSingleConf;
        if (NULL == pSingleConf)
        {
            fprintf(stderr, "Error: no memory available\n");
            iRet = -1;
        } else
        {
            iRet = pSingleConf->init(*pstCommDesc, outputDir);
            if (0 == iRet)
            {
                iRet = pSingleConf->outputConf();
                if (0 == iRet)
                {
                    fprintf(stdout, "成功生成单机通信tbus配置文件，保存在目录 '%s' 中。\n", outputDir.c_str());
                }
            }

            delete pSingleConf;
        }

    }

    delete pstCommDesc;

    return iRet;
}

int TbusGenConf::genMultiMachinesConf(const TbusOption& option)
{
    LPTDRMETA pstMeta = TbusTools::getMultiMeta();
    if (NULL == pstMeta)
    {
        return -1;
    }

    MULTI_MACHINES* pstCommDesc = new(std::nothrow) MULTI_MACHINES;
    if (NULL == pstCommDesc)
    {
        fprintf(stderr, "Error: failed to new MULTI_MACHINES\n");
        return -1;
    }

    TDRDATA stMemData;
    stMemData.pszBuff = (char*)pstCommDesc;
    stMemData.iBuff   = sizeof(*pstCommDesc);

    int iRet = loadCommDesc(pstMeta, &stMemData, option.pszDescFile);
    if (0 == iRet)
    {
        iRet = CheckMultiDesc::check(*pstCommDesc);
    }

    if (0 == iRet)
    {
        string outputDir;
        if (NULL != option.pszOutDir)
        {
            outputDir = option.pszOutDir;
        } else
        {
            outputDir = "multi_machines";
        }

        TbusMultiConf* pMultiConf = new (std::nothrow) TbusMultiConf();
        if (NULL == pMultiConf)
        {
            fprintf(stderr, "Error: no memory available\n");
            iRet = -1;
        } else
        {
            iRet = pMultiConf->init(*pstCommDesc, outputDir);
            if (0 == iRet)
            {
                iRet = pMultiConf->outputConf();
                if (0 == iRet)
                {
                    fprintf(stdout, "成功生成跨机通信tbus配置文件，保存在目录 '%s' 中。\n", outputDir.c_str());
                }
            }

            delete pMultiConf;
        }
    }
    delete pstCommDesc;

    return iRet;
}

int TbusGenConf::loadCommDesc(LPTDRMETA pstMeta, TDRDATA* pstMemData, const char* pszDescFile)
{
    int iRet = tdr_input_file(pstMeta, pstMemData, pszDescFile,
                              0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        fprintf(stderr, "Error: failed to read communication description from file '%s', for %s\n",
                pszDescFile, tdr_error_string(iRet));
        iRet = -1;
    } else
    {
        iRet = 0;
    }

    return iRet;
}
