#ifndef _TBUS_CONF_MGR_H_
#define _TBUS_CONF_MGR_H_

#include "tdr/tdr.h"
#include "tbus_desc_ex.h"
#include "tbus_desc_mgr.h"


#ifdef __cplusplus
extern "C" {
#endif

struct TbusConfInfo {
    OUT LPTBUSCONF  pstConf;   //保存配置的内存结构指针
    IN  int         iMetaIdx;  //可选的元数据索引,默认包
    IN  const char* pszFile;   //配置文件路径
    IN  int         iFormat;   //配置文件格式
    OUT char*       pszErr;    //保存错误信息的缓冲
    IN  unsigned    iSize;     //保存错误信息的缓冲的大
};

typedef struct TbusConfInfo    TBUSCONFINFO;
typedef struct TbusConfInfo*   LPTBUSCONFINFO;


void tbus_init_conf_info(LPTBUSCONFINFO a_pstConfInfo);

int tbus_load_conf(LPTBUSCONFINFO a_pstConf);

#ifdef __cplusplus
}
#endif

#endif
