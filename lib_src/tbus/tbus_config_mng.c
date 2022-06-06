#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "taa/tagentapi.h"
#include "tbus/tbus_error.h"
#include "tbus_config_mng.h"
#include "tbus_log.h"
#include "tbus_kernel.h"


#if defined (_WIN32) || defined (_WIN64)
#include "tbus_mmap_for_win32.h"
#define tbus_stat _stat
#else
#define tbus_stat stat
#endif



/**  check whether the string is decimal-digit string
*@return  nonzero if the string is decimal-digit,otherwise return zero
*/
static int tbus_is_decimal_string(IN const char *a_pszShmkey);


int tbus_is_channel_in_use(IN LPTBUSSHMCHANNELCNF a_pstShmChl,
        LPTBUSSHMGCIMHEAD a_pstHead, INOUT int* a_iIsInUse);


/**Try to attach share memory
*@param[in,out] ppShm To get the address of share memory
*@param[in] a_tKey	Key of share memory
*@param[in] a_iShmSize size of share memory
*@param[in] a_iSHmFlag control flags to attach share memory
*@param[in] a_iTimeout If the share memory do not exist, try to attach again, until time out
*@retval	0 success
*@retval <0 failed
*/
static int tbus_attach_shm(INOUT void **ppShm, IN key_t a_tKey, IN unsigned int *a_piShmSize,
					   IN int a_iSHmFlag, IN int a_iTimeout);

static int tbus_create_shm(INOUT void **ppShm, IN key_t a_tKey, IN unsigned int a_iShmSize,
						  IN int a_iSHmFlag, OUT int *a_piCreate);

static int tbus_create_file(const char *a_pszFile);

static int tbus_check_gcim_conf_i(IN LPTBUSGCIM a_pstGCIM, LPTBUSADDRTEMPLET a_pstAddrTemplet);

static int tbus_is_channel_configured_i(IN LPTBUSGCIM a_pstGCIM, LPTBUSSHMCHANNELCNF pstShmChl);


int tbus_check_gcim_channels(IN LPTBUSSHMGCIM a_pstShmGCIM, IN LPTBUSGCIM a_pstGCIM);



static int tbus_find_channel_in_gcim_i(IN LPTBUSSHMGCIM a_pstShmGCIM,  LPCHANNELCNF pstChl);


static int tbus_dump_gcim_i(LPTBUSSHMGCIM a_pstGCIM, const char* a_pszShmkey);
static void tbus_dump_gcim_head_i(LPTBUSSHMGCIMHEAD a_pstHead, FILE *a_fp);
static void tbus_dump_gcim_channels_i(LPTBUSSHMCHANNELCNF a_pstChannels, int a_iNum, FILE *a_fp);


////////////////////////////////////////////////////////////////////////////////////

int tbus_get_gcimshm(INOUT LPTBUSSHMGCIM *ppstGCIM, IN const char *a_pszShmkey,
					 IN int a_iBussId, IN unsigned int a_iShmSize,
                     IN int a_iTimeout, OUT HANDLE *a_phShmHandle)
{
	int iRet = 0;
	key_t tShmkey = -1;
	int iFlags = 0;
	LPTBUSSHMGCIM pstGCIM = NULL;
	unsigned int iSize;
	LPTBUSSHMGCIMHEAD pstHead;
#if defined (_WIN32) || defined (_WIN64)
	int iCreate = 0;
#endif

	assert(NULL != ppstGCIM);

    *ppstGCIM = NULL;

	if ((NULL == a_pszShmkey) || ('\0' == a_pszShmkey))
	{

		a_pszShmkey = TBUS_DEFAULT_GCIM_KEY;
		tbus_log(TLOG_PRIORITY_DEBUG,"null shmkey, so use the default shmkey %s", TBUS_DEFAULT_GCIM_KEY);
	}
	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmkey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s",
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}


	iSize = a_iShmSize;
#if !defined (_WIN32) && !defined (_WIN64)
	iFlags = TBUS_GCIM_SHM_ACCESS;
	iRet = tbus_attach_shm((void **)(void*)&pstGCIM, tShmkey, &iSize, iFlags, a_iTimeout);
    if (NULL != a_phShmHandle)
    {
        *a_phShmHandle = 0;
    }
#else
	iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_EXCL;
	iRet = tbus_open_mmap_by_key((void **)(void*)&pstGCIM, tShmkey, &iSize, iFlags, &iCreate, a_phShmHandle);
#endif
	if ( 0 > iRet )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed attch GCIM by  generate shmkey, by shmkey<%s> and bussId<%d>, for %s",
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}


    /* share memory avaiable now */
    if (!TBUS_ERR_IS_ERROR(iRet))
    {
        pstHead = &pstGCIM->stHead;
        if (iSize < (int)sizeof(TBUSSHMGCIMHEAD))
        {
            tbus_log(TLOG_PRIORITY_FATAL,"share memory size %d is less than the size of TUBSSHMHEADER %"PRIdPTR"\n",
                     iSize, sizeof(TBUSSHMGCIMHEAD)) ;
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
        }
    }

    if (!TBUS_ERR_IS_ERROR(iRet))
    {
        if (tbus_check_shmgcimheader_stamp(pstHead) != 0)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"failed to check GCIM  shm stamp, shmkey %d\n", tShmkey);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
        }
    }

    if (!TBUS_ERR_IS_ERROR(iRet))
    {
        if ((key_t)pstHead->dwShmKey != tShmkey)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"share memory key %d not match to the settings key:%d\n",
                     pstHead->dwShmKey, tShmkey);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
        }
    }

    if (!TBUS_ERR_IS_ERROR(iRet))
    {
        if (iSize != (int)pstHead->dwShmSize)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"share memory size not match, real size %d, but need size %d",
                     iSize, pstHead->dwShmSize);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
        }/*if ( 0 != iRet )*/
    }

    if (!TBUS_ERR_IS_ERROR(iRet))
    {
        *ppstGCIM = pstGCIM;
    } else
    {
        tbus_dump_gcim_i(pstGCIM, a_pszShmkey);
    }

    return iRet ;
}

int tbus_dump_gcim_i(LPTBUSSHMGCIM a_pstGCIM, const char* a_pszShmkey)
{
    int iRet = TBUS_SUCCESS;
    FILE *fpDumpFile = NULL;
    char szGCIMDumpFile[256] = {0};

    assert(NULL != a_pstGCIM);
    assert(NULL != a_pszShmkey);

    /* dump as binary */
    {
        sprintf(szGCIMDumpFile, "tbus_gcim_dump_%s.binary", a_pszShmkey);
        szGCIMDumpFile[sizeof(szGCIMDumpFile)-1] = '\0';
        fpDumpFile = fopen(szGCIMDumpFile, "wb");
        if (NULL != fpDumpFile)
        {
            fwrite(a_pstGCIM, sizeof(*a_pstGCIM), 1, fpDumpFile);
            fclose(fpDumpFile);
        }
    }

    /* dump as txt */
    {
        sprintf(szGCIMDumpFile, "tbus_gcim_dump_%s.txt", a_pszShmkey);
        szGCIMDumpFile[sizeof(szGCIMDumpFile)-1] = '\0';
        fpDumpFile = fopen(szGCIMDumpFile, "wb");
        if (NULL != fpDumpFile)
        {
            int iChannelNum = (int)a_pstGCIM->stHead.dwUsedCnt;

            tbus_dump_gcim_head_i(&a_pstGCIM->stHead, fpDumpFile);

            if (TBUS_MAX_CHANNEL_NUM_PREHOST < iChannelNum)
            {
                iChannelNum = TBUS_MAX_CHANNEL_NUM_PREHOST;
            }
            tbus_dump_gcim_channels_i(a_pstGCIM->astChannels, iChannelNum, fpDumpFile);
            fclose(fpDumpFile);
        }
    }

	return iRet ;
}

