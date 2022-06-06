/**
*
* @file     tdr_tools.c
* @brief    一些辅助工具
*
* @author jackyai
* @version 1.0
* @date 2007-06-01
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "pal/pal.h"
#include "tdr_tools.h"
#include "../../lib_src/tdr/tdr_os.h"
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

char *tdr_parse_version_string(unsigned int unVer)
{
	static char szVersion[64];
	char szVer[32];
	sprintf(szVer,"%08d",unVer);

	sprintf(szVersion," %c.%c ",szVer[0],szVer[1]);
	if (szVer[2] == '0')
	{
		strcat(szVersion,"Alpha");
	}
	else if (szVer[2] == '1')
	{
		strcat(szVersion,"Beta");
	}
	else
	{
		strcat(szVersion,"Release");
	}
	sprintf(szVersion+strlen(szVersion),"%c%c Build%c%c%c",szVer[3],szVer[4],szVer[5],szVer[6],szVer[7]);


	return &szVersion[0];
}

const char *tdr_basename(const char *a_pszObj)
{
	const char *pch;

	pch = strrchr(a_pszObj, TDR_OS_DIRSEP);
	if (NULL == pch)
	{
		return (char *)a_pszObj;
	}
	pch ++;

	return pch;
}

const char *tdr_suffix(const char *a_pszObj)
{
    const char *pszSuf = NULL;

    assert(NULL != a_pszObj);
    pszSuf = strrchr(a_pszObj, '.');
    if (NULL == pszSuf)
    {
        return pszSuf;
    }else
    {
        return ++pszSuf;
    }
}

using std::stringstream;

string int2str(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

string int2str(size_t i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

int tdr_check_dir(const char* pszDir)
{
    assert(NULL != pszDir);

    if (0 < strlen(pszDir))
    {
        if (0 != tfexist(pszDir))
        {
            int iRet = tfmkdir(pszDir);
            if (0 != iRet)
            {
                fprintf(stderr, "Error: failed to create direcotry '%s'\n", pszDir);
                return -1;
            }
        }
    }

    return 0;
}

int tdr_make_path_file(char *pszFileBuff, int iBuff, const char *pszPath, const char *pszFile)
{
    int iLen;
    char *pch;

    assert(NULL != pszFileBuff);
    assert(NULL != pszPath);
    assert(NULL != pszFile);
    assert(0 < iBuff);

    if ('\0' == pszPath[0])
    {
        iLen = snprintf(pszFileBuff, iBuff, "%s", pszFile);
    }else
    {
        pch = (char*)tdr_get_filename(pszFile, strlen(pszFile));
        if (tdr_is_end_with_dir_sep(pszPath, strlen(pszPath)))
        {
            iLen = snprintf(pszFileBuff, iBuff,"%s%s", pszPath, pch);
        }else
        {
            iLen = snprintf(pszFileBuff, iBuff,"%s%c%s", pszPath, TDR_OS_DIRSEP, pch);
        }
    }

    if (iLen < 0)
    {
        printf("error: failed to make output-file-name\n");
        return -1;
    }
    if (iLen >= iBuff)
    {
        printf("error: 生成的文件名太长，目前最多支持%d个字符\n", iBuff-1);
        return -1;
    }

    return 0;
}

void tdr_add_suffix(string& filePath, const string& targetSuffix)
{
    const char* pszSuffix = tdr_suffix(filePath.c_str());

    if (NULL == pszSuffix)
    {
        filePath += targetSuffix;
    } else if (0 != strcmp(pszSuffix, targetSuffix.c_str()))
    {
        filePath = string(filePath.c_str(), pszSuffix - filePath.c_str()) + targetSuffix;
    }
}

string tdr_filepath_to_guard(const string& filePath)
{
    string guard = string("_") + tdr_basename(filePath.c_str()) + string("_");

    for (string::size_type i = 0; i < guard.length(); i++)
    {
        if ('.' == guard[i])
        {
            guard[i] = '_';
        }
    }

    return guard;
}
