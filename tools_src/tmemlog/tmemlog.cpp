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

    // �������ļ��е�uri�������ļ�
    if (opt.pszLogConf)
    {
        if (!opt.iFollow)
        {
            tmemlog_dump_tlog_conf(opt.pszLogConf);
        }
        return -1;
    }

    // ��ָ��uri�е����ݵ�����ָ�����ļ���
    if (opt.pszUri && opt.pszOutPutFile)
    {
        // ����һ��tlogmemconf ��tlogmemdev������д��Ӧ�ĳ�Ա��
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
        printf("����ָ������ļ�����ʹ��-oѡ��\n");
        return -1;
    }

    if (opt.pszOutPutFile)
    {
        printf("����ͨ��ѡ��--uriָ��memlog���ڵ�uri\n");
        return -1;
    }

    // ʲô������û�����ʹ�ӡ������Ϣ�������ش���
    opt.Help(argc, argv);
    return -1;
}
