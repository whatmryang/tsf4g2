/*
**  @file $RCSfile: tconvert.c,v $
**  general description of this module
**  $Id: tconvert.c,v 1.3 2009-06-04 02:14:32 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009-06-04 02:14:32 $
**  @version $Revision: 1.3 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

#include "comm/tconvert.h" 
static char HEX_value[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};


long tconvert_get_bytesl(const char *a_szStr, char** a_ppszEnd, int a_iRadix)
{
	char* szEnd;
	long lRet;
	long lMulti;

	assert(a_szStr);

	lRet = strtol(a_szStr, (char **) &szEnd, a_iRadix);

	if ( (lRet <= 0) || (LONG_MAX==lRet && ERANGE==errno) )
		return -1;

	switch (szEnd[0]) 
	{
	case 'K': 
	case 'k':
		lMulti	=	1024;
		break;
	case 'M':
	case 'm':
		lMulti	=	1024 * 1024;
		break;
	case 'G':
	case 'g':
		lMulti	= 1024 * 1024 * 1024;
		break;
	default:
		errno	=	EINVAL;

		return -1;
	}

	if( 'b'!=szEnd[1] && 'B'!=szEnd[1] )
	{
		errno	=	EINVAL;
		return -1;
	}

	if( LONG_MAX/lMulti < lRet )
	{
		errno	=	ERANGE;
		return -1;
	}

	lRet	*=	lMulti;

	if( a_ppszEnd ) *a_ppszEnd	=	szEnd+2;

	return lRet;
}

long tconvert_get_daysl(const char *a_szStr, char** a_ppszEnd, int a_iRadix)
{
	char* szEnd;
	long lRet;
	long lMulti;

	assert(a_szStr);

	lRet = strtol(a_szStr, (char **) &szEnd, a_iRadix);

	if ( (lRet <= 0) || (LONG_MAX==lRet && ERANGE==errno) )
		return -1;

	switch (szEnd[0]) 
	{
	case 'Y': 
	case 'y':
		lMulti	=	365;
		break;
	case 'M':
	case 'm':
		lMulti	=	30;
		break;
	case 'D':
	case 'd':
		lMulti	= 1;
		break;
	default:
		errno	=	EINVAL;

		return -1;
	}

	if( LONG_MAX/lMulti < lRet )
	{
		errno	=	ERANGE;
		return -1;
	}

	lRet	*=	lMulti;

	if( a_ppszEnd ) *a_ppszEnd	=	szEnd+2;

	return lRet;
}

char* Bin2Hex(unsigned char * pbin,int binlen, char* phex,int *hexlen)
{
	unsigned char  low,hi;
	int j=0,i=0;
	if (*hexlen < (binlen * 2+1))
	{
		return NULL;
	}
	while (i <= binlen )
	{
              low = pbin[i] & 0X0F;
              hi = pbin[i]>>4;
		phex[j++] = HEX_value[hi];
		phex[j++] = HEX_value[low];
		i++;
	}
	*hexlen = binlen * 2;
	phex[*hexlen]=0;
      return phex;
}

unsigned char* Hex2Bin(char * phex,int hexlen, unsigned char* pbin,int * binlen)
{

	int j=0;
	int i=0;
	if (hexlen > (*binlen) * 2)
	{
		return NULL;
	}
	while (j < hexlen )
	{
		if (phex[j] < 'A')
		{
			pbin[i] = phex[j++] -'0';
		}
		else
		{
		       pbin[i] = phex[j++] -'A' + 10;
		}

		pbin[i] <<=4;
		
		if (phex[j] < 'A')
		{
			pbin[i] |= phex[j++] -'0';
		}
		else
		{
		       pbin[i] |= phex[j++] -'A' + 10;
		}
		i++;
	}
	*binlen=  hexlen / 2;
	
       return pbin;
}





