#ifndef _TMNG_TSINGLETON_H_
#define _TMNG_TSINGLETON_H_

#include <stdlib.h>
#include <string.h>


#define SINGLETON(TYPE, INIT) \
    TYPE *singleton_##TYPE() \
    { \
        static TYPE *instance = NULL; \
        if (NULL == instance) { \
            instance = (TYPE *)malloc(sizeof(TYPE)); \
            if (NULL != instance) { \
                bzero(instance, sizeof(TYPE)); \
                int (*func)(TYPE *) = INIT; \
                if (NULL != func && 0 != (*func)(instance)) { \
                    free(instance); \
                    instance = NULL; \
                } \
            } \
        } \
        return instance; \
    }


#endif