void tbus_dump_gcim_head_i(LPTBUSSHMGCIMHEAD a_pstHead, FILE *a_fp)
{
    int i = 0;

    assert(NULL != a_pstHead);
    assert(NULL != a_fp);

    fprintf(a_fp, "[stHead]:\n");
    fprintf(a_fp, "\t dwVersion   <hex:0x%08x, dec:%u>\n", a_pstHead->dwVersion, a_pstHead->dwVersion);
    fprintf(a_fp, "\t dwShmKey    <hex:0x%08x, dec:%u>\n", a_pstHead->dwShmKey, a_pstHead->dwShmKey);
    fprintf(a_fp, "\t dwShmSize   <hex:0x%08x, dec:%u>\n", a_pstHead->dwShmSize, a_pstHead->dwShmSize);
    fprintf(a_fp, "\t dwMaxCnt    <hex:0x%08x, dec:%u>\n", a_pstHead->dwMaxCnt, a_pstHead->dwMaxCnt);
    fprintf(a_fp, "\t dwUsedCnt   <hex:0x%08x, dec:%u>\n", a_pstHead->dwUsedCnt, a_pstHead->dwUsedCnt);
    fprintf(a_fp, "\t dwCreatTime: %s", ctime(&a_pstHead->dwCreateTime));
    fprintf(a_fp, "\t dwLastStamp: %s", ctime(&a_pstHead->dwLastStamp));
    fprintf(a_fp, "\t dwBusiID    <hex:0x%08x, dec:%u>\n", a_pstHead->dwBusiID, a_pstHead->dwBusiID);
    fprintf(a_fp, "\t dwCRC       <hex:0x%08x, dec:%u>\n", a_pstHead->dwCRC, a_pstHead->dwCRC);
    fprintf(a_fp, "\t iAlign      <hex:0x%08x, dec:%d>\n", a_pstHead->iAlign, a_pstHead->iAlign);
    fprintf(a_fp, "\t iAlignLevel <hex:0x%08x, dec:%d>\n", a_pstHead->iAlignLevel, a_pstHead->iAlignLevel);

    fprintf(a_fp, "\t stAddrTemplet:\n");
    fprintf(a_fp, "\t\t iSegNum <hex:0x%08x, dec:%d>\n",
            a_pstHead->stAddrTemplet.iSegNum,
            a_pstHead->stAddrTemplet.iSegNum);
    fprintf(a_fp, "\t\t Seg:\n");

    for (i = 0; i < a_pstHead->stAddrTemplet.iSegNum; i++)
    {
        fprintf(a_fp, "\t\t\t Seg[%d]: bStartBit<%d>, bBitNum<%d>, dwMaxVal<hex:0x%08x, dec:%u>\n", i,
                (int)a_pstHead->stAddrTemplet.astSeg[i].bStartBit,
                (int)a_pstHead->stAddrTemplet.astSeg[i].bBitNum,
                a_pstHead->stAddrTemplet.astSeg[i].dwMaxVal,
                a_pstHead->stAddrTemplet.astSeg[i].dwMaxVal);
    }

    fprintf(a_fp, "\t aiReserved:\n");
    for (i = 0; i < TBUS_SHM_GCIM_HEAD_RESERV_NUM; i++)
    {
        fprintf(a_fp, "\t\t aiReserved[%d] <hex:0x%08x, dec:%d>\n", i,
                a_pstHead->aiReserved[i], a_pstHead->aiReserved[i]);
    }
};

void tbus_dump_gcim_channels_i(LPTBUSSHMCHANNELCNF a_pstChannels, int a_iNum, FILE *a_fp)
{
    int i = 0;
    int j = 0;

    assert(NULL != a_fp);
    assert(NULL != a_pstChannels);

    fprintf(a_fp, "\n[astChannels]:\n");
    for (i = 0; i < a_iNum; i++)
    {
        fprintf(a_fp, "\t astChannels[%d]:\n", i);

        fprintf(a_fp, "\t\t dwFlag     <hex:0x%08x, dec:%u>\n",
                a_pstChannels[i].dwFlag, a_pstChannels[i].dwFlag);
        fprintf(a_fp, "\t\t dwPriority <hex:0x%08x, dec:%u>\n",
                a_pstChannels[i].dwPriority, a_pstChannels[i].dwPriority);
        fprintf(a_fp, "\t\t iShmID     <hex:0x%08x, dec:%d>\n",
                (int)a_pstChannels[i].iShmID, (int)a_pstChannels[i].iShmID);

        fprintf(a_fp, "\t\t astAddrs:\n");

        fprintf(a_fp, "\t\t\t astAddrs[0] <hex:0x%08x, dec:%d>\n",
                (int)a_pstChannels[i].astAddrs[0], (int)a_pstChannels[i].astAddrs[0]);
        fprintf(a_fp, "\t\t\t astAddrs[1] <hex:0x%08x, dec:%d>\n",
                (int)a_pstChannels[i].astAddrs[1], (int)a_pstChannels[i].astAddrs[1]);

        fprintf(a_fp, "\t\t dwRecvSize <hex:0x%08x, dec:%u>\n",
                a_pstChannels[i].dwRecvSize, a_pstChannels[i].dwRecvSize);
        fprintf(a_fp, "\t\t dwSendSize <hex:0x%08x, dec:%u>\n",
                a_pstChannels[i].dwSendSize, a_pstChannels[i].dwSendSize);
        fprintf(a_fp, "\t\t dwCreateTime: %s", ctime(&a_pstChannels[i].dwCTime));
        fprintf(a_fp, "\t\t dwInvaldTime: %s", ctime(&a_pstChannels[i].dwInvalidTime));

        fprintf(a_fp, "\t\t aiReserved:\n");
        for (j = 0; j < TBUS_CHANNEL_RESERVER_NUM; j++)
        {
            fprintf(a_fp, "\t\t\t aiReserved[%d] <hex:0x%08x, dec:%d>\n", j,
                    a_pstChannels[i].aiReserved[j], a_pstChannels[i].aiReserved[j]);
        }

        fprintf(a_fp, "\n");
    }
}

int tbus_create_gcimshm(INOUT LPTBUSSHMGCIM *a_ppstGCIM, IN const char *a_pszShmkey,
						IN int a_iBussId, IN unsigned int a_iShmSize, OUT HANDLE *a_phShmHandle)
{

	int	iRet = TBUS_SUCCESS ;
	int iCreate = 0;
	LPTBUSSHMGCIM pstGCIM = NULL;
	key_t tShmkey = -1;
	int iFlags;
    unsigned int dwShmExpectSize = a_iShmSize;
	assert(NULL != a_ppstGCIM);


	if ((NULL == a_pszShmkey) || ('\0' == a_pszShmkey))
	{
		a_pszShmkey = TBUS_DEFAULT_GCIM_KEY;
		tbus_log(TLOG_PRIORITY_DEBUG,"null shmkey, so use the default shmkey %s", TBUS_DEFAULT_GCIM_KEY);
	}
	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmkey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s",
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}

#if !defined (_WIN32) && !defined (_WIN64)
	iFlags = TBUS_GCIM_SHM_ACCESS |IPC_CREAT|IPC_EXCL ;
	iRet = tbus_create_shm((void **)(void*)&pstGCIM, tShmkey, a_iShmSize, iFlags, &iCreate);
    if (NULL != a_phShmHandle)
    {
        *a_phShmHandle = 0;
    }
#else
	iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_CREATE;
	iRet = tbus_open_mmap_by_key((void **)(void*)&pstGCIM, tShmkey, &a_iShmSize, iFlags, &iCreate, a_phShmHandle);
#endif
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to create GCIM share memory, iRet :%x", iRet);
		return iRet;
	}

	if (iCreate)
	{
		LPTBUSSHMGCIMHEAD pstHead = &pstGCIM->stHead;
		pthread_rwlockattr_t rwlattr;

		pthread_rwlockattr_init(&rwlattr);
		pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(&pstHead->stRWLock, &rwlattr);

		tbus_wrlock(&pstHead->stRWLock);
        pstHead->dwBuild = TBUS_GCIM_BUILD;
		pstHead->dwBusiID = a_iBussId;
		pstHead->dwCreateTime = (unsigned int)time(NULL);
		pstHead->dwMaxCnt = (dwShmExpectSize - offsetof(TBUSSHMGCIM,astChannels)) / sizeof(TBUSSHMCHANNELCNF);
		pstHead->dwShmKey = tShmkey;
		pstHead->dwShmSize = a_iShmSize;
		pstHead->dwUsedCnt = 0;
		pstHead->dwVersion = 0;
		pstHead->iAlign = TBUS_DEFAULT_CHANNEL_DATA_ALIGN;
		TBUS_CALC_ALIGN_LEVEL(pstHead->iAlignLevel, pstHead->iAlign);
		memset(&pstHead->stAddrTemplet, 0, sizeof(pstHead->stAddrTemplet));
		memset(&pstHead->aiReserved, 0, sizeof(pstHead->aiReserved));
		tbus_set_shmgcimheader_stamp(pstHead);
		tbus_unlock(&pstHead->stRWLock);
	}

	*a_ppstGCIM = pstGCIM;

	return iRet ;
}

