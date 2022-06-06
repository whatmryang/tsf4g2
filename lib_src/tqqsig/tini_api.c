#include "tqqsig/tini_api.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>



#if defined (_WIN32) || defined (_WIN64)
     #define snprintf _snprintf
#endif

#define TINIAPI_LINELENGTH 		4096
#define TINIAPI_LEFTSQUOTE 		'['
#define TINIAPI_RIGHTQUOTE 		']'
#define TINIAPI_CHARDIVIDE		'#'
#define TINIAPI_CHARESCAPE		'\\'
#define TINIAPI_CHARASSIGN		'='
#define TINIAPI_B_TRUE      	1
#define TINIAPI_B_FALSE     	0



/************************************************************
 *    desc:    Replace the Char Escaped by '\\' with SpecialChar.
 *        \b,\t,\n,\r,\TINI_CHARDIVIDE,\\,...
 *    rett:    void.
 ************************************************************/
void    tiniapi_escch( char * sOprt, int bDivide )
{
    char       * sSrc;
    char       * sDst;
    int        nModified;

    if( NULL == sOprt )
    {
        return ;
    }

    sSrc = sDst = sOprt;
    nModified = 0;

    while( '\0' != *sSrc )
    {
        if( bDivide && TINIAPI_CHARDIVIDE == *sSrc )
        {
            *sDst = '\0';
            return ;
        }
        else if( '\\' != *sSrc && 0 == nModified )
        {
            sSrc++;
            sDst++;
        }
        else if( '\\' != *sSrc && 0 != nModified )
        {
            *sDst = *sSrc;
            sSrc++;
            sDst++;
        }
        else
        {
            switch( *(sSrc+1) )
            {
            case    'b':
                *sDst = '\b';
                sSrc += 2;
                sDst += 1;
                break;
            case    'n':
                *sDst = '\n';
                sSrc += 2;
                sDst += 1;
                break;
            case    'r':
                *sDst = '\r';
                sSrc += 2;
                sDst += 1;
                break;
            case    't':
                *sDst = '\t';
                sSrc += 2;
                sDst += 1;
                break;
	     case  '\\':
		  *sDst = '\\';
                sSrc += 2;
                sDst += 1;
		  break;
            case    TINIAPI_CHARDIVIDE:
                *sDst = TINIAPI_CHARDIVIDE;
                sSrc += 2;
                sDst += 1;
                break;
            case    '\0':
                sSrc += 1;
                break;

            default:
                sSrc += 1;
                break;
            }
            nModified += 1;
        }
    }    /*    end of while()_Loop */
    *sDst = '\0';

    return ;
}




/************************************************************
 *    desc:    Left Trim the String inputed.
 ************************************************************/
void    tiniapi_trimleft( char * sOprt )
{
    int nHead, nCurr;

    if( NULL == sOprt )
    {
        return ;
    }

    nHead = 0;
    nCurr = 0;
    while( 0 == ( 0x80 & (unsigned int)(unsigned char)(*(sOprt+nCurr)) ) &&
        0 != isspace((int)*(sOprt+nCurr)) )
    {
        nCurr++;
    }
    while( '\0' != (*(sOprt+nCurr)) )
    {
        *(sOprt+nHead) = *(sOprt+nCurr);
        nHead++;
        nCurr++;
    }
    *(sOprt+nHead) = *(sOprt+nCurr);

    return ;
}


/************************************************************
 *    desc:    Right Trim the String inputed.
 ************************************************************/
void    tiniapi_trimright( char * sOprt )
{
    char *    sTail;

    if( NULL == sOprt )
        return ;

    if( 0 == (int)strlen(sOprt) )
    {
        return ;
    }
    sTail = sOprt+strlen(sOprt)-1;
    while( 0 == ( 0x80 & (unsigned int)(unsigned char)(*sTail) ) &&
            0 != isspace((int)*sTail) && sTail >= sOprt )
    {
        *sTail = '\0';
        sTail--;
    }

    return ;
}

/************************************************************
 *    desc:    Trim both end of the String inputed.
 ************************************************************/
