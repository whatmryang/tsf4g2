#ifndef _TREADINI_H
#define  _TREADINI_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef IN
#define  IN
#endif

#ifndef OUT
#define  OUT
#endif

#ifndef INOUT
#define INOUT
#endif



/*Ret:0 success*/
int tiniapi_read(	IN  const char    *a_pszFileName, 
					    	IN  const char      *a_pszGroupName,
			        	IN  const char      *a_pszIdentName, 
                OUT char            *a_pszResult,
                IN  int              a_iBufSize );



#ifdef __cplusplus
}
#endif


#endif
