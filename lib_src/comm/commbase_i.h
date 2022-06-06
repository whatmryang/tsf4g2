#ifndef __COMM_BASE_I_H_
#define __COMM_BASE_I_H_

#define COMMBASE_VERSION 1
#define COMMBASE_MAGIC 0xCCBB


enum tagCommTypes
{
	COMM_TYPE_FLOOR,
	COMM_TYPE_MEMPOOL,
	COMM_TYPE_SHT,
	COMM_TYPE_TIMER,
	COMM_TYPE_CEILING
};

struct tagCommBase
{
	int iId;
	int iMagic;
	int iVersion;
	char szName[64];
	int iIsShm;
	int iType;
	int iMaxSize;
	int iFlags;

	//Idx 2 Ptr
	//The following function lists should only be added.
	//It is for the usage of oo in lib comm
	//Function pointers for tlist.
	
};

typedef struct tagCommBase COMMBASE;
typedef struct tagCommBase* LPCOMMBASE;

int commbase_init(LPCOMMBASE a_pstBase,int a_iType,const char *a_szName);
int commbase_get_type(LPCOMMBASE a_pstBase);
int commbase_fini(LPCOMMBASE a_pstBase);

int commbase_attach(LPCOMMBASE *pstBase,void *addr);


#endif
