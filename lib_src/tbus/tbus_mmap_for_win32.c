#if defined (_WIN32) || defined (_WIN64)

#include <string.h>
#include <time.h>

#include "taa/tagentapi.h"
#include "tbus/tbus_error.h"
#include "tbus_log.h"
#include "tbus_kernel.h"
#include "tbus_mmap_for_win32.h"

static HANDLE tbus_get_mmap_id_i(IN const char* a_pszFile, IN const char* a_pszName,
        IN int a_iSize, IN int a_iFlag, OUT int* a_piCreate);
HANDLE tbus_create_file_mapping_i(IN HANDLE a_iHandle, IN int a_iSize, IN const char* a_pszName);

struct tagShareRestrictedSD
{
	PVOID  ptr;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
};
typedef struct tagShareRestrictedSD SHARERESTRICTEDSD;
typedef struct tagShareRestrictedSD *LPSHARERESTRICTEDSD;

SHARERESTRICTEDSD g_stShareSD = {NULL};
SECURITY_ATTRIBUTES* tbus_get_sa();

static char *GetLastErrorString()
{
	static char msgBuf[1024];

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		msgBuf, 1024, NULL );

	return &msgBuf[0];
}


//关于此接口的实现, 请参考CSDN blog:ttp://blog.csdn.net/jiangsheng/archive/2004/06/24/25563.aspx
PVOID tbus_build_share_sd(PSECURITY_DESCRIPTOR a_pSD)
{
	DWORD  dwAclLength;
	PSID   psidEveryone = NULL;
	PACL   pDACL   = NULL;
	BOOL   bResult = TRUE;
	PACCESS_ALLOWED_ACE pACE = NULL;
	SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY  ;
	SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;



	// initialize the security descriptor
	if (!InitializeSecurityDescriptor(a_pSD, SECURITY_DESCRIPTOR_REVISION))
	{
		printf("InitializeSecurityDescriptor() failed with error :%s\n",
			GetLastErrorString());
		bResult = FALSE;
	}

	// obtain a sid for the Authenticated Users Group
	if (bResult)
	{
		if (!AllocateAndInitializeSid(&siaWorld, 1,
			SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,
			&psidEveryone))
		{
			printf("AllocateAndInitializeSid() failed with error: %s\n",
				GetLastErrorString());
			bResult = FALSE;
		}
	}/*if (bResult)*/


	// NOTE:
	//
	// The Authenticated Users group includes all user accounts that
	// have been successfully authenticated by the system. If access
	// must be restricted to a specific user or group other than
	// Authenticated Users, the SID can be constructed using the
	// LookupAccountSid() API based on a user or group name.

	if (bResult)
	{
		// calculate the DACL length
		dwAclLength = sizeof(ACL)
			// add space for Authenticated Users group ACE
			+ sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)
			+ GetLengthSid(psidEveryone);

		// allocate memory for the DACL
		pDACL = (PACL) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
			dwAclLength);
		if (!pDACL)
		{
			printf("HeapAlloc() failed with error: %s\n", GetLastErrorString());
			bResult = FALSE;
		}
	}/*if (bResult)*/


	// initialize the DACL
	if (bResult)
	{
		if (!InitializeAcl(pDACL, dwAclLength, ACL_REVISION))
		{
			printf("InitializeAcl() failed with error: %s\n",
				GetLastErrorString());
			bResult = FALSE;
		}
	}/*if (bResult)*/


	// add the Authenticated Users group ACE to the DACL with
	// GENERIC_READ, GENERIC_WRITE, and GENERIC_EXECUTE access
	if (bResult)
	{
		if (!AddAccessAllowedAce(pDACL, ACL_REVISION,
			GENERIC_ALL,
			psidEveryone)) {
				printf("AddAccessAllowedAce() failed with error: %s\n",
					GetLastErrorString());
				bResult = FALSE;
		}
	}/*if (bResult)*/


	// set the DACL in the security descriptor
	if (bResult)
	{
		if (!SetSecurityDescriptorDacl(a_pSD, TRUE, pDACL, FALSE))
		{
			printf("SetSecurityDescriptorDacl() failed with error: %s\n",
				GetLastErrorString());
			bResult = FALSE;
		}
	}/*if (bResult)*/


	if (psidEveryone) FreeSid(psidEveryone);
	if (bResult == FALSE)
	{
		if (pDACL) HeapFree(GetProcessHeap(), 0, pDACL);
		pDACL = NULL;
	}

	return (PVOID) pDACL;
}

int tbus_init_security_attr()
{
	g_stShareSD.ptr=NULL;
	g_stShareSD.sa.nLength = sizeof(g_stShareSD.sa);
	g_stShareSD.sa.lpSecurityDescriptor = &g_stShareSD.sd;
	g_stShareSD.sa.bInheritHandle = FALSE;

	// build a restricted security descriptor
	g_stShareSD.ptr = tbus_build_share_sd(&g_stShareSD.sd);
	if (!g_stShareSD.ptr)
	{
		printf("BuildRestrictedSD() failed\n");
		return -1;
	}

	return 0;
}

