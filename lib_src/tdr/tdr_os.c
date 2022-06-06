#include <assert.h>
#include <ctype.h>
#include <time.h>

#include "tdr_ctypes_info_i.h"
#include "tdr_os.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

static const char* tdr_memrchr(const char *a_pchBegin, char a_chTarget, size_t a_tLen);
////////////////////////////////////////////////////////////////////////////////

static const char* tdr_memrchr(const char *a_pchBegin, char a_chTarget, size_t a_tLen)
{
    if (!a_pchBegin || a_tLen < 1)
    {
        return NULL;
    }

    for (; a_tLen > 0; a_tLen--)
    {
        if (*(a_pchBegin+a_tLen-1) == a_chTarget)
        {
            return a_pchBegin + a_tLen - 1;
        }
    }

    return NULL;
}


void tdr_os_file_to_macro_i(char* pszMacro, int iMacro, const char* pszFile)
{
	const char *pszTitle;
	int i;

	assert( pszMacro && pszFile && iMacro>0 );

#if defined (_WIN32) || defined (_WIN64)
    {
        const char *pszTitle1;
         pszTitle	=	strrchr(pszFile, TDR_OS_DIRSEP);
         pszTitle1	=	strrchr(pszFile, '/');
         if (NULL == pszFile)
         {
             pszFile = pszTitle1;
         }else if ((NULL != pszTitle1) && (pszTitle1 > pszTitle))
         {
             pszTitle = pszTitle1;
         }
    }

#else
    pszTitle	=	strrchr(pszFile, TDR_OS_DIRSEP);
#endif

    if( pszTitle )
        pszTitle++;
    else
        pszTitle=	pszFile;



	i	=	0;

	while(i<iMacro && i<(int)strlen(pszTitle) )
	{
		if( TDR_OS_DIRDOT==pszTitle[i] )
			pszMacro[i]	=	TDR_OS_DIRBAR;
		else
			pszMacro[i]	=	(char)toupper(pszTitle[i]);

		i++;
	}

	pszMacro[i]	=	'\0';
}


const char * tdr_get_filename(const char *a_pchPath, size_t a_tLen)
{
    const char* pchFileName;
    const char* pchDirEnd;

    if ((!a_pchPath) || (a_tLen < 1) || (*(a_pchPath+a_tLen-1) == TDR_OS_DIRSEP))
    {
        return NULL;
    }

#if defined (_WIN32) || defined (_WIN64)
    if (*(a_pchPath+a_tLen-1) == '/')
    {
        return NULL;
    }
#endif

    pchDirEnd = tdr_get_last_dir_sep(a_pchPath, a_tLen);

    if (!pchDirEnd) /* found no DIRSET */
    {
        pchFileName = a_pchPath;
    }else
    {
        pchFileName = pchDirEnd+1;
    }

    return pchFileName;
}

const char * tdr_get_last_dir_sep(const char *a_pchPath, size_t a_tLen)
{
    const char* pchEnd;

    if (!a_pchPath || a_tLen < 1)
    {
        return NULL;
    }

    pchEnd = tdr_memrchr(a_pchPath, TDR_OS_DIRSEP, a_tLen);

#if defined (_WIN32) || defined (_WIN64)
    if (!pchEnd)
    {
        pchEnd = tdr_memrchr(a_pchPath, '/', a_tLen);
    }
#endif

    return pchEnd;
}

int tdr_is_end_with_dir_sep(const char *a_pchPath, size_t a_tLen)
{
    int iRet;

    if (!a_pchPath || a_tLen < 1)
    {
        return 0;
    }

    iRet = (*(a_pchPath+a_tLen-1) == TDR_OS_DIRSEP);

#if defined (_WIN32) || defined (_WIN64)
    if (!iRet)
    {
        iRet = (*(a_pchPath+a_tLen-1) == '/');
    }
#endif

    return iRet;
}

const char * tdr_get_filename_suffix(const char *a_pchFileName, size_t a_tLen)
{
    if (!a_pchFileName || a_tLen < 1)
    {
        return NULL;
    }

    return tdr_memrchr(a_pchFileName, '.', a_tLen);
}

const char * tdr_get_pathname_suffix(const char *a_pchPath, size_t a_tLen)
{
    const char* pchFileName;
    const char* pchSuffix;

    if (!a_pchPath || a_tLen < 1)
    {
        return NULL;
    }

    pchSuffix = NULL;

    pchFileName = tdr_get_filename(a_pchPath, a_tLen);
    if (!pchFileName)
    {
        pchSuffix = NULL;
    }else if (a_pchPath <= pchFileName)
    {
        pchSuffix = tdr_get_filename_suffix(pchFileName, a_tLen - (pchFileName - a_pchPath));
    }

    return pchSuffix;
}

//#if !defined (_WIN32) && !defined (_WIN64)

size_t tdr_wcsnlen(const wchar_t *s, size_t maxlen)
{
	size_t i;
	tdr_wchar_t *pwch;

	assert(NULL != s);
	pwch = (tdr_wchar_t *)s;
	for (i = 0; i < maxlen; i++)
	{
		if (*pwch == (tdr_wchar_t)0)
		{
			break;
		}
		pwch++;
	}

	return i;
}

#if defined (_WIN32) || defined (_WIN64)
struct tm *tdr_localtime_r(const time_t *timep, struct tm *result)
{
	struct tm* pstRet;

	pstRet	=	localtime(timep);

	if( !pstRet )
		return NULL;

	memcpy(result, pstRet, sizeof(*result));

	return result;
}

const char *tdr_inet_ntop(int af, const void *src,
                          char *dst, int cnt)
{
    struct in_addr *pstAddr;
    int iWriteLen;

    if ((NULL == src) ||(NULL == dst) ||(0 >= cnt) || (AF_INET != af))
    {
        return NULL;
    }

    pstAddr = (struct in_addr *)src;
    iWriteLen = tdr_snprintf(dst, cnt, "%u.%u.%u.%u",
        pstAddr->s_net,pstAddr->s_host,
        pstAddr->s_lh,pstAddr->s_impno);
    if ((0 > iWriteLen) || (iWriteLen >= cnt))
    {
        return NULL;
    }
    return dst;
}


#endif