/** 根据字符串信息，生成
*@param[in] a_pszShmkey	保存生产GCIM共享内存key的信息串，此信息串中的信息只能为十进制数字
*	串或为一个文件的路径（当此文件不存在时，tbus系统会尝试生成此文件），根据此信息串生成GCIM共享内存key的算法是:
*	- 十进制数字串	则将此数字串转换整数，此整数作为GCIM的共享内存的key
*	- 文件路径	将此文件路径和a_iBussId作为参数，调用ftok接口生产GCIM共享内存的key
*/
int tbus_gen_shmkey(OUT key_t *ptShmkey, IN const char *a_pszShmkey,
						   IN int a_iBussId)
{
	int iRet = TBUS_SUCCESS;
	key_t tkey;

	assert(NULL != ptShmkey);
	assert(NULL != a_pszShmkey);

	if ('\0' == a_pszShmkey)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"a_pszShmkey %s is empty string, cannot generate shm key", a_pszShmkey);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GEN_SHMKEY_FAILED);
	}

	if (tbus_is_decimal_string(a_pszShmkey))
	{
		tkey = (key_t)atoi(a_pszShmkey);
		if (0 > tkey)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"invalid shmkey<%s>, decimal value trasformed by shmkey string is less than zero",
				a_pszShmkey);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GEN_SHMKEY_FAILED);
		}else
		{
			tbus_log(TLOG_PRIORITY_DEBUG,"gen shmkey<%d> by  info shmkey<%s> and bussid<%d>",
				tkey, a_pszShmkey, a_iBussId);
		}/*if (0 > tkey)*/
	}else
	{
		TFSTAT stFileStat;

		iRet = tbus_stat(a_pszShmkey, &stFileStat);
		if ((0 != iRet) && (errno == ENOENT))
		{
			/*file is not exist, so create */
			iRet = tbus_create_file(a_pszShmkey);
			if (0 != iRet)
			{
				tbus_log(TLOG_PRIORITY_ERROR,"failed to create file<%s>",	a_pszShmkey);
				return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GEN_SHMKEY_FAILED);
			}
		}/*if ((0 != iRet) && (errno == ENOENT))*/

		tkey = ftok(a_pszShmkey, a_iBussId);
		if (0 > tkey)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"ftok failed by shmkey<%s> and bussid<%d>, for %s",
				a_pszShmkey, a_iBussId, strerror(errno));
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GEN_SHMKEY_FAILED);
		}else
		{
			tbus_log(TLOG_PRIORITY_DEBUG,"gen shmkey<%d> by  info shmkey<%s> and bussid<%d>",
				tkey, a_pszShmkey, a_iBussId);
		}/*if (0 > tkey)*/
	}/*if (tbus_is_decimal_string(a_pszShmkey))*/

	*ptShmkey = tkey;
	return iRet;
}

int tbus_create_file(const char *a_pszFile)
{
	int iRet = TBUS_SUCCESS;
	FILE *fp;

	assert(NULL != a_pszFile);

	iRet = tos_mkdir_by_path(a_pszFile);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to mkdir by %s, %s",	a_pszFile, strerror(errno));
		return iRet;
	}

	fp = fopen(a_pszFile, "w");
	if (NULL == fp)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to open file %s to write, for %s",	a_pszFile, strerror(errno));
		return iRet;
	}
	fclose(fp);

	return iRet;
}

/**  check whether the string is decimal-digit string
*@return  nonzero if the string is decimal-digit,otherwise return zero
*/
int tbus_is_decimal_string(IN const char *a_pszShmkey)
{
	const char *pch;

	assert(NULL != a_pszShmkey);
	for (pch = a_pszShmkey; *pch != '\0'; pch++)
	{
		if (isspace(*pch))
		{
			continue;
		}
		if (!isdigit(*pch))
		{
			return 0;
		}
	}

	return 1;
}

void tbus_set_shmgcimheader_stamp(LPTBUSSHMGCIMHEAD pstHeader)
{
	assert(NULL != pstHeader);


	pstHeader->dwLastStamp = (unsigned int)time(NULL);
	pstHeader->dwCRC = pstHeader->dwVersion ^ pstHeader->dwCreateTime ^ pstHeader->dwLastStamp;
	pstHeader->dwCRC ^= pstHeader->dwShmKey;
	pstHeader->dwCRC ^= pstHeader->dwShmSize;
	pstHeader->dwCRC ^= pstHeader->dwBusiID;
	pstHeader->dwCRC ^= pstHeader->iAlign;
}


int tbus_check_shmgcimheader_stamp(LPTBUSSHMGCIMHEAD pstHeader)
{
	unsigned int dwTempInt = 0;
	assert(NULL != pstHeader);

	dwTempInt = pstHeader->dwVersion ^ pstHeader->dwCreateTime ^ pstHeader->dwLastStamp;
	dwTempInt ^= pstHeader->dwShmKey;
	dwTempInt ^= pstHeader->dwShmSize;
	dwTempInt ^= pstHeader->dwBusiID;
	dwTempInt ^= pstHeader->dwCRC;
	dwTempInt ^= pstHeader->iAlign;

	return (int)dwTempInt;
}


void tbus_set_shmgrmheader_stamp(LPTBUSSHMGRMHEAD pstHeader)
{
	assert(NULL != pstHeader);


	pstHeader->dwLastStamp = (unsigned int)time(NULL);
	pstHeader->dwCRC = pstHeader->dwVersion ^ pstHeader->dwCreateTime ^ pstHeader->dwLastStamp;
	pstHeader->dwCRC ^= pstHeader->dwShmKey;
	pstHeader->dwCRC ^= pstHeader->dwShmSize;
	pstHeader->dwCRC ^= pstHeader->dwBusiID;

}

int tbus_check_shmgrmheader_stamp(LPTBUSSHMGRMHEAD pstHeader)
{
	unsigned int dwTempInt = 0;
	assert(NULL != pstHeader);

	dwTempInt = pstHeader->dwVersion ^ pstHeader->dwCreateTime ^ pstHeader->dwLastStamp;
	dwTempInt ^= pstHeader->dwShmKey;
	dwTempInt ^= pstHeader->dwShmSize;
	dwTempInt ^= pstHeader->dwBusiID;
	dwTempInt ^= pstHeader->dwCRC;


	return (int)dwTempInt;
}

int tbus_attach_shm(INOUT void **a_ppShm, IN key_t a_tKey, IN unsigned int *a_piShmSize,
						   IN int a_iSHmFlag, IN int a_iTimeout)
{
	HANDLE iShmID = (HANDLE)-1;
	int	iRet = TBUS_SUCCESS ;
	int iTimeCount;
	struct shmid_ds stShmStat ;

	assert(NULL != a_ppShm);
	assert(!(a_iSHmFlag & IPC_CREAT));

	iTimeCount = 0;
	while (1)
	{
		iShmID = shmget ( a_tKey, *a_piShmSize, a_iSHmFlag) ;
		if ( (HANDLE)-1 != iShmID )
		{
			break;
		}

		/*如果共享内存不存在则尝试再次挂载*/
		if (errno != ENOENT)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shmget failed by key %d, for %s\n", a_tKey, strerror(errno));
			break;
		}

		if (iTimeCount >= a_iTimeout)
		{
			break;
		}
		else
		{
			fprintf(stderr,"TBUS: shared-memory<%d> not exists, try to attach to it again\n", a_tKey);
		}
		tbus_log(TLOG_PRIORITY_DEBUG,"shmget failed by key %d, for %s, so try to attach again\n", a_tKey, strerror(errno));
		tos_usleep(TBUS_SLEEP_PRELOOP*1000);
		iTimeCount += TBUS_SLEEP_PRELOOP;

	}/*while (1)*/

	if ((HANDLE)-1 == iShmID)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmget failed by key %d, for %s, timeout value is %d\n",
			a_tKey, strerror(errno), a_iTimeout);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED);
	}

	*a_ppShm =	shmat ( iShmID, NULL, 0 ) ;
	if ((void *)-1 == *a_ppShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmat failed by id %d, for %s\n", iShmID, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
	}
	if ( 0 == shmctl( iShmID, IPC_STAT, &stShmStat))
	{
		*a_piShmSize = (int)stShmStat.shm_segsz;
	}

	return iRet;
}

int tbus_register_bussid(void)
{
	int iRet = TBUS_SUCCESS;

#if !defined (_WIN32) && !defined (_WIN64)
    if (NULL == g_stBusGlobal.pvAgentMng)
    {
        LPEXCHANGEMNG pstMng = NULL;
        iRet = agent_api_init(&pstMng);
        if (0 != iRet)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"agent_api_init failed, iRet=%d", iRet);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_REGISTER_BUSSID);
            return iRet;
        }
        g_stBusGlobal.pvAgentMng = (void*)pstMng;
    }

	iRet = agent_api_register(g_stBusGlobal.pvAgentMng, ID_APPID_BUSCONFIG, g_stBusGlobal.iBussId);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"agent_api_init failed, iRet=%d", iRet);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_REGISTER_BUSSID);
        return iRet;
	}
#else
	//TODO tagentapi在windows环境下的接口实现还需要调整
#endif
	return iRet;
}

void tbus_agent_api_destroy(void)
{
#if !defined (_WIN32) && !defined (_WIN64)
    if (NULL != g_stBusGlobal.pvAgentMng)
    {
        agent_api_destroy(g_stBusGlobal.pvAgentMng);
        g_stBusGlobal.pvAgentMng = NULL;
    }
#else
	//TODO tagentapi在windows环境下的接口实现还需要调整
#endif
}

