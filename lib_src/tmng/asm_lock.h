#ifndef _ASM_LOCK_H_
#define _ASM_LOCK_H_
//============================================================================
// @Id:       $Id: asm_lock.h 15076 2010-11-23 02:56:24Z flyma $
// @Author:   $Author: flyma $
// @Date:     $Date:: 2010-11-23 10:56:24 +0800 #$
// @Revision: $Revision: 15076 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------

#if !defined(_WIN32) && !defined(_WIN64)

static __inline__ int asm_lock_func(int *p_iLock, int i)
{
    register int iRet = i;

    __asm__ __volatile__(
        "lock; "
        "xchg %1, %0"
        : "=m" (*p_iLock)
        : "q"  (iRet)
    );

    return i != iRet;
}

#else

int asm_lock_func(int *p_iLock, int i);

#endif

#define ASM_LOCK(L)   asm_lock_func(&(L), 1)
#define ASM_UNLOCK(L) asm_lock_func(&(L), 0)


//----------------------------------------------------------------------------
// THE END
//============================================================================
#endif
