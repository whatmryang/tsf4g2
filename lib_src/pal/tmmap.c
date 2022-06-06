/*
**  @file $RCSfile: tmmap.c,v $
**  general description of this module
**  $Id: tmmap.c,v 1.2 2008-11-12 04:07:08 jackyai Exp $
**  @author $Author: jackyai $
**  @date $Date: 2008-11-12 04:07:08 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tipc.h"
#include "pal/tfile.h"
#include "pal/tmmap.h"
#include "pal/tstdio.h"
#include "errno.h"

#if defined (_WIN32) || defined (_WIN64)
void* mmap(void *a_pvStart, size_t a_iLength, int a_iProt , int a_iFlags, HANDLE a_iFile, off_t a_iOffset)
{
	unsigned int dwAccess;
	void* pvBase;
	int iErr;

	dwAccess		=	0;

	if( a_iProt & PROT_WRITE )
		dwAccess	|=	FILE_MAP_WRITE;

	if( a_iProt & PROT_READ )
		dwAccess	|=	FILE_MAP_READ;

	pvBase	=	 MapViewOfFile(a_iFile, dwAccess, 0, a_iOffset, a_iLength);

	if( pvBase && (a_iFlags &  MAP_LOCKED) )
	{
		iErr	=	mlock(pvBase, a_iLength);
		if( -1==iErr )
		{
			munmap(pvBase, a_iLength);
			pvBase	=	NULL;
		}
	}

	return pvBase;
}

int munmap(void *a_pvStart, size_t a_iLength)
{
	if(UnmapViewOfFile(a_pvStart))
		return 0;
	else
		return -1;
}

int mprotect(const void *a_pvAddr, size_t a_iLen, int a_iProt)
{
	DWORD dwOldProt;

	if( VirtualProtect((void*)a_pvAddr, (DWORD)a_iLen, (DWORD)a_iProt, &dwOldProt) )
		return 0;
	else
		return -1;
}

int mlock(const void *a_pvAddr, size_t a_iLen)
{
	if( VirtualLock((void*)a_pvAddr, (DWORD)a_iLen) )
		return 0;
	else
		return -1;
}

int munlock(const void *a_pvAddr, size_t a_iLen)
{
	if( VirtualUnlock((void*)a_pvAddr, (DWORD)a_iLen) )
		return 0;
	else
		return -1;
}

/* pretend to be succeed. */
int mlockall(int a_iFlags)
{
	if(!SetProcessWorkingSetSize(GetCurrentProcess(),8*1024*1024,8*1024*1024))
	{
		errno	=	EPERM;
		return -1;
	}

	return 0;
}

/* pretend to be succeed. */
int munlockall(void)
{
	if (!SetProcessWorkingSetSize(GetCurrentProcess(),(DWORD)-1,(DWORD)-1))
	{
		errno	=	EPERM;
		return -1;
	}

	return 0;
}

int msync(void *a_pvStart, size_t a_iLength, int a_iFlags)
{
	if( a_iLength<0 )
		return -1;
	else if(FlushViewOfFile(a_pvStart, (DWORD)a_iLength))
		return 0;
	else
		return -1;

}

#endif


HANDLE tmmapopen(const char* a_pszPath, int a_iSize, int a_iFlags)
{
#if defined (_WIN32) || defined (_WIN64)

	HANDLE iFile = (HANDLE)-1;
	HANDLE iFileToMap;
	HANDLE iMap;
	char szName[TIPC_MAX_NAME];
	SECURITY_ATTRIBUTES sa;
	int iFileFlags;
	int iMapFlags;
	int iProtFlags;
	int iErr;

	sa.nLength		=	sizeof(sa);
	sa.bInheritHandle	=	FALSE;
	sa.lpSecurityDescriptor	=	0;

	if( 0==(TMMAPF_ACCESS_MASK & a_iFlags) )
	{
		iFileFlags	=	TF_MODE_READ | TF_MODE_WRITE;
		iProtFlags	=	PAGE_READWRITE;
		iMapFlags	=	FILE_MAP_ALL_ACCESS;
	}
	else if( a_iFlags & TMMAPF_WRITE )
	{
		iFileFlags	=	TF_MODE_WRITE | TF_MODE_READ;
		iProtFlags	=	PAGE_READWRITE;
		iMapFlags	=	FILE_MAP_WRITE;
	}
	else if( a_iFlags & TMMAPF_READ )
	{
		iFileFlags	=	TF_MODE_READ;
		iProtFlags	=	PAGE_READONLY;
		iMapFlags	=	FILE_MAP_READ;
	}
	else
	{
		iFileFlags	=	0;
		iProtFlags	=	0;
		iMapFlags	=	0;
	}

	iFile	=	tfopen(a_pszPath, iFileFlags);
	if( (HANDLE)-1==iFile )
	{
		iFileFlags |= TF_MODE_CREATE|TF_MODE_EXCL;
		iFile	=	tfopen(a_pszPath, iFileFlags);
	}
	if ((HANDLE)-1 == iFile)
	{
		return (HANDLE)-1;
	}

	if( a_iFlags & TMMAPF_PRIVATE )
		iFileToMap	=	(HANDLE)-1;
	else
		iFileToMap	=	iFile;

	if( tipc_path2name(szName, TIPC_MAX_NAME, a_pszPath, "tmmap:") < 0 )
	{
		tfclose(iFile);
		return (HANDLE)-1;
	}

	if( a_iFlags & TMMAPF_CREATE )
	{
#ifdef WINNT
		sa.lpSecurityDescriptor	=	tos_get_sd();
#else
		sa.lpSecurityDescriptor	=	NULL;
#endif	/* WINNT */
		iMap	=	CreateFileMapping(iFileToMap, &sa, iProtFlags, 0, a_iSize, szName );

		iErr	=	GetLastError();

		if( iMap && (TMMAPF_EXCL & a_iFlags) && ERROR_ALREADY_EXISTS==GetLastError() )
		{
			CloseHandle(iMap);
			iMap	=	NULL;
		}

#ifdef WINNT
		tos_free_sd(sa.lpSecurityDescriptor);
#endif	/* WINNT */
	}
	else
		iMap	= OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, szName );

	tfclose(iFile);

	if(!iMap)
		return (HANDLE)-1;
	else
		return iMap;
