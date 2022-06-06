/*
 * tsec_wrapper.c
 *
 *  Created on: 2010-7-23
 *      Author: mickeyxu
 */

#include "tsec/tsec.h"
#include "tsec_wrapper.h"
#include "pal/pal.h"



int tsec_wrapper_init(HTSECWRAPPER a_hQQClt, const char* pszGameKey, int iEncMethod)
{
    int iRet = 0;

    a_hQQClt->iEncMethod = iEncMethod;
    switch (iEncMethod)
    {
    case TCONN_SEC_NONE:
        break;
    case TCONN_SEC_QQ:
        a_hQQClt->iIsGameKeyValid = 1;
        memcpy(a_hQQClt->szGameKey, pszGameKey, sizeof(a_hQQClt->szGameKey));
        break;
    case TCONN_SEC_AES:
    case TCONN_SEC_AES2:
        a_hQQClt->iIsGameKeyValid = 1;
        memcpy(a_hQQClt->szGameKey, pszGameKey, sizeof(a_hQQClt->szGameKey));
        iRet = taes_setkey2(&(a_hQQClt->stAes), (const unsigned char *) pszGameKey, AES_KEY_BITS);
        break;
    default:
        iRet = -1;
    }

    return iRet;

}

int tsec_wrapper_encrypt(HTSECWRAPPER a_hQQClt, const char *a_pszInBuf, int a_iInLen, char **a_pszOutBuf, int *a_piOutLen, int a_iNeedCopy)
{
    int iRet = 0;
    char *pszEnc;
    assert(NULL != a_hQQClt);
    assert(NULL != a_pszOutBuf);
    assert(NULL != a_pszInBuf);
    assert(NULL != a_piOutLen);
    assert(NULL != *a_pszOutBuf);

    pszEnc = *a_pszOutBuf;
    switch (a_hQQClt->iEncMethod)
    {
    case TCONN_SEC_QQ:
        oi_symmetry_encrypt2(a_pszInBuf, a_iInLen, a_hQQClt->szGameKey, pszEnc, a_piOutLen);
        break;
    case TCONN_SEC_AES:
        iRet = taes_encrypt(a_pszInBuf, a_iInLen, &(a_hQQClt->stAes), pszEnc, a_piOutLen);
        if (iRet != 0)
        {
            return -1;
        }
        break;
    case TCONN_SEC_AES2:
        iRet = taes_encrypt2(a_pszInBuf, a_iInLen, &(a_hQQClt->stAes), pszEnc, a_piOutLen);
        if (iRet != 0)
        {
            return -1;
        }
        break;
    default:
    {
        if (0 != a_iNeedCopy)
        {
            if (*a_piOutLen < a_iInLen)
            {
                return -1;
            }
            else
            {
                memcpy(*a_pszOutBuf, a_pszInBuf, a_iInLen);
            }
        }
        else
        {
            *a_pszOutBuf = (char *) a_pszInBuf;
        }

        *a_piOutLen = a_iInLen;
    }
    }/*switch(a_hQQClt->iEncMethod)*/

    return 0;
}

/**
 param[in] a_iNeedCopy 如果是明文消息，此字段用于控制 是否将消息拷贝到输出缓冲区，还是修改指针，
 其值为0，不拷贝，其值为非0，则需要拷贝
 */
int tsec_wrapper_decrypt(HTSECWRAPPER a_hQQClt, const char *a_pszInBuf, int a_iInLen, char **a_pszOutBuf, int *a_piOutLen, int a_iIsPlain,
        int a_iNeedCopy)
{
    int iRet = 0;
    char *pszDec;
    assert(NULL != a_hQQClt);
    assert(NULL != a_pszOutBuf);
    assert(NULL != *a_pszOutBuf);
    assert(NULL != a_pszInBuf);
    assert(NULL != a_piOutLen);

    if (0 != a_iIsPlain)
    {
        //明文消息
        if (0 != a_iNeedCopy)
        {
            if (*a_piOutLen < a_iInLen)
            {
                return -1;
            }
            else
            {
                memcpy(*a_pszOutBuf, a_pszInBuf, a_iInLen);
            }
        }
        else
        {
            *a_pszOutBuf = (char *) a_pszInBuf;
        }

        *a_piOutLen = a_iInLen;

        return 0;
    }

    pszDec = *a_pszOutBuf;
    switch (a_hQQClt->iEncMethod)
    {
    case TCONN_SEC_QQ:
        iRet = oi_symmetry_decrypt2(a_pszInBuf, a_iInLen, a_hQQClt->szGameKey, pszDec, a_piOutLen);
        if (!iRet)
        {
            return -1;
        }
        break;
    case TCONN_SEC_AES:
        iRet = taes_decrypt(a_pszInBuf, a_iInLen, &(a_hQQClt->stAes), pszDec, a_piOutLen);
        if (iRet != 0)
        {
            return -1;
        }
        break;
    case TCONN_SEC_AES2:
        iRet = taes_decrypt2(a_pszInBuf, a_iInLen, &(a_hQQClt->stAes), pszDec, a_piOutLen);
        if (iRet != 0)
        {
            return -1;
        }
        break;
    default:
    {
        if (0 != a_iNeedCopy)
        {
            if (*a_piOutLen < a_iInLen)
            {
                return -1;
            }
            else
            {
                memcpy(*a_pszOutBuf, a_pszInBuf, a_iInLen);
            }
        }
        else
        {
            *a_pszOutBuf = (char *) a_pszInBuf;
        }

        *a_piOutLen = a_iInLen;
        break;
    }
    }

    return 0;
}

