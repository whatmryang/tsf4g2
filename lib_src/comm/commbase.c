#include "commbase_i.h"
#include "pal/pal.h"

#if 0

#if !defined (_WIN32) && !defined (_WIN64)
#define COMMDEBUG(args...) fprintf(stderr,##args)
#else
#define COMMDEBUG(fmt,...) fprintf(stderr,fmt,__VA_ARGS__)
#endif

int commbase_is_legal_type(int a_iType)
{
	return (a_iType > COMM_TYPE_FLOOR) && (a_iType < COMM_TYPE_CEILING);
}

int commbase_init(LPCOMMBASE a_pstBase,int a_iType,const char *a_szName)
{
	if(!commbase_is_legal_type(a_iType))
	{
		return -1;
	}

	memset(a_pstBase,0,sizeof(*a_pstBase));
	a_pstBase->iType = a_iType;
	a_pstBase->iMagic = COMMBASE_MAGIC;
	STRNCPY(a_pstBase->szName, a_szName, sizeof(a_pstBase->szName));
	return 0;
}

int commbase_get_type(LPCOMMBASE a_pstBase)
{
	return a_pstBase->iType;
}

int commbase_fini(LPCOMMBASE a_pstBase)
{
	return 0;
}

int commbase_attach(LPCOMMBASE *pstBase,void *addr)
{
	LPCOMMBASE res;

	if(NULL == addr)
	{
		return -1;
	}

//	COMMDEBUG("Attaching addr %"PRIdPTR"\n",addr);

	res = (LPCOMMBASE )addr;

	if(res->iMagic == COMMBASE_MAGIC)
	{
		int i = 0;
		for(i = 0;i <sizeof(res->szName);i++ )
		{
			if(res->szName[0] == '\0')
			{
				break;
			}
		}

		if(i == sizeof(res->szName))
		{
			COMMDEBUG("Invalid name\n");
			return -1;
		}

		return 0;
	}

	return -1;
}

int commbase_alloc(LPCOMMBASE *a_pstBase,size_t size)
{
	return 0;
}


int commbase_dump_to_fp(LPCOMMBASE *a_pstBase,FILE *fp)
{
	return 0;
}

int commbase_dump_to_file(LPCOMMBASE *a_pstBase,const char *pszFileName)
{
	return 0;
}

/*
	I should try to fix a broken shm
*/
int commbase_fix(LPCOMMBASE *a_pstBase)
{

	return 0;
}

#endif
