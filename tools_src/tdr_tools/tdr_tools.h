#ifndef _TDR_TOOLS_H
#define _TDR_TOOLS_H
/**
*
* @file     tdr_tools.h
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

#ifndef TDR_OS_DIRSEP
#if defined(_WIN32) || defined(_WIN64)
#define TDR_OS_DIRSEP		'\\'
#else
#define TDR_OS_DIRSEP		'/'
#endif
#endif

/**将表示版本信息的整数转换成约定的字符串
*版本:00101010
*分解:00        1        01              010
*变换:0.1       beta    01   build    010
*/

#include <string>

using std::string;

string int2str(int i);
string int2str(size_t i);
void tdr_get_version(string& outStr);

char *tdr_parse_version_string(unsigned int unVer);

const char *tdr_basename(const char *a_pszObj);

const char *tdr_suffix(const char *a_pszObj);

void tdr_output_version_info(const char* exeName);

int tdr_make_path_file(char *pszFileBuff, int iBuff,
                       const char *pszPath, const char *pszFile);

int tdr_check_dir(const char* pszDir);

void tdr_add_suffix(string& filePath, const string& targetSuffix);

string tdr_filepath_to_guard(const string& filePath);

#endif
