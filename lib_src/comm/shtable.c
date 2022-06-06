#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "comm/shtable.h"
#include "shtable_i.h"

int sht_find_i(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, unsigned int uCode);
int sht_make_free_chain_i(LPSHTABLE pstTab);
int sht_alloc_node_i(LPSHTABLE pstTab);
int sht_free_node_i(LPSHTABLE pstTab, int iNode);
int sht_alloc_node_i(LPSHTABLE pstTab);
void sht_insert_i(LPSHTABLE pstTab, int iNode, unsigned int uCode);
void sht_remove_i(LPSHTABLE pstTab, int iNode, unsigned int uCode);

unsigned int sht_get_code(const char* szKey)
{
	unsigned int uCode=0;

	while( szKey[0] )
	{
		uCode += (uCode<<5) + ((unsigned char)szKey[0]);
		szKey++;
	}

	return uCode;
}

size_t sht_get_size(size_t a_iBuck,size_t a_iMax,size_t a_iUnit)
{
	return SHT_SIZE(a_iBuck, a_iMax, a_iUnit);
}

int sht_make_free_chain_i(LPSHTABLE pstTab)
{
	LPSHITEM pstItem;
	LPSHITEM pstNext;
	int i;
	int n;

	pstTab->iFreeHead	=	-1;
	n	=	0;

	for(i=pstTab->iMax-1; i>=0; i--)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);

		if( pstItem->fValid )
			continue;

		n++;

		pstItem->iNext	=	pstTab->iFreeHead;
		pstItem->iPrev	=	-1;

		if( pstTab->iFreeHead<0 )
		{
			pstTab->iFreeHead	=	i;
		}
		else
		{
			pstNext	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
			pstNext->iPrev	=	i;
			pstTab->iFreeHead	=	i;
		}
	}

	return n;
}

void *sht_get_by_pos(LPSHTABLE pstTab,size_t iIdx)
{
	LPSHITEM pstItem;
	pstItem = SHT_GET_ITEM(pstTab, iIdx);
	if(pstItem->fValid)
	{
		return SHT_GET_DATA(pstTab, iIdx);
	}

	return NULL;
}

int sht_alloc_node_i(LPSHTABLE pstTab)
{
	int iNode;
	LPSHITEM pstItem;
	LPSHITEM pstHead;

	if( pstTab->iFreeHead<0 || pstTab->iFreeHead>= (intptr_t)pstTab->iMax )
		return -1;

	iNode	=	pstTab->iFreeHead;
	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	pstTab->iFreeHead	=	pstItem->iNext;

	pstItem->fValid	=	1;
	pstItem->uCode	=	0;
	pstItem->iNext	=	-1;
	pstItem->iPrev	=	-1;

	if( pstTab->iFreeHead>=0 && pstTab->iFreeHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
		pstHead->iPrev	=	-1;
	}

	pstTab->iItem++;

	return iNode;
}

int sht_free_node_i(LPSHTABLE pstTab, int iNode)
{
	LPSHITEM pstItem;
	LPSHITEM pstHead;

	if( iNode<0 || iNode>=pstTab->iMax )
		return -1;

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	pstItem->fValid	=	0;
	pstItem->uCode	=	0;
	pstItem->iPrev	=	-1;
	pstItem->iNext	=	-1;

	if( pstTab->iFreeHead>=0 && pstTab->iFreeHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
		pstHead->iPrev	=	iNode;
	}

	pstItem->iNext		=	pstTab->iFreeHead;
	pstTab->iFreeHead	=	iNode;

	pstTab->iItem--;

	return iNode;
}

void sht_insert_i(LPSHTABLE pstTab, int iNode, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	LPSHITEM pstHead;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);

	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	pstItem->uCode	=	uCode;

	if( pstBucket->iCount>0 )
		pstItem->iNext	=	pstBucket->iHead;
	else
		pstItem->iNext	=	-1;

	pstItem->iPrev	=	-1;

	if( pstBucket->iCount>0 && pstBucket->iHead>=0 && pstBucket->iHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstBucket->iHead);
		pstHead->iPrev	=	iNode;
	}

	pstBucket->iHead	=	iNode;
	pstBucket->iCount++;
}

void sht_remove_i(LPSHTABLE pstTab, int iNode, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	LPSHITEM pstPrev;
	LPSHITEM pstNext;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);

	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	if( pstItem->iPrev>=0 && pstItem->iPrev<pstTab->iMax )
	{
		pstPrev	=	SHT_GET_ITEM(pstTab, pstItem->iPrev);
		pstPrev->iNext	=	pstItem->iNext;
	}

	if( pstItem->iNext>=0 && pstItem->iNext<pstTab->iMax )
	{
		pstNext	=	SHT_GET_ITEM(pstTab, pstItem->iNext);
		pstNext->iPrev	=	pstItem->iPrev;
	}

	if( pstBucket->iHead==iNode )
		pstBucket->iHead	=	pstItem->iNext;

	pstBucket->iCount--;
}

