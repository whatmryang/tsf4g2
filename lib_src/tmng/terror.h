#ifndef _T_ERROR_H
#define _T_ERROR_H

#ifndef _T_ERROR_ENABLE

#define T_ERROR_PUSH(N, S) do { } while (0)
#define T_ERROR_POP() do { } while (0)
#define T_ERROR_CLEAR() do { } while (0)
#define T_ERROR_DUMP() do { } while (0)

#ifdef SINGLETON

#undef SINGLETON

#define TERROR 1
#define SINGLETON(T, F) 

#endif

#else

#include <stdio.h>

#define T_ERROR_MAX_SIZE   100
#define T_ERROR_MAX_LENGTH 255

typedef struct
{
    int  iCount;
    int  iErrNo[T_ERROR_MAX_SIZE];
    char szErrStr[T_ERROR_MAX_SIZE][T_ERROR_MAX_LENGTH];
} TERROR;

TERROR *singleton_TERROR(void);


#define T_ERROR_PUSH(N, S) \
    do { \
        TERROR *e = singleton_TERROR(); \
        if (e->iCount >= T_ERROR_MAX_SIZE) \
            break; \
        e->iErrNo[e->iCount] = (N); \
        snprintf(e->szErrStr[e->iCount], \
                 T_ERROR_MAX_LENGTH, \
                 "%s:%d(%s) %s", \
                 __FILE__, __LINE__, __FUNCTION__, \
                 NULL == (S) ? "..." : (S)); \
        ++e->iCount; \
    } while (0)

#define T_ERROR_POP() do { \
        TERROR *e = singleton_TERROR(); \
        if (e->iCount >= 0) \
            --e->iCount; \
    } while (0)

#define T_ERROR_CLEAR() do { \
        TERROR *e = singleton_TERROR(); \
        e->iCount = 0; \
    } while (0)

#define T_ERROR_DUMP() do { \
        TERROR *e = singleton_TERROR(); \
        int ie = e->iCount; \
        while (ie > 0) { \
            --ie; \
            printf("%d\t%s\n", e->iErrNo[ie], e->szErrStr[ie]); \
        } \
    } while (0)


#endif

#endif
