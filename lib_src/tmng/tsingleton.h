#ifndef _TSINGLETON_H_
#define _TSINGLETON_H_
//============================================================================
// @Id:       $Id: tsingleton.h 10522 2010-07-20 02:22:08Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-07-20 10:22:08 +0800 #$
// @Revision: $Revision: 10522 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#define TSINGLETON_VAR(T, N) g_s_tsingleton_T_##T##_N_##N
#define TSINGLETON_PTR(T, N) g_p_tsingleton_T_##T##_N_##N
#define TSINGLETON_SET(T, N) g_i_tsingleton_T_##T##_N_##N
#define TSINGLETON_GET(T, N) g_f_tsingleton_T_##T##_N_##N


#define TSINGLETON_H(T, N) T *TSINGLETON_GET(T, N)()

#define TSINGLETON_C(T, N, I) \
    static T     TSINGLETON_VAR(T, N); \
    static T    *TSINGLETON_PTR(T, N) = (T *)0; \
    static int (*TSINGLETON_SET(T, N))(T *) = (I); \
    \
    T *TSINGLETON_GET(T, N)() \
    { \
        if ((T *)0 == TSINGLETON_PTR(T, N)) \
        { \
            if ((void *)0 == (void *)TSINGLETON_SET(T, N) || \
                0 == TSINGLETON_SET(T, N)(&TSINGLETON_VAR(T, N))) \
            { \
                TSINGLETON_PTR(T, N) = &TSINGLETON_VAR(T, N); \
            } \
        } \
        return TSINGLETON_PTR(T, N); \
    }

#define TSINGLETON(T, N) TSINGLETON_GET(T, N)()


//----------------------------------------------------------------------------
// THE END
//============================================================================
#endif
