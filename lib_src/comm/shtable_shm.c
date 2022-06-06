/*
**  @file $RCSfile: shtable_shm.c,v $
**  general description of this module
**  $Id: shtable_shm.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008-05-28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "comm/shtable_shm.h"
#include "shtable_i.h"

LPSHTABLE sht_create_shm(int iBucket, int iMax, int iUnit, key_t uiKey)
{
	HANDLE iShmID=(HANDLE)(-1);
	int iSize=0;
	void* pvAddr=NULL;
	LPSHTABLE pstTab=NULL;
	int iIsExist=0;

	iSize	=	SHT_SIZE(iBucket, iMax, iUnit);
	if( iSize<=(int)SHT_HEADSIZE() )
		return NULL;


	iShmID	=	shmget(uiKey, iSize, 0666 | IPC_CREAT | IPC_EXCL);

	if( iShmID < (HANDLE)0 )
	{
		iIsExist	=	1;
		iShmID		=	shmget(uiKey, iSize, 0666);
	}

	if( iShmID  < (HANDLE)0 )
		return NULL;

	pvAddr	=	shmat(iShmID, NULL, 0);

	if( (void *)(-1) == pvAddr )
		return NULL;

	if( iIsExist )
		pstTab	=	sht_attach(pvAddr, iSize, iBucket, iMax, iUnit);
	else
		pstTab	=	sht_init(pvAddr, iSize, iBucket, iMax, iUnit);

	if( !pstTab )
		shmdt(pvAddr);

	return pstTab;
}
