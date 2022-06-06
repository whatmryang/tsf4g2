/**
 *
 * @file     get_meta_entry.c
 * @brief    从元数据库中查找某结构的描述句柄
 * @note 当利用TDR对数据结构进行初始化,正则化,网络编/解码,输入/输出,打印等操作时,
 * 先必须获取此数据结构在元数据库中的描述句柄,TDR才能正确地对数据进行处理.
 *
 * @note TDR提供的相关API函数有: tdr_get_meta_by_name tdr_get_meta_by_id
 * tdr_get_meta_based_version tdr_get_meta_current_version
 * tdr_get_entry_by_name tdr_get_entry_by_id
 *
 * @author jackyai
 * @version 1.0
 * @date 2007-06-15
 *
 *
 * Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
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
        printf("tdr_get_meta_by_name Pkg failed！\n");
    } else
    {
        iVersion = tdr_get_meta_current_version(pstMeta);
    }

    /* 下面演示如何取得 meta 的 entry 指针 */
    /*方法1 先获取下标，再获取成员句柄*/
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

    /*方法2，按名字获取成员句柄*/
    if (0 == iRet && pstMeta != NULL)
    {
        pstMetaEntry = tdr_get_entryptr_by_name(pstMeta, "head");
        if (NULL == pstMetaEntry)
        {
            printf("tdr_get_entryptr_by_name failed!\n");
        }
    }


    /*方法3,按路径获取成员句柄*/
    if (0 == iRet && pstMeta != NULL)
    {
        pstMetaEntry = tdr_get_entry_by_path(pstMeta, "head.cmd");
        if (NULL == pstMetaEntry)
        {
            printf("tdr_get_entry_by_path failed!\n");
        }
    }

    /*
    *获取数据结构的描述句柄后,通过该meta句柄获取meta里面的entry元素，
    *获取entry句柄后，就可以进一步操作entry了
    *
    */


    /*使用完毕释放元数据的空间*/
    tdr_free_lib(&pstLib);

    return 0;

}

