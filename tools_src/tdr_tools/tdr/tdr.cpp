/**
 *
 * @file     tdr.c
 * @brief    TDR���߽�Ԫ����������ת���ɶ������ļ���C�����ļ���h�����ļ�, as3���ļ�
 *
 * @author jackyai, flyma
 * @version 1.0
 * @date 2007-12-27
 *
 *
 * Copyright (c)  2010, ��Ѷ�Ƽ����޹�˾���������з���
 * All rights reserved.
 *
 */

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <iterator>
#include <algorithm>

using std::list;
using std::cout;
using std::endl;
using std::copy;
using std::string;
using std::getline;
using std::ofstream;
using std::ifstream;
using std::ostream_iterator;

#include "tdr/tdr.h"
#include "tdr_as3.h"
#include "tdr_cpp.h"
#include "tdr_csharp.h"
#include "tdr_option.h"
#include "../tdr_tools.h"
#include "version.h"
#include "../../lib_src/tdr/tdr_ctypes_info_i.h"
#include "../../lib_src/tdr/tdr_metalib_kernel_i.h"
#include "tdr_xml_desc.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#define  TDR_DEFAULT_DR_OUTPUT_FILE  "a.tdr"
#define  TDR_DEFAULT_DR_COUTPUT_FILE "a.c"
#define  TDR_DEFAULT_DR_TMPLT_FILE   "a.xml"

// functions for main-tdr-operations
static int tdr_xml2dr(TdrOption* pstOption, LPTDRMETALIB pstLib);
static int tdr_xml2c(TdrOption* pstOption, LPTDRMETALIB pstLib);
static int tdr_xml2h(TdrOption* pstOption, LPTDRMETALIB pstLib);
static int tdr_gen_tmplt(TdrOption* pstOption, LPTDRMETALIB pstLib);
static int tdr_xml2as3(TdrOption* pstOption, LPTDRMETALIB pstLib);
static int tdr_xml2cpp(TdrOption* pstOption, LPTDRMETALIB pstLib);
static int tdr_xml2csharp(TdrOption* pstOption, LPTDRMETALIB pstLib);
static int tdr_export_prefix(TdrOption* pstOption);

// utility functions
static int tdr_import_prefix(const char* pszFile);
static bool tdr_need_create_metalib(TdrOption* pstOption);
static int tdr_create_metalib(TdrOption* pstOption, LPTDRMETALIB* ppstLib);
static int tdr_set_outputfile(TdrOption* pstOption, const char* pszFileName);
static int tdr_read_one_string(ifstream& infile, string& outStr, bool isQuoted = true);

int main(int argc, char *argv[])
{
    int iRet = 0;

    TdrOption option;
    iRet = option.parse(argc, argv);
    if (0 != iRet || option.needExit)
    {
        return iRet;
    }

    LPTDRMETALIB pstLib = NULL;
    if (tdr_need_create_metalib(&option))
    {
        if (0 >= option.iXMLFileNum)
        {
            printf("error: ����ָ������һ��xml�����ļ�\n");
            return -1;
        }
        if (tdr_create_metalib(&option, &pstLib) < 0)
        {
            return -1;
        }
    }

    switch(option.enID)
    {
        case TdrOption::TDR_OPER_XML2DR:
            iRet = tdr_xml2dr(&option, pstLib);
            break;
        case TdrOption::TDR_OPER_XML2C:
            iRet = tdr_xml2c(&option, pstLib);
            break;
        case TdrOption::TDR_OPER_XML2H:
            iRet = tdr_xml2h(&option, pstLib);
            break;
        case TdrOption::TDR_OPER_GEN_TMPLT:
            iRet = tdr_gen_tmplt(&option, pstLib);
            break;
        case TdrOption::TDR_OPER_XML2AS3:
            iRet = tdr_xml2as3(&option, pstLib);
            break;
        case TdrOption::TDR_OPER_XML2CPP:
            iRet = tdr_xml2cpp(&option, pstLib);
            break;
        case TdrOption::TDR_OPER_XML2CSHARP:
            iRet = tdr_xml2csharp(&option, pstLib);
            break;
        case TdrOption::TDR_OPER_EXPORT_PREFIX_FILE:
            iRet = tdr_export_prefix(&option);
            break;
        default:
            break;
    }

    // it's safe to free pstLib even when pstLib == NULL
    tdr_free_lib(&pstLib);

    return iRet;
}