int tbus_create_shm(INOUT void **ppShm, IN key_t a_tKey, IN unsigned int a_iShmSize,
						   IN int a_iSHmFlag, OUT int *a_piCreate)
{
	HANDLE iShmID;

	assert(NULL != ppShm);
	assert(NULL != a_piCreate);


	a_iSHmFlag &= ~IPC_CREAT;
	a_iSHmFlag &= ~IPC_EXCL;
	*a_piCreate = 0;
	iShmID = shmget(a_tKey, 0, a_iSHmFlag) ;
	if ((HANDLE)-1 !=  iShmID)
	{
		/*share memory is exist*/
		struct shmid_ds stShmStat ;
		tbus_log(TLOG_PRIORITY_DEBUG,"shmget shmid %d by  shmkey<%d>", iShmID, a_tKey);
		if ( 0 != shmctl( iShmID, IPC_STAT, &stShmStat))
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shmctl IPC_STAT failed, by shmid %d, %s", iShmID, strerror(errno));
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED);
		}
		if (a_iShmSize != (unsigned int)stShmStat.shm_segsz)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shm size %d is not equal the size %d expected, so delete it and then create",
				(int)stShmStat.shm_segsz, a_iShmSize);
			if( shmctl(iShmID, IPC_RMID, NULL) )
			{
				tbus_log(TLOG_PRIORITY_ERROR,"Remove share memory(id:%d) failed, %s.\n", iShmID, strerror(errno));
				return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED);
			}
			iShmID = (HANDLE)-1;
		}/*if (a_iShmSize != (int)stShmStat.shm_segsz)*/
	}else
	{
		tbus_log(TLOG_PRIORITY_DEBUG,"shmget failed  by shmkey<%d> size:%u flag:0x%x, %s", a_tKey,0,
			a_iSHmFlag, strerror(errno));
	}/*if ((HANDLE-1) != iShmID)*/

	/*if not exist, try to create*/
	if ((HANDLE)-1 ==  iShmID)
	{
		a_iSHmFlag |= IPC_CREAT;
		a_iSHmFlag |= IPC_EXCL;
		iShmID = shmget(a_tKey, a_iShmSize, a_iSHmFlag) ;
		if ((HANDLE)-1 == iShmID)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to create share memory(key:%d) %s.\n", a_tKey, strerror(errno));
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED);
		}
		*a_piCreate = 1;
	}/*if ((HANDLE)-1 == iShmID)*/



	*ppShm = (LPTBUSSHMGCIM)shmat ( iShmID, NULL, 0 ) ;
	if ((void *)-1 == *ppShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"create gcim failed by <key:%d, id:%d>, for %s\n",
			a_tKey, iShmID, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
	}
	tbus_log(TLOG_PRIORITY_DEBUG,"create/attach GCIM, shmkey:%d, size:%d, flag:0x%x",
		a_tKey, a_iShmSize, a_iSHmFlag ) ;

	return TBUS_SUCCESS;
}

int tbus_check_gcim_channels(IN LPTBUSSHMGCIM a_pstShmGCIM, IN LPTBUSGCIM a_pstGCIM)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGCIMHEAD pstHead;
	unsigned int i;
	LPTBUSSHMCHANNELCNF pstShmChl;
	time_t tNow;

	assert(NULL != a_pstShmGCIM);
	assert(NULL != a_pstGCIM);

	pstHead = &a_pstShmGCIM->stHead;
	tNow = time(NULL);

	if ((0 > a_pstGCIM->iChannelDataAlign) ||(TBUS_MAX_CHANNEL_DATA_ALIGN < a_pstGCIM->iChannelDataAlign))
	{
		a_pstGCIM->iChannelDataAlign = TBUS_DEFAULT_CHANNEL_DATA_ALIGN;
	}

	for (i = 0; i < pstHead->dwUsedCnt; i++)
	{
		pstShmChl = &a_pstShmGCIM->astChannels[i];
		if (tbus_is_channel_configured_i(a_pstGCIM, pstShmChl))
		{
			/*check the share memory for channel*/
			TBUS_GCIM_CHANNEL_SET_ENABLE(pstShmChl);

			iRet = tbus_check_channel_shm_i(pstShmChl, pstHead, 0);
			continue;
		}

		/*the channel is not in configure, so delete it*/
		tbus_log(TLOG_PRIORITY_FATAL,"channel(0x%08x <--> 0x%08x) is not in config list, so disable it",
			pstShmChl->astAddrs[0], pstShmChl->astAddrs[1]);
		if (TBUS_GCIM_CHANNEL_IS_ENABLE(pstShmChl))
		{
			TBUS_GCIM_CHANNEL_CLR_ENABLE(pstShmChl);
			pstShmChl->dwInvalidTime = tNow;
		}else if ((tNow - pstShmChl->dwInvalidTime) >= TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP)
		{
			tbus_log(TLOG_PRIORITY_FATAL,"the time which the channel(0x%08x <--> 0x%08x) is not in config list reach %d seconds, so delete it",
				pstShmChl->astAddrs[0], pstShmChl->astAddrs[1], TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP);

            // 为了改进tbusmgr更新通道的处理方式，
            // 1, 对于正在被使用的通道给出警告.
            // 2, 对于没有被使用的通道，先删除重新创建.
            // 为了保证一致性，如果tbus_delete_channel_shm_i不成功，则不删除其配置信息.
            // 之前，此处是无论tbus_delete_channel_shm_i成功与否，都不会break;
			iRet = tbus_delete_channel_shm_i(pstShmChl, pstHead);
            if (0 > iRet)
                break;

			if (i < (pstHead->dwUsedCnt -1))
			{
				memcpy(&a_pstShmGCIM->astChannels[i], &a_pstShmGCIM->astChannels[pstHead->dwUsedCnt -1],
					sizeof(TBUSSHMCHANNELCNF));
			}
			pstHead->dwUsedCnt--;
			i--;
		}/*if (pstShmChl->dwFlag & TBUS_ROUTE_VALID)*/
	}/*for (i = 0; i < pstHead->dwUsedCnt; i++)*/

	return iRet;
}

int tbus_set_gcim_ex(IN LPTBUSSHMGCIM a_pstShmGCIM, IN LPTBUSGCIM a_pstGCIM, IN int a_iFlag)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGCIMHEAD pstHead;
	unsigned int i;
	LPTBUSSHMCHANNELCNF pstShmChl;

	assert(NULL != a_pstShmGCIM);
	assert(NULL != a_pstGCIM);


	pstHead = &a_pstShmGCIM->stHead;
	tbus_wrlock(&pstHead->stRWLock);


	/*分析地址模板*/
	iRet = tbus_parse_addrtemplet(&pstHead->stAddrTemplet, a_pstGCIM->szAddrTemplet);

	if (TBUS_SUCCESS == iRet)
	{
		iRet = tbus_check_gcim_conf_i(a_pstGCIM, &pstHead->stAddrTemplet);
	}

	/*设置数据对齐方式*/
	if (pstHead->iAlign != a_pstGCIM->iChannelDataAlign)
	{
		pstHead->iAlign = a_pstGCIM->iChannelDataAlign;
		TBUS_CALC_ALIGN_LEVEL(pstHead->iAlignLevel, pstHead->iAlign);
	}

	/*对于绝对模式，检测 SHM 已经配置的通道项*/
    /*如果这些通道项不在配置列表中，则在共享内存中清除此配置项*/
	if (TBUS_SUCCESS == iRet && !(a_iFlag & TBUS_SET_GCIM_FLAG_RELATIVE))
	{
		tbus_check_gcim_channels(a_pstShmGCIM, a_pstGCIM);
	}/*if (TBUS_SUCCESS == iRet)*/


	/*添加新的配置项*/
	if (TBUS_SUCCESS == iRet)
	{
		for (i = 0; i < a_pstGCIM->dwCount; i++)
		{
			LPCHANNELCNF pstChl = &a_pstGCIM->astChannels[i];
			int idx = tbus_find_channel_in_gcim_i(a_pstShmGCIM, pstChl);
			if (0 <= idx)
			{
                if (!(a_iFlag & TBUS_SET_GCIM_FLAG_RECREATE))
                {
                    continue;
                }
                pstShmChl = &a_pstShmGCIM->astChannels[idx];
                iRet = tbus_delete_channel_shm_i(pstShmChl, pstHead);
                if (0 > iRet)
                {
                    if ((int)TBUS_ERR_MAKE_ERROR(TBUS_ERROR_DELETE_USED_CHANNEL) == iRet)
                    {
                        tbus_log(TLOG_PRIORITY_ERROR, "channel<shmkey:%u, addr1:%u, addr2:%u> is in use",
                                 pstHead->dwShmKey, (unsigned)pstShmChl->astAddrs[0], (unsigned)pstShmChl->astAddrs[1]);
                        printf("channel<shmkey:%u, addr1:%u, addr2:%u> is in use, can't be rewrite\n",
                               pstHead->dwShmKey, (unsigned)pstShmChl->astAddrs[0], (unsigned)pstShmChl->astAddrs[1]);
                    }
                    iRet = 0;
                    continue;
                }
                else
                {
                    if (idx < (int)(pstHead->dwUsedCnt -1))
                    {
                        memcpy(&a_pstShmGCIM->astChannels[idx], &a_pstShmGCIM->astChannels[pstHead->dwUsedCnt -1],
                               sizeof(TBUSSHMCHANNELCNF));
                    }
                    pstHead->dwUsedCnt--;
                }
			}

			if (pstHead->dwUsedCnt >= pstHead->dwMaxCnt)
			{
				tbus_log(TLOG_PRIORITY_ERROR,"channel num reach the max limit(%d) of GCIM, so cannot add channel",
					pstHead->dwMaxCnt);
				iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_NUM_LIMITED);
				break;
			}
			pstShmChl = &a_pstShmGCIM->astChannels[pstHead->dwUsedCnt];
			memset(pstShmChl, 0, sizeof(TBUSSHMCHANNELCNF));
			pstShmChl->astAddrs[0] = pstChl->interAddr[0];
			pstShmChl->astAddrs[1] = pstChl->interAddr[1];
			pstShmChl->dwCTime = time(NULL);
			pstShmChl->dwPriority = pstChl->dwPriority;
			pstShmChl->dwSendSize = pstChl->dwSendSize;
			TBUS_CALC_ALIGN_VALUE(pstShmChl->dwSendSize, pstHead->iAlign);
			pstShmChl->dwRecvSize = pstChl->dwRecvSize;
			TBUS_CALC_ALIGN_VALUE(pstShmChl->dwRecvSize, pstHead->iAlign);
			pstShmChl->dwFlag = TBUS_CHANNEL_FLAG_ENABLE;
			pstShmChl->iShmID = (HANDLE)pstChl->iShmID;
			iRet = tbus_check_channel_shm_i(pstShmChl, pstHead, 0);
			if (TBUS_SUCCESS != iRet)
			{
				tbus_log(TLOG_PRIORITY_ERROR,"failed to create share memory for channel(address %d <--> address %d), iRet %x",
					pstShmChl->astAddrs[0], pstShmChl->astAddrs[1], iRet);
				break;
			}
			pstHead->dwUsedCnt++;
		}/*for (i = 0; i < a_pstGCIM->dwCount; i++)*/
	}

	pstHead->dwVersion++;
	tbus_set_shmgcimheader_stamp(pstHead);
	tbus_unlock(&pstHead->stRWLock);


	return iRet;
}