void tbus_fini_security_attr()
{
	if(g_stShareSD.ptr)
	{
		HeapFree(GetProcessHeap(), 0, g_stShareSD.ptr);
		g_stShareSD.ptr = NULL;
	}
}

SECURITY_ATTRIBUTES* tbus_get_sa()
{
	if(g_stShareSD.ptr)
	{
		return &g_stShareSD.sa;
	}

	return NULL;
}


int tbus_generate_path(IN key_t a_tKey, IN unsigned a_uAddr1,
        IN unsigned a_uAddr2, IN int a_iOnlyUseKey, INOUT char* a_pszFilePathBuf, IN size_t a_iBufLen)
{
    int iRet = 0;
	char *pszAppDataPath = NULL;
	char szPath[TBUS_MAX_PATH_LEN];
	char szAppPath[TBUS_MAX_PATH_LEN];

	iRet = GetEnvironmentVariable(TEXT(TBUS_APPDATA_PATH), &szAppPath[0], sizeof(szAppPath));
	if(!iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"GetEnvironmentVariable failed (%d) by name %s\n", GetLastError(), TBUS_APPDATA_PATH);
		pszAppDataPath = TBUS_DEFAULT_MMAP_FILE_DIR;
	}else
	{
		pszAppDataPath = &szAppPath[0];
	}

	iRet = snprintf(szPath, sizeof(szPath),  "%s", pszAppDataPath);
	if ((0 > iRet) || (iRet >= (int)sizeof(szPath)))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to write mmap filepath,  iwrite(%d) sizeofBuff:%d", iRet, sizeof(szPath));
		return -1;
	}

    if (a_iOnlyUseKey)
        iRet = snprintf(a_pszFilePathBuf, a_iBufLen,  "%s%ctbus_%d", szPath, TOS_DIRSEP, a_tKey);
    else
        iRet = snprintf(a_pszFilePathBuf, a_iBufLen,  "%s%ctbus_%d_%u_%u", szPath, TOS_DIRSEP, a_tKey, a_uAddr1, a_uAddr2);

	if ((0 > iRet) || (iRet >= (int)a_iBufLen))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to write mmap filepath,  iwrite(%d) sizeofBuff:%d", iRet, a_iBufLen);
		return -1;
	}
    tos_mkdir_fast(szPath);

    return iRet;
}

HANDLE tbus_create_file_mapping_i(IN HANDLE a_iHandle, IN int a_iSize, IN const char* a_pszName)
{
    HANDLE iMapid;

    iMapid	=	CreateFileMapping(a_iHandle, tbus_get_sa(), PAGE_READWRITE, 0, a_iSize, a_pszName );

    /* About CreateFileMapping:
     * If the object exists before the function call,
     * the function returns a handle to the existing object
     * (with its current size, not the specified size),
     * and GetLastError returns ERROR_ALREADY_EXISTS.
     */
    if( iMapid &&  ERROR_ALREADY_EXISTS == GetLastError() )
    {
        CloseHandle(iMapid);
        iMapid	=	NULL;
    }

    return iMapid;
}


int tbus_open_mmap_by_source_file_i(INOUT void **ppShm, IN const char* a_pszFile,
        IN unsigned int *a_piSize, IN int a_iFlag, OUT int *a_piCreate, OUT HANDLE *a_piMMapID)
{
	int iRet = 0 ;
	HANDLE iMapid = (HANDLE)-1;
	struct shmid_ds stShmStat ;
	int iFile = -1;
	char szName[TIPC_MAX_NAME];

	assert(NULL != ppShm);
	assert(NULL != a_piCreate);

    *ppShm = NULL;

    // mmap exists == source file exists
    // that's, TMMAPF_CREATE refers to creating source file

    if( tipc_path2name(szName, TIPC_MAX_NAME, &a_pszFile[0], "tmmap:") < 0 )
    {
        tbus_log(TLOG_PRIORITY_ERROR, "failed to create mmap name by file path(%s)", a_pszFile);
        return -1;
    }
    iMapid = tbus_get_mmap_id_i(a_pszFile, szName, *a_piSize, a_iFlag, a_piCreate);
    if (NULL == iMapid)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "failed to get mmap id by file(%s), name(%s), size(%d)",
                a_pszFile, szName, *a_piSize);
        return -1;
    }

	*ppShm = tmmap(iMapid, 0, *a_piSize, TMMAPF_PROT_WRITE|TMMAPF_PROT_READ);
	if (NULL == *ppShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to open mmap by id(%d),  isize:%d errno(%d): %s",
			iMapid, *a_piSize,GetLastError(), GetLastErrorString());
		return -1;
	}

	if ( 0 == shmctl( iMapid, IPC_STAT, &stShmStat))
	{
		*a_piSize = (int)stShmStat.shm_segsz;
	}

    if (a_piMMapID)
        *a_piMMapID = iMapid;

	return 0;
}

