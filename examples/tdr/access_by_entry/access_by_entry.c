/**
 *
 * @file     read_write_entry.c
 * @brief    ͨ��entry�������ֱ�ӷ����ڴ�����
 *
 * @note TDR�ṩ�����API������: tdr_get_entry_version tdr_get_entry_offset
 * tdr_get_entry_unitsize tdr_get_entry_refer
 *
 *
 * @author laynehu
 * @version 1.0
 * @date 2011-05-16
 *
 *
 * Copyright (c)  2011, ��Ѷ�Ƽ����޹�˾���������з���
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
    int iOffset;    /*ƫ��*/
    int iUnitsize;  /*��Ԫ��С*/
    int iCount;     /*���鳤��*/
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

    /*����xml�ļ�����Ԫ���ݿ�*/
    iRet = tdr_create_lib_file(&pstLib, "net_protocol.xml", 1, stderr);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("tdr_create_lib_file failed by file %s, for %s\n",
               "net_protocol.xml", tdr_error_string(iRet));
        return iRet;
    }


    /*��ȡnet_protocol.xml�ļ���Pkg���ݽṹ��Ԫ���ݿ��еľ��*/
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

            /*��ȡPKG�ṹ�е�entry����Ϣ*/
            if (0 != get_entry_info(&stEntryInfo, pstMetaEntry, iTargetVersion))
            {
                printf("this entry does not match with target version!\n");
            }
            else
            {
                int16_t *pwCmd = NULL;

                printf("entry attribute --> Offset:%d\tUnitsize:%d\tCount:%d\n",
                       stEntryInfo.iOffset, stEntryInfo.iUnitsize, stEntryInfo.iCount);

                /*�����ݽ��з��ʲ����������Ա��������ͨ���±����*/
                pwCmd = (int16_t *)((char *)&stPkgData + stEntryInfo.iOffset);

                printf("read nCmd:%hd\n", *pwCmd);

                printf("stHead.nCmd before write:%d\n", stPkgData.stHead.nCmd);
                *pwCmd = 101;
                printf("stHead.nCmd after write:%d\n", stPkgData.stHead.nCmd);
            }
        }

    }
    /*ʹ������ͷ�Ԫ���ݵĿռ�*/
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