int tbus_set_gcim(IN LPTBUSSHMGCIM a_pstShmGCIM, IN LPTBUSGCIM a_pstGCIM)
{
    return tbus_set_gcim_ex(a_pstShmGCIM, a_pstGCIM, 0);
}

int tbus_find_channel_in_gcim_i(IN LPTBUSSHMGCIM a_pstShmGCIM,  LPCHANNELCNF pstChl)
{
	unsigned int i = -1;

	assert(NULL != a_pstShmGCIM);
	assert(NULL != pstChl);

	for (i = 0; i < a_pstShmGCIM->stHead.dwUsedCnt; i++)
	{
		LPTBUSSHMCHANNELCNF pstChannels = &a_pstShmGCIM->astChannels[i];
		if ((pstChl->interAddr[0] == pstChannels->astAddrs[0]) &&
			(pstChl->interAddr[1] == pstChannels->astAddrs[1]))
		{
			break;
		}
	}/*for (i = 0; i < a_pstShmGCIM->stHead.dwUsedCnt; i++)*/
	if (i >= a_pstShmGCIM->stHead.dwUsedCnt)
	{
		return -1;
	}

	return i;
}

/**检查共享内存中配置的某通道是否在配置列表中
*return 在配置列表中返回非零值，否则返回0
*/
int tbus_is_channel_configured_i(IN LPTBUSGCIM a_pstGCIM, LPTBUSSHMCHANNELCNF pstShmChl)
{
	int iIsConfigured = 0;
	unsigned int i = 0;

	assert(NULL!= a_pstGCIM);
	assert(NULL != pstShmChl);

	for (i = 0; i < a_pstGCIM->dwCount; i++)
	{
		LPCHANNELCNF pstChlCnf = &a_pstGCIM->astChannels[i];
		if ((pstChlCnf->interAddr[0] == pstShmChl->astAddrs[0]) &&
			(pstChlCnf->interAddr[1] == pstShmChl->astAddrs[1]))
		{
			iIsConfigured = 1;
			break;
		}
	}/*for (i = 0; i < a_pstGCIM->dwCount; i++)*/

	return iIsConfigured;
}

void tbus_init_channel_head_i(CHANNELHEAD *pstHead, LPTBUSSHMCHANNELCNF a_pstShmChl, int iAlign)
{
    int i;

    assert(NULL != pstHead);
    assert(NULL != a_pstShmChl);

    memset(pstHead, 0, sizeof(CHANNELHEAD));
    for (i = 0; i < TBUS_CHANNEL_SIDE_NUM; i++)
    {
        pstHead->astAddr[i] = a_pstShmChl->astAddrs[i];
    }
    pstHead->dwAlignLevel = iAlign;
    pstHead->astQueueVar[0].dwSize =  a_pstShmChl->dwRecvSize - TBUS_SHM_CTRL_CHNNL_SIZE;
    pstHead->astQueueVar[1].dwSize =  a_pstShmChl->dwSendSize - TBUS_SHM_CTRL_CHNNL_SIZE;
    pstHead->iShmID = a_pstShmChl->iShmID;
    pstHead->dwMagic = TBUS_CHANNEL_MAGIC;
    pstHead->dwVersion = TBUS_SHM_CHANNEL_VERSION;
    TBUS_CALC_ALIGN_LEVEL(pstHead->dwAlignLevel, iAlign) ;

    pstHead->astQueueVar[0].dwCtrlChnnlSize = TBUS_SHM_CTRL_CHNNL_SIZE;
    pstHead->astQueueVar[1].dwCtrlChnnlSize = TBUS_SHM_CTRL_CHNNL_SIZE;
}

#if !defined (_WIN32) && !defined (_WIN64)
int tbus_check_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead, int a_iFlag)
{
	int iRet = TBUS_SUCCESS;
	int	iSize;
	char *sShm;
	CHANNELHEAD *pstHead ;

	int iFlags;
	int iChannelHeadLen;

    TOS_UNUSED_ARG(a_iFlag);

	assert(NULL != a_pstShmChl);
	assert(NULL != a_pstHead);

	/*检查通道是否存在*/
	{
		struct shmid_ds stShmStat ;
		iRet = shmctl( a_pstShmChl->iShmID, IPC_STAT, &stShmStat);
        if (0 == iRet)
        {
            return TBUS_SUCCESS;
        }
        iRet = 0;
	}

    /*如果不存在则创建*/
    tbus_log(TLOG_PRIORITY_FATAL,"chmctl IPC_STAT failed, for %s, channel shm(id: %d) ,so recreate",
            strerror(errno), a_pstShmChl->iShmID);

	iChannelHeadLen = sizeof(CHANNELHEAD);
	TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, a_pstHead->iAlign);
	iSize = iChannelHeadLen + a_pstShmChl->dwRecvSize + a_pstShmChl->dwSendSize;

    iFlags = TSHM_DFT_ACCESS | IPC_CREAT;
    a_pstShmChl->iShmID = shmget(IPC_PRIVATE, iSize, iFlags);
    if (0 > a_pstShmChl->iShmID)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"shmget IPC_PRIVATE failed to create share momory(size:%d), for %s",
                iSize, strerror(errno));
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMCTL_FAILED);
    }
    sShm = shmat(a_pstShmChl->iShmID, NULL , 0);
    if (NULL == sShm)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"shmat  failed to attach share momory(id:%d), for %s",
                a_pstShmChl->iShmID, strerror(errno));
        shmctl(a_pstShmChl->iShmID, IPC_RMID, NULL);
        a_pstShmChl->iShmID = -1;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
    }

    // set shm data
	pstHead = (CHANNELHEAD *)sShm;
    tbus_init_channel_head_i(pstHead, a_pstShmChl, a_pstHead->iAlign);


    shmdt(sShm);

	return iRet;
}
#else
int tbus_check_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead, int a_iFlag)
{
	int iRet = TBUS_SUCCESS;
	int	iSize;
	char *sShm;
	CHANNELHEAD *pstHead ;
	int i;
	int iFlags;
	int iChannelHeadLen;
    int iCreate = 0;


	assert(NULL != a_pstShmChl);
	assert(NULL != a_pstHead);

    // 在windows系统下，只要用来映射的文件存在，就应该认为共享内存是存在的
    // 这种意义下，在linux版本的tbus_check_channel_shm_i下执行的如下步骤：
    // /*检查通道是否存在*/
    // /*如果不存在则创建*/
    // 对windows系统而言是不够的
    // windows系统下的实现必须每次都通过名称openfilemaping
    // 如果openfilemapping失败，就要createfilemapping,
    // 这些正是函数tbus_open_mmap_by_key_and_addr的功能
    // 如果共享内在是重新创建的，则要初始化共享内存的数据

    iChannelHeadLen = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, a_pstHead->iAlign);
    iSize = iChannelHeadLen + a_pstShmChl->dwRecvSize + a_pstShmChl->dwSendSize;

    iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_CREATE;
    iRet = tbus_open_mmap_by_key_and_addr(&sShm, a_pstHead->dwShmKey,
            (unsigned int)a_pstShmChl->astAddrs[0],
            (unsigned int)a_pstShmChl->astAddrs[1],
            &iSize, iFlags, &iCreate, &a_pstShmChl->iShmID);
    if (0 > iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "tbus_open_mmap_by_key_and_addr failed, by "
                "shmkey(%d), addr1(%u), addr2(%u), size(%d)", a_pstHead->dwShmKey,
                (unsigned int)a_pstShmChl->astAddrs[0], (unsigned int)a_pstShmChl->astAddrs[1],
                iSize);
        return -1;
    }

    if (iCreate)
    {
        pstHead = (CHANNELHEAD *)sShm;
        tbus_init_channel_head_i(pstHead, a_pstShmChl, a_pstHead->iAlign);
    }

    shmdt(sShm);
    if (!(TBUS_CHECK_CHNNL_FLAG_KEEPOPEN & a_iFlag))
    {
        CloseHandle((HANDLE)a_pstShmChl->iShmID);
    }

	return iRet;
}
#endif


