/**
 *
 * @file     read_write_entry.c
 * @brief    通过entry描述句柄直接访问内存数据
 *
 * @note TDR提供的相关API函数有: tdr_get_entry_version tdr_get_entry_offset
 * tdr_get_entry_unitsize tdr_get_entry_refer
 *
 *
 * @author laynehu
 * @version 1.0
 * @date 2011-05-16
 *
 *
 * Copyright (c)  2011, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include "tdr/tdr.h"
#include "net_protocol.h"

#define VERSION_ONE 11

typedef struct _entry_info
{
    int iOffset;    /*偏移*/
    int iUnitsize;  /*单元大小*/
    int iCount;     /*数组长度*/
}EntryInfo, *LPEntryInfo;

int get_entry_info(LPEntryInfo pstEntryInfo,
                   LPTDRMETAENTRY pMetaEntry,
                   int targetversion);
void ex_init_host_pkg_data(LPPKG pstHostData);

int main()
{
    LPTDRMETALIB pstLib = NULL;
    LPTDRMETA pstMeta = NULL;
    LPTDRMETAENTRY pstMetaEntry = NULL;
    PKG stPkgData;
    int iRet;

    /*根据xml文件创建元数据库*/
    iRet = tdr_create_lib_file(&pstLib, "net_protocol.xml", 1, stderr);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("tdr_create_lib_file failed by file %s, for %s\n",
               "net_protocol.xml", tdr_error_string(iRet));
        return iRet;
    }


    /*获取net_protocol.xml文件中Pkg数据结构在元数据库中的句柄*/
    pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");

    if (NULL == pstMeta)
    {
        printf("tdr_get_meta_by_name failed!\n");
    }
    else
    {
        pstMetaEntry = tdr_get_entry_by_path(pstMeta, "head.cmd");

        if (NULL == pstMetaEntry)
        {
            printf("tdr_get_entry_by_path failed!\n");
        }
        else
        {
            EntryInfo stEntryInfo;
            int iTargetVersion = VERSION_ONE;

            ex_init_host_pkg_data(&stPkgData);

            /*读取PKG结构中的entry的信息*/
            if (0 != get_entry_info(&stEntryInfo, pstMetaEntry, iTargetVersion))
            {
                printf("this entry does not match with target version!\n");
            }
            else
            {
                int16_t *pwCmd = NULL;

                printf("entry attribute --> Offset:%d\tUnitsize:%d\tCount:%d\n",
                       stEntryInfo.iOffset, stEntryInfo.iUnitsize, stEntryInfo.iCount);

                /*对数据进行访问操作；如果成员是数组则通过下标访问*/
                pwCmd = (int16_t *)((char *)&stPkgData + stEntryInfo.iOffset);

                printf("read nCmd:%hd\n", *pwCmd);

                printf("stHead.nCmd before write:%d\n", stPkgData.stHead.nCmd);
                *pwCmd = 101;
                printf("stHead.nCmd after write:%d\n", stPkgData.stHead.nCmd);
            }
        }

    }
    /*使用完毕释放元数据的空间*/
    tdr_free_lib(&pstLib);

    return 0;
}


int get_entry_info(LPEntryInfo pstEntryInfo, LPTDRMETAENTRY pMetaEntry, int targetversion)
{
    int iVersion = tdr_get_entry_version(pMetaEntry);
    if (iVersion > targetversion)
    {
        printf("This entry does not exist!\n");
        return -1;
    }
    else
    {
        pstEntryInfo->iOffset = tdr_get_entry_offset(pMetaEntry);
        pstEntryInfo->iUnitsize =  tdr_get_entry_unitsize(pMetaEntry);
        pstEntryInfo->iCount = tdr_get_entry_count(pMetaEntry);

        return 0;
    }
}




void ex_init_host_pkg_data(LPPKG pstHostData)
{
    pstHostData->stHead.dwMsgid = 100;
    pstHostData->stHead.nCmd = 0;
    pstHostData->stHead.nVersion = 9;
    pstHostData->stHead.nMagic = 0x3344;
    tdr_str_to_tdrdatetime(&pstHostData->stHead.tTime, "2011-05-16 13:43:45");

    strncpy(pstHostData->stBody.stLogin.szName, "tencent", sizeof(pstHostData->stBody.stLogin.szName));
    strncpy(pstHostData->stBody.stLogin.szPass, "secret", sizeof(pstHostData->stBody.stLogin.szPass));
    strncpy(pstHostData->stBody.stLogin.szZone, "shenzhen", sizeof(pstHostData->stBody.stLogin.szZone));
}