int tdr_xml2dr(TdrOption* pstOption, LPTDRMETALIB pstLib)
{
    int iRet = 0;

    if (!pstOption)
    {
        printf("error: 'pstOption' to tdr_xml2dr is NULL\n");
        return -1;
    }

    if (!pstLib)
    {
        printf("error: 'pstLib' to tdr_xml2dr is NULL\n");
        return -1;
    }

    /*����Ԫ���ݿ������*/
    if (!pstOption->pszOutFile)
    {
        iRet = tdr_set_outputfile(pstOption, TDR_DEFAULT_DR_OUTPUT_FILE);
        if (0 != iRet)
        {
            printf("error: failed to make output-file name\n");
            return -1;
        }
    }

    char szDRFile[TDR_MAX_FILE_LEN] = {0};
    const char *pszOutFile = pstOption->pszOutFile;

    if ('\0' != pstOption->szOutPath[0])
    {
        iRet = tdr_make_path_file(szDRFile, sizeof(szDRFile), pstOption->szOutPath, pstOption->pszOutFile);
        if (0 != iRet)
        {
            return -1;
        }
        pszOutFile = szDRFile;
    }


    iRet = tdr_save_metalib(pstLib, pszOutFile);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("error: ��Ԫ���ݿ�ʧ����Ϊ�������ļ�<%s>ʧ��, ��Ϊ: %s\n",
               pszOutFile, tdr_error_string(iRet));
        return -1;
    }else
    {
        printf("�ɹ���XML�ļ�ת���ɶ�����Ԫ�����ļ�<%s>\n", pszOutFile);
    }

    return iRet;
}

int tdr_xml2as3(TdrOption* pstOption, LPTDRMETALIB pstLib)
{
    int iRet = 0;

    if (!pstOption)
    {
        printf("error: 'pstOption' to tdr_xml2as3 is NULL\n");
        return -1;
    }

    if (!pstLib)
    {
        printf("error: 'pstLib' to tdr_xml2as3 is NULL\n");
        return -1;
    }

    string outputDir;
    if ('\0' == pstOption->szOutPath[0])
    {
        outputDir = tdr_get_metalib_name(pstLib);
    } else
    {
        outputDir = pstOption->szOutPath;
    }

    iRet = tdr_check_dir(outputDir.c_str());
    if (0 != iRet)
    {
        cout << "error: ���ܴ򿪻򴴽����Ŀ¼ '" << outputDir << "'" << endl;
        return -1;
    }

    try {
        TTransAS3 as3(string(pstOption->uiIndentSize, ' '), outputDir);
        as3.translate(pstLib);
        cout << "�ɹ�����ActionScript3���ļ���������Ŀ¼ '" << outputDir << "' �С�" << endl;
    } catch (const string& s)
    {
        cout << "error: " << s << endl;
        iRet = -1;
    } catch (...)
    {
        cout << "error: ����ActionsScript3���ļ�ʧ�ܣ�δ֪����" << endl;
        iRet = -1;
    }

    return iRet;
};

