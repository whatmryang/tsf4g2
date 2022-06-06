#include "comm/shtable.h"
#include "string.h"

#define MAX_APP_ITEM  1000
#define MAX_APP_BUCKET  5*MAX_APP_ITEM
#define SCAN_NUM_ONE_LOOP 10

typedef struct tagUinShtData
{
	long lUin;	 /*uin of player*/
	int  iIdx;	/*index of player in memory pool*/
}UinShtData;

static int UinHashCmp( const void * pv1 , const void *pv2 )
{
	UinShtData *pstUinHashData1 = (UinShtData *)pv1;
	UinShtData *pstUinHashData2 = (UinShtData *)pv2;
	return pstUinHashData1->lUin - pstUinHashData2->lUin;
}

static unsigned int UinHashCode(const void* pvData)
{
	UinShtData *pstUinShtData = (UinShtData *)pvData;
	return (unsigned int)pstUinShtData->lUin;
}

int main(int argc, char* argv[])
{
	LPSHTABLE pstHashTab;
	size_t iHashTabSize;
	UinShtData stUinHashData;
	UinShtData *pstUinHashData;

	(void) argv;
	(void)argc;

	pstHashTab = sht_create(MAX_APP_BUCKET, MAX_APP_ITEM, sizeof(UinShtData), &iHashTabSize);

	if (NULL == pstHashTab)
	{
		return -1;
	}

	stUinHashData.lUin = 267478100;
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);

	if (!pstUinHashData)
	{
		return -1;
	}
	memcpy(pstUinHashData, &stUinHashData,
			sizeof(*pstUinHashData));

	stUinHashData.lUin = 267478100;
	pstUinHashData = (UinShtData *) sht_find(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);

	if (!pstUinHashData)
	{
		return -1;
	}

	stUinHashData.lUin = 267478100;
	sht_remove(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);

//#	/*遍历shtable的方法，一般来说游戏服务器不可能一次遍历完，都是每次遍历几个item*/
//#	for (i=0; i<SCAN_NUM_ONE_LOOP; i++)
//#	{
//#		pstUinHashData = sht_pos(pstHashTab, iIndex, &iValid);
//#		iIndex++;
//#		if (iIndex >= pstHashTab->iMax)
//#			iIndex = 0;
//#
//#		if (NULL == pstUinHashData || !iValid)
//#			continue;
//#
//#		//here item is valid
//#	}

	sht_destroy(&pstHashTab);

	return 0;
}

