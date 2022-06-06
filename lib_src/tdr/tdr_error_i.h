/**
*
* @file     tdr_error.h
* @brief    Internal error functions
*
* @author jackyai
* @version 1.0
* @date 2007-03-28
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/




#ifndef TDR_XERROR_I_H
#define TDR_XERROR_I_H

#include "tdr/tdr_error.h"

#ifdef TDR_TRACE_LAST_ERROR
#define TDR_SET_LAST_ERROR  tdr_set_last_error
#else
#define TDR_SET_LAST_ERROR  //
#endif

#define TDR_ERROR_BUF_SIZE 1024

#define TDR_ERR_GET_LEN() int _tdr_err_len = tdr_strnlen(tdr_err_buf(), TDR_ERROR_BUF_SIZE);

#define TDR_ERR_BUF\
    tdr_err_buf() + _tdr_err_len,\
    ((TDR_ERROR_BUF_SIZE > _tdr_err_len)? (TDR_ERROR_BUF_SIZE - _tdr_err_len) : 0)

#define TDR_CLR_ERR_BUF() tdr_err_buf()[0]=0;

#if defined(_WIN32) || defined(_WIN64)
#define TDR_SET_ERROR _snprintf
#else
#define TDR_SET_ERROR snprintf
#endif

/* Sets TDR internal last error. */
void tdr_set_last_error(int code);

/* Gets TDR internal last error. */
int tdr_get_last_error(void);

/**
* ��ȡ����Ĵ������
* @return �������
* @see tdr_error_string
*/
int tdr_error_code(void);

/* ȡ�ñ��������Ϣ�ġ�TSD/TLS ��ȫ�ֻ�������ָ�� */
char* tdr_err_buf(void);

#endif /* TDR_XERROR_I_H */