int tdr_xml2cpp(TdrOption* pstOption, LPTDRMETALIB pstLib)
{
    assert(NULL != pstOption);
    assert(pstOption->onlyCommFile || NULL != pstLib);

    string outputDir;
    if ('\0' == pstOption->szOutPath[0])
    {
        if (pstOption->onlyCommFile)
        {
            outputDir = "tdr_comm";
        } else
        {
            outputDir = tdr_get_metalib_name(pstLib);
        }
    } else
    {
        outputDir = pstOption->szOutPath;
    }

    int iRet = tdr_check_dir(outputDir.c_str());
    if (0 != iRet)
    {
        cout << "error: ���ܴ򿪻򴴽����Ŀ¼ '" << outputDir << "'" << endl;
        return -1;
    }

    /* ����ǰ׺�����ļ� */
    if (pstOption->stRule.iRule == TDR_HPPRULE_CUSTOMED_PREFIX)
    {
        iRet = tdr_import_prefix(pstOption->pszPrefixFile);
        if (iRet < 0)
        {
            return -1;
        }
    }

    try {
        list<XMLDesc> xmlDescList;
        XMLDesc::initXMLDesc(xmlDescList, pstOption->paszXMLFile, pstOption->iXMLFileNum);
        TTransCPP cpp(string(pstOption->uiIndentSize, ' '), outputDir);
        if (pstOption->needCommFile)
        {
            cpp.makeComm();
            cout << "�ɹ����ɹ��õ� C++ ���ļ���������Ŀ¼ '" << outputDir << "' �С�" << endl;
        }

        if (!pstOption->onlyCommFile)
        {
            cpp.translate(pstLib, xmlDescList, pstOption);
            cout << "�ɹ����� C++ ���ļ���������Ŀ¼ '" << outputDir << "' �С�" << endl;
        }
    } catch (const string& s)
    {
        cout << "error: " << s << endl;
        iRet = -1;
    } catch (...)
    {
        cout << "error: ���� C++ ���ļ�ʧ�ܣ�δ֪����" << endl;
        iRet = -1;
    }

    return iRet;

    return 0;
}

int tdr_xml2csharp(TdrOption* pstOption, LPTDRMETALIB pstLib)
{
    assert(NULL != pstOption);
    assert(NULL != pstLib);

    string outputDir;
    if ('\0' == pstOption->szOutPath[0])
    {
        outputDir = tdr_get_metalib_name(pstLib);
    } else
    {
        outputDir = pstOption->szOutPath;
    }

    int iRet = tdr_check_dir(outputDir.c_str());
    if (0 != iRet)
    {
        cout << "error: ���ܴ򿪻򴴽����Ŀ¼ '" << outputDir << "'" << endl;
        return -1;
    }

    try {
        list<XMLDesc> xmlDescList;
        XMLDesc::initXMLDesc(xmlDescList, pstOption->paszXMLFile, pstOption->iXMLFileNum);
        TTransCSharp cs(string(pstOption->uiIndentSize, ' '), outputDir);
        cs.translate(pstLib, xmlDescList, pstOption);
        cout << "�ɹ����� C# ���ļ���������Ŀ¼ '" << outputDir << "' �С�" << endl;
    } catch (const string& s)
    {
        cout << "error: " << s << endl;
        iRet = -1;
    } catch (...)
    {
        cout << "error: ���� C# ���ļ�ʧ�ܣ�δ֪����" << endl;
        iRet = -1;
    }

    return iRet;

    return 0;
}


