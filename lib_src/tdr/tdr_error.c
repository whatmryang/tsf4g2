/**
*
* @file     tdr_error.c
* @brief    TDR错误处理函数
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-04
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <assert.h>
#include <stdio.h>

#include "tdr/tdr_error.h"
#include "tdr_error_i.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

const char * tdr_error_string(IN int iErrorCode)
{
    int iTdrErrno = TDR_ERR_GET_ERROR_CODE(iErrorCode);
    const char *pchStr = NULL;

    static const char *message[] = {
        "没有错误",
        "剪裁版本错误，其取值应大于元数据基准版本,并且小于元数据的当前版本\n"
            "如果元数据有版本指示器，则剪裁版本必须不小于版本指示器允许的最小版本",
		"网络编码信息缓冲区剩余空间不够",
		"此元素的refer属性值不正确，其值不能为负数且必须比count属性值小",
		"元数据描述复合数据类型嵌套层次超过32层",
		"不支持的元数据类型",
		"string类型的元素其字符串长度超过了预定最大长度",
		"本地存储缓冲区剩余空间不够",
		"元素数版本指示器的值不正确",
        "分配存储空间失败",
        "解析XML信息错误",
        "XML元素树中没有根元素",
        "无效的元数据根元素",
        "元数据描述名字空间冲突，即不能将根元素name属性值不同的信息加到同一个库中",
        "宏定义元素必须指定name属性",
        "include元素必须定义file属性",
        "没有指定version属性",
        "根元素ID冲突，即不能将根元素ID属性值不同的信息加到同一个库中",
        "不支持的元数据描述XML标签集版本",
        "元数据库参数不正确",
        "元数据库校验和不正确",
        "需加到元数据描述库中的宏定义数比预定义的要多",
        "宏定义元素没有值属性",
        "不支持的数据类型",
        "元数据描述库根元素必须指定name属性",
        "没有足够的空间存储自定义数据类型",
        "字符串缓冲区空间不够",
        "union/stuct元素必须包含name属性",
        "同类型的union和stuct元素不容许同名",
        "该宏名没有定义",
        "同一父元素下不能出现ID相同的子元素",
        "entry元素必须包含type属性",
        "entry的type属性值无效",
        "entry的io属性值无效",
        "entry的unique属性不正确，正确取值为false,true",
        "entry的notnull属性不正确，正确取值为false,true",
        "entry的size属性值不正确",
        "entry的sortkey属性值不正确",
        "entry的select属性值不正确",
        "entry的maxid属性不正确",
        "entry的minid属性不正确",
        "entry的minid和maxid属性值不正确",
        "entry的count属性值不正确",
		"entry的id属性值不正确",
		"entry的default属性值不正确",
		"entry的sortmethod属性值不正确",
		"entry的datetime属性值不正确",
		"entry的date属性值不正确",
		"entry的time属性值不正确",
		"entry的ip属性值不正确",
		"entry的extendtotable属性不正确",
        "struct元素的size属性不正确",
        "struct元素的align属性值不正确",
        "struct元素的versionindicator属性不正确",
        "元素的sizetype/sizeinfo属性值不正确",
		"struct元素的splittablefactor属性值不正确",
		"struct元素的primarykey属性值不正确",
        "index元素的column属性值不正确",
		"struct元素的splittablekey属性值不正确",
		"struct元素的splittablerule属性值不正确",
		"struct元素的strictinput属性值不正确",
		"struct元素的dependonstruct属性值不正确",
        "元素的path不正确，不能正确匹配meta中的元素",
        "元素的偏移值不对",
        "将信息写到缓冲区时空间不够",
        "自定义数据类型没有包含任何子成员",
        "entry元素的refer属性值不正确",
        "entry元素的sizeinfo属性值不正确",
		"不支持的IO流",
		"写文件失败",
		"打开文件写失败",
		"二进制元数据库文件无效,请确定生成元数据文件的TDR库函数构建版本和读取此文件的TDR库函数构建版本一致",
		"打开文件读失败",
        "可以变数组必须指定refer属性",
        "元数据中sizeinfo成员前的成员的存储空间必须是固定的",
		"中文字符串转换成unicode字符串失败",
		"entry元素的值不满足键约束",
		"不支持的数据库管理系统DBMS",
		"不支持为复合数据类型数组成员生成建表语句",
		"连接数据库服务器失败",
		"不支持的数据操作",
		"该剪裁版本无法生成有效的主键信息",
		"执行数据库SQL语句失败",
		"数据库连接错误",
		"取SQL查询结果失败",
		"SQL查询结果集为空",
		"结果集中没有更多的数据记录或出现了错误",
		"当前数据行中不存在指定的数据域",
		"不支持为存储空间不固定的结构生成建表语句",
		"生成元数据库文件的tdr工具的核心数据构建版本和tdr库的核心数据构构建版本不一致",
		"元数据库的散列值和期望的散列值不一致",
		"结构体成员的实际索引数与预计的不一致",
		"成员的vesion属性值不正确",
		"数据所在的数据库表是分表存储的,但此数据的元数据描述没有指定分表关键字",
		"处理macrosgroup属性失败",
		"entry的bindmacrosgroup属性值无效",
		"成员取值已经超出此类型的值域范围",
		"在可扩展的结构体数组成员中定义了不容许出现的属性",
		"当发现复合数据类型数据成员时，调用回调失败",
		"当发现简单数据类型成员时,调用回调函数失败",
		"成员的autoincrement属性无效，目前只有非数组整数数据类型的成员才能定义此属性",
		"成员的custom属性值无效,请确定属性值的长度不超过最大长度限制",
		"结构体的uniqueentryname属性值无效,此属性的属性值只能为true/false",
		"将结构体成员展开存储时会出现重名",
		"参数无效，请检查每个参数是否满足接口的前置条件约束",
		"在展开存储时需要添加前缀的结构体不支持建立所有成员名字偏移索引表",
		"设置db属性失败",
		"结构体成员中，只容许最后一个成员是可变数组类型的成员",
		"结构题成员中，没有定义size属性的string类型成员(其存储空间是可变的)只能作为最后一个成员，且不能为数组",
        "名字串太长，超过了最大长度限制",
        "生成元数据库文件的系统平台与生成tdr库的系统平台不一致，请注意32位平台与64位平台的区别",
        "xml文件中存在无法解析的include关系",
        "网络数据包中的版本指示器值不正确，其取值应大于元数据基准版本,并且小于元数据的当前版本\n"
            "如果元数据有版本指示器，则版本指示器的值必须不小于版本指示器允许的最小版本",
        "entry的compress属性不正确",
        "entry的compress属性和extendtotable属性冲突",
        "数据压缩失败",
        "数据解压缩失败",
        "创建JSON解析器失败",
        "解析到JSON数据流结束",
        "JSON数据流的格式不正确",
        "输入缓冲区的长度太小，未包括完整的metalib",
        "sizeinfo引用的字段对应的内存空间太小，设置size值时溢出",
    };

    assert((sizeof(message) / sizeof(message[0])) == TDR_ERROR_COUNT);

    if (!TDR_ERR_IS_ERROR(iErrorCode))
    {
        pchStr = "没有错误";
    }else  if ((iTdrErrno < 0) || (iTdrErrno > TDR_ERROR_COUNT))
    {
        pchStr =  "未知错误";
    }
    else
    {
        pchStr = message[iTdrErrno];
    }

    return pchStr;
}
