/**
*
* @file     tdr_define_i.h
* @brief    �ڲ�ʹ�õĺ궨��
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-02
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
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

/** @name Ԫ���ݱ�־ֵ
* @{*/
#define TDR_FLAG_NETORDER	0x01
#define TDR_FLAG_COMPLEX		0x02

/** @}*/  //Ԫ���ݱ�־ֵ

/** @name meta��־ֵ
* @{*/
#define TDR_META_FLAG_FIXSIZE	0x0001	/* a type that does not change size. */
#define TDR_META_FLAG_HAVE_ID        0x0002    /**< ������id����*/
#define TDR_META_FLAG_RESOVLED	0x0004	/*�Ƿ��Ѿ������ı�־*/
#define TDR_META_FLAG_VARIABLE	0x0008  /*meta�Ƿ��ǿɱ��*/
#define TDR_META_FALG_STRICT_INPUT	0x0010 /*����ʱ�����ļ��ĸ�ʽ�Ƿ�������ϸ��*/
#define TDR_META_FALG_HAVE_AUTOINVREMENT_ENTRY	0x0020 /*�˽ṹ���а���autoincrement��Ա*/
#define TDR_META_FLAG_NEED_PREFIX_FOR_UNIQUENAME	0x0040	/*����Աչ���洢ʱ�������ǰ׺����֤����Ψһ��*/
/** @}*/


/**
build0009�����
    a���ṹ�弰��Ա���ִ���󳤶ȴ�32���128��
    b���궨�����ִ���󳤶ȴ�64���128
    c��֧��64λϵͳ
*/

/**
 * build000a�����
 * a) ����iReserve�ֶε�3��
 * b) ����У�����
 */

/**
 * build000b�����
 * a) ����indexԪ��
 */
#define	TDR_BUILD		0x000b			/*�����汾��metalib�����ݽṹÿ���һ�Σ��˰汾�����һ*/

/* ��¼����checksum���Ƶ� tdr build version */
#define TDR_CHECKSUM_VERSION 0x000a

/* ��¼����<index>��ǩ��ʱ�� */
#define TDR_INDEX_VERSION 0x000b

#define TDR_DEFAULT_ALIGN_VALUE		1	/**<tdr�������ֽڶ���ֵ*/




/**@name entry ��־
*@ {*/
#define TDR_ENTRY_FLAG_NONE		0x0000  /**< �������־*/
#define TDR_ENTRY_FLAG_RESOVLD          0x0001  /**<��entry�Ѿ���ȷ����*/
#define TDR_ENTRY_FLAG_POINT_TYPE		0x0002    /**<ָ������*/
#define TDR_ENTRY_FLAG_REFER_TYPE		0x0004    /**<��������*/
#define TDR_ENTRY_FLAG_HAVE_ID              0x0008        /**<entry������ID����*/
#define TDR_ENTRY_FLAG_HAVE_MAXMIN_ID		0x0010		/**<entry������maxminid����*/
#define TDR_ENTRY_FALG_FIXSIZE        0x0020        /**<entry�Ĵ洢�ռ��ǹ̶���*/
#define TDR_ENTRY_FLAG_REFER_COUNT	0x0040	/*�˳�Ա������һ����Ա�����������*/
#define TDR_ENTRY_FLAG_SELECTOR		0x0080	/*�˳�Ա������һ��union��Ա��ѡ����*/
#define TDR_ENTRY_FLAG_STR_COMPRESS 0x0100	/*��int��Ա���ַ�������ID�������־λֻ��������int�ĳ�Ա��Ч*/
/*@ }*/

/**@name entry�����ϵӳ���־
*@ {*/
#define TDR_ENTRY_DB_FLAG_NONE		0x00  /**< �������־*/
#define TDR_ENTRY_DB_FLAG_UNIQUE	0x01	/**<��ȡֵ��unique��*/
#define TDR_ENTRY_DB_FLAG_NOT_NULL	0x02	/**<��ȡֵ�ǲ�Ϊ��NOT NULL��*/
#define TDR_ENTRY_DB_FLAG_EXTEND_TO_TABLE 0x04			/**<entry�ڽ���ʱ������չ*/
#define TDR_ENTRY_DB_FLAG_PRIMARYKEY 0x10			/**<��entry��������ɲ���*/
#define TDR_ENTRY_DB_FLAG_AUTOINCREMENT	0x20		/**<�˳�ԱΪautoincremnet��Ա*/
#define TDR_ENTRY_DB_FLAG_COMPRESS 0x40			/**<blobѹ���洢*/
/*@ }*/

#define TDR_MAX_PATH       260      /**<·������󳤶�*/

#define TDR_MAX_HPP_STRING_WIDTH_LEN		48			/**<c���Դ������*/

#define  TDR_MIN(a,b)  (((a) < (b)) ? (a) : (b))

#define TDR_MIN_COPY		64

#define TDR_MIN_INT_VALUE	0x80000000

#define TDR_TAB_SIZE	4		/**<tab���Ŀո���*/

/**@name �ֱ����
*@ {*/
#define TDR_SPLITTABLE_RULE_NONE		0x00  /**< û��ָ���ֱ����*/
#define TDR_SPLITTABLE_RULE_BY_MOD          0x01  /**<���ֱ�����ģ�ķ�ʽ���зֱ�*/
/*@ }*/


#define TDR_MAX_UNIQUE_KEY_IN_TABLE   32  /**<���ݿ���������unique���������Ŀ*/




#define TDR_MACROSGROUP_MAP_NUM	2	/*�궨��������ӳ������ĸ���*/




#define TDR_MAX_INT   (int)0x7FFFFFFF


#if defined(_WIN64) || (__WORDSIZE == 64)
//64λϵͳ
    #define TDR_SIZEOF_POINTER 8         /**<ָ������洢���ֽ���*/
#else
//32λϵͳ
    #define TDR_SIZEOF_POINTER 4         /**<ָ������洢���ֽ���*/
#endif


/* Macros for printing   */
#if defined(_WIN64) || (__WORDSIZE == 64)
//64λϵͳ
    #define TDR_FLATFORM_MAGIC   64
    #define __TDR_PRI64_PREFIX	"l"
    #define __TDR_PRIPTR_PREFIX	"l"
#else
//32λϵͳ
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
