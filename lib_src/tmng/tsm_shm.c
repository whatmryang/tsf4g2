//============================================================================
// @Id:       $Id: tsm_shm.c 15076 2010-11-23 02:56:24Z flyma $
// @Author:   $Author: flyma $
// @Date:     $Date:: 2010-11-23 10:56:24 +0800 #$
// @Revision: $Revision: 15076 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------

#include "tsm_shm.h"

#if defined (_WIN32) || defined (_WIN64)

#pragma warning(disable:4996)

#include <sys/stat.h>
#include "err_stack.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int security_attr_init(WinSd *p);
static int security_attr_init(WinSd *p)
{
    char                     szErrBuf[1024];
    int                      iRet;

    DWORD                    iLen;
    PSID                     pSid = NULL;
    PACL                     pAcl = NULL;
    SID_IDENTIFIER_AUTHORITY sidW = SECURITY_WORLD_SID_AUTHORITY;

    p->ptr                     = NULL;
    p->sa.nLength              = sizeof(p->sa);
    p->sa.lpSecurityDescriptor = &p->sd;
    p->sa.bInheritHandle       = FALSE;

    iRet = 1;
    if (!InitializeSecurityDescriptor(&p->sd, SECURITY_DESCRIPTOR_REVISION))
    {
        goto err;
    }

    iRet = 2;
    if (!AllocateAndInitializeSid(&sidW,
                                  1,
                                  SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0,
                                  &pSid))
    {
        goto err;
    }

    iRet = 3;
    iLen  = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE)  - sizeof(DWORD);
    iLen += GetLengthSid(pSid);
    if (!(pAcl = (PACL)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, iLen)))
    {
        goto err;
    }

    iRet = 4;
    if (!InitializeAcl(pAcl, iLen, ACL_REVISION))
    {
        goto free;
    }

    iRet = 5;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, pSid))
    {
        goto free;
    }

    iRet = 6;
    if (!SetSecurityDescriptorDacl(&p->sd, TRUE, pAcl, FALSE))
    {
        goto free;
    }

    if (pSid)
    {
        FreeSid(pSid);
    }
    p->ptr = (PVOID)pAcl;

    return 0;

free:
    HeapFree(GetProcessHeap(), 0, pAcl);

