/**
*
* @file     tdr_define_i.h
* @brief    内部使用的宏定义
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-02
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/


#ifndef TDR_DEFINE__I_H
#define TDR_DEFINE__I_H

#include "tdr/tdr_define.h"
#include "tdr/tdr_types.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#pragma warning(disable:4127)
#endif

/** @name 元数据标志值
* @{*/
#define TDR_FLAG_NETORDER	0x01
#define TDR_FLAG_COMPLEX		0x02

/** @}*/  //元数据标志值

/** @name meta标志值
* @{*/
#define TDR_META_FLAG_FIXSIZE	0x0001	/* a type that does not change size. */
#define TDR_META_FLAG_HAVE_ID        0x0002    /**< 设置了id属性*/
#define TDR_META_FLAG_RESOVLED	0x0004	/*是否已经解析的标志*/
#define TDR_META_FLAG_VARIABLE	0x0008  /*meta是否是可变的*/
#define TDR_META_FALG_STRICT_INPUT	0x0010 /*输入时输入文件的格式是否必须是严格的*/
#define TDR_META_FALG_HAVE_AUTOINVREMENT_ENTRY	0x0020 /*此结构体中包含autoincrement成员*/
#define TDR_META_FLAG_NEED_PREFIX_FOR_UNIQUENAME	0x0040	/*当成员展开存储时必须添加前缀来保证名字唯一性*/
/** @}*/


/**
build0009变更：
    a）结构体及成员名字串最大长度从32变成128；
    b）宏定义名字串最大长度从64编程128
    c）支持64位系统
*/

/**
 * build000a变更：
 * a) 增加iReserve字段到3个
 * b) 增加校验机制
 */

/**
 * build000b变更：
 * a) 增加index元素
 */
#define	TDR_BUILD		0x000b			/*构建版本，metalib的数据结构每变更一次，此版本号需加一*/

/* 记录引入checksum机制的 tdr build version */
#define TDR_CHECKSUM_VERSION 0x000a

/* 记录引入<index>标签的时间 */
#define TDR_INDEX_VERSION 0x000b

#define TDR_DEFAULT_ALIGN_VALUE		1	/**<tdr声明的字节对齐值*/




/**@name entry 标志
*@ {*/
#define TDR_ENTRY_FLAG_NONE		0x0000  /**< 无特殊标志*/
#define TDR_ENTRY_FLAG_RESOVLD          0x0001  /**<此entry已经正确解析*/
#define TDR_ENTRY_FLAG_POINT_TYPE		0x0002    /**<指针类型*/
#define TDR_ENTRY_FLAG_REFER_TYPE		0x0004    /**<引用类型*/
#define TDR_ENTRY_FLAG_HAVE_ID              0x0008        /**<entry定义了ID属性*/
#define TDR_ENTRY_FLAG_HAVE_MAXMIN_ID		0x0010		/**<entry定义了maxminid属性*/
#define TDR_ENTRY_FALG_FIXSIZE        0x0020        /**<entry的存储空间是固定的*/
#define TDR_ENTRY_FLAG_REFER_COUNT	0x0040	/*此成员是另外一个成员的数组计数器*/
#define TDR_ENTRY_FLAG_SELECTOR		0x0080	/*此成员是另外一个union成员的选择器*/
#define TDR_ENTRY_FLAG_STR_COMPRESS 0x0100	/*此int成员是字符串索引ID，这个标志位只对类型是int的成员有效*/
/*@ }*/

