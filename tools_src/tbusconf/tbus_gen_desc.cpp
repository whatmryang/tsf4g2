#include "tbus_tools.h"
#include "tbus_gen_desc.h"

#include <cstdlib>
#include <cassert>

int TbusGenDesc::process(const TbusOption& option)
{
    int iRet = 0;

    assert(TbusOption::TBUS_COM_TYPE_UNDEFINE != option.eComType);

    switch (option.eComType)
    {
        case TbusOption::TBUS_COM_TYPE_SINGLE:
            iRet = genSingleMachineDesc(option);
            break;
        case TbusOption::TBUS_COM_TYPE_MULTI:
            iRet = genMultiMachinesDesc(option);
            break;
        default:
            iRet = -1;
            break;
    }

    return iRet;
}

int TbusGenDesc::genSingleMachineDesc(const TbusOption& option)
{
    LPTDRMETA pstMeta = TbusTools::getSingleMeta();
    if (NULL == pstMeta)
    {
        return -1;
    }

    int iRet = genMachinesDesc(option, pstMeta);
    if (0 == iRet)
    {
        assert(NULL != TbusTools::getOutPath());
        fprintf(stdout, "成功生成单机通信关系描述模板, 保存在文件 '%s' 中\n", TbusTools::getOutPath());
    }

    return iRet;
}

int TbusGenDesc::genMultiMachinesDesc(const TbusOption& option)
{
    LPTDRMETA pstMeta = TbusTools::getMultiMeta();
    if (NULL == pstMeta)
    {
        return -1;
    }

    int iRet = genMachinesDesc(option, pstMeta);
    if (0 == iRet)
    {
        assert(NULL != TbusTools::getOutPath());
        fprintf(stdout, "成功生成跨机通信关系描述模板, 保存在文件 '%s' 中\n", TbusTools::getOutPath());
    }

    return iRet;
}

int TbusGenDesc::genMachinesDesc(const TbusOption& option, LPTDRMETA pstMeta)
{
    int iRet = 0;

    assert(NULL != pstMeta);

    const char* pszOutPath = TbusTools::getOutPath(option.pszOutDir, option.pszDescFile);
    if (NULL == pszOutPath)
    {
        return -1;
    }

    FILE* fpOut = fopen(pszOutPath, "wb");
    if (NULL == fpOut)
    {
        fprintf(stderr, "Error: can't open file '%s' to write\n", pszOutPath);
        return -1;
    }

    iRet = tdr_gen_xmltmplt(pstMeta, fpOut, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        fprintf(stderr, "Error: tdr_gen_xmltmplt failed, for %s\n", tdr_error_string(iRet));
        iRet = -1;
    } else
    {
        iRet = 0;
    }

    fclose(fpOut);

    return iRet;
}
