#ifndef __SHTABLE_I_H_
#define __SHTABLE_I_H_
#include "comm/tlist.h"
#include "../lib_src/comm/tlist_i.h"

#define SHT_MIN_HEAD			(intptr_t)0x100
#define SHT_BUCKET_ALIGN		(intptr_t)0x100
#define SHT_MIN_ALIGN			(intptr_t)0x08
#define SHT_VERSION				0x0101

#define SHTF_NEEDFREE			0x01

struct tagSHitem
{
	intptr_t  iPrev;
	intptr_t  iNext;
	uint32_t uCode;
	uint32_t  fValid;
};

typedef struct tagSHitem		SHITEM;
typedef struct tagSHitem		*LPSHITEM;

struct tagSHbucket
{
	intptr_t iCount;
	intptr_t iHead;
};

typedef struct tagSHbucket		SHBUCKET;
typedef struct tagSHbucket		*LPSHBUCKET;

struct tagSHtable
{
	size_t cbSize;		/* the size of this struct. */
	uint32_t uFlags;		/* some flags. */
	uint32_t iVersion;				/* version number. */
	intptr_t iBuff;					/* the size of the buff. */

	intptr_t iBucket;				/* bucket number used. */
	intptr_t iMax;					/* maximum items can store. */
	intptr_t iItem;					/* current item number. */
	intptr_t iHeadSize;

	intptr_t iBucketOff;
	intptr_t iBucketSize;

	// The offset of data head.
	intptr_t iDataHeadOff;
	intptr_t iDataHeadSize;
	
	intptr_t iDataOff;
	intptr_t iDataSize;
	intptr_t iDataUnit;

	intptr_t iFreeHead;
	uint32_t iRes;					/* reserved. */
};
typedef struct tagSHtable			SHTABLE;

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

// Aline the page
#define SHT_PAGE_ALIAN(addr)  ( (( (addr) + 4096 - 1) /4096 + 1)*4096 )

// Min 256 byte
#define SHT_HEADSIZE()					( SHT_MIN_HEAD < sizeof(SHTABLE) ? sizeof(SHTABLE) : SHT_MIN_HEAD )

// Return the size of bucket.
#define SHT_BUCKET_SIZE(buck)			( (buck) * sizeof(SHBUCKET) )


#define SHT_DATAHEAD_SIZE(max) 				((max) * sizeof(SHITEM))

#define SHT_REAL_DATASIZE(max, unit)			((max) * (unit))

#define SHT_ROUND(size)					( ( (size) + 8192 - 1)&(~(8192-1)))

//This
#define SHT_SIZE(buck, max, unit)		SHT_ROUND( SHT_HEADSIZE() + SHT_BUCKET_SIZE(buck) + SHT_DATAHEAD_SIZE(max) + SHT_REAL_DATASIZE(max,unit))

#define SHT_GET_BUCKET(pstTab, i)		( (LPSHBUCKET) ( ((size_t)(pstTab)) + (pstTab)->iBucketOff + (i)*sizeof(SHBUCKET) ) )

#define SHT_GET_ITEM(pstTab, i)			( (LPSHITEM) ( ((size_t)(pstTab)) + pstTab->iDataHeadOff + (i)*sizeof(SHITEM) ) )

#define SHT_GET_DATA(pstTab,i)	((void *)( (size_t)(pstTab) + (pstTab)->iDataOff+(i) * (pstTab)->iDataUnit) )

#define SHT_ITEM_IDX(pstTab,pstItem)    (((size_t)(pstItem) - ((pstTab)->iDataHeadOff + (size_t)(pstTab))) / sizeof(SHITEM))
#define SHT_ITEM2DATA(pstTab,pstItem)	SHT_GET_DATA(pstTab,SHT_ITEM_IDX(pstTab,pstItem))

/**remove item in shtable by addr
*@param[in]	 pstTab  shtable 指针
*@param[in]  pstItem  item 在shtable中的地址指针

*@retval  not NULL  成功，shtable 内存放data的地址指针
          NULL      失败
       
*/
void* sht_remove_by_addr(LPSHTABLE pstTab, LPSHITEM pstItem);
#endif


