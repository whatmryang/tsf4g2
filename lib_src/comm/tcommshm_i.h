#ifndef TCOMMSHM_H_I
#define TCOMMSHM_H_I

#include "tcommshm.h"

#define TCOMMSHMMAGIC 142

struct tagTcommshm
{
	char szName[512];
	uint32_t iMagic;
	size_t iSize;
	size_t iRealSize;
	char checkSum;
	char szData[1];
};

typedef struct tagTcommshm TCOMMSHM;
#endif

