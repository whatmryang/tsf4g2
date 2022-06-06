#ifndef TCOMMSHM_H
#define TCOMMSHM_H

#include "pal/pal.h"

#define TCOMMSHM_NOERROR 0
#define TCOMMSHM_ERROR_CHECK_FAIL (-2)




struct tagTcommshm;
typedef struct tagTcommshm *LPTCOMMSHM;
//Init the shm
int tcommshm_init(LPTCOMMSHM *ppstShm,char *pszName,size_t iSize,int *piCreated);

//int tcommshm_attach(LPTCOMMSHM *ppstShm,char *pszName,size_t iSize);
//Get the data and size of the shm
int tcommshm_remove(char *pszName);

int tcommshm_get(LPTCOMMSHM pstShm,void **pvData,size_t *iSize);
//Free the mem.
int tcommshm_fini(LPTCOMMSHM *pstShm);

#endif

