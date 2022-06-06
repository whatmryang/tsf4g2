/*
**  @file RCSfile
**  general description of this module
**  Id
**  @author Author
**  @date Date
**  @version Revision
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include "tapp/tapp.h"
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"

#include "logparser_desc.h"

#include "tlib_db.h"
#include "version.h"

typedef struct
{
	off_t			log_so;
	off_t			log_eo;
} logmatch_t;

typedef struct
{
	logmatch_t	lm_timestamp;
	logmatch_t	lm_busid;
	logmatch_t	lm_fileinfo;
	logmatch_t	lm_module;
	logmatch_t	lm_priority;
	logmatch_t	lm_message;
} logdata_t;

struct taglogparserEnv
{
	LOGPARSERCONF*				pstConf;
	LOGPARSER_RUN_CUMULATE*	pstRunCumu;
	LOGPARSER_RUN_STATUS*		pstRunStat;
	TLOGCATEGORYINST* 			pstLogCat;

	char 						szLogBuffer[1024 * 1024 * 2];
	TLIB_DB_LINK				stDBLink;
	
	logdata_t						stLogData;
};

typedef struct taglogparserEnv		LOGPARSERENV;
typedef struct taglogparserEnv*	LPLOGPARSERENV;


static TAPPCTX					gs_stAppCtx;
extern unsigned char				g_szMetalib_logparserconf[];

LOGPARSERENV					gs_stEnv;
static LOGPARSER_RUN_CUMULATE	gs_stCumu;
static LOGPARSER_RUN_STATUS		gs_stStat;


static int logparser_is_newline(const char* pszBuff)
{
	if (pszBuff[0] == '[')
	{
		return 1;
	}
	
	return 0;
}

static int logparser_resolve_brackets(const char* pszBuff, int iPos, logmatch_t *pstLogMatch)
{
	int	i;
	char	*ptrs, *ptre;
	
	pstLogMatch->log_so	= -1;
	pstLogMatch->log_eo	= -1;
	
	if (iPos < 0)
	{
		return -1;
	}
	
	ptre	= (char *)pszBuff;
	for (i = 0; i <= iPos; i++)
	{
		ptrs	= strchr(ptre, '[');
		if (!ptrs)
		{
			return -1;
		}
		
		ptre	= strchr(ptrs, ']');
		if (!ptre)
		{
			return -1;
		}
	}
	
	pstLogMatch->log_so	= (int)(ptrs - pszBuff) + 1;
	pstLogMatch->log_eo	= (int)(ptre - pszBuff);
	
	return 0;
}

static int logparser_resolve(LOGPARSERENV *pstEnv, const char* pszBuff)
{
	int			iRet;
	
	char			*end;
	char			szErrMsg[1024];
	
	if (!pszBuff || !pszBuff[0])
	{
		return -1;
	}
	
	if (-1 == logparser_resolve_brackets(pszBuff, 0, &pstEnv->stLogData.lm_timestamp))
	{
		return -1;
	}
	
	if (-1 == logparser_resolve_brackets(pszBuff, 1, &pstEnv->stLogData.lm_busid))
	{
		return -1;
	}
	
	if (-1 == logparser_resolve_brackets(pszBuff, 2, &pstEnv->stLogData.lm_fileinfo))
	{
		return -1;
	}
	
	if (-1 == logparser_resolve_brackets(pszBuff, 3, &pstEnv->stLogData.lm_module))
	{
		return -1;
	}
	
	if (-1 == logparser_resolve_brackets(pszBuff, 4, &pstEnv->stLogData.lm_priority))
	{
		return -1;
	}
	
	pstEnv->stLogData.lm_message.log_so = pstEnv->stLogData.lm_priority.log_eo + 1;
	pstEnv->stLogData.lm_message.log_eo = strlen(pszBuff);
	
	end = pstEnv->stDBLink.sQuery + sprintf(pstEnv->stDBLink.sQuery, "%s", "INSERT INTO tlogEvents(Time,Hostname,Fileinfo,Module,Priority,Message) values(");
	*end++ = '\'';
	end += mysql_real_escape_string(&pstEnv->stDBLink.pstCurMysqlConn->stMysql, end, pszBuff + pstEnv->stLogData.lm_timestamp.log_so, pstEnv->stLogData.lm_timestamp.log_eo - pstEnv->stLogData.lm_timestamp.log_so);
	*end++ = '\'';
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(&pstEnv->stDBLink.pstCurMysqlConn->stMysql, end, pszBuff + pstEnv->stLogData.lm_busid.log_so, pstEnv->stLogData.lm_busid.log_eo - pstEnv->stLogData.lm_busid.log_so);
	*end++ = '\'';
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(&pstEnv->stDBLink.pstCurMysqlConn->stMysql, end, pszBuff + pstEnv->stLogData.lm_fileinfo.log_so, pstEnv->stLogData.lm_fileinfo.log_eo - pstEnv->stLogData.lm_fileinfo.log_so);
	*end++ = '\'';
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(&pstEnv->stDBLink.pstCurMysqlConn->stMysql, end, pszBuff + pstEnv->stLogData.lm_module.log_so, pstEnv->stLogData.lm_module.log_eo - pstEnv->stLogData.lm_module.log_so);
	*end++ = '\'';
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(&pstEnv->stDBLink.pstCurMysqlConn->stMysql, end, pszBuff + pstEnv->stLogData.lm_priority.log_so, pstEnv->stLogData.lm_priority.log_eo - pstEnv->stLogData.lm_priority.log_so);
	*end++ = '\'';
	*end++ = ',';
	*end++ = '\'';
	end += mysql_real_escape_string(&pstEnv->stDBLink.pstCurMysqlConn->stMysql, end, pszBuff + pstEnv->stLogData.lm_message.log_so, pstEnv->stLogData.lm_message.log_eo - pstEnv->stLogData.lm_message.log_so);
	*end++ = '\'';
	*end++ = ')';
	*end++ = '\0';
	
	iRet	= TLib_DB_ExecSQL(&pstEnv->stDBLink, szErrMsg);
	if (0 > iRet)
	{
		printf ("%s\n", szErrMsg);
		return -1;
	}
	
	return kill(getpid(), 15);;
}
int logparser_init(TAPPCTX *pstAppCtx, LOGPARSERENV *pstEnv)
{
	char	szErrMsg[1024];
	
	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta || NULL == pstAppCtx->pszConfFile )
	{
		return -1;
	}
	
	pstEnv->pstConf = (LOGPARSERCONF *)pstAppCtx->stConfData.pszBuff;
	pstEnv->pstRunCumu = (LOGPARSER_RUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	if (NULL == pstEnv->pstRunCumu)
	{
		pstEnv->pstRunCumu = &gs_stCumu;
	}
	
	pstEnv->pstRunStat = (LOGPARSER_RUN_STATUS *)pstAppCtx->stRunDataStatus.pszBuff;
	if (NULL == pstEnv->pstRunStat)
	{
		pstEnv->pstRunStat = &gs_stStat;
	}
	
	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (int *)(&pstEnv->pstLogCat)))	
	{		
		printf("tapp_get_category run fail\n");
		return -1;	
	}
	
	TLib_DB_Init(&pstEnv->stDBLink, 1);
	if (TLib_DB_SetDB(&pstEnv->stDBLink, pstEnv->pstConf->szDBMSConnectionInfo, pstEnv->pstConf->szDBMSUser, pstEnv->pstConf->szDBMSPassword, pstEnv->pstConf->szDBMSCurDatabaseName, szErrMsg))	
	{		
		printf ("%s\n", szErrMsg);
		return -1;
	}
	
	return 0;
}

int logparser_proc(TAPPCTX *pstAppCtx, LOGPARSERENV *pstEnv)
{
	FILE*	fp;
	
	char		szBuffer[8192];
	size_t	iBufLen;
	
	int		iResult;
	
	pstEnv->szLogBuffer[0] = 0;
	
	fp		= fopen(pstEnv->pstConf->szLogFile, "r");
	if (!fp)
	{
		return -1;
	}
	
	while (!feof(fp))
	{
		memset(szBuffer,  0,  sizeof(szBuffer));    	
		if (!fgets((szBuffer), sizeof((szBuffer)), fp))
		{
			continue;
		}
		
		iBufLen	= strlen(szBuffer);
		iResult	= logparser_is_newline(szBuffer);
		
		if (iResult)
		{   
			logparser_resolve(pstEnv, pstEnv->szLogBuffer);

			strncpy(pstEnv->szLogBuffer, szBuffer, iBufLen);
			pstEnv->szLogBuffer[iBufLen] = '\0';
		}
		else
		{
			strncat(pstEnv->szLogBuffer, szBuffer, iBufLen);
		}
	}
    
	logparser_resolve(pstEnv, pstEnv->szLogBuffer);
	fclose (fp);
	
	return 0;
}

int logparser_reload(TAPPCTX *pstAppCtx, LOGPARSERENV *pstEnv)
{
	LOGPARSERCONF	*pstPreConf;
	
	pstPreConf 		= (LOGPARSERCONF *)pstAppCtx->stConfPrepareData.pszBuff;
	
	printf("logparser reload\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "logparser reload");

	return 0;
}

int logparser_fini(TAPPCTX *pstAppCtx, LOGPARSERENV *pstEnv)
{
	(void)pstAppCtx;
	
	TLib_DB_CloseCurDB(&pstEnv->stDBLink);
	
	return 0;
}

int logparser_tick(TAPPCTX *pstAppCtx, LOGPARSERENV *pstEnv)
{
	(void)pstAppCtx;
	(void)pstEnv;
	
	return 0;
}

int logparser_stop(TAPPCTX *pstAppCtx, LOGPARSERENV *pstEnv)
{
	(void)pstAppCtx;
	(void)pstEnv;
	
	return 0;
}

int main(int argc, char* argv[])
{
	int iRet;
	void* pvArg	=	&gs_stEnv;

	memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
	memset(&gs_stEnv, 0, sizeof(gs_stEnv));

	gs_stAppCtx.argc	=	argc;
	gs_stAppCtx.argv	=	argv;

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)logparser_init;
	
	gs_stAppCtx.iLib = (int)g_szMetalib_logparserconf;
	gs_stAppCtx.stConfData.pszMetaName = "logparserconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);
		
	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: app Initialization failed.\n");
		return iRet;
	}

	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)logparser_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)logparser_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)logparser_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)logparser_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)logparser_stop;
	
	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);
	return iRet;
}

