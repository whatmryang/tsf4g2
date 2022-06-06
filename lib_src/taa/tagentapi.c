/*
**  @file $RCSfile: tagentapi.c,v $
**  general description of this module
**  $Id: tagentapi.c,v 1.13 2009-09-22 09:54:18 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009-09-22 09:54:18 $
**  @version $Revision: 1.13 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "pal/pal.h"
#include "pal/tmmap.h"
#include "pal/ttypes.h"
#include "../comm/comm_i.h"

#include "taa/tagentapi.h"

#define TAGENT_MSG_MAGIC (unsigned short)0x8899

//#include "../apps/tagent/src/tagent_proto.h"

void*	semcreate  	(int, int);
void	semdestroy 	(void *);
int		semreset	(void *, int);
int		semacquire 	(void *);
int		semrelease 	(void *);

#if defined (_WIN32) || defined (_WIN64)

struct tagShareRestrictedSD
{
	PVOID  ptr;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
};

typedef struct tagShareRestrictedSD SHARERESTRICTEDSD;
typedef struct tagShareRestrictedSD *LPSHARERESTRICTEDSD;

static SHARERESTRICTEDSD	g_stShareSD = {NULL};
SECURITY_ATTRIBUTES* 	tagent_get_sa();

static char *	GetLastErrorString();
static int		tagent_init_security_attr(void);
static void	tagent_fini_security_attr(void);


static char *GetLastErrorString()
{
	static char msgBuf[4096];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msgBuf, 1024, NULL);

	return &msgBuf[0];
}

PVOID tagent_build_share_sd(PSECURITY_DESCRIPTOR a_pSD)
{
	DWORD  dwAclLength;
	PSID   psidEveryone = NULL;
	PACL   pDACL   = NULL;
	BOOL   bResult = TRUE;
	PACCESS_ALLOWED_ACE pACE = NULL;
	SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
	SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;

	// initialize the security descriptor
	if (!InitializeSecurityDescriptor(a_pSD, SECURITY_DESCRIPTOR_REVISION))
	{
		printf("InitializeSecurityDescriptor() failed with error :%s\n",GetLastErrorString());
		bResult = FALSE;
	}

	// obtain a sid for the Authenticated Users Group
	if (bResult)
	{
		if (!AllocateAndInitializeSid(&siaWorld, 1,
			SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,
			&psidEveryone))
		{
			printf("AllocateAndInitializeSid() failed with error: %s\n",GetLastErrorString());
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

int tagent_init_security_attr(void)
{
	g_stShareSD.ptr=NULL;
	g_stShareSD.sa.nLength = sizeof(g_stShareSD.sa);
	g_stShareSD.sa.lpSecurityDescriptor = &g_stShareSD.sd;
	g_stShareSD.sa.bInheritHandle = FALSE;

	// build a restricted security descriptor
	g_stShareSD.ptr = tagent_build_share_sd(&g_stShareSD.sd);
	if (!g_stShareSD.ptr)
	{
		printf("BuildRestrictedSD() failed\n");
		return -1;
	}

	return 0;
}

void tagent_fini_security_attr(void)
{
	if(g_stShareSD.ptr)
	{
		HeapFree(GetProcessHeap(), 0, g_stShareSD.ptr);
		g_stShareSD.ptr = NULL;
	}
}

SECURITY_ATTRIBUTES* tagent_get_sa()
{
	if(g_stShareSD.ptr)
	{
		return &g_stShareSD.sa;
	}

	return NULL;
}

#endif

static char* shmcreate(key_t key, int size)
{
	char*	pShm	= NULL;


#if !defined (_WIN32) && !defined (_WIN64)
    int 		iShmid	= -1;
	iShmid	= shmget(key, size,  IPC_EXCL | 0666);
	if (iShmid < 0)
	{
		if(errno != ENOENT)
		{
			return NULL;
		}

		iShmid	=	shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);
		if(iShmid < 0)
		{
			if(errno != EEXIST)
			{
				return NULL;
			}

			iShmid =	shmget(key, 0, 0666);
			if( iShmid < 0 )
			{
				return NULL;
			}
			else
			{
				if(shmctl(iShmid, IPC_RMID, NULL))
				{
					return NULL;
				}

				iShmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);

				if(iShmid < 0)
				{
					return NULL;
				}
			}
		}
	}

	pShm	= (char *)shmat(iShmid, NULL, 0);
	if (pShm == (void *)-1L)
	{
		return NULL;
	}

#else
	int		iFlag;
	HANDLE		iFile;
    HANDLE 		iShmid	= 0;
	char		szFile[80];
	char		szName[TIPC_MAX_NAME];

	snprintf(szFile, sizeof(szFile),  "C:\\tagent%d", key);
	iFlag	= TF_MODE_WRITE | TF_MODE_READ;

	iFile	= tfopen(&szFile[0], iFlag);
	if ((HANDLE)-1 == iFile)
	{
		iFlag	|=	TF_MODE_CREATE|TF_MODE_EXCL;
		iFile	=	tfopen(&szFile[0], iFlag);
	}

	if ((HANDLE)-1 == iFile)
	{
		return NULL;
	}

	if (tipc_path2name(szName, TIPC_MAX_NAME, &szFile[0], "tmmap:") < 0)
	{
		tfclose(iFile);
		return NULL;
	}

	iShmid	= OpenFileMapping (FILE_MAP_ALL_ACCESS, 0, szName);
	if (0 == iShmid)
	{
		iShmid	= CreateFileMapping((HANDLE)iFile, tagent_get_sa(), PAGE_READWRITE, 0, size, szName);
	}

	tfclose(iFile);
	if (0 == iShmid)
	{
		return NULL;
	}

	pShm	= tmmap(iShmid, 0, size, TMMAPF_PROT_WRITE | TMMAPF_PROT_READ);
	if (NULL == pShm)
	{
		return NULL;
	}
#endif

	return pShm;
}

static int agent_api_getid(char *pShm, unsigned int *puiAppid, unsigned int *puiBusid)
{
	assert (pShm);

	int4store(puiAppid,	*(uint32_t *)pShm);
	pShm += sizeof(uint32_t);

	int4store(puiBusid,	*(uint32_t *)pShm);

	return 0;
}

static void agent_api_setid(char *pShm, unsigned int uiAppid, unsigned int uiBusid)
{
	assert (pShm);

	int4store(pShm,	uiAppid);
	pShm += sizeof(uint32_t);

	int4store(pShm,	uiBusid);
}

int agent_api_init(LPEXCHANGEMNG *ppstExcMng)
{
	LPEXCHANGEMNG	pstExcMng;

#if defined (_WIN32) || defined (_WIN64)
	tagent_init_security_attr();
#endif

	pstExcMng =	(LPEXCHANGEMNG)malloc(sizeof(EXCHANGEMNG));
	if (NULL == pstExcMng)
	{
		return -1;
	}

	memset(pstExcMng, 0x00, sizeof(EXCHANGEMNG));
	pstExcMng->aSem	=	semcreate(EXCHANGEKEY, 1);
	if (NULL == pstExcMng->aSem)
	{

#if defined (_WIN32) || defined (_WIN64)
		tagent_fini_security_attr();
#endif

		free (pstExcMng);
		return -1;
	}

	pstExcMng->pShm =	shmcreate(EXCHANGEKEY, EXCHANGESIZE);
	if (NULL == pstExcMng->pShm)
	{
		semdestroy(pstExcMng->aSem);

#if defined (_WIN32) || defined (_WIN64)
		tagent_fini_security_attr();
#endif

		free (pstExcMng);

		return -1;
	}

	pstExcMng->pstExcHead	=	(EXCHANGEHEAD *)pstExcMng->pShm;

	/* lock by sem */
	semacquire(pstExcMng->aSem);
	if (TAGENT_MSG_MAGIC != pstExcMng->pstExcHead->iMagic)
	{
		pstExcMng->pstExcHead->iExchangeSize	= EXCHANGESIZE;
		pstExcMng->pstExcHead->iBlockSize 		= EXCBLOCKSIZE;
		pstExcMng->pstExcHead->iCreateTime 	= (int)time(NULL);
		pstExcMng->pstExcHead->iMagic			= TAGENT_MSG_MAGIC;
		pstExcMng->pstExcHead->iUsed			= 0;
	}

	strncpy(pstExcMng->pstExcHead->szDesc, "Create by TAGENT_API", 128);
	/* unlock */
	semrelease(pstExcMng->aSem);
	*ppstExcMng	= pstExcMng;

	return 0;
}

