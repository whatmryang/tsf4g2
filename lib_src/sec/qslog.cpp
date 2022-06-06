/*============================================================================*/
// File Name : qslog.cpp
// Purpose : QSpace System Log Function Sourcecode
// Function List :
//     void OpenQSLog( char* sModuleName );
//     void QSLog( int iPriority, char* sFormat, ... );
//     void CloseQSLog( void );
//     void QSLog_GetCurLogTime( char* psCurLogTime );
//     void QSLog_GetLocalTime( char* psCurTime );
// Notes :
//     Lane Zhang Created, 2003/02/28.
// Modify: 	toby 2006-04-20 
//		add trace log & auto create sub directory
/*============================================================================*/

#ifndef _QSLOG_CPP

#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h> ///< for pthread_*()
#include "qslog.h"

static int g_iLogLevel = 2;
//define log file 
static FILE* g_pfTraceLogFile = NULL;
static FILE* g_pfErrorLogFile = NULL;  		//Error ��־
static FILE* g_pfDebugLogFile = NULL;	//Debug��־
static FILE* g_pfInfoLogFile = NULL;    //Info��־
static FILE* g_pfBackupLogFile = NULL;  //���ڱ��ݵ���־
//define log path
static char g_sLogFilePath[128] = QSLOGPATH;

static char g_CurTraceLogFileName[12] = {0}; //��ǰTrace ��־�ļ���ʱ�䲿��
static char g_CurDebugLogFileName[12] = {0}; //��ǰDebug��־�ļ����Ƶ�ʱ�䲿��
static char g_CurInfoLogFileName[12] = {0};  //��ǰinfo��־�ļ����Ƶ�ʱ�䲿��
static char g_CurErrorLogFileName[12] = {0}; //��ǰerror��־�ļ�����
static char g_CurBackupLogFileName[12] = {0};

static char g_sModuleName[64] = "OTHER";
//thread_safe
static pthread_mutex_t mtMutexCAO ;

void QSLog_GetCurLogTime( char* psCurLogTime );
void QSLog_GetLocalTime( char* psCurTime );
void QSLog_CheckDir(const char * szPath);


	
/*============================================================================*/
// Function Name : OpenQSLog
// Purpose : Open QSLog File
// Notes : 
//     Lane Zhang Created, 2003/02/28
/*============================================================================*/

void OpenQSLog( const char* sModuleName, int iLogLevel, const char* sLogFilePath )
{
    if ( sModuleName != NULL )
    {
        strncpy( g_sModuleName, sModuleName, sizeof(g_sModuleName));
        g_sModuleName[63] = '\0';
    }
    g_iLogLevel = iLogLevel;
    if ( sLogFilePath != NULL )
    {
        strncpy( g_sLogFilePath, sLogFilePath, sizeof(g_sLogFilePath));
        g_sLogFilePath[127] = '\0';
    }
	
	QSLog_CheckDir(sLogFilePath);

    if ( pthread_mutex_init ( &mtMutexCAO, NULL ) != 0 ) { // init global mutex
		return;
    }
	else{
		return;
	}
    
}

/*============================================================================*/
// Function Name : QSLog
// Purpose : ��¼QQMail System Log
// Notes : 
//     Lane Zhang Created, 2003/02/28
//     One Process, No File Lock
/*============================================================================*/