static HANDLE tbus_get_mmap_id_i(IN const char* a_pszFile, IN const char* a_pszName,
        IN int a_iSize, IN int a_iFlag, OUT int* a_piCreate)
{
    HANDLE iMapid = NULL;
    HANDLE iFile = (HANDLE)-1;
    int iFileFlags;

    assert(NULL != a_piCreate);

    /* 在windows系统下，只要用来映射的文件存在，就应该认为共享内存是存在的 */
    /* 这种意义下，TMMAPF_CREATE意味着首先要创建映射文件，然后再把文件映射到内存中 */
	iFileFlags = TF_MODE_WRITE | TF_MODE_READ;
	iFile	=	tfopen(&a_pszFile[0], iFileFlags);
	if (((HANDLE)-1 == iFile) && !(a_iFlag & TMMAPF_CREATE))
	{
        return iMapid;
	}

    if ((HANDLE)-1 == iFile)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to open file(%s),  error: %s",
                a_pszFile, GetLastErrorString());
        iFileFlags |= TF_MODE_CREATE|TF_MODE_EXCL;
        iFile	=	tfopen(a_pszFile, iFileFlags);
        if ((HANDLE)-1 == iFile)
        {
            /* 有一种情况是，文件此时又存在了 */
            /* 如果文件又存在了，意味着存在其他同时的操作, 返回错误更容易检测到冲突 */
            /* 所以这里没有对这种情况做特殊处理 */
            tbus_log(TLOG_PRIORITY_ERROR,"failed to create file(%s),  error: %s",
                    a_pszFile, GetLastErrorString());
            return iMapid;
        }
        *a_piCreate = 1;
    }

	iMapid = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, a_pszName );
	if (NULL == iMapid)
	{
		iMapid	=	tbus_create_file_mapping_i(iFile, a_iSize, a_pszName);
        if (NULL == iMapid)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "failed to create mmap by path(%s), errno(%d): %s",
                    a_pszFile, GetLastError(), GetLastErrorString());
            tfclose(iFile);
            return iMapid;
        }
	}/*if (NULL == iMapid) */
	tfclose(iFile);

    return iMapid;
}

int tbus_open_mmap_i(INOUT void **ppShm, IN key_t a_tKey, IN unsigned a_uAddr1, IN unsigned a_uAddr2,
        IN int a_iUseOnlyKey, IN unsigned int *a_piSize, IN int a_iFlag, OUT int *a_piCreate, OUT HANDLE* a_piMMapID)
{
	char szFile[TBUS_MAX_PATH_LEN];
	int iRet = 0 ;
	HANDLE iMapid = (HANDLE)-1;
	int iFile = -1;
    unsigned int iExpectedSize;

	assert(NULL != ppShm);
	assert(NULL != a_piCreate);

    if (a_iUseOnlyKey)
        iRet = tbus_key_to_path(a_tKey, szFile, sizeof(szFile));
    else
        iRet = tbus_key_and_addr_to_path(a_tKey, a_uAddr1, a_uAddr2, szFile, sizeof(szFile));
    if (0 > iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "failed to generate filepath from key(%d)", a_tKey);
        return -1;
    }

    iExpectedSize = *a_piSize;
    iRet = tbus_open_mmap_by_source_file_i(ppShm, szFile, a_piSize, a_iFlag, a_piCreate, &iMapid);
    if (0 > iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "failed to open file mapping by filepath(%s)", szFile);
        return -1;
    }


#if 0
    if ((a_iFlag & TMMAPF_CREATE) && (*a_piCreate == 0) && (iExpectedSize != *a_piSize))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "the shm sizee<%d> mmap by file(%s) is diff from the expected size(%d), so relloc the shm",
            *a_piSize,szFile, iExpectedSize);

        tmunmap(*ppShm,*a_piSize);
        tmmapclose(iMapid);
        iRet = remove(szFile);
        if (0 > iRet)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "remove file<%s> failed, for %s", szFile, GetLastErrorString());
            return -1;
        }

        *a_piSize = iExpectedSize;
        iRet = tbus_open_mmap_by_source_file_i(ppShm, szFile, a_piSize, a_iFlag, a_piCreate, &iMapid);
        if (0 > iRet)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "failed to open file mapping by filepath(%s)", szFile);
            return -1;
        }
    }
#endif

    if (a_piMMapID)
    {
       *a_piMMapID = iMapid;
    }



	return 0;
}

int tbus_delete_channel_file_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead)
{
    int iRet = 0;
    char szFile[TIPC_MAX_NAME];

    iRet = tbus_key_and_addr_to_path(a_pstHead->dwShmKey, (unsigned)a_pstShmChl->astAddrs[0],
            (unsigned)a_pstShmChl->astAddrs[1], szFile, sizeof(szFile));

    if (0 > iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "tbus_key_and_addr_to_path failed by key<%u>, addr1<%u>, addr2<%u>",
                a_pstHead->dwShmKey, (unsigned)a_pstShmChl->astAddrs[0], (unsigned)a_pstShmChl->astAddrs[1]);
        return -1;
    }

    iRet = remove(szFile);
    if (0 > iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "remove file<%s> failed, for %s", szFile, GetLastErrorString());
        return -1;
    }

    return iRet;
}
#endif