#else
	int iFileFlags=0;
	TOS_UNUSED_ARG(a_iSize);
	if( 0==(TMMAPF_ACCESS_MASK & a_iFlags) )
		iFileFlags	=	O_RDWR;

	if( a_iFlags & TMMAPF_WRITE )
		iFileFlags	|=	O_WRONLY;

	if( a_iFlags & TMMAPF_READ )
		iFileFlags	|=	O_RDONLY;

	return	open(a_pszPath, iFileFlags);
#endif
}

int tmmapclose(HANDLE a_iMap)
{
#if defined (_WIN32) || defined (_WIN64)
	if( CloseHandle(a_iMap ) )
		return 0;
	else
		return -1;
#else
	return close(a_iMap);
#endif
}

void* tmmap(HANDLE a_iMap, int a_iStart, int a_iSize, int a_iFlags)
{
#if defined (_WIN32) || defined (_WIN64)
	unsigned int dwAccess;
	void* pvBase;
	int iErr;

	if( 0==a_iFlags )
		dwAccess	=	FILE_MAP_ALL_ACCESS;
	else if( a_iFlags & TMMAPF_PROT_WRITE )
		dwAccess	=	FILE_MAP_WRITE;
	else if( a_iFlags & TMMAPF_PROT_READ )
		dwAccess	=	FILE_MAP_READ;
	else
		dwAccess	=	0;

	pvBase	=	 MapViewOfFile(a_iMap, dwAccess, 0, a_iStart, a_iSize);

	iErr	=	GetLastError();

	return pvBase;

#else
	int iProt=0;

	if( 0==a_iFlags )
		iProt	=	PROT_READ | PROT_WRITE;

	if( a_iFlags & TMMAPF_PROT_WRITE )
		iProt	|=	PROT_WRITE;

	if( a_iFlags & TMMAPF_PROT_READ )
		iProt	|=	PROT_READ;

	return mmap((void*)0, (size_t)a_iSize, iProt, MAP_SHARED, a_iMap, a_iStart);
#endif
}

int tmunmap(void* a_pvBase, int a_iSize)
{
#if defined (_WIN32) || defined (_WIN64)
	if(UnmapViewOfFile(a_pvBase))
		return 0;
	else
		return -1;
#else
	return munmap(a_pvBase, (size_t)a_iSize);
#endif
}

int tmprotect(const void* a_pvBase, size_t a_dwSize, int a_iProt)
{
#if defined (_WIN32) || defined (_WIN64)
	DWORD dwOldProt;

	if( VirtualProtect((void*)a_pvBase, a_dwSize, (DWORD)a_iProt, &dwOldProt) )
		return 0;
	else
		return -1;
#else
	return mprotect( (void*)a_pvBase, a_dwSize, a_iProt );
#endif
}

int tmsync(void* a_pvAddr, int a_iSize)
{
#if defined (_WIN32) || defined (_WIN64)
	if( a_iSize<0 )
		return -1;
	else if(FlushViewOfFile(a_pvAddr, (DWORD)a_iSize))
		return 0;
	else
		return -1;
#else
	return msync(a_pvAddr, (size_t)a_iSize, MS_SYNC);
#endif
}

int tmpsync(void* a_pvAddr, int a_iSize, int a_iPage)
{
#if defined (_WIN32) || defined (_WIN64)
	if( a_iSize<=0 )
		return -1;
	else if(FlushViewOfFile(a_pvAddr, (DWORD)a_iSize))
		return 0;
	else
		return -1;
#else
	void* pvBase;

	pvBase	=	(void*) ( ( (intptr_t) a_pvAddr/a_iPage) * a_iPage);

	a_iSize	=	((intptr_t)a_pvAddr - (intptr_t)pvBase) + a_iSize;

	return msync(pvBase, (size_t)a_iSize, MS_SYNC);
#endif
}

int tmlock(const void *a_pvAddr, size_t a_iLen)
{
#if defined (_WIN32) || defined (_WIN64)
	if( VirtualLock((void*)a_pvAddr, (DWORD)a_iLen) )
		return 0;
	else
		return -1;
#else
	return mlock(a_pvAddr, a_iLen);
#endif
}

int tmunlock(const void *a_pvAddr, size_t a_iLen)
{
#if defined (_WIN32) || defined (_WIN64)
	if( VirtualUnlock((void*)a_pvAddr, (DWORD)a_iLen) )
		return 0;
	else
		return -1;
#else
	return munlock(a_pvAddr, a_iLen);
#endif
}
