/*
 * tmemlog.c
 *
 *  Created on: 2010-7-12
 *      Author: mickeyxu
 */

#include <stdio.h>

#include "version.h"
#include "pal/pal.h"
#include "TmemlogOption.h"

#include "../lib_src/tlog/tlogmem.h"
#include "../lib_src/tlog/tlog_i.h"
#include "../lib_src/tloghelp/tlogload_i.h"
#include "../lib_src/tmng/err_stack.h"

int tmemlog_fallow_uri(char *pszUri)
{
    return 0;
}

int main(int argc, char **argv)
{
    int iRet;
    Option opt;
    iRet = opt.Parse(argc, argv);
    if (iRet)
    {
        return -1;
    }

    if (opt.iNeedExit)
    {
        return -1;
    }

    // 将配置文件中的uri导出到文件
    if (opt.pszLogConf)
    {
        if (!opt.iFollow)
        {
            tmemlog_dump_tlog_conf(opt.pszLogConf);
        }
        return -1;
    }

    // 将指定uri中的数据导出到指定的文件中
    if (opt.pszUri && opt.pszOutPutFile)
    {
        // 构造一个tlogmemconf 和tlogmemdev，并填写相应的成员。
        TLOGDEVMEM stMemCfg;

        memset(&stMemCfg, 0, sizeof(stMemCfg));
        STRNCPY(stMemCfg.szUri,opt.pszUri,sizeof(stMemCfg.szUri));
        stMemCfg.dwSizeLimit = 10240000;
        STRNCPY(stMemCfg.szPattern,opt.pszOutPutFile,sizeof(stMemCfg.szPattern));

        tmemlog_dump_memdev(&stMemCfg);
        return 0;
    }

    if (opt.pszUri && !opt.pszOutPutFile)
    {
        printf("必须指定输出文件名：使用-o选项\n");
        return -1;
    }

    if (opt.pszOutPutFile)
    {
        printf("必须通过选项--uri指定memlog所在的uri\n");
        return -1;
    }

    // 什么操作都没做，就打印帮助信息，并返回错误
    opt.Help(argc, argv);
    return -1;
}
