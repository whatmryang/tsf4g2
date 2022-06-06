#include "tcommshm_i.h"
#include "pal/tos.h"
#define TCOMMSHM_PATH_LEN 512


int tcommshm_make_file_i(char *pszName)
{
	HANDLE fd;
	char *dir;
	char *tmp;
	if(0 == access(pszName,F_OK))
	{
		return 0;
	}

	tmp = strdup(pszName);
	dir = dirname(tmp);
	tos_mkdir(dir);
	free(tmp);

	fd = tfopen(pszName,TF_MODE_CREATE |TF_MODE_WRITE );
	if((HANDLE)-1 == fd)
	{
		return -1;
	}
	tfclose(fd);
	return 0;
}

int tcommshm_check_i(LPTCOMMSHM pstShm,char *pszName,size_t iSize)
{
	if(pstShm->iMagic != TCOMMSHMMAGIC)
	{
		return -1;
	}

	if(strncmp(pstShm->szName,pszName,sizeof(pstShm->szName)))
	{
		return -1;
	}

	if(pstShm->iSize != iSize)
	{
		return -1;
	}

	return 0;
}

int tcommshm_remove(char *pszName)
{
	int iRet;
	char szFile[TCOMMSHM_PATH_LEN];
	memset(szFile,0,sizeof(szFile));
	#if defined (_WIN32) || defined (_WIN64)
	snprintf(szFile, sizeof(szFile),  "%s%s", "C:\\tsf4g\\tcommshm\\",pszName);
	#else
	snprintf(szFile, sizeof(szFile),  "%s%s", "/tmp/",pszName);
	#endif

	//Remove the shm. We have to call the tshm
	iRet = tshmdelete(szFile, 10);
	if(iRet)
	{
		return -1;
	}

	return 0;
}

// Attach to a known shm.
//
int tcommshm_attach(LPTCOMMSHM *ppstShm,char *pszName,size_t iSize)
{

	return 0;
}


int tcommshm_init(LPTCOMMSHM *ppstShm,char *pszName,size_t iSize,int *piCreated)
{
	size_t iRealSize = sizeof(TCOMMSHM) + iSize;
	char szFile[TCOMMSHM_PATH_LEN];
	HANDLE HMemMap;
	LPTCOMMSHM pstShm;
	int iRet;
	int iCreated;


	memset(szFile,0,sizeof(szFile));
	#if defined (_WIN32) || defined (_WIN64)
	do{
		char szSystemdir[1024];
		GetWindowsDirectoryA(szSystemdir,sizeof(szSystemdir));
		snprintf(szFile, sizeof(szFile),  "%s%s",szSystemdir,pszName);
	}while(0);
	#else
	snprintf(szFile, sizeof(szFile),  "%s%s", "/tmp/",pszName);
	#endif

	iRet = tcommshm_make_file_i(szFile);
	if(iRet)
	{
		return -1;
	}

	HMemMap = tshmopen(szFile,iRealSize,TSHMF_CREATE,10);
	if(!HMemMap)
	{
		return -1;
	}

	pstShm = tshmat(HMemMap,0);
	if((void *)-1 == pstShm)
	{
		return -1;
	}

	if(TCOMMSHMMAGIC == pstShm->iMagic)
	{
		//It is already a shm,I have to attach to it.
		iRet = tcommshm_check_i(pstShm,szFile,iSize);
		if(iRet)
		{
			return TCOMMSHM_ERROR_CHECK_FAIL;
		}
		iCreated = 0;
	}
	else
	{
		STRNCPY(pstShm->szName,szFile,sizeof(pstShm->szName ));
		pstShm->iRealSize = iRealSize;
		pstShm->iSize = iSize;
		pstShm->iMagic = TCOMMSHMMAGIC;
		iCreated = 1;
	}


	if(piCreated)
	{
		*piCreated = iCreated;
	}

	*ppstShm = pstShm;
	return 0;
}

int tcommshm_get(LPTCOMMSHM pstShm,void **pvData,size_t *iSize)
{
	if(!pstShm)
	{
		return -1;
	}

	if(pvData)
	{
		*pvData = pstShm->szData;
	}

	if(iSize)
	{
		*iSize = pstShm->iSize;
	}

	return 0;
}

int tcommshm_fini(LPTCOMMSHM *pstShm)
{
	*pstShm = NULL;
	return 0;
}