int tbus_delete_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD  a_pstHead)
{
    int iRet;
    int iIsInUse;

	assert(NULL != a_pstShmChl);

    iRet = tbus_is_channel_in_use(a_pstShmChl, a_pstHead, &iIsInUse);
    if (0 > iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "tbus_is_channel_in_use failed");
        return -1;
    }
    if (iIsInUse)
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_DELETE_USED_CHANNEL);
    }

#if !defined (_WIN32) && !defined (_WIN64)
    tbus_log(TLOG_PRIORITY_ERROR,"delete share memory(id %d)", a_pstShmChl->iShmID);
    if( shmctl(a_pstShmChl->iShmID, IPC_RMID, NULL) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"Remove share memory(id:%d) failed, %s.\n", a_pstShmChl->iShmID, strerror(errno));
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMCTL_FAILED);
    }
    a_pstShmChl->iShmID = -1;
#else
    // tbus_delete_channel_file_i 的返回值这里是故意忽略的.
    // 因为既然已经决断通道没有正在被使用，就应该认为删除成功了.
    // 至于这个文件是否删除已经无关紧要.
    tbus_delete_channel_file_i(a_pstShmChl, a_pstHead);
#endif

	return TBUS_SUCCESS;
}

int tbus_check_gcim_conf_i(IN LPTBUSGCIM a_pstGCIM, LPTBUSADDRTEMPLET a_pstAddrTemplet)
{
	int iRet = TBUS_SUCCESS;
	unsigned int i = 0;

    assert(NULL != a_pstGCIM);
    assert(NULL != a_pstAddrTemplet);

	/*检查配置信息*/
	if ((sizeof(a_pstGCIM->astChannels)/sizeof(a_pstGCIM->astChannels[0])) < a_pstGCIM->dwCount)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"channel count(%d) is invalid, it must be 0~%"PRIdPTR"\n",
                 a_pstGCIM->dwCount, (sizeof(a_pstGCIM->astChannels)/sizeof(a_pstGCIM->astChannels[0])));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
	}
	for (i = 0; i < a_pstGCIM->dwCount; i++)
	{
		LPCHANNELCNF pstChlCnf = &a_pstGCIM->astChannels[i];
		if (TBUS_CHANNEL_SIDE_NUM > pstChlCnf->dwAddressCount)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"%dth channel configure is invalid, the address count is less than %d",
				i+1, pstChlCnf->dwAddressCount);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
			break;
		}

		iRet = tbus_addr_aton_by_addrtemplet(a_pstAddrTemplet, pstChlCnf->aszAddress[0],
			(LPTBUSADDR)&pstChlCnf->interAddr[0]);
		if (TBUS_SUCCESS != iRet)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to convert %s to tbusd address, please check %dth channel configure",
				pstChlCnf->aszAddress[0], i+1);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
			break;
		}
		iRet = tbus_addr_aton_by_addrtemplet(a_pstAddrTemplet, pstChlCnf->aszAddress[1],
			(LPTBUSADDR)&pstChlCnf->interAddr[1]);
		if (TBUS_SUCCESS != iRet)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to convert %s to tbusd address, please check %dth channel configure",
				pstChlCnf->aszAddress[1], i+1);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
			break;
		}
		if (pstChlCnf->interAddr[0] == pstChlCnf->interAddr[1])
		{
			tbus_log(TLOG_PRIORITY_ERROR,"the two address of channel must be different, but %dth channel was configured the same address %s",
				i+1, tbus_addr_nota_by_addrtemplet(a_pstAddrTemplet, pstChlCnf->interAddr[0]));
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
			break;
		}

		/*adjust the order of address, first set the litter*/
		if (pstChlCnf->interAddr[0] > pstChlCnf->interAddr[1])
		{
			unsigned int dwSize = pstChlCnf->dwRecvSize;
			TBUSADDR tTmp = pstChlCnf->interAddr[0];
			pstChlCnf->dwRecvSize = pstChlCnf->dwSendSize;
			pstChlCnf->dwSendSize = dwSize;
			pstChlCnf->interAddr[0] = pstChlCnf->interAddr[1];
			pstChlCnf->interAddr[1] = tTmp;
		}
	}/*for (i = 0; i < a_pstGCIM->dwCount; i++)*/

	TBUS_NORMALIZE_ALIGN(a_pstGCIM->iChannelDataAlign);
	return iRet;
}

int tbus_enable_channel_by_index(IN LPTBUSSHMGCIM a_pstShmGCIM, IN int a_idx)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGCIMHEAD pstHead ;
	LPTBUSSHMCHANNELCNF pstShmChl;

	assert(NULL != a_pstShmGCIM);

	pstHead = &a_pstShmGCIM->stHead;
	if ((0 > a_idx) || (a_idx >= (int)pstHead->dwUsedCnt))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid channel index(%d), it must be in 0~%d",
			a_idx, pstHead->dwUsedCnt);
		return TBUS_ERR_ARG;
	}

	tbus_wrlock(&pstHead->stRWLock);

	pstShmChl = &a_pstShmGCIM->astChannels[a_idx];
    TBUS_GCIM_CHANNEL_SET_ENABLE(pstShmChl);
	tbus_set_shmgcimheader_stamp(pstHead);

	tbus_unlock(&pstHead->stRWLock);

	return iRet;
}

int tbus_disable_channel_by_index(IN LPTBUSSHMGCIM a_pstShmGCIM, IN int a_idx)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGCIMHEAD pstHead ;
	LPTBUSSHMCHANNELCNF pstShmChl;

	assert(NULL != a_pstShmGCIM);

	pstHead = &a_pstShmGCIM->stHead;
	if ((0 > a_idx) || (a_idx >= (int)pstHead->dwUsedCnt))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid channel index(%d), it must be in 0~%d",
			a_idx, pstHead->dwUsedCnt);
		return TBUS_ERR_ARG;
	}

	tbus_wrlock(&pstHead->stRWLock);

	pstShmChl = &a_pstShmGCIM->astChannels[a_idx];
    TBUS_GCIM_CHANNEL_CLR_ENABLE(pstShmChl);
	tbus_set_shmgcimheader_stamp(pstHead);

	tbus_unlock(&pstHead->stRWLock);

	return iRet;
}

int tbus_delete_channel_by_index(IN LPTBUSSHMGCIM a_pstShmGCIM, IN int a_idx)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGCIMHEAD pstHead ;
	LPTBUSSHMCHANNELCNF pstShmChl;

	assert(NULL != a_pstShmGCIM);

	pstHead = &a_pstShmGCIM->stHead;
	if ((0 > a_idx) || (a_idx >= (int)pstHead->dwUsedCnt))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid channel index(%d), it must be in 0~%d",
			a_idx, pstHead->dwUsedCnt);
		return TBUS_ERR_ARG;
	}

	tbus_wrlock(&pstHead->stRWLock);


	pstShmChl = &a_pstShmGCIM->astChannels[a_idx];
	iRet = tbus_delete_channel_shm_i(pstShmChl, pstHead);
	if (0 != iRet)
	{
		tbus_unlock(&pstHead->stRWLock);
		return iRet;
	}

	//成功删除，修改全局配置数据
	if (a_idx < (int)(pstHead->dwUsedCnt -1))
	{
		memcpy(&a_pstShmGCIM->astChannels[a_idx], &a_pstShmGCIM->astChannels[pstHead->dwUsedCnt -1],
			sizeof(TBUSSHMCHANNELCNF));
	}
	pstHead->dwUsedCnt--;

	pstHead->dwVersion++;
	tbus_set_shmgcimheader_stamp(pstHead);
	tbus_unlock(&pstHead->stRWLock);

	return iRet;
}