void QSLog( int iPriority, char* sFormat, ... )
{
    char sLogFileFullName[256] = {0};
    char sCurLogTime[12] = {0};   //Current Log File Name, yyyymmddhh
    char sCurTime[24] = {0};
    va_list otherarg;
    FILE* pFile = NULL;

	//�����趨����־����ȷ���Ƿ����־
    if( g_iLogLevel <= 0 ){  //write none
        return;
    }
	else if(g_iLogLevel < iPriority){	
		return;
	}

try{

    QSLog_GetCurLogTime( sCurLogTime );
	pthread_mutex_lock ( &mtMutexCAO ) ;
	switch(iPriority)
	{
	case LOGTYPE_LOG_INFO:
		//get log file by time
        if ( strcmp( sCurLogTime, g_CurInfoLogFileName ) != 0 ){ 
        	if ( g_pfInfoLogFile != NULL ){
				fclose( g_pfInfoLogFile );
			}
			snprintf( sLogFileFullName,sizeof(sLogFileFullName),"%s/info/%s.log", g_sLogFilePath, sCurLogTime );
			g_pfInfoLogFile = fopen( sLogFileFullName, "a" );
			strcpy( g_CurInfoLogFileName, sCurLogTime );
        }
        pFile = g_pfInfoLogFile;

		break;
	case LOGTYPE_LOG_ERR:
		//get log file by time
		if ( strcmp( sCurLogTime, g_CurErrorLogFileName ) != 0 ){
			if ( g_pfErrorLogFile != NULL ){ 
			  	fclose( g_pfErrorLogFile );
			}
			snprintf( sLogFileFullName,sizeof(sLogFileFullName),"%s/error/%s.log", g_sLogFilePath, sCurLogTime );
			g_pfErrorLogFile = fopen( sLogFileFullName, "a" );
			strcpy( g_CurErrorLogFileName, sCurLogTime );  //switch log file
        }
        pFile = g_pfErrorLogFile;
		
		break;
	case LOGTYPE_LOG_DEBUG:
		//get log file name by time
		if ( strcmp( sCurLogTime, g_CurDebugLogFileName) != 0 ){  
			if ( g_pfDebugLogFile!= NULL ){
				fclose( g_pfDebugLogFile );
			}
			snprintf( sLogFileFullName,sizeof(sLogFileFullName), "%s/debug/%s.log", g_sLogFilePath, sCurLogTime );
			g_pfDebugLogFile = fopen( sLogFileFullName, "a" );
			strcpy( g_CurDebugLogFileName, sCurLogTime );  //swtich log file
        }
        pFile = g_pfDebugLogFile;
		
		break;
	case LOGTYPE_LOG_TRACE:
		//get log file name by time
		if ( strcmp( sCurLogTime, g_CurTraceLogFileName) != 0 ){  
			if ( g_pfTraceLogFile != NULL ){
				fclose( g_pfTraceLogFile);
			}
			snprintf( sLogFileFullName,sizeof(sLogFileFullName), "%s/trace/%s.log", g_sLogFilePath, sCurLogTime );
			g_pfTraceLogFile = fopen( sLogFileFullName, "a" );
			strcpy( g_CurTraceLogFileName, sCurLogTime );  //switch log file
        }
        pFile = g_pfTraceLogFile;
		
		break;
	default:	//unknow log type
		break;
	}
    pthread_mutex_unlock ( &mtMutexCAO ) ;
	//write log
    if ( pFile != NULL && !ferror(pFile) )
    {
        //Write Log
        char sLogHead[256];
        char sLogBuffer[1024];
        int liHeadLen = 0, liBodyLen = 0;
        
        QSLog_GetLocalTime( sCurTime );  //Now Time, yyyy-mm-dd hh:mm:ss
        
        va_start( otherarg, sFormat );
        //vsprintf( sLogBuffer, sFormat, otherarg );
        liBodyLen = vsnprintf( sLogBuffer, sizeof(sLogBuffer), sFormat, otherarg );
        va_end( otherarg );

        liHeadLen = snprintf( sLogHead, sizeof(sLogHead), "%s|%s(%d)|", sCurTime, g_sModuleName, getpid() );
		fprintf ( pFile, "%s%s\n", sLogHead, sLogBuffer ) ;

#if 0
        fwrite( sLogHead, 1, liHeadLen, pFile );
        fwrite( sLogBuffer, 1, liBodyLen, pFile );

        fwrite( "\n", 1, 1, pFile );
#endif

        fflush( pFile );
    }
}
catch(...){	//if error ,do nothing
	return;
}

    return;
}

/*============================================================================*/
// Function Name : QSBackupLog
// Purpose : Write QSBackupLog File
// Notes : 
//     Lane Zhang Created, 2003/02/28
//     One Process, No File Lock
/*============================================================================*/

void QSBackupLog( char* sFormat, ... )
{
    char sLogFileFullName[256] = {0};
    char sCurLogTime[12] = {0};   //Current Log File Name, yyyymmddhh
    char sCurTime[24] = {0};
    va_list otherarg;
    
    QSLog_GetCurLogTime( sCurLogTime );
    if ( strcmp( sCurLogTime, g_CurBackupLogFileName ) != 0 )
    {  //Open New Log File
        if ( g_pfBackupLogFile != NULL )
        {
            fclose( g_pfBackupLogFile );
            g_pfBackupLogFile = NULL;
        }
        sprintf( sLogFileFullName, "%s/%s.backup", g_sLogFilePath, sCurLogTime );
        g_pfBackupLogFile = fopen( sLogFileFullName, "a" );
        strcpy( g_CurBackupLogFileName, sCurLogTime );
    }
    if ( g_pfBackupLogFile != NULL )
    {  //Write Log
        char sLogHead[256];
        char sLogBuffer[1024*10];
        
        QSLog_GetLocalTime( sCurTime );  //Now Time, yyyy-mm-dd hh:mm:ss
        
        va_start( otherarg, sFormat );
        vsprintf( sLogBuffer, sFormat, otherarg );
        va_end( otherarg );
        
        sprintf( sLogHead, "%s ", sCurTime ); //Log Head
        fwrite( sLogHead, 1, strlen( sLogHead ), g_pfBackupLogFile );
        fwrite( sLogBuffer, 1, strlen( sLogBuffer ), g_pfBackupLogFile );
        fwrite( "\n", 1, 1, g_pfBackupLogFile );
        fflush( g_pfBackupLogFile );
    }
    
    return;
}