int sht_find_i(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	int iNode;
	int n;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);
	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	if( pstBucket->iCount<=0 )
		return -1;

	iNode	=	pstBucket->iHead;
	n		=	0;

	while(iNode>=0 && iNode<pstTab->iMax && n<pstBucket->iCount )
	{
		pstItem	=	SHT_GET_ITEM(pstTab, iNode);

		if( pstItem->uCode==uCode && !pfnCmp(SHT_ITEM2DATA(pstTab,pstItem), pvData) )
			return iNode;

		iNode	=	pstItem->iNext;
		n++;
	}

	return -1;
}

LPSHTABLE sht_init(void* pvBuff, size_t iBuff, int iBucket, int iMax, int iUnit)
{
	LPSHTABLE pstTab;
	LPSHITEM pstItem;
	int i;

	if( iBuff< SHT_HEADSIZE() || iBucket<0 || iMax<0 || iUnit<=0 )
		return NULL;

	memset(pvBuff, 0, sizeof(SHTABLE));

	pstTab	=	(LPSHTABLE) pvBuff;

	pstTab->cbSize		=	sizeof(SHTABLE);
	pstTab->uFlags		=	0;

	pstTab->iVersion	=	SHT_VERSION;
	pstTab->iBuff		=	iBuff;

	pstTab->iBucket		=	iBucket;
	pstTab->iMax		=	iMax;
	pstTab->iItem		=	0;
	pstTab->iHeadSize	=	SHT_HEADSIZE();

	pstTab->iFreeHead	=	-1;

	if( (iBuff - pstTab->iHeadSize)/sizeof(SHBUCKET) < (size_t)iBucket )
		return NULL;

	pstTab->iBucketOff	=	pstTab->iHeadSize;
	pstTab->iBucketSize	=	sizeof(SHBUCKET)*iBucket;


	pstTab->iDataHeadOff = pstTab->iBucketOff + pstTab->iBucketSize;
	pstTab->iDataHeadSize    = sizeof(SHITEM) * iMax;

	pstTab->iDataOff		=	pstTab->iDataHeadOff + pstTab->iDataHeadSize;
	pstTab->iDataUnit	=	iUnit;
	pstTab->iDataSize		=	pstTab->iDataUnit*iMax;

	//2008.06.10 by kent, init item valid flag
	for (i = 0; i<pstTab->iMax; i++)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);
		pstItem->fValid = 0;
	}
	//by kent end

	//2008.09.25 by kent, init bucket
	memset(((char *)pstTab) + pstTab->iBucketOff, 0, pstTab->iBucketSize);
	//by kent end

	sht_make_free_chain_i(pstTab);

	return pstTab;
}

int sht_is_empty(LPSHTABLE pstTab)
{
	if( pstTab->iItem<=0 )
		return 1;
	else
		return 0;
}

int sht_is_full(LPSHTABLE pstTab)
{
	if( pstTab->iItem>=pstTab->iMax )
		return 1;
	else
		return 0;
}

LPSHTABLE sht_create(int iBucket, int iMax, int iUnit, size_t* piBuff)
{
	LPSHTABLE pstTab;
	size_t iAlloc;

	iAlloc	=	SHT_SIZE(iBucket, iMax, iUnit);
	if( iAlloc<= SHT_HEADSIZE() )
		return NULL;

	pstTab	=	(LPSHTABLE) calloc(1, iAlloc);

	if( !pstTab )
		return NULL;

	if( piBuff )
		*piBuff	=	iAlloc;

	if( NULL==sht_init(pstTab, iAlloc, iBucket, iMax, iUnit) )
	{
		free(pstTab);
		return NULL;
	}

	pstTab->uFlags	=	SHTF_NEEDFREE;

	return pstTab;
}

