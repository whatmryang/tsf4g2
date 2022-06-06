#include "tbus_conf_mgr.h"
#include <assert.h>


void tbus_init_conf_info(LPTBUSCONFINFO a_pstConfInfo)
{
    assert(NULL != a_pstConfInfo);

    a_pstConfInfo->pstConf    = NULL;
    a_pstConfInfo->iMetaIdx   = -1;
    a_pstConfInfo->pszFile    = NULL;
    a_pstConfInfo->iFormat    = TDR_IO_NEW_XML_VERSION;
    a_pstConfInfo->pszErr     = NULL;
    a_pstConfInfo->iSize      = 0;
}

int tbus_load_conf(LPTBUSCONFINFO a_pstConf)
{
    return 0;
}