/*============================================================================*/
// Function Name : OpenQSLog
// Purpose : Open QSLog File
// Notes : 
//     Lane Zhang Created, 2003/02/28
/*============================================================================*/

void CloseQSLog( void )
{
    pthread_mutex_lock ( &mtMutexCAO ) ;
    if ( g_pfInfoLogFile != NULL ){
        fclose( g_pfInfoLogFile );
        g_pfInfoLogFile = NULL;
    }
	
	if( g_pfErrorLogFile != NULL){
		fclose(g_pfErrorLogFile);
		g_pfErrorLogFile = NULL;
	}
	
	if ( g_pfDebugLogFile!= NULL ){
        fclose( g_pfDebugLogFile);
        g_pfDebugLogFile = NULL;
    }
	
	if ( g_pfTraceLogFile != NULL){
		fclose( g_pfTraceLogFile);
		g_pfTraceLogFile = NULL;
	}
	
    if ( g_pfBackupLogFile != NULL ){
        fclose( g_pfBackupLogFile );
        g_pfBackupLogFile = NULL;
    }
	
    pthread_mutex_unlock ( &mtMutexCAO ) ;

    memset( g_CurDebugLogFileName, 0, 12 );
    memset( g_CurErrorLogFileName, 0, 12 );
    memset( g_CurTraceLogFileName, 0, 12 );	
    memset( g_CurInfoLogFileName, 0, 12 );
    memset( g_CurBackupLogFileName, 0, 12 );
    
    pthread_mutex_destroy ( &mtMutexCAO ) ;

    return;
}

/*============================================================================*/
// Function Name : QSLog_GetCurLogTime
// Purpose : Get Current Log File Name, It is the current time.
// Notes : 
//     Lane Zhang Created, 2003/02/28
/*============================================================================*/

void QSLog_GetCurLogTime( char* psCurLogTime )
{
    time_t tt = time( NULL );  //get systime
    struct tm *curtime = localtime( &tt );  //ת��ʱ���ʽ
    int year = 1900 + curtime->tm_year; //��1900�꿪ʼ������
    int month = curtime->tm_mon + 1;   //��0��ʼ������
    int day = curtime->tm_mday;   //��1��ʼ������
    int hour = curtime->tm_hour;  //��0��ʼ��Сʱ��

    sprintf( psCurLogTime, "%04d%02d%02d%02d", year, month, day, hour );
    return;
}

/*============================================================================*/
// Function Name : QSLog_GetLocalTime
// Purpose : Get System Current Time
// Notes : 
//     Lane Zhang Created, 2003/02/28
/*============================================================================*/
void QSLog_GetLocalTime( char* psCurTime )
{
    time_t tt = time( NULL );  //get systime
    struct tm *curtime = localtime( &tt );  //ת��ʱ���ʽ
    int year = 1900 + curtime->tm_year; //��1900�꿪ʼ������
    int month = curtime->tm_mon + 1;   //��0��ʼ������
    int day = curtime->tm_mday;   //��1��ʼ������
    int hour = curtime->tm_hour;  //��0��ʼ��Сʱ��
    int min = curtime->tm_min;  //��0��ʼ�ķ�����
    int sec = curtime->tm_sec;  //��0��ʼ������

    sprintf( psCurTime, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, sec );
    return;
}

/**
@ Function Name QSLog_CheckDir
@ Purpose: Check Sub dir,if not exist,create
*/
void QSLog_CheckDir(const char * szPath)
{
	if (NULL == szPath){
		return;
	}
	struct stat stBuf;
	char szDir[256] = {0};
	
	if(lstat(szPath,&stBuf) < 0){	//no such path, return
		return;
	}

	if(!S_ISDIR(stBuf.st_mode)){	//not a directory, return
		return;
	}

	//check sub dir
	snprintf(szDir,sizeof(szDir),"%s/info",szPath);
	if(lstat(szDir,&stBuf) < 0){	// sub directory not exist,create
		mkdir(szDir,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);
	}

	snprintf(szDir,sizeof(szDir),"%s/error",szPath);
	if(lstat(szDir,&stBuf) < 0){	//sub directory not exist,create
		mkdir(szDir,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);
	}

	snprintf(szDir,sizeof(szDir),"%s/debug",szPath);
	if(lstat(szDir,&stBuf) < 0){	//sub directory not exist,create
		mkdir(szDir,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);
	}

	snprintf(szDir,sizeof(szDir),"%s/trace",szPath);
	if(lstat(szDir,&stBuf) < 0){	//sub directory not exist,create
		mkdir(szDir,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);
	}
	
}


#endif //_QSLOG_C

/*===========================End Of File======================================*/
