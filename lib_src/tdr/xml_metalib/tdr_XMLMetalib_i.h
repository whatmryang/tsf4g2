/**
*
* @file     tdr_XMLMetalib_i.h 
* @brief    元数据描述库与XML描述相互转换 内部函数
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-03-22 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_XMLMETALIB_I_H
#define TDR_XMLMETALIB_I_H


#include <stdio.h>
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_types.h"


/**根据参数为元数据库分配空间，并初始化
*@param[out] pstLib 创建的元数据库
*@Param[in]	stLibParam 初始化元数据的参数
*@return 成功返回0，否则返回错误代码
*/
int tdr_init_metalib_i(LPTDRMETALIB *ppstLib, LPTDRLIBPARAM pstLibParam);





#endif /*TDR_XMLMETALIB_I_H*/