int tdr_import_prefix(const char* pszFile)
{
    int iRet = 0;

    if (!pszFile)
    {
        printf("error: 'pszFile' to tdr_import_prefix is NULL\n");
        return -1;
    }

    ifstream infile(pszFile);
    if (infile.fail())
    {
        printf("error: ���ļ� '%s' ��ʧ��\n", pszFile);
        return -1;
    }

    unsigned int maxPrefixLen;

    string line;
    getline(infile, line);
    if (infile.fail())
    {
        printf("error: ǰ׺�����ļ���ʽ����\n");
        return -1;
    }

    string typeName;
    string sprefix;
    string mprefix;
    maxPrefixLen = tdr_get_prefix_max_len();
    while (!infile.eof())
    {
        // get TYPENAME
        if (tdr_read_one_string(infile, typeName, false) < 0)
        {
            if (!infile.eof())
            {
                printf("error: �������ļ��ж�ȡTYPE�ֶδ���\n");
                iRet = -1;
            }
            break;
        }

        // get PREFIX-FOR-SINGLE
        if (tdr_read_one_string(infile, sprefix, true) < 0)
        {
            printf("error: �������ļ��ж�ȡTYPE<%s>��RPEFIX-FOR-SINGLE�ֶ�ʧ��\n", typeName.c_str());
            iRet = -1;
            break;
        }
        if (sprefix.length() > maxPrefixLen)
        {
            printf("error :�����ļ���TYPE<%s>��PREFIX-FOR-SINGLE�ֶγ���<%"TDRPRI_SIZET">�������֧�ֳ���<%d>\n",
                   typeName.c_str(), sprefix.length(), maxPrefixLen);
            iRet = -1;
            break;
        }

        // get PREFIX-FOR-ARRAY
        if (tdr_read_one_string(infile, mprefix, true) < 0)
        {
            printf("error: �������ļ��ж�ȡTYPE<%s>��RPEFIX-FOR-ARRAY�ֶ�ʧ��\n", typeName.c_str());
            iRet = -1;
            break;
        }
        if (mprefix.length() > maxPrefixLen)
        {
            printf("error :�����ļ���TYPE<%s>��PREFIX-FOR-ARRAY�ֶγ���<%"TDRPRI_SIZET">�������֧�ֳ���<%d>\n",
                   typeName.c_str(), mprefix.length(), maxPrefixLen);
            iRet = -1;
            break;
        }

        // set sprefix and mprefix
        if (tdr_set_sprefix_by_name(typeName.c_str(), sprefix.c_str(), stderr) < 0)
        {
            printf("error: Ϊ����<%s>����PREFIX-FOR-SINGLEʱ����\n", typeName.c_str());
            iRet = -1;
            break;
        }
        if (tdr_set_mprefix_by_name(typeName.c_str(), mprefix.c_str(), stderr) < 0)
        {
            printf("error: Ϊ����<%s>����PREFIX-FOR-ARRAYʱ����\n", typeName.c_str());
            iRet = -1;
            break;
        }
        // one line for one TYPE, skip all other data
        getline(infile, line);
    }

    infile.close();

    return iRet;
}

int tdr_export_prefix(TdrOption* pstOption)
{
    int iRet = TDR_SUCCESS;

    if (!pstOption)
    {
        printf("error: 'pstOption' to tdr_export_prefix is NULL\n");
        return -1;
    }

    if (!pstOption->pszPrefixFile)
    {
        printf("error: ʹ�õ���ǰ׺���ñ�Ĺ��ܣ�����ָ��������Ŀ���ļ���\n");
        return -1;
    }

    ofstream outfile(pstOption->pszPrefixFile);
    if (outfile.fail())
    {
        printf("error: ���ļ� '%s' дʧ��\n", pstOption->pszPrefixFile);
        return -1;
    }

    string typeName;
    string sprefix;
    string mprefix;
    list<string> prefixLines;

    // add headline
    typeName = "TYPE";
    typeName += string(10-typeName.length(), ' ');
    sprefix = string("PREFIX-FOR-SINGLE");
    sprefix += string(22-sprefix.length(), ' ');
    mprefix = string("PREFIX-FOR-ARRAY");
    prefixLines.push_back(typeName + sprefix + mprefix);

    // construct each line for output
    unsigned int tableSize = tdr_get_prefix_table_size();
    for (unsigned j = 0; j < tableSize; j++)
    {
        typeName = tdr_get_typename_by_index(j, stderr);
        typeName += string(16-typeName.length(), ' ');
        sprefix = string("\"") + tdr_get_sprefix_by_index(j, stderr);
        sprefix += "\"";
        sprefix += string(16-sprefix.length(), ' ');
        mprefix = string("\"") + tdr_get_mprefix_by_index(j, stderr);
        mprefix += "\"";
        prefixLines.push_back(typeName + sprefix + mprefix);
    }

    // output into file
    ostream_iterator<string> outIter(outfile, "\n");
    copy(prefixLines.begin(), prefixLines.end(), outIter);
    if (outfile.fail())
    {
        printf("error: д���ļ� %s ʧ��\n", pstOption->pszPrefixFile);
        return -1;
    } else
    {
        printf("�ɹ�����ǰ׺���ñ��ļ�%s��\n", pstOption->pszPrefixFile);
    }
    outfile.close();

    return iRet;
}