void agent_api_destroy(LPEXCHANGEMNG pstExcMng)
{
	if (pstExcMng == NULL)
	{
		return;
	}

	if (pstExcMng->pShm)
	{
		shmdt(pstExcMng->pShm);
		pstExcMng->pShm	= NULL;
	}

	if (pstExcMng->aSem)
	{
		semdestroy(pstExcMng->aSem);
		pstExcMng->aSem	= NULL;
	}

	free(pstExcMng);
	pstExcMng 				= NULL;

#if defined (_WIN32) || defined (_WIN64)
		tagent_fini_security_attr();
#endif
}

int agent_api_get_bussid(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int astuiBusid[], int *piLen)
{
	int		ndx;
	char*	pShm;

	uint32_t	uiTmpAppid	= 0;
	uint32_t	uiTmpBusid	= 0;

	EXCHANGEHEAD*	pstHead;

	if (NULL 	== pstExcMng)
	{
		return -1;
	}

	if (NULL 	== pstExcMng->pShm)
	{
		return -1;
	}

	pstHead	= pstExcMng->pstExcHead;
	pShm	= pstExcMng->pShm	+ EXCBLOCKSIZE;

	if (*piLen < pstHead->iUsed)
	{
		return -1;
	}

	*piLen	= 0;

	semacquire(pstExcMng->aSem);

	for (ndx = 0; ndx < pstHead->iUsed; ndx++)
	{
		agent_api_getid(pShm, &uiTmpAppid, &uiTmpBusid);

		if (uiAppid == uiTmpAppid)
		{
			astuiBusid[(*piLen) ++]	=	uiTmpBusid;
		}

		pShm += EXCBLOCKSIZE;
	}

	semrelease(pstExcMng->aSem);

	return 0;
}

