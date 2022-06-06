/**
*
* @file     tdr_XMLTags_i_h.h
* @brief    ��ȡ��ͬ�汾��Ԫ��������XML��ǩ����
*
* @author jackyai
* @version 1.0
* @date 2007-04-05
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_AUXTOOLS_H
#define TDR_AUXTOOLS_H

#include <stdlib.h>
#include "tdr/tdr_types.h"
#include "tdr/tdr_ctypes_info.h"


/*vinsonzuo 20090810 add TDR_ATOULL to fix bug*/
#if defined(_WIN32) || defined(_WIN64)
uint64_t tdr_strtoull(IN const char *a_pszStr, INOUT char **a_ppszEnd, IN unsigned int a_iBase);
tdr_longlong tdr_strtoll(IN const char *a_pszStr, INOUT char **a_ppszEnd, IN unsigned int a_iBase);
#define TDR_ATOLL(str)  tdr_strtoll(str, NULL, 0)
#define TDR_ATOULL(str) tdr_strtoull(str, NULL, 0)
#else
#define  TDR_ATOLL(str)	strtoll(str, NULL, 0)
#define TDR_ATOULL(str) strtoull(str, NULL, 0)
#endif



/**
*�޳�pszSrcStrǰ��ĵĿհ��ַ���Ȼ�󽫴����Ĵ�������a_pszDstStr��
*@param[out] a_pszDstStr ���洦��󴮵Ļ�����
*@param[in] a_iDstStrSize ����������Ĵ�С
*@param[in] a_pszSrcStr ������Ĵ�
*@return �ɹ����ش�����ַ����ĳ��ȣ�����null�ַ��������򷵻ظ���
*@note �������������ռ䲻������ضϿ���
*/
size_t tdr_normalize_string(char *a_pszDstStr, size_t a_iDstStrSize, const char *a_pszSrcStr);

/**
*�޳�a_strInputǰ��ĵĿհ��ַ���Ȼ�󽫴����Ĵ�������a_strInput��
*@param[in,out] a_strInput ���洦��󴮵Ļ�����
*
*@note �������������ռ䲻������ضϿ���
*/
void tdr_trim_str( char *a_strInput );

/**
*�������ַ���ת����unicode�ַ���
*@param[out] a_pszWcstr ����unicode�ַ����Ļ�����
*@param[in,out] a_piWcsLen
*	- ����: ����unicode�ַ����Ĵ�С����charΪ��λ����
*   - �����ת����unicode�ַ����Ĵ�С,��charΪ��λ����
*@param[in] a_pszMbs ��Ҫת���������ַ���������ָ��
*@param[in] a_iMbsLen ��Ҫת�����ֽ���
*@return 0 �ɹ� ��0 ʧ��
*
*@note �������������ռ䲻������ضϿ���
*/
int tdr_chinesembstowcs(OUT char *a_pszWcstr, INOUT size_t *a_piWcsLen, IN char *a_pszMbs, IN size_t a_iMbsLen);

/**
*��unicode�ַ���ת���������ַ���
*@param[out] a_pszMbs ���������ַ����Ļ�����
*@param[in,out] a_piMbsLen
*	- ����: ���������ַ����Ĵ�С����wchar_tΪ��λ����
*	- �������ת���������ַ�����С
*@param[in] a_pszWcstr ��Ҫת����unicode�ַ���������ָ��
*@param[in] a_iWcsLen ��Ҫת�����ֽ���
*@return 0 �ɹ� ��0 ʧ��
*
*@note �������������ռ䲻������ضϿ���
*/
int tdr_wcstochinesembs(OUT char *a_pszMbs, INOUT size_t *a_piMbsLen, IN tdr_wchar_t *a_pszWcstr, IN size_t a_iWcsLen);

/**
*��unicode�ַ���ת���������ַ���
*@param[in] a_pszWcstr ��Ҫת����unicode�ַ���������ָ��
*@param[in] a_iWcsLen ��Ҫת�����ֽ���
*@return �ɹ� ����ת����������ַ�����ָ��,���򷵻�NULL
*
*@note ת����������ַ����������Ŀռ��ں����ڷ���,ʹ�����,�������free���������ͷ�
*/
char *tdr_wcstochinesembs_i(IN tdr_wchar_t *a_pszWcstr, IN size_t a_iWcsLen);

/**
strnmov(dst,src,length) moves length characters, or until end, of src to
dst and appends a closing NUL to dst if src is shorter than length.
The result is a pointer to the first NUL in dst, or is dst+n if dst was
truncated.
*/
char *tdr_strnmov(register char *dst, register const char *src, unsigned int n);

/**
tdr_strmov(dst, src) moves all the  characters  of  src  (including  the
closing NUL) to dst, and returns a pointer to the new closing NUL in
dst.	 The similar UNIX routine strcpy returns the old value of dst,
which I have never found useful.  tdr_strmov(tdr_strmov(dst,a),b) moves a//b
into dst, which seems useful.
*/
char *tdr_strmov(register char *dst, register const char *src);

unsigned int tdr_get_hash_code(const char* szKey, size_t iKeyLen);

/***
*tdr_qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*   quicksort the array of elements
*   side effects:  sorts in place
*   maximum array size is number of elements times size of elements,
*   but is limited by the virtual address space of the processor
*
*Entry:
*   char *base = pointer to base of array
*   size_t num  = number of elements in the array
*   size_t width = width in bytes of each array element
*   int (*comp)() = pointer to function returning analog of strcmp for
*           strings, but supplied by user for comparing the array elements.
*           it accepts 2 pointers to elements.
*           Returns neg if 1<2, 0 if 1=2, pos if 1>2.
*
*Exit:
*   returns void
*
*Exceptions:
*   Input parameters are validated. Refer to the validation section of the function.
*
*******************************************************************************/
void  tdr_qsort (		   void *base,
				 size_t num,
				 size_t width,
				 int ( *comp)(const void *, const void *)
			  );

#endif /*TDR_AUXTOOLS_H*/
