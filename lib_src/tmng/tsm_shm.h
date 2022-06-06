#ifndef _TSM_SHM_H_
#define _TSM_SHM_H_
//============================================================================
// @Id:       $Id: tsm_shm.h 15076 2010-11-23 02:56:24Z flyma $
// @Author:   $Author: flyma $
// @Date:     $Date:: 2010-11-23 10:56:24 +0800 #$
// @Revision: $Revision: 15076 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------

#if !defined(_WIN32) && !defined(_WIN64)

    #include <sys/ipc.h>
    #include <sys/shm.h>

#endif


#ifdef __cplusplus
extern "C"
{
#endif


#if defined(_WIN32) || defined(_WIN64)

    #include "tsingleton.h"

    #include "pal/tos.h"
    #include "pal/ttypes.h"
    #include "pal/tstdio.h"
    #include "pal/tstring.h"

    #include "pal/tipc.h"
    #include "pal/tfile.h"
    #include "pal/tmmap.h"
    #include "pal/tshm.h"

    typedef struct
    {
        PVOID               ptr;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;
    } WinSd;

    TSINGLETON_H(WinSd, SD);

    typedef struct
    {
        int    iMagic;
        int    iVersion;
        key_t  iKey;
        int    iShmId;
        size_t iSize;
        int    iStatus;
    } tsm_shm_prop;

    /*
    #define SHM_R       0400        // or S_IRUGO from <linux/stat.h>
    #define SHM_W       0200        // or S_IWUGO from <linux/stat.h>

    // Flags for `shmat'.
    #define SHM_RDONLY  010000      // attach read-only else read-write
    #define SHM_RND     020000      // round attach address to SHMLBA
    #define SHM_REMAP   040000      // take-over region on attach

    // Commands for `shmctl'.
    #define SHM_LOCK    11          // lock segment (root only)
    #define SHM_UNLOCK  12          // unlock segment (root only)

    // Segment low boundary address multiple.
    #define SHMLBA      (tos_getpagesize ())


    // Type to count number of attaches.
    typedef unsigned long int shmatt_t;

    // Data structure describing a set of semaphores.
    struct shmid_ds
    {
        struct ipc_perm shm_perm;        // operation permission struct
        size_t shm_segsz;                // size of segment in bytes
        time_t shm_atime;                // time of last shmat()
        unsigned long int __unused1;
        time_t shm_dtime;                // time of last shmdt()
        unsigned long int __unused2;
        time_t shm_ctime;                // time of last change by shmctl()
        unsigned long int __unused3;
        pid_t shm_cpid;                  // pid of creator
        pid_t shm_lpid;                  // pid of last shmop
        shmatt_t shm_nattch;             // number of current attaches
        unsigned long int __unused4;
        unsigned long int __unused5;
    };

    // ipcs ctl commands
    #define SHM_STAT     13
    #define SHM_INFO     14

    // shm_mode upper byte flags
    #define SHM_DEST    01000   // segment will be destroyed on last detach
    #define SHM_LOCKED  02000   // segment will not be swapped
    #define SHM_HUGETLB 04000   // segment is mapped via hugetlb

    struct shminfo
    {
        unsigned long int shmmax;
        unsigned long int shmmin;
        unsigned long int shmmni;
        unsigned long int shmseg;
        unsigned long int shmall;
        unsigned long int __unused1;
        unsigned long int __unused2;
        unsigned long int __unused3;
        unsigned long int __unused4;
    };

    struct shm_info
    {
        int used_ids;
        unsigned long int shm_tot;    // total allocated shm
        unsigned long int shm_rss;    // total resident shm
        unsigned long int shm_swp;    // total swapped shm
        unsigned long int swap_attempts;
        unsigned long int swap_successes;
    };
    */

    int   tsm_shmctl(int    shmid, int     cmd,     struct shmid_ds *buf);
    int   tsm_shmget(key_t  key,   size_t  size,    int              shmflg);
    void *tsm_shmat (int    shmid, void   *shmaddr, int              shmflg);
    int   tsm_shmdt (void  *shmaddr);

#else

    #define tsm_shmctl shmctl
    #define tsm_shmget shmget
    #define tsm_shmat  shmat
    #define tsm_shmdt  shmdt

#endif

#ifdef __cplusplus
}
#endif


//----------------------------------------------------------------------------
// THE END
//============================================================================
#endif
