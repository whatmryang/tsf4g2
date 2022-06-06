#include "tdr_error_i.h"
#include "tdr/tdr_error.h"
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)

    #include <windows.h>
    #define PTHREAD_ONCE_INIT 0

	typedef long	pthread_once_t;
	typedef DWORD	pthread_key_t;

    #define pthread_once(once, func) {          \
            if(0==InterlockedIncrement(once))   \
                func();                         \
        }

    #define pthread_key_create(pkey, func)  *pkey    = TlsAlloc()
    #define pthread_key_delete(key)         (TlsFree(key) ? 0 : -1)
    #define pthread_setspecific(key, arg)   (TlsSetValue(key, arg) ? 0 : -1)
    #define pthread_getspecific(key)        TlsGetValue(key)

#else
    #include <pthread.h>
#endif



/* 分配这么多的空间是为防止逻辑中存在BUG, 写过TDR_ERROR_BUF_SIZE */
static char szDefErrBuf[2*TDR_ERROR_BUF_SIZE];

static pthread_key_t gs_key;
static pthread_once_t init_done = PTHREAD_ONCE_INIT;

static void tdr_error_free(void* a_pszTsdErrBuf)
{
    if (NULL != a_pszTsdErrBuf)
    {
        free(a_pszTsdErrBuf);
        pthread_setspecific(gs_key, NULL);
    }
}

static void tdr_error_init(void)
{
    pthread_key_create(&gs_key, tdr_error_free);
}

int tdr_init_for_thread(void)
{
    char* pszTsdErrBuf = NULL;

    pthread_once(&init_done, tdr_error_init);

    pszTsdErrBuf = (char*)pthread_getspecific(gs_key);
    if (NULL == pszTsdErrBuf)
    {
        pszTsdErrBuf = malloc(TDR_ERROR_BUF_SIZE);
        if (NULL == pszTsdErrBuf)
        {
            return -1;
        } else
        {
            return pthread_setspecific(gs_key, pszTsdErrBuf);
        }
    }

    return 0;
}

void tdr_fini_for_thread(void)
{
    char* pszTsdErrBuf = NULL;
    pszTsdErrBuf = pthread_getspecific(gs_key);

    tdr_error_free(pszTsdErrBuf);
}

const char* tdr_get_error_detail()
{
    char* pszErrDetail = NULL;
    char* pszTsdErrBuf = NULL;

    pszTsdErrBuf = pthread_getspecific(gs_key);
    if (NULL == pszTsdErrBuf)
    {
        pszErrDetail = szDefErrBuf;
    } else
    {
        pszErrDetail = pszTsdErrBuf;
    }

    if ('\0' != pszErrDetail[TDR_ERROR_BUF_SIZE-1])
    {
        pszErrDetail[TDR_ERROR_BUF_SIZE-1] = '\0';
    }

    return pszErrDetail;
}

int tdr_max_error_detail(void)
{
    return 2*TDR_ERROR_BUF_SIZE;
}

char* tdr_err_buf(void)
{
    char* pszErrBuf = NULL;
    char* pszTsdErrBuf = NULL;

    pszTsdErrBuf = pthread_getspecific(gs_key);
    if (NULL == pszTsdErrBuf)
    {
        pszErrBuf = szDefErrBuf;
    } else
    {
        pszErrBuf = pszTsdErrBuf;
    }

    return pszErrBuf;
}
