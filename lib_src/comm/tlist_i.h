#ifndef TLIST_I_H__
#define TLIST_I_H__

#if 0
#include "comm/tlist.h"

#define INITNODE(pstList,NODE) do{\
			(NODE).stPrev.idx = -1;\
			(NODE).stPrev.iOffset = (intptr_t)(&NODE)-(intptr_t)(pstList);\
			(NODE).stNext.idx = -1;\
			(NODE).stNext.iOffset =(intptr_t)(&NODE)-(intptr_t)(pstList);\
		}while(0)

struct tlist_head 
{
	int iNeedFree;
	TLISTNODE stNode;
	//intptr_t iBaseOffset;
	//char *pvBase;
};

typedef struct tlist_head TLIST;


#define TLIST_ADDR2PTR(pstList,pstAddr) (pstList->pvBase + pstAddr->iOffset)

int tlist_init_from_mem(LPTLIST pstList,void* pstPool);
#endif

#endif
