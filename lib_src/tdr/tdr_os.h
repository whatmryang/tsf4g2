#ifndef TDR_OS_H
#define TDR_OS_H






#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include "tdr/tdr_types.h"





#define TDR_OS_DIRDOT			'.'
#define	TDR_OS_DIRBAR			'_'

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WIN32) || defined(_WIN64)
	#define	TDR_OS_DIRSEP			'\\'
	#define tdr_stricmp(s1, s2)				stricmp(s1, s2)
	#define tdr_strnicmp(s1, s2, n)			strnicmp(s1, s2, n)
	#define tdr_strupr(sz)				_strupr(sz)

	#define tdr_snprintf				_snprintf


	size_t tdr_wcsnlen(const wchar_t *s, size_t maxlen);



	#define tdr_vsnprintf				_vsnprintf

	#if _MSC_VER < 1400
		#define	tdr_strnlen(sz,n)				strlen(sz)
	#else
		#define	tdr_strnlen(sz,n)				strnlen(sz, n)
	#endif	/* _MSC_VER */

	struct tm *tdr_localtime_r(const time_t *timep, struct tm *result);

	#if _MSC_VER >= 1400
	#define tdr_strtok_r	strtok_s
	#else
	#define tdr_strtok_r	strtok_r
	#endif
	const char *tdr_inet_ntop(int af, const void *src,
		char *dst, int cnt);

	extern int gs_iTdrWSAStartup;
	#define TDR_WSA_STARTUP {\
		if (0 == gs_iTdrWSAStartup)		\
		{								\
			WORD wVersionRequested;		\
			WSADATA wsaData;			\
			gs_iTdrWSAStartup = 1;		\
			wVersionRequested = MAKEWORD( 2, 2 );\
			WSAStartup( wVersionRequested, &wsaData );\
		}								\
	}
#else /*#if defined(_WIN32) || defined(_WIN64)*/
	#define	TDR_OS_DIRSEP			'/'
	#define tdr_stricmp(s1, s2)			strcasecmp(s1, s2)
	#define tdr_strnicmp(s1, s2, n)		strncasecmp(s1, s2, n)
	#define tdr_strupr(p)			{			      \
		int i=0;					      \
		while(p[i])					      \
		{						      \
			p[i]	=	(char)toupper(p[i]);	      \
			i++;					      \
		}						      \
	}

	#define tdr_localtime_r		localtime_r
	#define tdr_snprintf				snprintf
	#define tdr_vsnprintf				vsnprintf
	#define  tdr_strnlen	strnlen
	size_t tdr_wcsnlen(const wchar_t *s, size_t maxlen);

	#define tdr_strtok_r	strtok_r
	#define tdr_inet_ntop inet_ntop
	#define TDR_WSA_STARTUP

#endif/*#if defined(_WIN32) || defined(_WIN64)*/

#define TDR_STRNCPY(pszDst, pszSrc, iLen)					      \
	do								      \
	{								      \
		strncpy(pszDst, pszSrc, (iLen)-1);			      \
		*(pszDst+(iLen)-1) = 0;					      \
	}								      \
	while(0)

#define TDR_MEMCPY(d, s, size, min)			{			      \
	size_t i;								      \
	if( size<=min )							      \
	{								      \
		for(i=0; i<size; i++)					      \
		{							      \
			d[0]	=	s[0];				      \
			d++; s++;					      \
		}							      \
	}								      \
	else								      \
	{								      \
		memcpy(d, s, size);					      \
		s +=	size;						      \
		d +=	size;						      \
	}								      \
}



void tdr_os_file_to_macro_i(char* pszMacro, int iMacro, const char* pszFile);
char * tdr_strptime( const char *buf,  const char *format, struct tm *timeptr);
const char * tdr_get_filename(const char *a_pchPath, size_t a_tLen);
const char * tdr_get_last_dir_sep(const char *a_pchPath, size_t a_tLen);
int tdr_is_end_with_dir_sep(const char *a_pchPath, size_t a_tLen);
const char * tdr_get_filename_suffix(const char *a_pchFileName, size_t a_tLen);
const char * tdr_get_pathname_suffix(const char *a_pchPath, size_t a_tLen);

#ifdef __cplusplus
}
#endif

#endif /* TDR_OS_H */