err:
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  szErrBuf,
                  sizeof(szErrBuf) - 1,
                  NULL);
    return terror_push(NULL, iRet, szErrBuf);
}
TSINGLETON_C(WinSd, SD, security_attr_init);


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int get_dir_i(char *path, size_t len);
static int get_dir_i(char *path, size_t len)
{
    int iRet = 1;

    if (!GetEnvironmentVariable("TSM_DATA_PATH", path, len))
    {
        iRet = snprintf(path, len - 1, "C:\\WINDOWS\\TSM_DATA");
        iRet = iRet > 0 && iRet < len;
    }

    if (iRet)
    {
        tfmkdir(path);
    }

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int get_name_i(char *file, size_t len, int shmid);
static int get_name_i(char *file, size_t len, int shmid)
{
    int iRet = snprintf(file, len - 1, "%08X", shmid);
    return iRet > 0 && iRet < len;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int get_file_i(char *file, size_t len, int shmid);
static int get_file_i(char *file, size_t len, int shmid)
{
    char szName[20];
    int  iRet;

    if (!get_name_i(szName, 20, shmid))
    {
        return 0;
    }

    iRet = snprintf(file, len - 1, "tsm_%s", szName);
    return iRet > 0 && iRet < len;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int get_path_i(char *path, size_t len, int shmid, const char *ext);
static int get_path_i(char *path, size_t len, int shmid, const char *ext)
{
    char szPath[100];
    char szFile[24];
    int  iRet;

    if (!get_dir_i(szPath, 100))
    {
        return 0;
    }
    if (!get_file_i(szFile, 24, shmid))
    {
        return 0;
    }

    iRet = snprintf(path, len - 1, "%s\\%s%s", szPath, szFile, ext);
    return iRet > 0 && iRet < len;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int get_shmid_i(size_t size, key_t key);
static int get_shmid_i(size_t size, key_t key)
{
    int  i;
    int  iShmId;
    int  flag = TF_MODE_READ | TF_MODE_WRITE | TF_MODE_CREATE | TF_MODE_EXCL;
    char szPath[1024];
    HANDLE       fp;
    tsm_shm_prop tsp = { 0x18790314,
                         1,
                         key,
                         -1,
                         size + sizeof(tsm_shm_prop),
                         1 };

    srand(GetTickCount());

    for (i = 0; i < 10; ++i)
    {
        if (-1 == (iShmId = rand()))
        {
            continue;
        }

        tassert1(get_path_i(szPath, 1024, iShmId, ".mmf"),
                 NULL, 1, "取得路径失败 mmf(%08X)", iShmId);
        if ((HANDLE)-1 != (fp = tfopen(szPath, flag)))
        {
            tsp.iShmId = iShmId;
            tfwrite(fp, (const char *)&tsp, sizeof(tsm_shm_prop));
            tfclose(fp);

            return iShmId;
        }
    }

    return -1;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int get_size_i(int shmid);
static int get_size_i(int shmid)
{
    HANDLE       fp;
    char         szMMF[1024];
    tsm_shm_prop tsp;

    tassert_r1((size_t)0, get_path_i(szMMF, 1024, shmid, ".mmf"),
               NULL, 1, "取得路径失败 mmf(%08X)", shmid);

    if (tfexist(szMMF))
    {
       return (size_t)0;
    }

    if ((HANDLE)-1 == (fp = tfopen(szMMF, TF_MODE_READ | TF_MODE_WRITE)))
    {
        return (size_t)0;
    }
    tfread(fp, (char *)&tsp, sizeof(tsm_shm_prop));
    tfclose(fp);

    return tsp.iSize;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
    MEMORY_BASIC_INFORMATION  mbi;
    void                     *pvBase;
    tsm_shm_prop             *tsp;
    int                       iRet = 0;

    switch(cmd)
    {
    case IPC_STAT:
        pvBase = tsm_shmat(shmid, NULL, FILE_MAP_READ);
        memset(&mbi, 0, sizeof(mbi));
        memset(buf,  0, sizeof(*buf));
        if ((void *)-1 == pvBase)
        {
            iRet = EINVAL;
        }
        else
        {
            VirtualQuery(pvBase, &mbi, sizeof(mbi));
            buf->shm_segsz = mbi.RegionSize;
        }
        break;

    case IPC_SET:
        break;

    case IPC_RMID:
        pvBase = tsm_shmat(shmid, NULL, 0);
        tsp = (tsm_shm_prop *)((char *)pvBase - sizeof(tsm_shm_prop));
        --tsp->iStatus;
        iRet = tsm_shmdt(pvBase);
        break;
    }

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_shmget(key_t key, size_t size, int sflag)
{
    HANDLE f = (HANDLE)-1;
    int    iKey = (int)key;
    int    iShmId;
    int    iFlag = TF_MODE_READ | TF_MODE_WRITE;
    char   szPath[1024];

    if (IPC_PRIVATE == key)
    {
        iShmId = get_shmid_i(size, (key_t)-1);
        tassert(-1 != iShmId, NULL, 1, "取得 shmid失败");

        return iShmId;
    }

    tassert1(get_path_i(szPath, sizeof(szPath), iKey, ".mmk"),
             NULL, 4, "根据 key取得路径失败 (%08X)", iKey);

    if ((sflag & IPC_CREAT) && ((sflag & IPC_EXCL) || -1 == tfexist(szPath)))
    {
        iFlag |= TF_MODE_CREATE | TF_MODE_EXCL;
        if ((HANDLE)-1 == (f = tfopen(szPath, iFlag)))
        {
            return -1;
        }
        if (-1 == (iShmId = get_shmid_i(size, (key_t)key)))
        {
            tfclose(f);
            tfdelete(szPath);
            terror_push(NULL, 5, "取得 shmid失败");
        }
        else
        {
            tfwrite(f, (const char *)&iShmId, sizeof(iShmId));
            tfclose(f);
        }
        return iShmId;
    }

    if ((HANDLE)-1 == (f = tfopen(szPath, iFlag)))
    {
        return -1;
    }
    tfread(f, (char *)&iShmId, sizeof(iShmId));
    tfclose(f);

    if (!get_size_i(iShmId))
    {
        tfdelete(szPath);
        return -1;
    }
    return iShmId;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void *tsm_shmat(int shmid, void *shmaddr, int shmflg)
{
    HANDLE  mp;
    HANDLE  fp;
    char    szFile[1024];
    char    szPath[1024];
    size_t  iSize;
    void   *pvBase;
    tsm_shm_prop *tsp;

    tassert_r1((void *)NULL, (iSize = get_size_i(shmid)),
               NULL, 1, "get_size_i 失败 (%d)", shmid);
    tassert_r1((void *)NULL, get_file_i(szFile, 1024, shmid),
               NULL, 2, "get_file_i 失败 (%d)", shmid);
    tassert_r1((void *)NULL, get_path_i(szPath, 1024, shmid, ".mmf"),
               NULL, 3, "get_path_i 失败 (%d)", shmid);

    mp = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, szFile);
    if (!mp)
    {
        fp = tfopen(szPath, TF_MODE_READ | TF_MODE_WRITE | TMMAPF_CREATE);
        if ((HANDLE)-1 != fp)
        {
            mp = CreateFileMapping(fp,
                                   &TSINGLETON(WinSd, SD)->sa,
                                   PAGE_READWRITE,
                                   0,
                                   (DWORD)iSize,
                                   szFile);
            tfclose(fp);
        }
    }
    if (!mp)
    {
        return (void *)-1;
    }

    if (shmflg & SHM_RDONLY)
    {
        pvBase = MapViewOfFile(mp, FILE_MAP_READ, 0, 0, 0);
    }
    else
    {
        pvBase = MapViewOfFile(mp, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    }
    CloseHandle(mp);

    if (pvBase)
    {
        tsp = (tsm_shm_prop *)pvBase;
        ++tsp->iStatus;
        return (void *)((char *)pvBase + sizeof(tsm_shm_prop));
    }
    return (void *)-1;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_shmdt(void *addr)
{
    tsm_shm_prop *tsp = (tsm_shm_prop *)((char *)addr - sizeof(tsm_shm_prop));
    key_t         iKey = tsp->iKey;
    int           iShmId = tsp->iShmId;
    char          szPath[1024];
    int           iRet;
    int           i;

    if ((i = --tsp->iStatus) > 0)
    {
        FlushViewOfFile((void *)tsp, tsp->iSize + sizeof(tsm_shm_prop));
    }
    iRet = UnmapViewOfFile((void *)tsp) ? 0 : -1;

    if (i <= 0 && get_path_i(szPath, 1024, iShmId, ".mmf"))
    {
        tfdelete(szPath);
        if ((key_t)-1 != iKey && (get_path_i(szPath, 1024, (int)iKey, ".mmk")))
        {
            tfdelete(szPath);
        }
    }

    return iRet;
}

#endif

//----------------------------------------------------------------------------
// THE END
//============================================================================