/**@name entry对象关系映射标志
*@ {*/
#define TDR_ENTRY_DB_FLAG_NONE		0x00  /**< 无特殊标志*/
#define TDR_ENTRY_DB_FLAG_UNIQUE	0x01	/**<其取值是unique的*/
#define TDR_ENTRY_DB_FLAG_NOT_NULL	0x02	/**<其取值是不为空NOT NULL的*/
#define TDR_ENTRY_DB_FLAG_EXTEND_TO_TABLE 0x04			/**<entry在建表时进行扩展*/
#define TDR_ENTRY_DB_FLAG_PRIMARYKEY 0x10			/**<此entry是主键组成部分*/
#define TDR_ENTRY_DB_FLAG_AUTOINCREMENT	0x20		/**<此成员为autoincremnet成员*/
#define TDR_ENTRY_DB_FLAG_COMPRESS 0x40			/**<blob压缩存储*/
/*@ }*/

#define TDR_MAX_PATH       260      /**<路径名最大长度*/

#define TDR_MAX_HPP_STRING_WIDTH_LEN		48			/**<c语言串最大宽度*/

#define  TDR_MIN(a,b)  (((a) < (b)) ? (a) : (b))

#define TDR_MIN_COPY		64

#define TDR_MIN_INT_VALUE	0x80000000

#define TDR_TAB_SIZE	4		/**<tab键的空格数*/

/**@name 分表规则
*@ {*/
#define TDR_SPLITTABLE_RULE_NONE		0x00  /**< 没有指定分表规则*/
#define TDR_SPLITTABLE_RULE_BY_MOD          0x01  /**<按分表因子模的方式进行分表*/
/*@ }*/


#define TDR_MAX_UNIQUE_KEY_IN_TABLE   32  /**<数据库表中容许的unique键的最大数目*/




#define TDR_MACROSGROUP_MAP_NUM	2	/*宏定义组数据映射区块的个数*/




#define TDR_MAX_INT   (int)0x7FFFFFFF


#if defined(_WIN64) || (__WORDSIZE == 64)
//64位系统
    #define TDR_SIZEOF_POINTER 8         /**<指针变量存储的字节数*/
#else
//32位系统
    #define TDR_SIZEOF_POINTER 4         /**<指针变量存储的字节数*/
#endif


/* Macros for printing   */
#if defined(_WIN64) || (__WORDSIZE == 64)
//64位系统
    #define TDR_FLATFORM_MAGIC   64
    #define __TDR_PRI64_PREFIX	"l"
    #define __TDR_PRIPTR_PREFIX	"l"
#else
//32位系统
    #define TDR_FLATFORM_MAGIC   32
    #define __TDR_PRI64_PREFIX	"ll"
    #define __TDR_PRIPTR_PREFIX
#endif

/* Macros for printing   */
#if defined(_WIN32)
    #define TDRPRI_INTPTRT "d"         /**<printf(intptr_t)*/
    #define TDRPRI_PTRDIFFT "d"         /**<printf(ptrdiff_t)*/
    #define TDRPRI_SIZET "u"         /**<printf(size_t)*/
#elif defined(_WIN64)
    #define TDRPRI_INTPTRT __TDR_PRI64_PREFIX"d"         /**<printf(intptr_t)*/
    #define TDRPRI_PTRDIFFT __TDR_PRI64_PREFIX"d"         /**<printf(ptrdiff_t)*/
    #define TDRPRI_SIZET __TDR_PRI64_PREFIX"d"         /**<printf(size_t)*/
#elif  (__WORDSIZE == 64)
    #define TDRPRI_INTPTRT "ld"         /**<printf(intptr_t)*/
    #define TDRPRI_PTRDIFFT __TDR_PRI64_PREFIX"d"         /**<printf(ptrdiff_t)*/
    #define TDRPRI_SIZET "lu"         /**<printf(size_t)*/
#else
    #define TDRPRI_INTPTRT "d"         /**<printf(intptr_t)*/
    #define TDRPRI_SIZET "u"         /**<printf(size_t)*/
    #define TDRPRI_PTRDIFFT "d"         /**<printf(ptrdiff_t)*/
#endif

#define TDRPRId64 __TDR_PRI64_PREFIX"d"
#define TDRPRIud64 __TDR_PRI64_PREFIX"u"


#endif /* TDR_DEFINE__I_H */