int agent_api_get_blocks(LPEXCHANGEMNG pstExcMng, LPEXCHANGEBLOCK pstBlock[], int *piLen)
{
	int		ndx;
	char*	pShm;

	EXCHANGEHEAD*	pstHead;

	if (NULL 	== pstExcMng)
	{
		return -1;
	}

	if (NULL 	== pstExcMng->pShm)
	{
		return -1;
	}

	pstHead	= pstExcMng->pstExcHead;
	pShm	= pstExcMng->pShm	+ EXCBLOCKSIZE;

	if (*piLen < pstHead->iUsed)
	{
		return -1;
	}

	*piLen	= pstHead->iUsed;
	semacquire(pstExcMng->aSem);

	for (ndx = 0; ndx < pstHead->iUsed; ndx++)
	{
		pstBlock[ndx]	= (LPEXCHANGEBLOCK)pShm;
		pShm += EXCBLOCKSIZE;
	}

	semrelease(pstExcMng->aSem);
	return 0;
}


char *agent_api_get_freebufptr(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int uiBusid)
{
	int		index;
	char*	pShm;
	char*	pDst;

	uint32_t	uiTmpAppid	= 0;
	uint32_t	uiTmpBusid	= 0;

	EXCHANGEHEAD*	pstHead;

	if (NULL 	== pstExcMng)
	{
		return NULL;
	}

	if (NULL 	== pstExcMng->pShm)
	{
		return NULL;
	}

	pstHead		= pstExcMng->pstExcHead;
	pShm		= pstExcMng->pShm	+ EXCBLOCKSIZE;
	pDst		= NULL;

	semacquire(pstExcMng->aSem);

	for (index = 0; index < pstHead->iUsed; index++)
	{
		agent_api_getid(pShm, &uiTmpAppid, &uiTmpBusid);

		if (uiAppid == uiTmpAppid && uiBusid == uiTmpBusid)
		{
			pDst	= pShm + EXCHEADSIZE;
			break;
		}

		pShm += EXCBLOCKSIZE;
	}

	semrelease(pstExcMng->aSem);
	return pDst;
}

// 注册一个ID和一个业务号
int agent_api_register(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int uiBusid)
{
	int		ndx;
	char*	pShm;

	uint32_t	uiTmpAppid	= 0;
	uint32_t	uiTmpBusid	= 0;

	EXCHANGEHEAD*	pstHead;

	if (NULL 	== pstExcMng)
	{
		return -1;
	}

	if (NULL 	== pstExcMng->pShm)
	{
		return -1;
	}

	pstHead	= pstExcMng->pstExcHead;
	pShm	= pstExcMng->pShm	+ EXCBLOCKSIZE;

	semacquire(pstExcMng->aSem);

	for (ndx = 0; ndx < pstHead->iUsed; ndx++)
	{
		agent_api_getid(pShm, &uiTmpAppid, &uiTmpBusid);

		if (uiAppid == uiTmpAppid && uiBusid == uiTmpBusid)
		{
			break;
		}

		pShm += EXCBLOCKSIZE;
	}

	if (pstHead->iUsed >= (EXCHANGESIZE / EXCBLOCKSIZE -1))
	{
		semrelease(pstExcMng->aSem);
		return -2;
	}

	if (ndx == pstHead->iUsed)
	{
		memset (pShm, 0x00, EXCBLOCKSIZE);
		agent_api_setid(pShm, uiAppid, uiBusid);

		pstHead->iUsed ++;
	}

	semrelease(pstExcMng->aSem);
	return 0;
}