int sht_check(void* pvBuff, size_t iBuff, size_t iBucket, size_t iMax, size_t iUnit)
{
	LPSHTABLE pstTab;
	size_t iSize;

	pstTab	=	(LPSHTABLE) pvBuff;

	if( iBuff<(int)SHT_HEADSIZE() || iBucket<=0 || iMax<=0 || iUnit<=0 )
		return -1;

	iSize	=	SHT_SIZE(iBucket, iMax, iUnit);
	if( iBuff<iSize )
		return -1;

	if( pstTab->iBuff!=(intptr_t)iBuff || pstTab->iVersion!=SHT_VERSION ||
		pstTab->cbSize!=sizeof(SHTABLE) || (size_t)pstTab->iBucket!=iBucket ||
		(size_t)pstTab->iMax!=iMax || pstTab->iItem>pstTab->iMax ||
		pstTab->iHeadSize!=(int)SHT_HEADSIZE() )
		return -1;

	if( pstTab->iHeadSize>pstTab->iBucketOff ||
		pstTab->iBucketSize/sizeof(SHBUCKET)!=iBucket ||
		iBuff - pstTab->iBucketOff<(size_t)pstTab->iBucketSize ||
		pstTab->iBucketOff+pstTab->iBucketSize>pstTab->iDataOff ||
		(size_t)pstTab->iDataUnit!=iUnit ||
		(size_t)pstTab->iDataSize/pstTab->iDataUnit!=iMax ||
		iBuff - pstTab->iDataOff<(size_t)pstTab->iDataSize )
		return -1;

	return 0;
}

LPSHTABLE sht_attach(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit)
{
	if( sht_check(pvBuff, iBuff, iBucket, iMax, iUnit)<0 )
		return NULL;
	else
		return (LPSHTABLE)pvBuff;
}

void* sht_find(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, SHT_HASHCODE pfnHashCode)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

	uCode	=	pfnHashCode(pvData);
	iNode	=	sht_find_i(pstTab, pvData, pfnCmp, uCode);

	if( iNode<0 )
		return NULL;
	pstItem = SHT_GET_ITEM(pstTab, iNode);
	return SHT_ITEM2DATA(pstTab,pstItem);
}

void* sht_insert_unique(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, SHT_HASHCODE pfnHashCode)
{
	if( sht_find(pstTab, pvData, pfnCmp, pfnHashCode) )
		return NULL;

	return sht_insert_multi(pstTab, pvData, pfnHashCode);
}

void* sht_insert_multi(LPSHTABLE pstTab, const void* pvData, SHT_HASHCODE pfnHashCode)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

	iNode	=	sht_alloc_node_i(pstTab);

	if( iNode<0 )
		return NULL;

	uCode	=	pfnHashCode(pvData);

	sht_insert_i(pstTab, iNode, uCode);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return SHT_ITEM2DATA(pstTab,pstItem);
}

void *sht_insert_force(LPSHTABLE pstTab, unsigned int uCode)
{
	int iNode;
	LPSHITEM pstItem;

	iNode	=	sht_alloc_node_i(pstTab);

	if( iNode<0 )
		return NULL;

	sht_insert_i(pstTab, iNode, uCode);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return SHT_ITEM2DATA(pstTab,pstItem);
}

void* sht_remove(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, SHT_HASHCODE pfnHashCode)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

	uCode	=	pfnHashCode(pvData);

	iNode	=	sht_find_i(pstTab, pvData, pfnCmp, uCode);
	if( iNode < 0 )
		return NULL;

	sht_remove_i(pstTab, iNode, uCode);
	sht_free_node_i(pstTab, iNode);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return SHT_ITEM2DATA(pstTab,pstItem);
}

void* sht_remove_by_pos(LPSHTABLE pstTab, int iPos)
{
	LPSHITEM pstItem;

	if( iPos<0 || iPos>=pstTab->iMax )
		return NULL;

	pstItem	= SHT_GET_ITEM(pstTab, iPos);

	sht_remove_i(pstTab, iPos, pstItem->uCode);
	sht_free_node_i(pstTab, iPos);

	return SHT_ITEM2DATA(pstTab,pstItem);
}

int sht_remove_by_dataptr(LPSHTABLE pstTab,void *pvData)
{
	int iPos;
	void *pvRes;

    if (((size_t)pvData) < ((size_t)pstTab + pstTab->iDataOff))
    {
        return -1;
    }

	iPos = ((size_t)(pvData)-(((size_t)pstTab + pstTab->iDataOff)))/pstTab->iDataUnit;
	pvRes = sht_remove_by_pos(pstTab,iPos);

	if(pvData == pvRes)
	{
		return 0;
	}

	return -1;
}

size_t sht_get_max_items(LPSHTABLE pstTab)
{
	if(!pstTab)
	{
		return 0;
	}

	return pstTab->iMax;
}

size_t sht_get_used_items(LPSHTABLE pstTab)
{
	if(!pstTab)
	{
		return 0;
	}

	return pstTab->iItem;
}

void* sht_pos(LPSHTABLE pstTab, int iPos, int* pfValid)
{
	LPSHITEM pstItem;

	if( iPos<0 || iPos>=pstTab->iMax )
		return NULL;

	pstItem	=	SHT_GET_ITEM(pstTab, iPos);

	if( pfValid )
		*pfValid	=	pstItem->fValid;

	return SHT_ITEM2DATA(pstTab,pstItem);
}