int tdr_gen_tmplt(TdrOption* pstOption, LPTDRMETALIB pstLib)
{
    int iRet = 0;

    if (!pstOption)
    {
        printf("error: 'pstOption' to tdr_gen_tmplt is NULL\n");
        return -1;
    }

    if (!pstLib)
    {
        printf("error: 'pstLib' to tdr_gen_tmplt is NULL\n");
        return -1;
    }

    if (!pstOption->pszMetaName)
    {
        printf("error: ʹ����������ģ��Ĺ��ܱ���ָ��Ԫ������\n");
        return -1;
    }

    LPTDRMETA pstMeta = tdr_get_meta_by_name(pstLib, pstOption->pszMetaName);
    if (NULL == pstMeta)
    {
        printf("error: ��Ԫ���ݿ��л�ȡ %s ��������Ϣʱ����\n", pstOption->pszMetaName);
        return -1;
    }

    // check cutoffversion
    if (TDR_MAX_VERSION < pstOption->iVersion)
    {
        printf("error: ָ���İ汾<%d>���ڵ�ǰTDR֧�ֵ���߰汾<%d>\n",
                pstOption->iVersion, TDR_MAX_VERSION);
        return -1;
    }
    if (0 == pstOption->iVersion)
    {
        pstOption->iVersion = (unsigned)pstMeta->iCurVersion;
    }
    if ((int)(pstOption->iVersion) < pstMeta->iBaseVersion)
    {
        printf("warning: ָ���İ汾Ϊ%d, С��ָ��Ԫ���ݵĻ����汾%d, ��ʹ��ָ��Ԫ���ݵ���߰汾 %d ���������ļ�\n",
                (int)(pstOption->iVersion), pstMeta->iBaseVersion, (int)pstMeta->iCurVersion);
        pstOption->iVersion = (unsigned)pstMeta->iCurVersion;
    }

    // output formatted XML template
    if (!pstOption->pszOutFile)
    {
        iRet = tdr_set_outputfile(pstOption, TDR_DEFAULT_DR_TMPLT_FILE);
        if (0 != iRet)
        {
            printf("error: failed to make output-file name\n");
            return -1;
        }
    }

    FILE *fpDest = fopen(pstOption->pszOutFile, "wb");
    if (NULL == fpDest)
    {
        printf("error: ���ļ� %s д����\n", pstOption->pszOutFile);
        return -1;
    }

    if (NULL != pstOption->pszEncoding)
    {
        tdr_set_encoding(pstOption->pszEncoding);
    }

    iRet = tdr_gen_xmltmplt(pstMeta, fpDest, pstOption->iVersion, pstOption->outputXmlFileFormat);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("error: Ϊ %s ����ģ��ʱ����: %s\n", pstOption->pszMetaName, tdr_error_string(iRet));
        return -1;
    }else
    {
        printf("�ɹ�Ϊ %s ����ģ���ļ� %s\n", pstOption->pszMetaName, pstOption->pszOutFile);
    }
    fclose(fpDest);

    return iRet;
};

int tdr_xml2c(TdrOption* pstOption, LPTDRMETALIB pstLib)
{
    int iRet = TDR_SUCCESS;

    if (!pstOption)
    {
        printf("error: 'pstOption' to tdr_xml2c is NULL\n");
        return -1;
    }

    if (!pstLib)
    {
        printf("error: 'pstLib' to tdr_xml2c is NULL\n");
        return -1;
    }

    /*����Ԫ���ݿ������*/
    if (!pstOption->pszOutFile)
    {
        iRet = tdr_set_outputfile(pstOption, TDR_DEFAULT_DR_COUTPUT_FILE);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            printf("error: failed to make output-file name\n");
            return -1;
        }
    }

    char szCFile[TDR_MAX_FILE_LEN] = {0};
    const char *pszOutFile = pstOption->pszOutFile;

    if ('\0' != pstOption->szOutPath[0])
    {
        iRet = tdr_make_path_file(szCFile, sizeof(szCFile), pstOption->szOutPath, pstOption->pszOutFile);
        if (0 != iRet)
        {
            return -1;
        }
        pszOutFile = szCFile;
    }

    iRet = tdr_metalib_to_cfile(pstLib, pszOutFile);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("error: ��XML�����ļ�ת����c�ļ�<%s>ʧ�ܣ���Ϊ: %s\n", pszOutFile, tdr_error_string(iRet));
    }else
    {
        printf("�ɹ���XML�����ļ�ת����c�ļ�<%s>\n", pszOutFile);
    }


    return iRet;
}

