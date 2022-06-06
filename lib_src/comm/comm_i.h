#ifndef _COMM_I_H_
#define _COMM_I_H_

#include "comm/comm.h"

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) (&((TYPE*)0)->MEMBER))
#endif

#ifndef container
#define container(TYPE,MEMBER,pMember) (NULL == pMember ? NULL:((TYPE*)((size_t)(pMember)-offsetof(TYPE,MEMBER))))
#endif

#ifndef arraysize
#define  arraysize(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifndef NDEBUG
#define TASSERTRET(x,ret) assert(x)
#define TASSERT(x) assert(x)
#else
#define TASSERT(x) do{}while(0)
#define TASSERTRET(x,ret) {if(!(x)){ return (ret);}}
#endif

#define uint2korr(A)			(*((uint16_t *) (A)))
#define uint4korr(A)			(*((unsigned int *) (A)))

#define int2store(T,A)			*((uint16_t*) (T))= (uint16_t) (A)
#define int4store(T,A)			*((unsigned int *) (T))= (unsigned int) (A)

#endif