int sht_destroy(LPSHTABLE* ppstTab)
{
	if( !ppstTab )
		return 0;

	if( !*ppstTab )
		return 0;

	if( (*ppstTab)->uFlags & SHTF_NEEDFREE )
	{
		free(*ppstTab);
		*ppstTab	=	NULL;
	}

	return 0;
}


int sht_rebuild(LPSHTABLE pstTab)
{
	LPSHITEM pstItem;
	LPSHBUCKET pstBucket;
	int i;

	sht_make_free_chain_i(pstTab);

	for(i=0; i<pstTab->iBucket; i++)
	{
		pstBucket	=	SHT_GET_BUCKET(pstTab, i);
		pstBucket->iCount	=	0;
		pstBucket->iHead	=	-1;
	}

	pstTab->iItem	=	0;

	for(i=0; i<pstTab->iMax; i++)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);

		if( !pstItem->fValid )
			continue;

		sht_insert_i(pstTab, i, pstItem->uCode);

		pstTab->iItem++;
	}

	return 0;
}


int sht_dump_all(LPSHTABLE pstTab, FILE* fp, SHT_PRINT pfnPrint)
{
	int i;
	LPSHITEM pstItem;

	fprintf(fp, "--------------Dump hash table(ALL) start----------------\n");
	fprintf(fp, "Bucket:%" PRIdPTR ", Buff:%"PRIdPTR"\n", pstTab->iBucket, pstTab->iBuff);
	fprintf(fp,"Max Items:%"PRIdPTR" Total Items:%"PRIdPTR"\n", pstTab->iMax, pstTab->iItem);

	for(i=0; i<pstTab->iMax; i++)
	{

		pstItem	=	SHT_GET_ITEM(pstTab,i);

		if( pstItem->fValid )
		{
			fprintf(fp, "\t(VALID) Item pos=%d code=%08x ", i, pstItem->uCode);
			if(pfnPrint)
			pfnPrint(fp,  SHT_ITEM2DATA(pstTab,pstItem));
			fprintf(fp, "\n");
		}
		else
		{
			fprintf(fp, "\t(FREE) Item pos=%d code=%08x \n", i, pstItem->uCode);
		}
	}

	fprintf(fp, "--------------Dump hash table(ALL) end-------------------\n");

	return 0;
}

int sht_dump_valid(LPSHTABLE pstTab, FILE* fp, SHT_PRINT pfnPrint)
{
	int i;
	int n;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;

	fprintf(fp, "--------------Dump hash table(VALID) start---------------\n");
	fprintf(fp, "Bucket:%"PRIdPTR", Buff:%"PRIdPTR"\n", pstTab->iBucket, pstTab->iBuff);
	fprintf(fp,"Max Items:%"PRIdPTR" Total Items:%"PRIdPTR"\n", pstTab->iMax, pstTab->iItem);

	for(i=0; i<pstTab->iBucket; i++)
	{
		pstBucket	=	SHT_GET_BUCKET(pstTab, i);

		//fprintf(fp, "Bucket %4d Items:%d\n", i, pstBucket->iCount );

		if( pstBucket->iCount<=0 )
			continue;

		if( pstBucket->iHead<0 || pstBucket->iHead>pstTab->iMax )
		{
			fprintf(fp, "\t[ERROR] Head Pos=%"PRIdPTR"\n", pstBucket->iHead);
			continue;
		}

		pstItem	=	SHT_GET_ITEM(pstTab, pstBucket->iHead);
		n		=	0;

		do
		{
			fprintf(fp, "\t(VALID) Item pos=%d code=%08x ", i, pstItem->uCode);
			if(pfnPrint)
			pfnPrint(fp, SHT_ITEM2DATA(pstTab,pstItem));
			fprintf(fp, "\n");

			if( pstItem->iNext<0 )
				break;

			pstItem = SHT_GET_ITEM(pstTab, pstItem->iNext);
			n++;
		}
		while( n<pstBucket->iCount );
	}

	fprintf(fp, "--------------Dump hash table(VALID) end-----------------\n");

	return 0;
}

void* sht_remove_by_addr(LPSHTABLE pstTab, LPSHITEM pstItem)
{
	LPSHITEM pstFirstItem;
	int iPos;

	if (NULL == pstItem)
	{
		return NULL;
	}

	pstFirstItem = SHT_GET_ITEM(pstTab, 0);
	//changed by vinsonzuo 20090903
	//iPos = pstItem - pstFirstItem;
	iPos = ((size_t)(pstItem) - (size_t)(pstFirstItem))/sizeof(SHITEM);

	return sht_remove_by_pos(pstTab, iPos);
}