int tdr_xml2h(TdrOption* pstOption, LPTDRMETALIB pstLib)
{
    int iRet = 0;

    if (!pstOption)
    {
        printf("error: 'pstOption' to tdr_xml2h is NULL\n");
        return -1;
    }

    if (!pstLib)
    {
        printf("error: 'pstLib' to tdr_xml2h is NULL\n");
        return -1;
    }

    char szHFile[TDR_MAX_FILE_LEN] = {0};
    int i;

    assert(NULL != pstOption);

    /* ����ǰ׺�����ļ� */
    if (pstOption->stRule.iRule == TDR_HPPRULE_CUSTOMED_PREFIX)
    {
        iRet = tdr_import_prefix(pstOption->pszPrefixFile);
        if (iRet < 0)
        {
            return -1;
        }
    }

    for (i = 0 ; i < pstOption->iXMLFileNum; i++)
    {
        char* pszSuffix;

        strncpy(pstOption->szOutFile, pstOption->paszXMLFile[i], sizeof(pstOption->szOutFile));
        pstOption->szOutFile[sizeof(pstOption->szOutFile)-1] = 0;
        if (strlen(pstOption->paszXMLFile[i])+3 > sizeof(pstOption->szOutFile))
        {
            printf("error: ��������ļ���ʧ�ܣ��ļ��� \"%s\" ����<%d>����֧�ֵ���󳤶�<%d>\n",
                    pstOption->paszXMLFile[i], (int)strlen(pstOption->paszXMLFile[i]),
                    (int)sizeof(pstOption->szOutFile)-3);
            return -1;
        }
        pszSuffix = (char*)tdr_get_pathname_suffix(pstOption->szOutFile, strlen(pstOption->szOutFile));
        if (pszSuffix)
        {
            *++pszSuffix = 'h';
            *++pszSuffix = 0;
        }else
        {
            strcat(pstOption->szOutFile, ".h");
        }

        iRet = tdr_make_path_file(szHFile, sizeof(szHFile), pstOption->szOutPath, pstOption->szOutFile);
        if (0 != iRet)
        {
            return -1;
        }

        iRet = tdr_metalib_to_hpp_spec(pstLib, (const char *) pstOption->paszXMLFile[i],
                                       pstOption->iTagSetVersion, szHFile, &pstOption->stRule, stderr);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }else
        {
            printf("�ɹ���XML�����ļ�<%s>ת����ͷ�ļ�<%s>\n", pstOption->paszXMLFile[i], szHFile);
        }

    }/*for (i = 0 ; i < stOption.iXMLFileNum; i++)*/


    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("error: ��XML�����ļ�<%s>ת����ͷ�ļ�ʧ��: %s\n", pstOption->paszXMLFile[i], tdr_error_string(iRet));
    }

    return iRet;
}

void tdr_get_version(string& outStr)
{
    outStr = int2str(MAJOR)
        + "." + int2str(MINOR)
        + "." + int2str(REV)
        + ", build at " + int2str(BUILD);
}

void tdr_output_version_info(const char* exeName)
{
    printf("%s:version %d.%d.%d  build at %d.\n", exeName, MAJOR,
           MINOR, REV, BUILD);
}

int tdr_read_one_string(ifstream& infile, string& outStr, bool isQuoted)
{
    int iRet = 0;

    if (infile.eof() || infile.fail())
    {
        return -1;
    }

    infile >> outStr;
    if (infile.fail())
    {
        return -1;
    }

    if (!isQuoted)
    {
        return 0;
    } else
    {
        size_t begPos;
        size_t endPos;
        begPos = outStr.find_first_of("\"");
        if (begPos == string::npos)
        {
            return -1;
        }
        begPos++;
        endPos = outStr.find_first_of("\"", begPos);
        if (endPos == string::npos)
        {
            return -1;
        }
        outStr = outStr.substr(begPos, endPos - begPos);

        return 0;
    }

    return iRet;
}