int tbus_create_grmshm(INOUT LPTBUSSHMGRM *a_ppstGRM, IN const char *a_pszShmkey,
					   IN int a_iBussId, IN unsigned int a_iShmSize, OUT HANDLE *a_phShmHandle)
{

	int	iRet = TBUS_SUCCESS ;
	int iCreate = 0;
	LPTBUSSHMGRM pstGRM = NULL;
	key_t tShmkey = -1;
	int iFlags;
    unsigned int dwShmExpectSize = a_iShmSize;

	assert(NULL != a_ppstGRM);


	if ((NULL == a_pszShmkey) || ('\0' == a_pszShmkey))
	{

		a_pszShmkey = TBUS_DEFAULT_GRM_KEY;
		tbus_log(TLOG_PRIORITY_DEBUG,"null shmkey, so use the default shmkey %s", TBUS_DEFAULT_GRM_KEY);
	}
	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmkey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s",
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}

#if !defined (_WIN32) && !defined (_WIN64)
	iFlags = TSHM_DFT_ACCESS |IPC_CREAT|IPC_EXCL ;
	iRet = tbus_create_shm((void **)(void*)&pstGRM, tShmkey, a_iShmSize, iFlags, &iCreate);
    if (NULL != a_phShmHandle)
    {
        *a_phShmHandle = 0;
    }
#else
	iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_CREATE;
	iRet = tbus_open_mmap_by_key((void **)(void*)&pstGRM, tShmkey, &a_iShmSize, iFlags, &iCreate, a_phShmHandle);
#endif
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to create GCIM share memory, iRet :%x", iRet);
		return iRet;
	}

	if (iCreate)
	{
		LPTBUSSHMGRMHEAD pstHead = &pstGRM->stHead;
		pthread_rwlockattr_t rwlattr;

		pthread_rwlockattr_init(&rwlattr);
		pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(&pstHead->stRWLock, &rwlattr);

		tbus_wrlock(&pstHead->stRWLock);
		pstHead->dwBusiID = a_iBussId;
		pstHead->dwCreateTime = time(NULL);
		pstHead->dwMaxCnt = (dwShmExpectSize - offsetof(TBUSSHMGRM,astRelays)) / sizeof(TBUSSHMRELAYCNF);
		pstHead->dwShmKey = tShmkey;
		pstHead->dwShmSize = a_iShmSize;
		pstHead->dwUsedCnt = 0;
		pstHead->dwVersion = 0;
		memset(&pstHead->stAddrTemplet, 0, sizeof(pstHead->stAddrTemplet));
		memset(&pstHead->aiReserved, 0, sizeof(pstHead->aiReserved));
		tbus_set_shmgrmheader_stamp(pstHead);
		tbus_unlock(&pstHead->stRWLock);
	}

	*a_ppstGRM = pstGRM;

	return iRet ;
}

int tbus_get_grmshm(INOUT LPTBUSSHMGRM *a_ppstGRM, IN const char *a_pszShmkey,
                    IN int a_iBussId, IN unsigned int a_iShmSize, OUT HANDLE *a_phShmHandle)
{
	int iRet = 0;
	key_t tShmkey = -1;
	int iFlags;
	LPTBUSSHMGRM pstGRM = NULL;
	unsigned int iSize;
	LPTBUSSHMGRMHEAD pstHead;
    unsigned int iCount;
#if defined (_WIN32) || defined (_WIN64)
	int iCreate = 0;
#endif

	assert(NULL != a_ppstGRM);

	if ((NULL == a_pszShmkey) || ('\0' == a_pszShmkey))
	{

		a_pszShmkey =TBUS_DEFAULT_GRM_KEY;
		tbus_log(TLOG_PRIORITY_DEBUG,"null shmkey, so use the default shmkey %s", TBUS_DEFAULT_GRM_KEY);
	}
	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmkey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s",
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}


	iSize = a_iShmSize;
#if !defined (_WIN32) && !defined (_WIN64)
	iFlags = TSHM_DFT_ACCESS;
	iRet = tbus_attach_shm((void **)(void*)&pstGRM, tShmkey, &iSize, iFlags, 0);
    if (NULL != a_phShmHandle)
    {
        *a_phShmHandle = 0;
    }
#else
	iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_EXCL;
	iRet = tbus_open_mmap_by_key((void **)(void*)&pstGRM, tShmkey, &iSize, iFlags, &iCreate, a_phShmHandle);
#endif
	if ( 0 > iRet )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed attch GRM by  generate shmkey, by shmkey<%s> and bussId<%d>, for %s",
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}


	/* share memory avaiable now */
	pstHead = &pstGRM->stHead;
	if (iSize < sizeof(TBUSSHMGRMHEAD))
	{
		tbus_log(TLOG_PRIORITY_FATAL,"share memory size %d is less than the size of TUBSSHMHEADER %"PRIdPTR"\n",
			iSize, sizeof(TBUSSHMGRMHEAD)) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if (tbus_check_shmgrmheader_stamp(pstHead) != 0)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to check GCIM  shm stamp, shmkey %d\n", tShmkey);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if ((key_t)pstHead->dwShmKey != tShmkey)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"share memory key %d not match to the settings key:%d\n",
			pstHead->dwShmKey, tShmkey);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if (iSize < pstHead->dwShmSize)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"share memory size not match, real size %d, but need size %d",
			iSize, pstHead->dwShmSize);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}/*if ( 0 != iRet )*/

    if (pstHead->dwMaxCnt > (unsigned int)(sizeof(pstGRM->astRelays)/sizeof(pstGRM->astRelays[0])))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"the max relay count(%u) in GRM is beyond the max limit:%u",
            pstHead->dwMaxCnt, (unsigned int)(sizeof(pstGRM->astRelays)/sizeof(pstGRM->astRelays[0])));
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
    }


    /*TBUS_GCIM_BUILD 为1时 扩展了 TBUS_MAX_RELAY_NUM_PREHOST，因此 需要重新计算dwMaxCnt */
    iCount = (iSize - offsetof(TBUSSHMGRM,astRelays)) / sizeof(TBUSSHMRELAYCNF);
    if (0 >= iCount)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"the memory size(%"PRIdPTR") astRelays(size:%u) in GRM is too small",
            (iSize - offsetof(TBUSSHMGRM,astRelays)) , (unsigned int)sizeof(TBUSSHMRELAYCNF));
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
    }
    if (pstHead->dwMaxCnt > iCount)
    {
        pstHead->dwMaxCnt = iCount;
        tbus_log(TLOG_PRIORITY_INFO,"the memory size(%"PRIdPTR") for astRelays(size:%u) in GRM can store max %u relay",
            (iSize - offsetof(TBUSSHMGRM,astRelays)) , (unsigned int)sizeof(TBUSSHMRELAYCNF),iCount);
    }
    if (pstHead->dwUsedCnt > pstHead->dwMaxCnt)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"the used relay count(%u) in GRM is beyond the max limit:%u",
            pstHead->dwUsedCnt, pstHead->dwMaxCnt);
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
    }

	*a_ppstGRM = pstGRM;

	return iRet ;
}

int tbus_set_grm(IN LPTBUSSHMGRM a_pstShmGRM, IN LPRELAYMNGER a_pstRelayConf)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGRMHEAD pstHead;
	unsigned int i,j;
	LPTBUSSHMRELAYCNF pstShmRelay;

	assert(NULL != a_pstShmGRM);
	assert(NULL != a_pstRelayConf);

	pstHead = &a_pstShmGRM->stHead;
	tbus_wrlock(&pstHead->stRWLock);

	/*分析地址模板*/
	iRet = tbus_parse_addrtemplet(&pstHead->stAddrTemplet, a_pstRelayConf->szAddrTemplet);
	if (TBUS_SUCCESS == iRet)
	{

		pstHead->dwUsedCnt = 0;
		for (i = 0; i < a_pstRelayConf->dwUsedCnt; i++)
		{
			LPSHMRELAY pstRelay = &a_pstRelayConf->astRelays[i];
			TBUSADDR iAddr;
			if (TBUS_SUCCESS != tbus_addr_aton_by_addrtemplet(&pstHead->stAddrTemplet,
				pstRelay->szAddr, &iAddr))
			{
				tbus_log(TLOG_PRIORITY_ERROR,"failed to convert %s to tbus address", pstRelay->szAddr);
				iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
				break;
			}
			for (j = 0; j < pstHead->dwUsedCnt; j ++)
			{
				if (iAddr == a_pstShmGRM->astRelays[j].dwAddr)
				{
					break;
				}
			}
			if (j < pstHead->dwUsedCnt)
			{
				continue;
			}

			if (pstHead->dwUsedCnt >= pstHead->dwMaxCnt)
			{
				tbus_log(TLOG_PRIORITY_ERROR,"relay route num reach the max limit(%d) of GCIM, so cannot add channel",
					pstHead->dwMaxCnt);
				iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_RELAY_NUM_LIMITED);
				break;
			}
			pstShmRelay = &a_pstShmGRM->astRelays[pstHead->dwUsedCnt];
			memset(pstShmRelay, 0, sizeof(TBUSSHMRELAYCNF));
			pstShmRelay->dwAddr = iAddr;
			pstShmRelay->dwFlag = pstRelay->dwEnable;
			pstShmRelay->dwPriority = pstRelay->dwPriority;
			pstShmRelay->dwStrategy = pstRelay->dwStrategy;
			STRNCPY(pstShmRelay->szMConn, pstRelay->szMConn, sizeof(pstShmRelay->szMConn));
			STRNCPY(pstShmRelay->szSConn, pstRelay->szSConn, sizeof(pstShmRelay->szSConn));

			pstHead->dwUsedCnt++;
		}/*for (i = 0; i < a_pstGCIM->dwCount; i++)*/
	}/*if (TBUS_SUCCESS == iRet)*/



	pstHead->dwVersion++;
	tbus_set_shmgrmheader_stamp(pstHead);
	tbus_unlock(&pstHead->stRWLock);

	return iRet;
}

