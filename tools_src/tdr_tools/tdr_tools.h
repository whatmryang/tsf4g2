#ifndef _TDR_TOOLS_H
#define _TDR_TOOLS_H
/**
*
* @file     tdr_tools.h
* @brief    һЩ��������
*
* @author jackyai
* @version 1.0
* @date 2007-06-01
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
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

/**����ʾ�汾��Ϣ������ת����Լ�����ַ���
*�汾:00101010
*�ֽ�:00        1        01              010
*�任:0.1       beta    01   build    010
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