bool tdr_need_create_metalib(TdrOption* pstOption)
{
    if (TdrOption::TDR_OPER_EXPORT_PREFIX_FILE == pstOption->enID)
    {
        return false;
    }

    if (TdrOption::TDR_OPER_XML2CPP == pstOption->enID && pstOption->onlyCommFile)
    {
        return false;
    }

    return true;
}

int tdr_create_metalib(TdrOption* pstOption, LPTDRMETALIB* ppstLib)
{
    int iRet = 0;

    if (0 >= pstOption->iXMLFileNum)
    {
        printf("error: no xml-file was specified for creating metalib\n");
        return -1;
    }

    *ppstLib = NULL;
    switch (pstOption->enID)
    {
        case TdrOption::TDR_OPER_XML2DR:
        case TdrOption::TDR_OPER_XML2C:
        case TdrOption::TDR_OPER_XML2H:
        case TdrOption::TDR_OPER_XML2AS3:
        case TdrOption::TDR_OPER_XML2CPP:
        case TdrOption::TDR_OPER_XML2CSHARP:
            {
                iRet = tdr_create_lib_multifile(ppstLib, (const char **)pstOption->paszXMLFile,
                                                pstOption->iXMLFileNum, pstOption->iTagSetVersion, stderr);
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    printf("error: ����xml�����ļ�����Ԫ���ݿ�ʧ��: %s\n", tdr_error_string(iRet));
                    iRet = -1;
                    break;
                }
            }
            break;
        case TdrOption::TDR_OPER_GEN_TMPLT:
            {
                const char* pszSuffix = tdr_suffix(pstOption->paszXMLFile[0]);
                if ((NULL != pszSuffix) && (0 == strcmp(pszSuffix, "tdr")))
                {
                    iRet = tdr_load_metalib(ppstLib, pstOption->paszXMLFile[0]);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        printf("error: ����Ԫ���ݿ� %s ʧ��: %s\n",
                               pstOption->paszXMLFile[0],
                               tdr_error_string(iRet));
                        iRet = -1;
                        break;
                    }
                }else
                {
                    iRet = tdr_create_lib_multifile(ppstLib, (const char **)pstOption->paszXMLFile,
                                                    pstOption->iXMLFileNum, pstOption->iTagSetVersion, stderr);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        printf("error: ����xml�����ļ�����Ԫ���ݿ�ʧ��: %s\n", tdr_error_string(iRet));
                        iRet = -1;
                        break;
                    }
                }
            }
            break;
        default:
            {
                printf("error: no need to create metalib for pstOption->enID<%d>\n", pstOption->enID);
                iRet = -1;
            }
            break;
    }

    if (iRet < 0)
    {
        return iRet;
    }

    if (pstOption->needDumpLib)
    {
        tdr_dump_metalib_file(*ppstLib, "dump_lib.txt");
    }

    return iRet;
}

int tdr_set_outputfile(TdrOption* pstOption, const char* pszFileName)
{
    int iRet = 0;

    if (!pstOption)
    {
        printf("error: 'pstOption' to tdr_set_outputfile is NULL\n");
        return -1;
    }

    if (!pszFileName)
    {
        printf("error: 'pszFileName' to tdr_set_outputfile is NULL\n");
        return -1;
    }

    if (!pstOption->pszOutFile)
    {
        char szOutFile[TDR_MAX_FILE_LEN];

        if (strlen(pszFileName) > sizeof(szOutFile) - 1)
        {
            printf("error: ָ��������ļ�������<%"TDRPRI_SIZET">����֧�ֵ���󳤶�<%"TDRPRI_SIZET">\n",
                   strlen(pszFileName), sizeof(szOutFile) - 1);
            return -1;
        }

        strncpy(szOutFile, pszFileName, sizeof(szOutFile));
        szOutFile[sizeof(szOutFile)-1] = 0;

        iRet = tdr_make_path_file(pstOption->szOutFile, sizeof(pstOption->szOutFile),
                pstOption->szOutPath, szOutFile);
        if (0 != iRet)
        {
            return -1;
        }
        pstOption->pszOutFile = pstOption->szOutFile;
    }

    return iRet;
}