int tbus_wrlock(pthread_rwlock_t *a_pstRWLock)
{
	int iRet = TBUS_SUCCESS;
	int iTimeCount = 0;

	assert(NULL != a_pstRWLock);
#if defined (_WIN32) || defined (_WIN64)
	//TODO 目前pal的pthread_rwlock系列函数在windows平台下不支持PTHREAD_PROCESS_SHARED
#else
	/*很粗陋的设计,这样设计的前提是，正常情况读写锁的释放时间不会超过
	预定时间，如果超过了预定时间还没有获取到锁，则强制将锁夺过来*/
	for (iTimeCount = 0; iTimeCount < TBUS_RWLOCK_TIMEOUT;)
	{
		iRet = pthread_rwlock_trywrlock(a_pstRWLock);
		if (0 == iRet)
		{
			break;
		}
		tos_usleep(TBUS_SLEEP_PRELOOP*1000);
		iTimeCount += TBUS_SLEEP_PRELOOP;
	}

	if (iTimeCount >= TBUS_RWLOCK_TIMEOUT)
	{
		pthread_rwlockattr_t rwlattr;
		tbus_log(TLOG_PRIORITY_FATAL,"timeout(%d milliseconds) to acqure lock, so force to acqure it", iTimeCount);

		pthread_rwlockattr_init(&rwlattr);
		pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(a_pstRWLock, &rwlattr);
		pthread_rwlock_trywrlock(a_pstRWLock);
	}
#endif
	return iRet;
}

int tbus_rdlock(pthread_rwlock_t *a_pstRWLock)
{
	int iRet = TBUS_SUCCESS;
	int iTimeCount;

	TOS_UNUSED_ARG(iTimeCount);
	assert(NULL != a_pstRWLock);
#if defined (_WIN32) || defined (_WIN64)
	//TODO 目前pal的pthread_rwlock系列函数在windows平台下不支持PTHREAD_PROCESS_SHARED
#else
	/*很粗陋的设计,这样设计的前提是，正常情况读写锁的释放时间不会超过
	预定时间，如果超过了预定时间还没有获取到锁，则强制将锁夺过来*/
	for (iTimeCount = 0; iTimeCount < TBUS_RWLOCK_TIMEOUT;)
	{
		iRet = pthread_rwlock_tryrdlock(a_pstRWLock);
		if (0 == iRet)
		{
			break;
		}
		tos_usleep(TBUS_SLEEP_PRELOOP*1000);
		iTimeCount += TBUS_SLEEP_PRELOOP;
	}

	if (iTimeCount >= TBUS_RWLOCK_TIMEOUT)
	{
		pthread_rwlockattr_t rwlattr;
		tbus_log(TLOG_PRIORITY_FATAL,"timeout(%d milliseconds) to acqure lock, so force to acqure it", iTimeCount);

		pthread_rwlockattr_init(&rwlattr);
		pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(a_pstRWLock, &rwlattr);
		pthread_rwlock_tryrdlock(a_pstRWLock);
	}
#endif

	return iRet;
}

void tbus_unlock(pthread_rwlock_t *a_pstRWLock)
{
	assert(NULL != a_pstRWLock);
	pthread_rwlock_unlock(a_pstRWLock);
}

void tbus_detach_grmshm(IN LPTBUSSHMGRM a_pstGRM, HANDLE a_hShmHandle)
{
	if (NULL != a_pstGRM)
	{
		shmdt(a_pstGRM);
#if defined (_WIN32) || defined (_WIN64)
        if (NULL != a_hShmHandle)
        {
            CloseHandle(a_hShmHandle);
        }
#endif
	}
}

void tbus_detach_gcim(IN LPTBUSSHMGCIM a_pstShmGCIM, HANDLE a_hShmHandle)
{
	if (NULL != a_pstShmGCIM)
	{
		shmdt(a_pstShmGCIM);
#if defined (_WIN32) || defined (_WIN64)
        if (NULL != a_hShmHandle)
        {
            CloseHandle(a_hShmHandle);
        }
#endif
	}
}

int tbus_get_attached_num_i(HANDLE a_iShmID, int* a_piNum)
{
    int iRet = 0;
	struct shmid_ds stShmStat ;

	iRet = shmctl(a_iShmID, IPC_STAT, &stShmStat);
    if (0 > iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "shmctl failed to get IPC_STAT, by shmID<%d>, for %s", a_iShmID, strerror(errno));
        return -1;
    }
    *a_piNum = stShmStat.shm_nattch;

    return 0;
}

/* this interface is mainly for tbusmgr */
/* so return value not follows tbus-api-standard */
int tbus_get_attached_num(const char* a_pszShmKey, int a_iBussId, int *a_piNum, HANDLE *a_piShmID)
{
    int iRet = 0;
	key_t tShmkey = -1;
    HANDLE iShmID = (HANDLE)-1;

	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmKey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s",
			a_pszShmKey, a_iBussId, tbus_error_string(iRet));
		return -1;
	}

    iShmID = shmget(tShmkey, 0, 0) ;
    if ((HANDLE)-1 == iShmID)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "shmget failed to get shmID, by shmkey<%d>, for %s", tShmkey, strerror(errno));
        return -1;
    }

    iRet = tbus_get_attached_num_i(iShmID, a_piNum);
    if (0 != iRet)
    {
        return -1;
    }

    if (NULL != a_piShmID)
    {
        *a_piShmID = iShmID;
    }

	return 0;
}

int tbus_is_channel_in_use(IN LPTBUSSHMCHANNELCNF a_pstShmChl,
        LPTBUSSHMGCIMHEAD a_pstHead, INOUT int* a_iIsInUse)
{
    int iRet = 0;

#if !defined (_WIN32) && !defined (_WIN64)
    {
        int iAttachNum = 0;

        TOS_UNUSED_ARG(a_pstHead);
        iRet = tbus_get_attached_num_i(a_pstShmChl->iShmID, &iAttachNum);
        if (0 > iRet)
            return -1;

        if (0 < iAttachNum)
            *a_iIsInUse = 1;
        else
            *a_iIsInUse = 0;
    }
#else
    {
        HANDLE hMapHandle;
        char szFileName[TIPC_MAX_NAME];
        char szMapName[TIPC_MAX_NAME];
        iRet = tbus_key_and_addr_to_path(a_pstHead->dwShmKey,
                (unsigned int)a_pstShmChl->astAddrs[0],
                (unsigned int)a_pstShmChl->astAddrs[1],
                szFileName, sizeof(szFileName));
        if (0 > iRet)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "tbus_key_and_addr_to_path failed, by shmkey<%u> addr1<%u> addr2<%u>",
                    a_pstHead->dwShmKey, (unsigned)a_pstShmChl->astAddrs[0], (unsigned)a_pstShmChl->astAddrs[1]);
            return -1;
        }

        if( tipc_path2name(szMapName, TIPC_MAX_NAME, szFileName, "tmmap:") < 0 )
        {
            tbus_log(TLOG_PRIORITY_ERROR, "failed to create mmap name by file path(%s)", szFileName);
            return -1;
        }

        // 这里根据是否能使用名称打开mmap对象来判断是否有其他进程在使用通道，
        // 如果能打开，认为有其他进程正在使用通道；
        // 如果不能打开，认为没有其他进程正在使用通道；
        // 可能还需要加强，判断是什么原因导致不能打开。
        hMapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, szMapName);
        if (NULL != hMapHandle)
        {
            CloseHandle(hMapHandle);
            *a_iIsInUse = 1;
        }
        else
            *a_iIsInUse = 0;
    }
#endif

    return iRet;
}