void    tiniapi_trimall( char * sOprt )
{
    if( NULL == sOprt )
    {
        return ;
    }
    tiniapi_trimright( sOprt );
    tiniapi_trimleft( sOprt );
    return ;
}


/************************************************************
 *    desc:    read the Item configed in the IniFile with String Value.
 ************************************************************/
int tiniapi_read(	IN  const char    *a_pszFileName,
					    	  IN  const char      *a_pszGroupName,
			        	  IN  const char      *a_pszIdentName,
                  OUT char            *a_pszResult,
                  IN  int              a_iBufSize )
{
    FILE    *fp;
    char    szGName[ TINIAPI_LINELENGTH ];
    char    szIName[ TINIAPI_LINELENGTH ];
    char    szIdent[ TINIAPI_LINELENGTH ];
    char    szGroup[ TINIAPI_LINELENGTH ];
    char    *pszTmp;
    int     iTmp;
    int     iLen;

    if( NULL == a_pszFileName ||
        NULL == a_pszGroupName ||
        NULL == a_pszIdentName ||
        NULL == a_pszResult )
    {
        return -1;
    }

    fp = fopen( a_pszFileName, "r" );
    if( NULL == fp )
    {
        return -2;
    }

    strcpy( a_pszResult, "" );
    pszTmp = NULL;
    snprintf( szIName, sizeof(szIName), "%s", a_pszGroupName );
    tiniapi_trimall( szIName );
    snprintf( szGName, sizeof(szGName), "[%s]", szIName );
    snprintf( szIName, sizeof(szIName), "%s", a_pszIdentName );
    tiniapi_trimall( szIName );
    if( 0 == strlen(szGName) || 0 == strlen(szIName) )
    {
        fclose( fp );
        return -3;
    }

    /*不区分大小写*/
    iLen = (int)strlen(szGName);
    for( iTmp=0; iTmp<iLen; iTmp++ )
    {
       szGName[iTmp] = (char)toupper(szGName[iTmp]);
    }
    iLen = (int)strlen(szIName);
    for( iTmp=0; iTmp<iLen; iTmp++ )
    {
       szIName[iTmp] = (char)toupper(szIName[iTmp]);
    }


    /*    Find Group    */
    iLen = (int)strlen(szGName);
    while( 1 )
    {
        if( NULL == fgets(szGroup,sizeof(szGroup),fp) )
        {
            fclose( fp );
            return -4;
        }
        tiniapi_escch( szGroup, TINIAPI_B_TRUE );
        tiniapi_trimall( szGroup );
        if( (int)strlen(szGroup) != iLen )
        {
            continue;
        }
        for( iTmp=0; iTmp<iLen; iTmp++ )
        {
            szGroup[iTmp] = (char)toupper(szGroup[iTmp]);
        }
        if( 0 == strcmp(szGroup,szGName) )
        {
            break;    /*    Group found */
        }
    }

    /*   Find Ident */
    iLen = (int)strlen(szIName);
    while( 1 )
    {
        if( NULL == fgets(szIdent,sizeof(szIdent),fp) )
        {
            fclose( fp );
            return -5;
        }
        if( '\n' == szIdent[strlen(szIdent)-1] )
        {
            szIdent[strlen(szIdent)-1] = '\0';
        }

        tiniapi_escch( szIdent, TINIAPI_B_TRUE );
        tiniapi_trimleft( szIdent );

        for( iTmp=0; iTmp<iLen; iTmp++ )
        {
            szIdent[iTmp] = (char)toupper(szIdent[iTmp]);
        }

        if( 0 == strncmp(szIdent,szIName,iLen) )
        {
            pszTmp = szIdent+iLen;
            while( isspace(*pszTmp) )
            {
                pszTmp++;
            }
            if( TINIAPI_CHARASSIGN != *pszTmp )
            {
                continue;
            }
            else
            {
                pszTmp++;    //    goto Content
                break;
            }
        }
    }

    fclose( fp );

    tiniapi_trimall( pszTmp );
    snprintf( a_pszResult, a_iBufSize-1, "%s", pszTmp );

    return 0;
}