int agent_api_unregister(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int uiBusid)
{
	int 		ndx;
	char*	pShm;
	char*	pLast;

	uint32_t	uiTmpAppid	= 0;
	uint32_t	uiTmpBusid	= 0;

	EXCHANGEHEAD*	pstHead;

	if (NULL 	== pstExcMng)
	{
		return -1;
	}

	if (NULL 	== pstExcMng->pShm)
	{
		return -1;
	}

	pstHead	= pstExcMng->pstExcHead;
	pShm	= pstExcMng->pShm + EXCBLOCKSIZE;
	pLast	= pShm + pstHead->iUsed * EXCBLOCKSIZE;

	semacquire(pstExcMng->aSem);

	for (ndx = 0; ndx < pstHead->iUsed; ndx++)
	{
		agent_api_getid(pShm, &uiTmpAppid, &uiTmpBusid);

		if (uiAppid == uiTmpAppid && uiBusid == uiTmpBusid)
		{
			if (ndx != pstHead->iUsed - 1)
			{
				memcpy(pShm, pLast, EXCBLOCKSIZE);
			}

			pstHead->iUsed --;

			break;
		}

		pShm += EXCBLOCKSIZE;
	}

	semrelease(pstExcMng->aSem);
	return 0;
}

int semacquire(void *sem)
{
#if defined (_WIN32) || defined (_WIN64)

	HANDLE	hMutex = (void *)sem;

	if (WaitForSingleObject(hMutex, INFINITE) == WAIT_FAILED)
	{
		return -1;
	}

	return 0;

#else

	struct sembuf sb = { 0, -1, SEM_UNDO };
	if (sem && semop(*(int *)sem, &sb, 1) < 0)
	{
		return -1;
	}

#endif

	return 0;
}

int semrelease(void *sem)
{
#if defined (_WIN32) || defined (_WIN64)

	HANDLE	hMutex = (void *)sem;

	if (!ReleaseMutex(hMutex))
	{
		return -1;
	}

	return 0;

#else

	struct sembuf sb = { 0, 1, SEM_UNDO };
	if (sem && semop(*(int *)sem, &sb, 1) < 0)
	{
		return -1;
	}

#endif

	return 0;
}

int semreset(void *sem, int val)
{
	/* I think semaphores with UNDO won't need resetting... */
	(void)sem;
	(void)val;

	return 0;
}

void semdestroy(void *sem)
{
#if defined (_WIN32) || defined (_WIN64)

	HANDLE	hMutex = (void *)sem;

	CloseHandle (hMutex);

#else

	union semun u;
	u.val = 1;

	semctl(*(int *)sem, 0, IPC_RMID, u);
	free (sem);

#endif
}

void *semcreate(key_t key, int val)
{
#if defined (_WIN32) || defined (_WIN64)

	char		szName[32];
	HANDLE	hMutex;

	SECURITY_DESCRIPTOR	sd;
	SECURITY_ATTRIBUTES		sa;

	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

	sa.nLength			=   sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle		=   TRUE;
	sa.lpSecurityDescriptor	=   &sd;

	snprintf(szName, sizeof(szName), "%d", key);
	hMutex	= CreateMutex(&sa, FALSE, szName);

	return (void *)hMutex;

#else

	int *semid	= (int *)malloc(sizeof(*semid));

	union semun u;

	if (semid == NULL)
	{
		return NULL;
	}

	u.val = val;

	if ((*semid	= semget(key, 1, 0666)) < 0)
	{
		*semid	= semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
		if (*semid < 0)
		{
			return NULL;
		}
	}

	if (semctl(*semid, 0, SETVAL, u) < 0)
	{
		return NULL;
	}

	return (void *)semid;

#endif
}

#if 0
int main(int argc, char *argv[])
{
	int	iRet;
	EXCHANGEMNG pstExcMng;

	iRet	= agent_api_init(&pstExcMng);
	if (iRet < 0)
	{
		fprintf (stderr, "agent_api_init error\n");
		return -1;
	}

	iRet	= agent_api_register(pstExcMng, 23, 1);
	if (iRet < 0)
	{
		fprintf (stderr, "agent_api_register error\n");
		return -1;
	}

	iRet	= agent_api_unregister(pstExcMng, 23, 1);
	if (iRet < 0)
	{
		fprintf (stderr, "agent_api_unregister error\n");
		return -1;
	}

	agent_api_destroy(pstExcMng);

	return 0;
}
#endif

