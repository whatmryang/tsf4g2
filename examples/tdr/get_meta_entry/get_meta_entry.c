/**
 *
 * @file     get_meta_entry.c
 * @brief    ��Ԫ���ݿ��в���ĳ�ṹ���������
 * @note ������TDR�����ݽṹ���г�ʼ��,����,�����/����,����/���,��ӡ�Ȳ���ʱ,
 * �ȱ����ȡ�����ݽṹ��Ԫ���ݿ��е��������,TDR������ȷ�ض����ݽ��д���.
 *
 * @note TDR�ṩ�����API������: tdr_get_meta_by_name tdr_get_meta_by_id
 * tdr_get_meta_based_version tdr_get_meta_current_version
 * tdr_get_entry_by_name tdr_get_entry_by_id
 *
 * @author jackyai
 * @version 1.0
 * @date 2007-06-15
 *
 *
 * Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
 * All rights reserved.
 *
 */

#include <stdio.h>
#include "tdr/tdr.h"



int main()
{
    LPTDRMETALIB pstLib = NULL;
    LPTDRMETA pstMeta = NULL;
    LPTDRMETAENTRY pstMetaEntry = NULL;
    int iIndex;
    int iVersion;
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
        printf("tdr_get_meta_by_name Pkg failed��\n");
    } else
    {
        iVersion = tdr_get_meta_current_version(pstMeta);
    }

    /* ������ʾ���ȡ�� meta �� entry ָ�� */
    /*����1 �Ȼ�ȡ�±꣬�ٻ�ȡ��Ա���*/
    if (0 == iRet && pstMeta != NULL)
    {
        iRet = tdr_get_entry_by_name(&iIndex, pstMeta, "head");
        if (TDR_ERR_IS_ERROR(iRet))
        {
            printf("tdr_get_entry_by_name head, for %s\n",
                   tdr_error_string(iRet));
        }
        if (0 == iRet)
        {
            pstMetaEntry = tdr_get_entry_by_index(pstMeta, iIndex);

            if (NULL == pstMetaEntry)
            {
                printf("tdr_get_entry_by_index failed!\n");
            }
        }

    }

    /*����2�������ֻ�ȡ��Ա���*/
    if (0 == iRet && pstMeta != NULL)
    {
        pstMetaEntry = tdr_get_entryptr_by_name(pstMeta, "head");
        if (NULL == pstMetaEntry)
        {
            printf("tdr_get_entryptr_by_name failed!\n");
        }
    }


    /*����3,��·����ȡ��Ա���*/
    if (0 == iRet && pstMeta != NULL)
    {
        pstMetaEntry = tdr_get_entry_by_path(pstMeta, "head.cmd");
        if (NULL == pstMetaEntry)
        {
            printf("tdr_get_entry_by_path failed!\n");
        }
    }

    /*
    *��ȡ���ݽṹ�����������,ͨ����meta�����ȡmeta�����entryԪ�أ�
    *��ȡentry����󣬾Ϳ��Խ�һ������entry��
    *
    */


    /*ʹ������ͷ�Ԫ���ݵĿռ�*/
    tdr_free_lib(&pstLib);

    return 0;

}

