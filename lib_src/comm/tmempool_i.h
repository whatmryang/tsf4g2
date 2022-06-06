#ifndef TMEMPOOL_I_H__
#define TMEMPOOL_I_H__

#include "comm/tlist.h"
#include "commbase_i.h"

#define TMEMPOOL_MAGIC			0x4324
#define TMEMPOOL_BUILD			2

#define TMEMPOOL_FIFO			0x00
#define TMEMPOOL_LIFO			0x01

#define TMEMPOOL_PAGE_SIZE      8192    //�ڴ�ҳ�Ĵ�С
#define TMEMPOOL_USER_DATA_ALIGN    8   //�û����ݶ����ֽ���
struct tagTMemBlock
{
	int fValid;
	int iIdx;
	int iPrev;
	int iNext;
	int iRes;
};

typedef struct tagTMemBlock TMEMBLOCK;
typedef struct tagTMemBlock *LPTMEMBLOCK;

#define TMEMBLOCK_IS_VALID(p)	((p)->fValid)

struct tagTMemPool
{
	int iMagic; //���ڼ��mempool�ڴ�
	int iBuild; //���ڼ��mempool�ڴ�
	int iMax;//mempool������

	int iFreeHead; //��������ͷָ��
	int iFreeTail;//��������βָ��
	int iUsedHead; //ʹ������ͷָ��
	size_t iUsed;//�ж���memblock�Ѿ���ʹ��

	int iLastIdx;//�������memblock����
	int iInited;//mempool�Ƿ񾭹��˳�ʼ��
	int iIsCalloc;//mempool�Ƿ������ڴ�
	int iMethod;//TMEMPOOL_FIFO  memblock��������Ƚ��ȳ�,��Ĭ�ϵķ���; TMEMPOOL_LIFO

	volatile int iStart;//�ڲ�ʹ�ã���iEnd��ϼ���tmempool_alloc��tmempool_free������ԭ����
	volatile int iEnd;

	size_t iUnit;//Ӧ�����ݵĴ�С
	size_t iRealUnit;//��mempool�п���Բ��������ռ���ڴ�Ĵ�С
	size_t iSize;//mempoolռ���ڴ�Ĵ�С
	size_t iRealSize;//��iSize��ͬ, Ŀǰû��ʹ��


	size_t iBlockOff;
	size_t iBlockSize;

	size_t iDataOff;
	size_t iDataSize;

	intptr_t iLastCursor;
	intptr_t iIteratorLastCursor;
	int iIsIteratorReset;

	int iUseProtect;
	size_t iProtectLen;

	char szRes[64];//Ԥ��

	//������������������������
	TMEMBLOCK szBlocks[1];//��������ʼλ��
};

#define TMEMPOOL_ROUND(unit, round)	( ( ( (intptr_t)(unit) + (intptr_t)(round) - 1 ) / (intptr_t)(round) ) * (intptr_t)(round) )

#define TMEMPOOL_HEAD_SIZE   (offsetof(TMEMPOOL, szBlocks))

#define TMEMPOOL_BLOCK_SIZE(iMax) (iMax * sizeof(TMEMBLOCK))

#define TMEMPOOL_KERNEL_DATA_BASE_SIZE(iMax)   (TMEMPOOL_HEAD_SIZE + TMEMPOOL_BLOCK_SIZE(iMax) )

/*mempool ͷ����Ϣ���������������Ĵ�С����������С�ᰴ���ڴ�ҳ��С���룬
 �Ա�ʵ�ְ�ҳ����
 */
#define TMEMPOOL_KERNEL_DATA_SIZE(iMax)   TMEMPOOL_ROUND(TMEMPOOL_KERNEL_DATA_BASE_SIZE(iMax),TMEMPOOL_PAGE_SIZE)

#define TMEMPOOL_USER_DATA_UNIT_SIZE(iSize)  TMEMPOOL_ROUND(iSize,TMEMPOOL_USER_DATA_ALIGN)

#define TMEMPOOL_DATA_SIZE(iMax,iSize) (iMax * TMEMPOOL_USER_DATA_UNIT_SIZE(iSize))

//mempool���ܴ�С
#define TMEMPOOL_CALC(iMax,iUnit)  (TMEMPOOL_KERNEL_DATA_SIZE(iMax) + TMEMPOOL_DATA_SIZE(iMax,iUnit))

#define TMEMPOOL_GET_PTR(pstPool, idx)	((LPTMEMBLOCK)((pstPool)->szBlocks + ((size_t)(idx) % (pstPool)->iMax) ))

#define TMEMPOOL_GET_DATA_I(pstPool,idx)  ((void *)((size_t)(pstPool)+(pstPool)->iDataOff + (pstPool)->iRealUnit * ((size_t)(idx) % (pstPool)->iMax )))

#define TMEMPOOL_GET_DATA(pstPool,pstBlock)  TMEMPOOL_GET_DATA_I(pstPool,(pstBlock)->iIdx)

#define TMEMPOOL_GET_CAP(pool)		((pool)->iMax)
#define TMEMPOOL_GET_BLOCK_IDX(blk)	((blk)->iIdx)

#define TMEMPOOL_IS_INVALIDPTR(pstPool,pvData) (((size_t)(pvData) - (size_t)(pstPool)- (size_t)(pstPool)->iDataOff)%(pstPool)->iRealUnit)

#define TMEMPOOL_PTR2POS(pstPool,pvData) (((size_t)(pvData) - (size_t)(pstPool)- (size_t)(pstPool)->iDataOff)/(pstPool)->iRealUnit)

void tmempool_init_head_i(TMEMPOOL* pstPool, int iMax, int iUnit, size_t iSize);
void tmempool_init_body_i(TMEMPOOL* pstPool);
int tmempool_check_head_i(TMEMPOOL* pstPool, int iMax, int iUnit, size_t iSize);
int tmempool_check_chain_i(TMEMPOOL *pstPool);

#endif

