/*
 * tsec_wrapper.h
 *
 *  Created on: 2010-7-23
 *      Author: mickeyxu
 */

#ifndef TSEC_WRAPPER_H_
#define TSEC_WRAPPER_H_


#include "tsec/tsec.h"
#include "tsec_wrapper.h"
#include "pal/pal.h"

#define TQQ_KEY_LEN                                         16
#define TQQ_MAX_NAME_LEN                                    32
#define TQQ_MAX_IDENT_LEN                                   16
#define TQQ_MAX_SIGN_LEN                                    128
#define TQQ_MAX_SIGN2_LEN                                   64
#define TQQ_IDENT_LEN                                       16
#define TQQ_MAX_USERIDENT_LEN                               64
#define TQQ_SVCBITMAP_LEN                                   12
#define TQQ_SVCBITMAP_EXT_LEN                               8
#define TQQ_UNIFIED_MAX_SIGN_LEN                            256
#define TQQ_UNIFIED_MAX_ENCSIGN_LEN                         128
#define TQQ_UNIFIED_MAX_ENCSIGN2_LEN                        32
#define TQQ_UNIFIED_CUSTOMINFO_LEN                          32

struct tagTsecWrapper
{
    int iIsGameKeyValid;
    char szGameKey[TQQ_KEY_LEN];
    /*通信加解密方法*/
    int iEncMethod;
    //AES 句柄
    AES stAes;
};

typedef struct tagTsecWrapper TSECWRAPPER;
typedef struct tagTsecWrapper *HTSECWRAPPER;
/* TCONN_SEC_ENC*/
enum tagTCONN_SEC_ENC
{
    TCONN_SEC_NONE = 0,
    TCONN_SEC_TEA = 1,
    TCONN_SEC_QQ = 2,
    TCONN_SEC_AES = 3,
    TCONN_SEC_AES2 = 4,
/* 与TCONN_SEC_AES算法不同的是修改了数据填充算法 */
};

int tsec_wrapper_init(HTSECWRAPPER a_hQQClt, const char* pszGameKey,
        int iEncMethod);
int tsec_wrapper_encrypt(HTSECWRAPPER a_hQQClt, const char *a_pszInBuf,
        int a_iInLen, char **a_pszOutBuf, int *a_piOutLen, int a_iNeedCopy);

int tsec_wrapper_decrypt(HTSECWRAPPER a_hQQClt, const char *a_pszInBuf,
        int a_iInLen, char **a_pszOutBuf, int *a_piOutLen, int a_iIsPlain,
        int a_iNeedCopy);

#endif /* TSEC_WRAPPER_H_ */
