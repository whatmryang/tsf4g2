#include <ctime>
#include <string>
#include <cassert>
#include <iostream>
#include "tdr/tdr.h"
#include "tdr_csharp.h"
#include "../tdr_tools.h"
#include "tdr_debug.h"
#include "../../lib_src/tdr/tdr_md5.h"
#include "../../lib_src/tdr/tdr_ctypes_info_i.h"
#include "../../lib_src/tdr/tdr_metalib_kernel_i.h"
#include "../../lib_src/tdr/tdr_metalib_manage_i.h"


#define TSF4G_TDR_CS_DEBUG_TRACE(lines,indentLevel)\
    do{\
        lines.push_back("#if (DEBUG)");\
        lines.push_back(getIndentStr(indentLevel) + "StackTrace st = new StackTrace(true);");\
        lines.push_back(getIndentStr(indentLevel) + "for (int i = 0; i < st.FrameCount; i++)");\
        lines.push_back(getIndentStr(indentLevel) + "{");\
        lines.push_back(getIndentStr(indentLevel + 1) + "if (null != st.GetFrame(i).GetFileName())");\
        lines.push_back(getIndentStr(indentLevel + 1) + "{");\
        lines.push_back(getIndentStr(indentLevel + 2) + "Console.WriteLine(\"TSF4G_TRACE:  \" + st.GetFrame(i).ToString());");\
        lines.push_back(getIndentStr(indentLevel +1) + "}");\
        lines.push_back(getIndentStr(indentLevel) + "}");\
        lines.push_back("#endif");\
    }while(0)

#define TDR_MD5_DIGEST_LENGTH 16

using std::cout;
using std::cerr;
using std::endl;
using std::string;

string TTransCSharp::ttranslatorVersion = "1.0";
TTransCSharp::TTransCSharp(const string& indentStr, const string& outputDir)
    : TTransBase(indentStr, outputDir, "// ") {}

void
TTransCSharp::checkMetaLib()
{
    bool checkFailed = false;
    string xmlTag;

    for (int i = 0; i < pstMetaLib->iCurMetaNum; i++)
    {
        LPTDRMETA pstMeta = TDR_IDX_TO_META(pstMetaLib, i);

        for (int j = 0; j < pstMeta->iEntriesNum; j++)
        {
            LPTDRMETAENTRY pstEntry = tdr_get_entry_by_index(pstMeta, j);
            if (0 == pstEntry->iCount)
            {
                cerr << "error: for C#, count value=0 is NOT support yet, ";
                cerr << "invalid entry:" << endl;

                checkFailed = true;
            } else if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
            {
                cerr << "error: for C#, reference type is NOT support yet, ";
                cerr << "invalid entry:" << endl;

                checkFailed = true;
            } else if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
            {
                cerr << "error: for C#, pointer type is NOT support yet, ";
                cerr << "invalid entry:" << endl;

                checkFailed = true;
            } else if (TDR_TYPE_WSTRING == pstEntry->iType
                       && TDR_INVALID_PTR != pstEntry->ptrDefaultVal)
            {
                cerr << "warning: for C#, defaultvalue for 'wstring' is NOT supported yet, ";
                cerr << "related entry:" << endl;
            } else
            {
                continue;
            }

            if (TDR_TYPE_UNION == pstMeta->iType)
            {
                xmlTag = "union";
            } else if (TDR_TYPE_STRUCT == pstMeta->iType)
            {
                xmlTag = "struct";
            }

            cerr << "\t<" << xmlTag << " name=\"" << tdr_get_meta_name(pstMeta) << "\"..." << endl;
            cerr << "\t\t<entry name=\"" << tdr_get_entry_name(pstEntry) << "\"..." << endl;
        }
    }

    if (checkFailed)
    {
        throw string("failed to check metalib");
    }
}


void
TTransCSharp::loadCodeRes(unsigned int indentLevel, std::list<string>& lines,
                          const char *pszSrc[], size_t lineCount)
{
    for (size_t i = 0; i < lineCount; i++)
    {
        string inputLine(pszSrc[i]);
        string::size_type pos;
        pos = inputLine.find_first_not_of(" \t");
        if (string::npos == pos)
        {
            pos = 0;
        }
        lines.push_back(getIndentStr(indentLevel + (unsigned)pos/4) + inputLine.substr(pos));
    }
}


void
TTransCSharp::output(std::list<string>& lines, const string& fileName, bool careMeta)
{
    if (fileName.empty())
    {
        throw string("fileName to output is empty");
    }

    if (0 >= lines.size())
    {
        return;
    }

    std::ios_base::openmode mode;
    if (touchedFiles.end() != touchedFiles.find(fileName))
    {
        mode = std::ios_base::app;
        lines.push_front("");
        lines.push_front("");
    } else
    {
        mode = std::ios_base::trunc;
        addTdrInfo(lines, careMeta);
    }

    TTransBase::output(lines, fileName, mode);

    touchedFiles.insert(fileName);
}

void
TTransCSharp::addTdrInfo(std::list<string>& lines, bool careMeta)
{
    const string& indent = getIndentStr(0);

    lines.push_front("");

    // tdr version
    string tdrVersion;
    tdr_get_version(tdrVersion);

    lines.push_front(indent + "/* tdr version: " + tdrVersion + " */");

    // create time
    time_t curTime;
    time(&curTime);
    string strTime = ctime(&curTime);
    strTime = strTime.substr(0, strTime.length()-1);
    lines.push_front(indent + "/* creation time: " + strTime + " */");

    if (careMeta)
    {
        lines.push_front("");
        // metalib md5sum
        unsigned char szMetalibHash[TDR_MD5_DIGEST_LENGTH];
        char szHash[TDR_MD5_DIGEST_LENGTH * 2 + 1] = {0};
        tdr_md5hash_buffer(szMetalibHash, (const unsigned char *)pstMetaLib,
                           (unsigned int)pstMetaLib->iSize);
        tdr_md5hash2str(szMetalibHash, &szHash[0], sizeof(szHash));
        lines.push_front(indent + "/* metalib md5sum: " + string(szHash) + " */");

        // metalib version
        lines.push_front(indent + "/* metalib version: "
                         + int2str(tdr_get_metalib_version(pstMetaLib)) + " */");
    }

    lines.push_front("");
    lines.push_front(indent + "/* No manual modification is permitted. */");
    lines.push_front(indent + "/* This file is generated by tdr. */");
}

void
TTransCSharp::translate(LPTDRMETALIB pstMetaLib,const list<XMLDesc>& xmlDescList,TdrOption* pstOption)
{
    if (!pstMetaLib)
    {
        throw string("pstMetaLib to translate is null");
    }

    if (!pstOption)
    {
        throw string("pstOption to translate is null");
    }

    TDEBUG_PRINTLN(TTransCSharp::ttranslatorVersion);
    this->pstMetaLib = pstMetaLib;
    this->pstOption = pstOption;
    this->packageName = tdr_get_metalib_name(pstMetaLib);


    checkMetaLib();
    makeReadMe();
    makeTdrErr();
    makeTdrBuf();
    makeTdrTypeUtil();
    makeTdrTime();
    makeTdrBufUtil();
    /* meta-lib-dependent files */
    makeMetaLibInfo();

    flagMeta4Default();

    translateMacros(xmlDescList);

    std::list<XMLDesc>::const_iterator descIter;
    bool bFirst = true;
    for (descIter = xmlDescList.begin(); descIter != xmlDescList.end(); ++descIter)
    {
        if (descIter->groups.empty() && descIter->metas.empty())
        {
            continue;
        }

        this->csFileName= descIter->csFileName;

        {
            DEFINE_CSHARP_CONTAINER();

            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER("using System;");
            APPEND_CSHARP_CONTAINER("using System.Diagnostics;");
            APPEND_CSHARP_CONTAINER("using System.Collections.Generic;");
            APPEND_CSHARP_CONTAINER("using System.Runtime.InteropServices;");
            APPEND_CSHARP_CONTAINER("using System.Text;");
            APPEND_CSHARP_CONTAINER("using tsf4g_tdr_csharp;");
            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER("namespace " + this->packageName);
            APPEND_CSHARP_CONTAINER("{");

            OUTPUT_CSHARP_CONTAINER(this->csFileName);

        }

        if (bFirst)
        {
            translateInterface();
            bFirst = false;
        }
        translateGroups(descIter->groups);

        translateMetas(descIter->metas);

        {
            DEFINE_CSHARP_CONTAINER();
            APPEND_CSHARP_CONTAINER("}");
            OUTPUT_CSHARP_CONTAINER(this->csFileName);
        }

    }


}

void
TTransCSharp::translateMacros(const list<XMLDesc>& xmlDescList)
{


    std::list<string> Lines;
    Lines.push_back("namespace " + this->packageName);
    Lines.push_back("{");
    Lines.push_back("");

    const unsigned int indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);

    Lines.push_back(indent+"public class  " + this->packageName + "Macros");
    Lines.push_back(indent + "{");

    std::list<XMLDesc>::const_iterator descIter;
    for (descIter = xmlDescList.begin(); descIter != xmlDescList.end(); ++descIter)
    {
        if (descIter->macros.empty())
        {
            continue;
        }

        {
            const unsigned int indentLevel = 1;
            const string& indent = getIndentStr(indentLevel);

            std::list<string>::const_iterator iter;
            for (iter = descIter->macros.begin(); iter != descIter->macros.end(); ++iter)
            {
                LPTDRMACRO pstMacro = tdr_get_macro_by_name(pstMetaLib, iter->c_str());
                if (NULL == pstMacro)
                {
                    throw string("tdr_get_macro_by_name in TTransCPP::translateMacros failed");
                }

                int iValue;
                int iRet = tdr_get_macro_value_by_ptr(&iValue, pstMacro);
                if (TDR_SUCCESS != iRet)
                {
                    throw string("tdr_get_macro_value_by_ptr in TTransCPP::translatemacros failed");
                }

                string desc = tdr_get_macro_desc_by_ptr(pstMetaLib,pstMacro);
                if (!desc.empty())
                {
                    desc = " // " + desc ;
                }
                Lines.push_back(indent + "public const int "+ tdr_get_macro_name_by_ptr(pstMetaLib, pstMacro)
                                + " = " + int2str(iValue) + ";" + desc);
            }
        }
    }

    Lines.push_back(indent + "}");

    Lines.push_back("");
    Lines.push_back("}");
    this->output(Lines, this->packageName + "_macros.cs");

}

void TTransCSharp::translateInterface()
{
    std::list<string> lines;
    const unsigned int indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);

    lines.push_back(indent + "public interface tsf4g_csharp_interface");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TdrError.ErrorType construct();");
        lines.push_back(indent + "TdrError.ErrorType pack(ref byte[] buffer, int size, ref int usedSize, uint cutVer);");
        lines.push_back(indent + "TdrError.ErrorType pack(ref TdrWriteBuf destBuf, uint cutVer);");
        lines.push_back(indent + "TdrError.ErrorType unpack(ref byte[] buffer, int size, ref int usedSize, uint cutVer);");
        lines.push_back(indent + "TdrError.ErrorType unpack(ref TdrReadBuf srcBuf, uint cutVer);");
        lines.push_back(indent + "TdrError.ErrorType load(ref byte[] buffer, int size, ref int usedSize, uint cutVer);");
        lines.push_back(indent + "TdrError.ErrorType load(ref TdrReadBuf srcBuf, uint cutVer);");
        lines.push_back(indent + "TdrError.ErrorType visualize(ref string buffer,int indent , char separator);");
        lines.push_back(indent + "TdrError.ErrorType visualize(ref TdrVisualBuf destBuf, int indent, char separator);");
        lines.push_back(indent + "TdrError.ErrorType getSizeInfo(byte[] buffer, int size, ref uint sizeInfo);");
        lines.push_back(indent + "TdrError.ErrorType getSizeInfo(TdrReadBuf srcBuf, ref uint sizeInfo);");
    }
    lines.push_back(indent + "}");

    this->output(lines, this->csFileName);
}

void
TTransCSharp::translateGroups(const list<string>& groups)
{
    std::list<string> csLines;
    const unsigned int indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);

    std::list<string>::const_iterator iter;
    for (iter = groups.begin(); iter != groups.end(); ++iter)
    {
        LPTDRMACROSGROUP pstGroup = tdr_get_macrosgroup_by_name(pstMetaLib, iter->c_str());
        if (NULL == pstGroup)
        {
            throw string("tdr_get_macrosgroup_by_name failed, by name ") + *iter;
        }


        csLines.push_back(indent + "public enum  " + *iter);
        csLines.push_back(indent + "{");
        {
            const unsigned int indentLevel = 1;
            const string& indent = getIndentStr(indentLevel);

            int iCount = tdr_get_macrosgroup_macro_num(pstGroup);
            for (int i = 0; i < iCount; i++)
            {
                LPTDRMACRO pstMacro = tdr_get_macrosgroup_macro_by_index(pstMetaLib, pstGroup, i);
                if (NULL == pstMacro)
                {
                    throw string("tdr_get_macro_by_name failed, by name ") + *iter;
                }

                int iValue;
                int iRet = tdr_get_macro_value_by_ptr(&iValue, pstMacro);
                if (TDR_SUCCESS != iRet)
                {
                    throw string("tdr_get_macro_value_by_ptr in TTransCPP::translatemacros failed");
                }

                string desc = tdr_get_macro_desc_by_ptr(pstMetaLib,pstMacro);
                if (!desc.empty())
                {
                    desc = " // " + desc ;
                }

                csLines.push_back(indent + tdr_get_macro_name_by_ptr(pstMetaLib, pstMacro)
                                  + " = " + int2str(iValue) + "," + desc);
            }
        }
        csLines.push_back(indent + "}");
    }

    this->output(csLines, this->csFileName);
}

void TTransCSharp::translateMetas(const list<string>& metas)
{
    std::list<string>::const_iterator iter;
    for (iter = metas.begin(); iter != metas.end(); ++iter)
    {
        LPTDRMETA pstMeta = tdr_get_meta_by_name(pstMetaLib, iter->c_str());
        if (NULL == pstMeta)
        {
            throw string("tdr_get_meta_by_name failed, by name ") + *iter;
        }

        if (pstMeta->idxType == TDR_TYPE_UNION)
        {
            translateUnion(pstMeta);
        } else if (pstMeta->idxType == TDR_TYPE_STRUCT)
        {
            translateStruct(pstMeta);
        }
    }
}
void
TTransCSharp::translateUnion(LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to translateUnion is null");
    }
    string desc = tdr_get_meta_desc(pstMeta);
    if (!desc.empty())
    {
        desc = "/* " + desc + " */";
    }
    DEFINE_CSHARP_CONTAINER();
    {
        // 1rt indent-level
        const string& indent = getIndentStr(0);

        if (!desc.empty())
        {
            APPEND_CSHARP_CONTAINER(indent + desc);
        }
        APPEND_CSHARP_CONTAINER(indent + "public class " + tdr_get_meta_name(pstMeta));
        APPEND_CSHARP_CONTAINER(indent + "{");
        {
            const unsigned int indentLevel = 1;
            const string& indent = getIndentStr(indentLevel);
            APPEND_CSHARP_CONTAINER(indent + "/* public members */");
            {
                declaration(indentLevel , Lines, pstMeta);
            }

            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER(indent + "/* public methods */");
            {
                const unsigned int indentLevel = 1;

                unionConstruct(indentLevel, Lines,  pstMeta);
                APPEND_CSHARP_CONTAINER("");
                processUnion(indentLevel, Lines, pstMeta, Parameter::TYPE_INIT);
                APPEND_CSHARP_CONTAINER("");

                processUnion(indentLevel, Lines, pstMeta, Parameter::TYPE_ENCODE);
                APPEND_CSHARP_CONTAINER("");

                processUnion(indentLevel, Lines, pstMeta, Parameter::TYPE_DECODE);
                APPEND_CSHARP_CONTAINER("");

                processUnion(indentLevel, Lines, pstMeta, Parameter::TYPE_LOAD);
                APPEND_CSHARP_CONTAINER("");

                processUnion(indentLevel, Lines, pstMeta, Parameter::TYPE_VISUALIZE);

            }

            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER(indent + "/* version infomation */");
            {
                const unsigned int indentLevel = 1;
                version(indentLevel, Lines, pstMeta);
            }
        }
        APPEND_CSHARP_CONTAINER(indent + "}");
    }

    {
        this->output(Lines, this->csFileName);
    }

}


void TTransCSharp::unionConstruct(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to Construct is null");
    }

    const string& indent = getIndentStr(indentLevel);
    // data-members
    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryType;
    lines.push_back(indent + "public "
                    + TDR_GET_STRING_BY_PTR(pstMetaLib, pstMeta->ptrName) + "()");
    lines.push_back(indent +"{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            pstEntry = pstMeta->stEntries + i;
            entryName = getEntryName(pstEntry);
            if (1 != pstEntry->iCount || !baseType.has(pstEntry->iType))
            {
                lines.push_back(indent + entryName + " = null;");
            }
        }

    }
    lines.push_back(indent +"}");
}

void
TTransCSharp::structConstruct(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to Construct is null");
    }

    const string& indent = getIndentStr(indentLevel);
    // data-members
    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryType;
    lines.push_back(indent + "public "
                    + TDR_GET_STRING_BY_PTR(pstMetaLib, pstMeta->ptrName) + "()");
    lines.push_back(indent +"{");
    {
        // 3rd indent-level
        const string& indent = getIndentStr(indentLevel + 1);
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            pstEntry = pstMeta->stEntries + i;
            entryName = getEntryName(pstEntry);
            getEntryType(pstEntry, entryType);
            string strLen;

            if (1 != pstEntry->iCount)
            {
                if (TDR_TYPE_STRING == pstEntry->iType || TDR_TYPE_WSTRING == pstEntry->iType)
                {
                    lines.push_back(indent + entryName + " = new " + entryType +
                                    "[" + int2str(pstEntry->iCount) + "][];");
                    string loopVarName = entryName + "_i";
                    lines.push_back(indent + "for (int " + loopVarName + " = 0;" + loopVarName + " < "
                                    + int2str(pstEntry->iCount) + ";" + loopVarName + "++)");
                    lines.push_back(indent +"{");
                    {
                        const string& indent = getIndentStr(indentLevel + 2);

                        if (pstEntry->iCustomHUnitSize > 0)
                        {

                            lines.push_back(indent + entryName + "[" + loopVarName +"] = new " + entryType + "["
                                            + int2str(pstEntry->iCustomHUnitSize) +"];");
                        }
                        else
                        {
                            lines.push_back(indent + entryName + "["  + loopVarName +"] = new " + entryType + "[1];");

                        }

                    }
                    lines.push_back(indent +"}");
                }
                else
                {
                    strLen = "["  + int2str(pstEntry->iCount) + "]";
                    string loopVarName = getEntryName(pstEntry) + "_i";
                    lines.push_back(indent + entryName + " = new " + entryType + strLen + ";");
                    if (!baseType.has(pstEntry->iType))
                    {
                        lines.push_back(indent + "for(int " + loopVarName + " = 0; " + loopVarName + " < " +
                                        int2str(pstEntry->iCount) + "; " + loopVarName + "++)" );
                        lines.push_back(indent + "{");
                        {
                            const string& indent = getIndentStr(indentLevel + 2);
                            lines.push_back(indent + entryName + "[" + loopVarName + "] = new " + entryType + "();");
                        }
                        lines.push_back(indent + "}");
                        lines.push_back("");
                    }
                }

            }
            else
            {

                if (!baseType.has(pstEntry->iType))
                {
                    lines.push_back(indent + entryName + " = new " + entryType + "();");
                }

                else if (TDR_TYPE_STRING == pstEntry->iType || TDR_TYPE_WSTRING == pstEntry->iType)
                {
                    if (0 < pstEntry->iCustomHUnitSize)
                    {
                        strLen = "[" + int2str(pstEntry->iCustomHUnitSize)+ "]";
                    }
                    else
                    {
                        strLen = "[1]";
                    }

                    lines.push_back(indent + entryName + " = new " + entryType + strLen + ";");
                }
            }
        }
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::translateStruct(LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to translateStruct is null");
    }

    string desc = tdr_get_meta_desc(pstMeta);
    if (!desc.empty())
    {
        desc = "/* " + desc + " */";
    }

    DEFINE_CSHARP_CONTAINER();
    {
        const unsigned int indentLevel = 0;
        const string& indent = getIndentStr(indentLevel);
        if (!desc.empty())
        {
            APPEND_CSHARP_CONTAINER(indent + desc);
        }
        APPEND_CSHARP_CONTAINER(indent + "public class " + tdr_get_meta_name(pstMeta) + " : tsf4g_csharp_interface");
        APPEND_CSHARP_CONTAINER(indent + "{");
        {
            const unsigned int indentLevel = 1;
            const string& indent = getIndentStr(indentLevel);
            APPEND_CSHARP_CONTAINER(indent + "/* public members */");
            {
                declaration(indentLevel , Lines, pstMeta);
            }

            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER(indent + "/* construct methods */");

            structConstruct(indentLevel,  Lines, pstMeta);

            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER(indent + "/* public methods */");
            {
                const unsigned int indentLevel = 1;

                constructStruct(indentLevel, Lines, pstMeta);
                APPEND_CSHARP_CONTAINER("");

                structPack(indentLevel, Lines, pstMeta);
                APPEND_CSHARP_CONTAINER("");

                structUnpack(indentLevel, Lines, pstMeta);
                APPEND_CSHARP_CONTAINER("");

                structLoad(indentLevel, Lines, pstMeta);
                APPEND_CSHARP_CONTAINER("");

                structVisualize(indentLevel, Lines, pstMeta);

                APPEND_CSHARP_CONTAINER("");
                structSizeInfo( indentLevel,Lines, pstMeta);
                APPEND_CSHARP_CONTAINER("");
            }

            {
                version(indentLevel , Lines, pstMeta);
            }
        }
        APPEND_CSHARP_CONTAINER(indent + "}");
    }

    {
        this->output(Lines, this->csFileName);
    }
}


void
TTransCSharp::declaration(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to declaration is null");
    }

    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryType;


    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {

        const string &indent = getIndentStr(indentLevel);

        pstEntry = pstMeta->stEntries + i;
        entryName = getEntryName(pstEntry);
        TDEBUG_PRINTLN(entryName);
        getEntryType(pstEntry, entryType);

        string desc;
        desc =  tdr_get_entry_desc(pstEntry);


        if (!desc.empty())
        {
            desc = " // " + desc;
        }

        string strLen;
        if (TDR_TYPE_STRING == pstEntry->iType || TDR_TYPE_WSTRING == pstEntry->iType)
        {
            TDEBUG_PRINTLN(entryType);
            strLen = "[]";

        }

        if (pstEntry->iCount != 1)
        {
            if (TDR_TYPE_STRING == pstEntry->iType || TDR_TYPE_WSTRING == pstEntry->iType)
            {
                lines.push_back(indent + entryType  + "[][] "+ getEntryName(pstEntry) + ";" + desc);
            }
            else
            {
                lines.push_back(indent + "public " + entryType + " [] " + getEntryName(pstEntry) + ";" + desc);
            }
        }
        else
        {
            lines.push_back(indent +"public " +  entryType + " " + strLen + " " + getEntryName(pstEntry) +  ";" + desc);
        }

    }
}

void
TTransCSharp::constructStruct(unsigned int indentLevel, list<string>& lines,LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to constructStruct is null");
    }

    lines.push_back(getIndentStr(indentLevel) + "public TdrError.ErrorType construct()");
    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        lines.push_back(indent + "TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;");
        lines.push_back("");
        if (needSetDefaultValue(pstMeta))
        {
            constructStructEntry(indentLevel + 1, lines, pstMeta);
        }
        else
        {
            lines.push_back(indent + "/*no need to  set defaultvalue */");
        }
        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::constructStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to constructStructEntry is null");
    }

    //    TDEBUG_PRINT_STR("constructStructEntry......");
    //    TDEBUG_PRINT_STRLN(tdr_get_meta_name(pstMeta));

    LPTDRMETAENTRY pstEntry;
    string entryNameWithThis;
    bool hasOutputOneEntry = false;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        pstEntry = pstMeta->stEntries + i;

        entryNameWithThis = getEntryNameWithThis(pstEntry);
        Parameter parameter(lines, pstMeta, pstEntry, entryNameWithThis, Parameter::TYPE_INIT);

        if (hasOutputOneEntry)
        {
            lines.push_back("");
        }

        processEntry4Construct(indentLevel, parameter);

        hasOutputOneEntry = true;
    }
}



void
TTransCSharp::structPack(unsigned int indentLevel, list<string>& lines,LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structPack is null");
    }

    {
        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "public TdrError.ErrorType pack(ref byte[] buffer, int size,ref int usedSize , uint cutVer)");

        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);


            lines.push_back(indent + "if (null == buffer || 0 == buffer.GetLength(0) || (size > buffer.GetLength(0)))");
            lines.push_back(indent + "{");
            lines.push_back(indent + "    return TdrError.ErrorType.TDR_ERR_INVALID_BUFFER_PARAMETER;");
            lines.push_back(indent + "}");
            lines.push_back("");
            lines.push_back(indent + "TdrWriteBuf destBuf = new TdrWriteBuf(ref buffer, size);");
            lines.push_back(indent + "TdrError.ErrorType ret = pack(ref destBuf, cutVer);");
            lines.push_back(indent + "if (ret == TdrError.ErrorType.TDR_NO_ERROR)");
            lines.push_back(indent +"{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                lines.push_back(indent + "buffer = destBuf.getBeginPtr();");
                lines.push_back("");
                lines.push_back(indent + "usedSize = destBuf.getUsedSize();");
            }

            lines.push_back(indent +"}");
            lines.push_back("");
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");

    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "public TdrError.ErrorType pack(ref TdrWriteBuf destBuf,  uint cutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;");

        lines.push_back("");
        adjustCutVersion(indentLevel + 1, lines, pstMeta);

        // save space for sizeinfo
        if (pstMeta->stSizeType.iUnitSize > 0)
        {
            lines.push_back("");
            prepare4sizeinfo(indentLevel + 1, lines, pstMeta);
        }

        // records destBuf.getUsedSize()
        if (0 < pstMeta->stVersionIndicator.iUnitSize
            || 0 < pstMeta->stSizeType.iUnitSize)
        {
            lines.push_back("");
            lines.push_back(indent + "int savedBeginPos = destBuf.getUsedSize();");
        } else
        {
            for (int i = 0; i < pstMeta->iEntriesNum; i++)
            {
                LPTDRMETAENTRY pstEntry = pstMeta->stEntries + i;
                if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
                    && pstEntry->stSizeInfo.iUnitSize > 0)
                {
                    lines.push_back("");
                    lines.push_back(indent + "int savedBeginPos = destBuf.getUsedSize();");
                    break;
                }
            }
        }

        lines.push_back("");
        packStructEntry(indentLevel + 1, lines, pstMeta);

        packVersionIndicator(indentLevel + 1, lines, pstMeta);

        packSizeinfo(indentLevel + 1, lines, pstMeta);

        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

}

void
TTransCSharp::prepare4sizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to prepare4sizeinfo is null");
    }

    const string& indent = getIndentStr(indentLevel);
    TDEBUG_PRINTLN(pstMeta->stSizeType.iUnitSize);
    TDEBUG_PRINTLN(pstMeta->stSizeType.iNOff);

    unsigned int offset = 0;
    string position = "destBuf.getUsedSize()";
    if (pstMeta->stSizeType.iNOff > 0)
    {
        offset = pstMeta->stSizeType.iNOff;
        position += " + " + int2str((int)offset);
    }
    lines.push_back(indent + "int metaSizePos = " + position + ";");

    if (pstMeta->stSizeType.idxSizeType != TDR_INVALID_INDEX)
    {
        lines.push_back("");
        lines.push_back(indent + "/* reserve space for struct's sizeinfo */");
        switch (pstMeta->stSizeType.iUnitSize)
        {
            case 1:
                lines.push_back(indent + "ret = destBuf.reserve(sizeof(byte));");
                break;
            case 2:
                lines.push_back(indent + "ret = destBuf.reserve(sizeof(UInt16));");
                break;
            case 4:
                lines.push_back(indent + "ret = destBuf.reserve(sizeof(UInt32));");
                break;
            default:
                throw string("invalid sizeinfo iUnitSize, this is a tdr-lib error");
                break;
        }
        lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);

            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
    }
}

void
TTransCSharp::packStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to packStructEntry is null");
    }

    //    TDEBUG_PRINT_STR("packStructEntry......");
    //    TDEBUG_PRINT_STRLN(tdr_get_meta_name(pstMeta));
    const string& indent = getIndentStr(indentLevel);

    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryNameWithThis;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        if (0 != i)
        {
            lines.push_back("");
        }

        pstEntry = pstMeta->stEntries + i;
        entryName = getEntryName(pstEntry);
        entryNameWithThis = getEntryNameWithThis(pstEntry);

        lines.push_back(indent + "/* pack member: " + entryNameWithThis + " */");

        Parameter parameter(lines, pstMeta, pstEntry, entryNameWithThis, Parameter::TYPE_ENCODE);
        if (pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            string entryVersion = getEntryVersion(pstEntry, pstMeta);
            lines.push_back(indent + "if (" + entryVersion + " <= cutVer)");
        }
        lines.push_back(indent + "{");
        {
            processEntry4Code(indentLevel + 1, parameter);
        }
        lines.push_back(indent + "}");
    }

}


void TTransCSharp::packVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to packVersionIndicator is null");
    }

    if (pstMeta->stVersionIndicator.iUnitSize > 0)
    {
        lines.push_back("");
        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "/* set version-indicator */");
        directWrite(indentLevel, lines, pstMeta->stVersionIndicator.iUnitSize,
                    "destBuf", "savedBeginPos + " + int2str((int)pstMeta->stVersionIndicator.iNOff),
                    "cutVer", "invalid versionindicator iUnitSize, this is a tdr-lib error");
    }
}

void
TTransCSharp::packSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to packSizeinfo is null");
    }

    const string& indent = getIndentStr(indentLevel);

    if (pstMeta->stSizeType.iUnitSize > 0)
    {
        TDEBUG_PRINTLN(pstMeta->stSizeType.iUnitSize);
        TDEBUG_PRINTLN(pstMeta->stSizeType.iNOff);

        lines.push_back("");
        lines.push_back(indent + "/* set sizeinfo for " + tdr_get_meta_name(pstMeta) + " */");
        lines.push_back(indent + "int sizeInBuf = destBuf.getUsedSize() - savedBeginPos;");
        directWrite(indentLevel, lines, pstMeta->stSizeType.iUnitSize,
                    "destBuf", "metaSizePos",
                    "sizeInBuf", "invalid sizeinfo iUnitSize, this is a tdr-lib error");
    }

}

void TTransCSharp::structLoad(unsigned int indentLevel, list<string>& lines,LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structLoad is null");
    }
    {
        const string& indent = getIndentStr(indentLevel);

        lines.push_back(indent + "public TdrError.ErrorType load(ref byte [] buffer, int size, ref int usedSize , uint cutVer)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            lines.push_back(indent + "if (null == buffer || 0 == buffer.GetLength(0) || size > buffer.GetLength(0))");
            lines.push_back(indent + "{");
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel + 2);
            lines.push_back(indent + "    return TdrError.ErrorType.TDR_ERR_INVALID_BUFFER_PARAMETER;");
            lines.push_back(indent + "}");
            lines.push_back("");
            lines.push_back(indent + "TdrReadBuf srcBuf = new TdrReadBuf(ref buffer, size);");
            lines.push_back(indent + "TdrError.ErrorType ret = load(ref srcBuf, cutVer);");
            lines.push_back("");
            lines.push_back(indent + "usedSize = srcBuf.getUsedSize();");
            lines.push_back("");
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "public TdrError.ErrorType load(ref TdrReadBuf srcBuf, uint cutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "srcBuf.disableEndian();");
        lines.push_back(indent + "TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;");

        lines.push_back("");

        if (pstMeta->stVersionIndicator.iUnitSize > 0)
        {
            loadVersionIndicator(indentLevel + 1, lines, pstMeta);
        } else
        {
            adjustCutVersion(indentLevel + 1, lines, pstMeta);
        }

        if (pstMeta->stSizeType.iUnitSize > 0)
        {
            lines.push_back("");
            unpackSizeinfo(indentLevel + 1, lines, pstMeta);
        }

        // records srcBuf.getUsedSize()
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            LPTDRMETAENTRY pstEntry = pstMeta->stEntries + i;
            if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
                && pstEntry->stSizeInfo.iUnitSize > 0)
            {
                lines.push_back("");
                lines.push_back(indent + "int savedBeginPos = srcBuf.getUsedSize();");
                break;
            }
        }

        lines.push_back("");
        loadStructEntry(indentLevel + 1, lines, pstMeta);

        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::structUnpack(unsigned int indentLevel, list<string>& lines,LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structUnpack is null");
    }

    {
        const string& indent = getIndentStr(indentLevel);

        lines.push_back(indent + "public TdrError.ErrorType unpack(ref byte [] buffer, int size, ref int usedSize , uint cutVer)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            lines.push_back(indent + "if (null == buffer || 0 == buffer.GetLength(0) || size > buffer.GetLength(0))");
            lines.push_back(indent + "{");
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel + 2);
            lines.push_back(indent + "    return TdrError.ErrorType.TDR_ERR_INVALID_BUFFER_PARAMETER;");
            lines.push_back(indent + "}");
            lines.push_back("");
            lines.push_back(indent + "TdrReadBuf srcBuf = new TdrReadBuf(ref buffer, size);");
            lines.push_back(indent + "TdrError.ErrorType ret = unpack(ref srcBuf, cutVer);");
            lines.push_back("");
            lines.push_back(indent + "usedSize = srcBuf.getUsedSize();");
            lines.push_back("");
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "public TdrError.ErrorType unpack(ref TdrReadBuf srcBuf, uint cutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;");

        lines.push_back("");

        if (pstMeta->stVersionIndicator.iUnitSize > 0)
        {
            unpackVersionIndicator(indentLevel + 1, lines, pstMeta);
        } else
        {
            adjustCutVersion(indentLevel + 1, lines, pstMeta);
        }

        if (pstMeta->stSizeType.iUnitSize > 0)
        {
            lines.push_back("");
            unpackSizeinfo(indentLevel + 1, lines, pstMeta);
        }

        // records srcBuf.getUsedSize()
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            LPTDRMETAENTRY pstEntry = pstMeta->stEntries + i;
            if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
                && pstEntry->stSizeInfo.iUnitSize > 0)
            {
                lines.push_back("");
                lines.push_back(indent + "int savedBeginPos = srcBuf.getUsedSize();");
                break;
            }
        }

        lines.push_back("");
        unpackStructEntry(indentLevel + 1, lines, pstMeta);

        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

}

void
TTransCSharp::loadStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to loadStructEntry is null");
    }


    const string& indent = getIndentStr(indentLevel);

    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryNameWithThis;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        if (0 != i)
        {
            lines.push_back("");
        }

        pstEntry = pstMeta->stEntries + i;
        entryName = getEntryName(pstEntry);
        entryNameWithThis = getEntryNameWithThis(pstEntry);

        lines.push_back(indent + "/* load member: " + entryNameWithThis + " */");

        Parameter parameter(lines, pstMeta, pstEntry, entryNameWithThis, Parameter::TYPE_LOAD);
        if (pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            string entryVersion = getEntryVersion(pstEntry, pstMeta);
            lines.push_back(indent + "if (" + entryVersion + " <= cutVer)");
        }
        lines.push_back(indent + "{");
        {
            processEntry4Load(indentLevel + 1, parameter);
        }
        lines.push_back(indent + "}");
        if (pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            lines.push_back(indent + "else /* set default value */");
            lines.push_back(indent + "{");

            processEntry4DefaultLoad(indentLevel + 1, parameter);

            lines.push_back(indent + "}");
        }
    }

}

void
TTransCSharp::unpackStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to unpackStructEntry is null");
    }


    const string& indent = getIndentStr(indentLevel);

    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryNameWithThis;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        if (0 != i)
        {
            lines.push_back("");
        }

        pstEntry = pstMeta->stEntries + i;
        entryName = getEntryName(pstEntry);
        entryNameWithThis = getEntryNameWithThis(pstEntry);

        lines.push_back(indent + "/* unpack member: " + entryNameWithThis + " */");

        Parameter parameter(lines, pstMeta, pstEntry, entryNameWithThis, Parameter::TYPE_DECODE);
        if (pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            string entryVersion = getEntryVersion(pstEntry, pstMeta);
            lines.push_back(indent + "if (" + entryVersion + " <= cutVer)");
        }
        lines.push_back(indent + "{");
        {
            processEntry4Code(indentLevel + 1, parameter);
        }
        lines.push_back(indent + "}");
        if (pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            lines.push_back(indent + "else /* set default value */");
            lines.push_back(indent + "{");

            processEntry4Default(indentLevel + 1, parameter);

            lines.push_back(indent + "}");
        }
    }

}

void TTransCSharp::loadVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to loadVersionIndicator is null");
    }

    const string& indent = getIndentStr(indentLevel);


    lines.push_back(indent + "/* get versionindicator */");
    directRead(indentLevel, lines, pstMeta->stVersionIndicator.iUnitSize,
               "netCutVer", "srcBuf", "srcBuf.getUsedSize() + " + int2str((int)pstMeta->stVersionIndicator.iNOff),
               "invalid versionindicator iUnitSize, this is a tdr-lib error");

    lines.push_back("");
    lines.push_back(indent + "/* check versionindicator */");
    lines.push_back(indent + "if (" + tdr_get_meta_name(pstMeta)
                    + ".BASEVERSION > netCutVer || netCutVer > "
                    + tdr_get_meta_name(pstMeta) + ".CURRVERSION)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_NET_CUTVER_INVALID;");
    }
    lines.push_back(indent + "}");

    string version;
    offset2version(pstMeta, pstMeta->stVersionIndicator.iHOff, version);
    lines.push_back(indent + "if (" + version + " > netCutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_CUTVER_CONFILICT;");
    }
    lines.push_back(indent + "}");

    lines.push_back(indent + "cutVer = netCutVer;");

}


void TTransCSharp::unpackVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to unpackVersionIndicator is null");
    }

    const string& indent = getIndentStr(indentLevel);


    lines.push_back(indent + "/* get versionindicator */");
    directRead(indentLevel, lines, pstMeta->stVersionIndicator.iUnitSize,
               "netCutVer", "srcBuf", "srcBuf.getUsedSize() + " + int2str((int)pstMeta->stVersionIndicator.iNOff),
               "invalid versionindicator iUnitSize, this is a tdr-lib error");

    lines.push_back("");
    lines.push_back(indent + "/* check versionindicator */");
    lines.push_back(indent + "if (" + tdr_get_meta_name(pstMeta)
                    + ".BASEVERSION > netCutVer || netCutVer > "
                    + tdr_get_meta_name(pstMeta) + ".CURRVERSION)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_NET_CUTVER_INVALID;");
    }
    lines.push_back(indent + "}");

    string version;
    offset2version(pstMeta, pstMeta->stVersionIndicator.iHOff, version);
    lines.push_back(indent + "if (" + version + " > netCutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_CUTVER_CONFILICT;");
    }
    lines.push_back(indent + "}");

    lines.push_back(indent + "cutVer = netCutVer;");

}

void
TTransCSharp::unpackSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to unpackSizeinfo is null");
    }

    const string& indent = getIndentStr(indentLevel);

    if (pstMeta->stSizeType.iUnitSize > 0)
    {
        lines.push_back(indent + "/* get sizeinfo for " + tdr_get_meta_name(pstMeta) + " */");

        TDEBUG_PRINTLN(pstMeta->stSizeType.iUnitSize);
        TDEBUG_PRINTLN(pstMeta->stSizeType.iNOff);

        unsigned int offset = 0;
        string position = "srcBuf.getUsedSize()";
        if (pstMeta->stSizeType.iNOff >= 0)
        {
            offset = pstMeta->stSizeType.iNOff;
            position = "srcBuf.getUsedSize() + " + int2str((int)offset);
        }

        if (pstMeta->stSizeType.idxSizeType != TDR_INVALID_INDEX)
        {
            // sizeinfo refers to a base-type
            serialRead(indentLevel, lines, pstMeta->stSizeType.iUnitSize,
                       "sizeInBuf", "srcBuf", "invalid sizeinfo iUnitSize, this is a tdr-lib error");
        } else
        {
            lines.push_back(indent + "/* get meta's sizeinfo-value */");
            directRead(indentLevel, lines, pstMeta->stSizeType.iUnitSize,
                       "sizeInBuf", "srcBuf", position, "invalid sizeinfo iUnitSize, this is a tdr-lib error");
        }

        // check available bytes are enough
        lines.push_back("");
        lines.push_back(indent + "/* check srcBuf contains enough bytes specified by sizeinfo */");
        lines.push_back(indent + "if (srcBuf.getLeftSize() < sizeInBuf)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);

            lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_SHORT_BUF_FOR_READ;");
        }
        lines.push_back(indent + "}");
    }

}


void
TTransCSharp::structSizeInfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structPack is null");
    }

    {
        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "public TdrError.ErrorType getSizeInfo(byte[] buffer ,int size , ref uint sizeInfo)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            lines.push_back(indent + "if (0 == buffer.GetLength(0) || size > buffer.GetLength(0))");
            lines.push_back(indent + "{");
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel + 2);
            lines.push_back(indent + "    return TdrError.ErrorType.TDR_ERR_INVALID_BUFFER_PARAMETER;");
            lines.push_back(indent + "}");
            lines.push_back("");
            lines.push_back(indent + "TdrReadBuf srcBuf = new TdrReadBuf(ref buffer, size);");
            lines.push_back(indent + "TdrError.ErrorType ret = getSizeInfo(srcBuf, ref sizeInfo);");
            lines.push_back("");
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
    }


    {
        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "public TdrError.ErrorType getSizeInfo(TdrReadBuf srcBuf, ref uint sizeInfo)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            if (pstMeta->stSizeType.iUnitSize)
            {
                lines.push_back(indent + "TdrError.ErrorType ret  = TdrError.ErrorType.TDR_NO_ERROR;");
                lines.push_back(indent + "/* get sizeinfo for " + tdr_get_meta_name(pstMeta) + " */");
                unsigned int offset = 0;
                string position = "srcBuf.getUsedSize()";
                if (pstMeta->stSizeType.iNOff >= 0)
                {
                    offset = pstMeta->stSizeType.iNOff;
                    position = "srcBuf.getUsedSize() + " + int2str((int)offset);
                }

                if (pstMeta->stSizeType.idxSizeType != TDR_INVALID_INDEX)
                {
                    // sizeinfo refers to a base-type
                    serialRead(indentLevel+1, lines, pstMeta->stSizeType.iUnitSize,
                               "sizeInBuf", "srcBuf", "invalid sizeinfo iUnitSize, this is a tdr-lib error");
                    lines.push_back(indent +"sizeInfo = (uint)sizeInBuf + " + int2str(pstMeta->stSizeType.iUnitSize) + ";");
                } else
                {
                    lines.push_back(indent + "/* get meta's sizeinfo-value */");
                    directRead(indentLevel+1, lines, pstMeta->stSizeType.iUnitSize,
                               "sizeInBuf", "srcBuf", position, "invalid sizeinfo iUnitSize, this is a tdr-lib error");
                }
                lines.push_back("");
                lines.push_back(indent + "return ret;");
            }

            else
            {
                lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_HAVE_NOT_SET_SIZEINFO;");

            }

        }
        lines.push_back(indent + "}");
    }
}

void
TTransCSharp::processEntry4DefaultLoad(unsigned int indentLevel, const Parameter& a_parameter)
{
    if (!a_parameter.pstMeta)
    {
        throw string("'a_parameter.pstMeta' to processEntry4DefaultLoad is null");
    }
    if (!a_parameter.pstEntry)
    {
        throw string("'a_parameter.pstEntry' to processEntry4DefaultLoad is null");
    }
    if (a_parameter.variable.empty())
    {
        throw string("'a_parameter.variable' to processEntry4DefaultLoad is empty");
    }

    std::list<string>& lines = a_parameter.lines;
    //LPTDRMETA pstMeta = a_parameter.pstMeta;
    LPTDRMETAENTRY pstEntry = a_parameter.pstEntry;

    const string& indent = getIndentStr(indentLevel);
    string arrIndex;

    string referPath;
    string loopVarType;

    loopVarType = "Int32";
    string loopCountVar;
    string loopVarName;
    if (1 != pstEntry->iCount)
    {
        loopCountVar = int2str(pstEntry->iCount);
        loopVarName = getEntryName(pstEntry) + "_i";
    }

    unsigned int furtherIndent = 0;

    if (TDR_TYPE_UNION == pstEntry->iType
        || TDR_TYPE_STRUCT == pstEntry->iType)
    {
        if (1 != pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
        }

        constructNonLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
    } else if (TDR_TYPE_STRING != pstEntry->iType
               && TDR_TYPE_WSTRING != pstEntry->iType
               && TDR_TYPE_IP != pstEntry->iType
               && TDR_TYPE_DATE != pstEntry->iType
               && TDR_TYPE_TIME != pstEntry->iType
               && TDR_TYPE_DATETIME != pstEntry->iType)
    {
        constructCommLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarType, loopCountVar);
    } else if (TDR_TYPE_STRING == pstEntry->iType)
    {

        if (1 != pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
        }
        constructStringEntry(indentLevel + furtherIndent + 1, a_parameter, loopVarName);

    } else if (TDR_TYPE_IP == pstEntry->iType
               || TDR_TYPE_DATE == pstEntry->iType
               || TDR_TYPE_TIME == pstEntry->iType
               || TDR_TYPE_DATETIME == pstEntry->iType)
    {
        if (1 == pstEntry->iCount)
        {
            constructBinarySpecilEntry(indentLevel, a_parameter, loopVarName);
        } else
        {
            constructBinarySpecilEntry(indentLevel, a_parameter, "0");
            {
                const string& variable = a_parameter.variable;
                lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 1; "
                                + loopVarName + " < " + loopCountVar + "; "
                                + loopVarName + "++)");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + variable + "[" + loopVarName + "] = " + variable + "[0];");
                }
                lines.push_back(indent + "}");
            }
        }
    }

    if (1 != pstEntry->iCount)
    {
        if (0 < furtherIndent)
        {
            lines.push_back(indent + "}");
        }
    }
}

void
TTransCSharp::processEntry4Default(unsigned int indentLevel, const Parameter& a_parameter)
{
    if (!a_parameter.pstMeta)
    {
        throw string("'a_parameter.pstMeta' to processEntry4Default is null");
    }
    if (!a_parameter.pstEntry)
    {
        throw string("'a_parameter.pstEntry' to processEntry4Default is null");
    }
    if (a_parameter.variable.empty())
    {
        throw string("'a_parameter.variable' to processEntry4Default is empty");
    }

    std::list<string>& lines = a_parameter.lines;
    LPTDRMETA pstMeta = a_parameter.pstMeta;
    LPTDRMETAENTRY pstEntry = a_parameter.pstEntry;

    const string& indent = getIndentStr(indentLevel);
    string arrIndex;

    string referPath;
    string loopVarType;
    if (pstEntry->stRefer.iUnitSize > 0)
    {
        offset2path(pstMeta, pstEntry->stRefer.iHOff, referPath);
        LPTDRMETAENTRY pstReferEntry = tdr_get_entry_refer(pstEntry);
        loopVarType = baseType.getType(pstReferEntry->idxType);
        if ("sbyte" == loopVarType)
        {
            /* compiler will give a warning to use char as array-index */
            loopVarType = "Int32";
        }

        if (1 != pstEntry->iCount)
        {
            if (TDR_TYPE_UCHAR != pstReferEntry->iType
                && TDR_TYPE_SMALLUINT != pstReferEntry->iType
                && TDR_TYPE_UINT != pstReferEntry->iType
                && TDR_TYPE_ULONG != pstReferEntry->iType
                && TDR_TYPE_ULONGLONG != pstReferEntry->iType)
            {
                lines.push_back(indent + "if (0 > " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);

                    lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_MINUS_REFER_VALUE;");
                }
                lines.push_back(indent + "}");
            }
            if ((unsigned int)pstEntry->iCount < getReferTypeTopValue(pstReferEntry))
            {
                lines.push_back(indent + "if (" + int2str(pstEntry->iCount) + " < " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);

                    lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_REFER_SURPASS_COUNT;");
                }
                lines.push_back(indent + "}");
            }
        }
    }


    loopVarType = "Int32";
    string loopCountVar;
    string loopVarName;
    if (1 != pstEntry->iCount)
    {
        loopCountVar = int2str(pstEntry->iCount);
        loopVarName = getEntryName(pstEntry) + "_i";
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            loopCountVar = referPath;
        }
    }

    unsigned int furtherIndent = 0;

    if (1 == pstEntry->iCount)
    {
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            lines.push_back(indent + "if (1 == " + referPath + ")");
            lines.push_back(indent + "{");
            furtherIndent++;
        }
    }

    if (TDR_TYPE_UNION == pstEntry->iType
        || TDR_TYPE_STRUCT == pstEntry->iType)
    {
        if (1 != pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
        }

        constructNonLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
    } else if (TDR_TYPE_STRING != pstEntry->iType
               && TDR_TYPE_WSTRING != pstEntry->iType
               && TDR_TYPE_IP != pstEntry->iType
               && TDR_TYPE_DATE != pstEntry->iType
               && TDR_TYPE_TIME != pstEntry->iType
               && TDR_TYPE_DATETIME != pstEntry->iType)
    {
        constructCommLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarType, loopCountVar);
    } else if (TDR_TYPE_STRING == pstEntry->iType)
    {

        if (1 != pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
        }
        constructStringEntry(indentLevel + furtherIndent + 1, a_parameter, loopVarName);

    } else if (TDR_TYPE_IP == pstEntry->iType
               || TDR_TYPE_DATE == pstEntry->iType
               || TDR_TYPE_TIME == pstEntry->iType
               || TDR_TYPE_DATETIME == pstEntry->iType)
    {
        if (1 == pstEntry->iCount)
        {
            constructBinarySpecilEntry(indentLevel, a_parameter, loopVarName);
        } else
        {
            constructBinarySpecilEntry(indentLevel, a_parameter, "0");
            {
                const string& variable = a_parameter.variable;
                lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 1; "
                                + loopVarName + " < " + loopCountVar + "; "
                                + loopVarName + "++)");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + variable + "[" + loopVarName + "] = " + variable + "[0];");
                }
                lines.push_back(indent + "}");
            }
        }
    }

    if (0 < furtherIndent)
    {
        lines.push_back(indent + "}");
    }

}

void
TTransCSharp::structVisualize(unsigned int indentLevel, list<string>& lines,LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structVisualize is null");
    }

    {
        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "/* set indent = -1 to disable indent , default: separator = '\\n' */");

        lines.push_back(indent + "public TdrError.ErrorType visualize(ref string buffer,int indent , char separator)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            lines.push_back(indent + "TdrVisualBuf destBuf = new TdrVisualBuf();");
            lines.push_back(indent + "TdrError.ErrorType ret = visualize(ref destBuf, indent, separator);");

            lines.push_back("");
            lines.push_back(indent + "buffer = destBuf.getVisualBuf();");
            lines.push_back("");

            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "/* set indent = -1 to disable indent , default: separator = '\\n' */");
    lines.push_back(indent + "public TdrError.ErrorType visualize(ref TdrVisualBuf destBuf, int indent, char separator)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        lines.push_back(indent + "TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;");

        lines.push_back("");
        visualizeEntry(indentLevel + 1, lines, pstMeta);

        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

}

void
TTransCSharp::visualizeEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to visualizeEntry is null");
    }

    //    TDEBUG_PRINT_STR("visualizeEntry......");
    //    TDEBUG_PRINT_STRLN(tdr_get_meta_name(pstMeta));
    const string& indent = getIndentStr(indentLevel);

    LPTDRMETAENTRY pstEntry;
    string entryNameWithThis;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        if (0 != i)
        {
            lines.push_back("");
        }

        pstEntry = pstMeta->stEntries + i;
        entryNameWithThis = getEntryNameWithThis(pstEntry);

        lines.push_back(indent + "/* visualize member: " + entryNameWithThis + " */");

        Parameter parameter(lines, pstMeta, pstEntry, entryNameWithThis, Parameter::TYPE_VISUALIZE);
        processEntry4Visualize(indentLevel, parameter);
    }

}



void TTransCSharp::processEntry4Visualize(unsigned int indentLevel, const Parameter& a_parameter)
{
    if (!a_parameter.pstMeta)
    {
        throw string("'a_parameter.pstMeta' to processEntry4Visualize is null");
    }
    if (!a_parameter.pstEntry)
    {
        throw string("'a_parameter.pstEntry' to processEntry4Visualize is null");
    }
    if (a_parameter.variable.empty())
    {
        throw string("'a_parameter.variable' to processEntry4Visualize is empty");
    }

    std::list<string>& lines = a_parameter.lines;
    LPTDRMETA pstMeta = a_parameter.pstMeta;
    LPTDRMETAENTRY pstEntry = a_parameter.pstEntry;

    const string& indent = getIndentStr(indentLevel);
    string arrIndex;

    string referPath;
    string loopVarType;
    if (pstEntry->stRefer.iUnitSize > 0)
    {
        offset2path(pstMeta, pstEntry->stRefer.iHOff, referPath);
        LPTDRMETAENTRY pstReferEntry = tdr_get_entry_refer(pstEntry);
        loopVarType = baseType.getType(pstReferEntry->idxType);
        if ("sbyte" == loopVarType)
        {
            /* compiler will give a warning to use char as array-index */
            loopVarType = "int";
        }

        if (1 != pstEntry->iCount)
        {
            if (TDR_TYPE_UCHAR != pstReferEntry->iType
                && TDR_TYPE_SMALLUINT != pstReferEntry->iType
                && TDR_TYPE_UINT != pstReferEntry->iType
                && TDR_TYPE_ULONG != pstReferEntry->iType
                && TDR_TYPE_ULONGLONG != pstReferEntry->iType)
            {
                lines.push_back(indent + "if (0 > " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);

                    lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_MINUS_REFER_VALUE;");
                }
                lines.push_back(indent + "}");
            }
            if ((unsigned int)pstEntry->iCount < getReferTypeTopValue(pstReferEntry))
            {
                lines.push_back(indent + "if (" + int2str(pstEntry->iCount) + " < " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
                    lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_REFER_SURPASS_COUNT;");
                }
                lines.push_back(indent + "}");
            }
        }

    }


    loopVarType = "Int32";


    unsigned int furtherIndent = 0;

    string loopCountVar;
    string loopVarName;
    if (1 != pstEntry->iCount)
    {
        loopCountVar = int2str(pstEntry->iCount);
        loopVarName = getEntryName(pstEntry) + "_i";
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            loopCountVar = referPath;
        }
    } else
    {
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            lines.push_back(indent + "if (1 == " + referPath + ")");
            lines.push_back(indent + "{");
            furtherIndent++;
        }
    }

    if (TDR_TYPE_UNION == pstEntry->iType
        || TDR_TYPE_STRUCT == pstEntry->iType)
    {
        if (1 != pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
            {
                const string& indent = getIndentStr(indentLevel +1);
                lines.push_back(indent + "if (null == " + a_parameter.variable +"[" + loopVarName + "])");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel +2);
                    lines.push_back(indent + "continue;");
                }
                lines.push_back(indent + "}");
                lines.push_back("");
            }

        }

        visualizeNonLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
    } else if (TDR_TYPE_STRING != pstEntry->iType
               && TDR_TYPE_WSTRING != pstEntry->iType
               && TDR_TYPE_IP != pstEntry->iType
               && TDR_TYPE_DATE != pstEntry->iType
               && TDR_TYPE_TIME != pstEntry->iType
               && TDR_TYPE_DATETIME != pstEntry->iType)
    {
        visualizeCommLeafEntry(indentLevel, a_parameter, loopVarType, loopCountVar);
    } else
    {
        if (1 != pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
        }
        visualizeSpecialLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
    }

    if (0 < furtherIndent)
    {
        lines.push_back(indent + "}");
    }

}


void TTransCSharp::visualizeNonLeafEntry(unsigned int indentLevel,
                                         const Parameter& parameter, const string& arrIndex)
{

    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to visualizeNonLeafEntry is null");
    }
    if (!parameter.pstMeta)
    {
        throw string("'parameter.pstMeta' to visualizeNonLeafEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to visualizeNonLeafEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    string variable = parameter.variable;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    LPTDRMETA pstMeta = parameter.pstMeta;
    const string entryName = getEntryName(parameter.pstEntry);
    string arrIndexPara;
    if (arrIndex.length() > 0)
    {
        variable = variable + "[" + arrIndex + "]";
        arrIndexPara = arrIndex + ", ";
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "ret = TdrBufUtil.printVariable(ref destBuf, indent, separator, \"["
                    + entryName + "]\", " + arrIndexPara + "true);");
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");
    lines.push_back(indent + "/* visualize children of " + variable + " */");

    string selectParameter;
    if (pstEntry->iType == TDR_TYPE_UNION)
    {
        string entryName = getEntryName(pstEntry);
        const string selectName = string("selector4") + entryName;
        string selectPath;
        offset2path(pstMeta, pstEntry->stSelector.iHOff, selectPath);
        lines.push_back(indent + "Int64 " + selectName + " = " + selectPath + ";");
        selectParameter = selectName + ", ";
    }

    lines.push_back(indent + "if (0 > indent)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "ret = " + variable + ".visualize("
                        + selectParameter + "ref destBuf, indent, separator);");
    }
    lines.push_back(indent + "} else");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "ret = " + variable + ".visualize("
                        + selectParameter + "ref destBuf, indent + 1, separator);");
    }
    lines.push_back(indent + "}");
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void TTransCSharp::visualizeCommLeafEntry(unsigned int indentLevel, const Parameter& parameter,
                                          const string& loopVarType, const string& loopCountVar)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to visualizeCommLeafEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to visualizeCommLeafEntry is empty");
    }

    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    std::list<string>& lines = parameter.lines;
    const string& variable = parameter.variable;
    const string entryName = getEntryName(parameter.pstEntry);

    const string& indent = getIndentStr(indentLevel);

    if (loopCountVar.length() > 0)
    {
        const string& loopVarName = getEntryName(pstEntry) + "_i";

        lines.push_back(indent + "ret = TdrBufUtil.printArray(ref destBuf, indent, separator, \"["
                        + entryName + "]\", " + loopCountVar + ");");
        lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");

        lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                        + loopVarName + " < " + loopCountVar + "; " + loopVarName + "++)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "ret = destBuf.sprintf(" + baseType.getPrintInfo(pstEntry->idxType)
                            + ", " + variable + "[" + loopVarName + "]);");
            lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
            lines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +2);
                lines.push_back(indent + "return ret;");
            }
            lines.push_back(indent + "}");
        }
        lines.push_back(indent + "}");
        lines.push_back("");

        lines.push_back(indent + "ret = destBuf.sprintf(\"{0}\",separator);");
    } else
    {
        lines.push_back(indent + "ret = TdrBufUtil.printVariable(ref destBuf, indent, separator, \"["
                        + entryName + "]\", " + baseType.getPrintInfo(pstEntry->idxType) + ", " + variable + ");");
    }

    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

}

void TTransCSharp::visualizeSpecialLeafEntry(unsigned int indentLevel,
                                             const Parameter& parameter, const string& arrIndex)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to visualizeSpecialLeafEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to visualizeSpecialLeafEntry is empty");
    }

    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    std::list<string>& lines = parameter.lines;
    string variable = parameter.variable;
    const string entryName = getEntryName(parameter.pstEntry);

    string arrIndexPara;
    if (arrIndex.length() > 0)
    {
        variable = variable + "[" + arrIndex + "]";
        arrIndexPara = arrIndex + ", ";
    }

    const string& indent = getIndentStr(indentLevel);

    string printFunc;
    if (TDR_TYPE_STRING == pstEntry->iType)
    {
        printFunc = "TdrBufUtil.printString";
    } else if (TDR_TYPE_WSTRING == pstEntry->iType)
    {
        printFunc = "TdrBufUtil.printWString";
    } else if (TDR_TYPE_IP == pstEntry->iType)
    {
        printFunc = "TdrBufUtil.printTdrIP";
    } else if (TDR_TYPE_TIME == pstEntry->iType)
    {
        printFunc = "TdrBufUtil.printTdrTime";
    } else if (TDR_TYPE_DATE == pstEntry->iType)
    {
        printFunc = "TdrBufUtil.printTdrDate";
    } else if (TDR_TYPE_DATETIME == pstEntry->iType)
    {
        printFunc = "TdrBufUtil.printTdrDateTime";
    }

    assert(printFunc.length() > 0);

    lines.push_back(indent + "ret = " + printFunc + "(ref destBuf, indent, separator, \"["
                    + entryName + "]\", " + arrIndexPara + variable + ");");
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

}

std::string
TTransCSharp::getEntryName(LPTDRMETAENTRY pstEntry)
{
    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryName is null");
    }

    if (TDR_HPPRULE_NO_TYPE_PREFIX & this->pstOption->stRule.iRule)
    {
        return tdr_get_entry_name(pstEntry);
    }

    const char* pszPrefix = "";

    if (strlen(tdr_get_entry_name(pstEntry)) > 1)
    {
        if (1 == pstEntry->iCount)
        {
            pszPrefix = tdr_get_sprefix_by_index(pstEntry->idxType, stderr);
        } else
        {
            pszPrefix = tdr_get_mprefix_by_index(pstEntry->idxType, stderr);
        }
    }

    if (NULL == pszPrefix)
    {
        throw string("failed to get prefix for entry: ") + tdr_get_entry_name(pstEntry);
    }

    string prefix = pszPrefix;

    if (TDR_ENTRY_IS_REFER_TYPE(pstEntry) || TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
    {
        prefix = "p" + prefix;
    }

    string entryName = tdr_get_entry_name(pstEntry);

    if (prefix.length() > 0)
    {
        return prefix + (char)toupper(entryName[0]) + entryName.substr(1);
    } else
    {
        return entryName;
    }
}

void
TTransCSharp::getEntryType(LPTDRMETAENTRY pstEntry, string& entryType)
{
    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryType is null");
    }

    if (TDR_TYPE_STRUCT < pstEntry->iType)
    {
        entryType = baseType.getType(pstEntry->idxType);
    } else
    {
        LPTDRMETA pstEntryMeta = TDR_PTR_TO_META(pstMetaLib, pstEntry->ptrMeta);
        entryType = tdr_get_meta_name(pstEntryMeta);
    }
}

void
TTransCSharp::processUnion(unsigned int indentLevel, list<string>& csLines,
                           LPTDRMETA pstMeta, Parameter::OperType oper)
{

    if (!pstMeta)
    {
        throw string("pstMeta to processUnion is null");
    }

    if (Parameter::TYPE_INIT == oper)
    {
        csLines.push_back(getIndentStr(indentLevel) + "/*   Construct function */");

    }
    else if (Parameter::TYPE_ENCODE == oper)
    {
        const string& indent = getIndentStr(indentLevel);
        csLines.push_back(getIndentStr(indentLevel) + "/*   pack function */");
        csLines.push_back(indent + "public TdrError.ErrorType pack(Int64 selector, ref byte[] buffer, int size, ref int usedSize , uint cutVer)");
        csLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);


            csLines.push_back(indent + "if (0 == buffer.GetLength(0) || size > buffer.GetLength(0))");
            csLines.push_back(indent + "{");
            csLines.push_back(indent + "    return TdrError.ErrorType.TDR_ERR_INVALID_BUFFER_PARAMETER;");
            csLines.push_back(indent + "}");
            csLines.push_back("");
            csLines.push_back(indent + "TdrWriteBuf destBuf = new TdrWriteBuf(ref buffer, size);");
            csLines.push_back(indent + "TdrError.ErrorType ret = pack(selector, ref destBuf, cutVer);");
            csLines.push_back(indent + "if (ret == TdrError.ErrorType.TDR_NO_ERROR)");
            csLines.push_back(indent +"{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                csLines.push_back(indent + "buffer = destBuf.getBeginPtr();");
                csLines.push_back("");
                csLines.push_back(indent + "usedSize = destBuf.getUsedSize();");
            }
            csLines.push_back(indent +"}");
            csLines.push_back("");
            csLines.push_back(indent + "return ret;");
        }
        csLines.push_back(indent + "}");
        csLines.push_back("");

    }
    else if (Parameter::TYPE_DECODE == oper)
    {
        const string& indent = getIndentStr(indentLevel);

        csLines.push_back(indent + "public TdrError.ErrorType unpack(Int64 selector, ref byte[] buffer,int size, ref int usedSize , uint cutVer)");
        csLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);


            csLines.push_back(indent + "if (0 == buffer.GetLength(0) || size > buffer.GetLength(0))");
            csLines.push_back(indent + "{");
            csLines.push_back(indent + "    return TdrError.ErrorType.TDR_ERR_INVALID_BUFFER_PARAMETER;");
            csLines.push_back(indent + "}");
            csLines.push_back("");
            csLines.push_back(indent + "TdrReadBuf srcBuf = new TdrReadBuf(ref buffer,size);");
            csLines.push_back(indent + "TdrError.ErrorType ret = unpack(selector, ref srcBuf, cutVer);");
            csLines.push_back("");
            csLines.push_back(indent + "usedSize = srcBuf.getUsedSize();");
            csLines.push_back("");
            csLines.push_back(indent + "return ret;");
        }
        csLines.push_back(indent + "}");
        csLines.push_back("");
    }
    else if (Parameter::TYPE_LOAD == oper)
    {
        const string& indent = getIndentStr(indentLevel);

        csLines.push_back(indent + "public TdrError.ErrorType load(Int64 selector, ref byte[] buffer,int size, ref int usedSize , uint cutVer)");
        csLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);


            csLines.push_back(indent + "if (0 == buffer.GetLength(0) || size > buffer.GetLength(0))");
            csLines.push_back(indent + "{");
            csLines.push_back(indent + "    return TdrError.ErrorType.TDR_ERR_INVALID_BUFFER_PARAMETER;");
            csLines.push_back(indent + "}");
            csLines.push_back("");
            csLines.push_back(indent + "TdrReadBuf srcBuf = new TdrReadBuf(ref buffer,size);");
            csLines.push_back(indent + "TdrError.ErrorType ret = load(selector, ref srcBuf, cutVer);");
            csLines.push_back("");
            csLines.push_back(indent + "usedSize = srcBuf.getUsedSize();");
            csLines.push_back("");
            csLines.push_back(indent + "return ret;");
        }
        csLines.push_back(indent + "}");
        csLines.push_back("");
    }
    else if (Parameter::TYPE_VISUALIZE == oper)
    {
        const string& indent = getIndentStr(indentLevel);

        csLines.push_back(indent + "TdrError.ErrorType visualize(Int64 selector, ref string buffer, int indent, char separator)");
        csLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            csLines.push_back(indent + "TdrVisualBuf destBuf = new TdrVisualBuf();");
            csLines.push_back(indent + "TdrError.ErrorType ret = visualize(selector, ref destBuf, indent, separator);");

            csLines.push_back("");

            csLines.push_back(indent + "buffer = destBuf.getVisualBuf();");

            csLines.push_back("");
            csLines.push_back(indent + "return ret;");
        }
        csLines.push_back(indent + "}");
        csLines.push_back("");
    }
    else
    {
        throw string("unsupported operation to processUnion");
    }


    const string& indent = getIndentStr(indentLevel);

    if (Parameter::TYPE_INIT == oper)
    {
        csLines.push_back(getIndentStr(indentLevel) + "public TdrError.ErrorType construct(Int64 selector)");

    } else if (Parameter::TYPE_ENCODE == oper)
    {
        csLines.push_back(indent + "public TdrError.ErrorType pack(Int64 selector, ref TdrWriteBuf destBuf, uint cutVer)");
    } else if (Parameter::TYPE_DECODE == oper)
    {
        csLines.push_back(getIndentStr(indentLevel) + "/*   unpack function */");
        csLines.push_back(indent + "public TdrError.ErrorType unpack(Int64 selector, ref TdrReadBuf srcBuf, uint cutVer)");
    }
    else if (Parameter::TYPE_LOAD == oper)
    {
        csLines.push_back(getIndentStr(indentLevel) + "/*   load function */");
        csLines.push_back(indent + "public TdrError.ErrorType load(Int64 selector, ref TdrReadBuf srcBuf, uint cutVer)");
    }
    else if (Parameter::TYPE_VISUALIZE == oper)
    {
        csLines.push_back(getIndentStr(indentLevel) + "/*   visualize function */");
        csLines.push_back(indent + "public TdrError.ErrorType visualize(Int64 selector, ref TdrVisualBuf destBuf, int indent, char separator)");
    }

    csLines.push_back(indent + "{");

    if (Parameter::TYPE_LOAD == oper)
    {
        csLines.push_back(getIndentStr(indentLevel+1) + "srcBuf.disableEndian();");
    }

    generateUnionMethods(indentLevel + 1, csLines, pstMeta, oper);

    csLines.push_back(indent + "}");

}

void
TTransCSharp::generateUnionMethods(unsigned int indentLevel, std::list<string>& lines,
                                   LPTDRMETA pstMeta, Parameter::OperType oper)
{
    const string& indent = getIndentStr(indentLevel);
    if (Parameter::TYPE_ENCODE == oper
        || Parameter::TYPE_DECODE == oper
        || Parameter::TYPE_LOAD == oper)
    {
        adjustCutVersion(indentLevel, lines, pstMeta);
        lines.push_back("");
    }
    lines.push_back(indent + "TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;");
    lines.push_back("");

    LPTDRMETAENTRY pstEntry;
    LPTDRMETAENTRY pstDefaultEntry = NULL;
    string entryType;
    string entryName;
    string entryNameWithThis;
    string elseString;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        pstEntry = pstMeta->stEntries + i;

        if (TDR_ENTRY_DO_HAVE_ID(pstEntry))
        {

            string entryIdName = int2str(pstEntry->iID);

            lines.push_back(indent + elseString + "if (" + entryIdName + " == selector)");
            lines.push_back(indent + "{");
        } else if (TDR_ENTRY_DO_HAVE_MAXMIN_ID(pstEntry))
        {

            string entryMaxIdName = int2str(pstEntry->iMaxId);

            string entryMinIdName = int2str(pstEntry->iMinId);
            lines.push_back(indent + elseString + "if (" + entryMinIdName + " <= selector"
                            + " && selector <= " + entryMaxIdName + ")");
            lines.push_back(indent + "{");
        } else
        {
            pstDefaultEntry = pstEntry;
            continue;
        }

        if (elseString.empty())
        {
            elseString = "else ";
        }

        {
            entryName = getEntryName(pstEntry);
            entryNameWithThis = getEntryNameWithThis(pstEntry);
            Parameter parameter(lines, pstMeta, pstEntry, entryNameWithThis, oper);

            processUnionEntry(indentLevel + 1, lines, parameter);
        }

        lines.push_back(indent + "}");
    }

    if (pstDefaultEntry != NULL)
    {
        int furtherIndent = 0;
        if (1 < pstMeta->iEntriesNum)
        {
            furtherIndent++;
            lines.push_back(indent + "else");
            lines.push_back(indent + "{");
        }
        {
            entryNameWithThis = getEntryNameWithThis(pstDefaultEntry);
            Parameter parameter(lines, pstMeta, pstDefaultEntry, entryNameWithThis, oper);

            processUnionEntry(indentLevel + furtherIndent, lines, parameter);
        }
        if (furtherIndent > 0)
        {
            lines.push_back(indent + "}");
        }
    }

    lines.push_back("");
    lines.push_back(indent + "return ret;");
}



void
TTransCSharp::processUnionEntry(unsigned int indentLevel, list<string>& lines, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to processUnionEntry is null");
    }

    if (!parameter.pstMeta)
    {
        throw string("'parameter.pstMeta' to processUnionEntry is null");
    }

    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    LPTDRMETA pstMeta = parameter.pstMeta;
    const string& indent = getIndentStr(indentLevel);
    if (Parameter::TYPE_ENCODE == parameter.operType ||
        Parameter::TYPE_DECODE == parameter.operType ||
        Parameter::TYPE_LOAD == parameter.operType)
    {
        if (pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            string entryVersion = getEntryVersion(pstEntry, pstMeta);
            lines.push_back(indent + "if (" + entryVersion + " <= cutVer)");
            lines.push_back(indent + "{");
            {
                if (Parameter::TYPE_ENCODE == parameter.operType && !baseType.has(pstEntry->iType))
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + "if (null == " + parameter.variable + ")");
                    lines.push_back(indent +"{");
                    {
                        const string& indent = getIndentStr(indentLevel + 2);
                        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel + 2);
                        lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_ARG_IS_NULL;");
                    }
                    lines.push_back(indent + "}");
                    lines.push_back("");
                }

                else if (Parameter::TYPE_DECODE == parameter.operType && !baseType.has(pstEntry->iType))
                {
                    const string& indent = getIndentStr(indentLevel +1);
                    string entryType;
                    getEntryType(pstEntry,entryType);
                    lines.push_back(indent + "if (null " + parameter.variable + ")");
                    lines.push_back(indent +"{");
                    {
                        const string& indent = getIndentStr(indentLevel + 2);
                        lines.push_back(indent + parameter.variable + " = new " + entryType + "();");
                    }
                    lines.push_back(indent + "}");
                    lines.push_back("");
                }
                else if (Parameter::TYPE_LOAD == parameter.operType && !baseType.has(pstEntry->iType))
                {
                    const string& indent = getIndentStr(indentLevel +1);
                    string entryType;
                    getEntryType(pstEntry,entryType);
                    lines.push_back(indent + "if (null " + parameter.variable + ")");
                    lines.push_back(indent +"{");
                    {
                        const string& indent = getIndentStr(indentLevel + 2);
                        lines.push_back(indent + parameter.variable + " = new " + entryType + "();");
                    }
                    lines.push_back(indent + "}");
                    lines.push_back("");
                }
                (Parameter::TYPE_LOAD == parameter.operType) ? \
                                       processEntry4Load(indentLevel + 1, parameter) : \
                                       processEntry4Code(indentLevel + 1, parameter);
            }
            lines.push_back(indent + "}");
        } else
        {
            if (Parameter::TYPE_ENCODE == parameter.operType && !baseType.has(pstEntry->iType))
            {
                const string& indent = getIndentStr(indentLevel);
                lines.push_back(indent + "if (null == " + parameter.variable + ")");
                lines.push_back(indent +"{");
                {
                    TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel + 1);
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_ARG_IS_NULL;");
                }
                lines.push_back(indent + "}");
                lines.push_back("");
            }

            else if (Parameter::TYPE_DECODE == parameter.operType && !baseType.has(pstEntry->iType))
            {
                const string& indent = getIndentStr(indentLevel);
                string entryType;
                getEntryType(pstEntry,entryType);
                lines.push_back(indent + "if (null == " + parameter.variable + ")");
                lines.push_back(indent +"{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + parameter.variable + " = new " + entryType + "();");
                }
                lines.push_back(indent + "}");
                lines.push_back("");
            }
            else if (Parameter::TYPE_LOAD == parameter.operType && !baseType.has(pstEntry->iType))
            {
                const string& indent = getIndentStr(indentLevel);
                string entryType;
                getEntryType(pstEntry,entryType);
                lines.push_back(indent + "if (null == " + parameter.variable + ")");
                lines.push_back(indent +"{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + parameter.variable + " = new " + entryType + "();");
                }
                lines.push_back(indent + "}");
                lines.push_back("");
            }
            (Parameter::TYPE_LOAD == parameter.operType) ? \
                                   processEntry4Load(indentLevel, parameter) : \
                                   processEntry4Code(indentLevel, parameter);

        }

        if (Parameter::TYPE_DECODE == parameter.operType
            && pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            lines.push_back(indent + "else /* set default value */");
            lines.push_back(indent + "{");
            /* for entry of union, refer has no meaning */
            processEntry4Construct(indentLevel + 1, parameter);
            lines.push_back(indent + "}");
        }
        if (Parameter::TYPE_LOAD == parameter.operType
            && pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            lines.push_back(indent + "else /* set default value */");
            lines.push_back(indent + "{");
            /* for entry of union, refer has no meaning */
            processEntry4Construct(indentLevel + 1, parameter);
            lines.push_back(indent + "}");
        }
    } else if (Parameter::TYPE_VISUALIZE == parameter.operType)
    {

        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "if (null == " + parameter.variable + ")");
        lines.push_back(indent +"{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel + 1);
            lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_ARG_IS_NULL;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
        processEntry4Visualize(indentLevel, parameter);
    } else if (Parameter::TYPE_INIT == parameter.operType)
    {
        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "if (null == " + parameter.variable + ")");
        lines.push_back(indent +"{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            //TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel + 1);
            lines.push_back(indent + "return TdrError.ErrorType.TDR_NO_ERROR;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
        processEntry4Construct(indentLevel, parameter);
    }
}

void
TTransCSharp::processEntry4Load(unsigned int indentLevel, const Parameter& a_parameter)
{
    if (!a_parameter.pstMeta)
    {
        throw string("'a_parameter.pstMeta' to processEntry4Load is null");
    }
    if (!a_parameter.pstEntry)
    {
        throw string("'a_parameter.pstEntry' to processEntry4Load is null");
    }
    if (a_parameter.variable.empty())
    {
        throw string("'a_parameter.variable' to processEntry4Load is empty");
    }

    std::list<string>& lines = a_parameter.lines;
    LPTDRMETA pstMeta = a_parameter.pstMeta;
    LPTDRMETAENTRY pstEntry = a_parameter.pstEntry;
    const string& variable = a_parameter.variable;
    Parameter::OperType operation = a_parameter.operType;

    const string& indent = getIndentStr(indentLevel);
    unsigned int furtherIndent = 0;
    string arrIndex;

    const string& entryName = getEntryName(pstEntry);

    //string referPath;
    string loopVarType = "Int32";

    if (pstEntry->iType == TDR_TYPE_UNION)
    {
        const string selectName = string("selector4") + entryName;
        string selectPath;
        offset2path(pstMeta, pstEntry->stSelector.iHOff, selectPath);
        lines.push_back(indent + "Int64 " + selectName + " = " + selectPath + ";");
    }

    string loopCountVar;
    if (pstEntry->iCount != 1)
    {

        loopCountVar = int2str(pstEntry->iCount);

        const string& loopVarName = getEntryName(pstEntry) + "_i";
        lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                        + loopVarName + " < " + loopCountVar + "; "
                        + loopVarName + "++)");
        lines.push_back(indent + "{");
        furtherIndent = 1;
        arrIndex ="[" + loopVarName + "]";

    }

    Parameter parameter(lines, pstMeta, pstEntry, variable + arrIndex, operation);



    if (operation == Parameter::TYPE_LOAD)
    {
        processEntryLoad(indentLevel + furtherIndent, parameter);
    }
    if (pstEntry->iCount != 1)
    {

        if (furtherIndent)
        {
            lines.push_back(indent + "}");
        }
    }
}

void
TTransCSharp::processEntry4Code(unsigned int indentLevel, const Parameter& a_parameter)
{
    if (!a_parameter.pstMeta)
    {
        throw string("'a_parameter.pstMeta' to processEntry4Code is null");
    }
    if (!a_parameter.pstEntry)
    {
        throw string("'a_parameter.pstEntry' to processEntry4Code is null");
    }
    if (a_parameter.variable.empty())
    {
        throw string("'a_parameter.variable' to processEntry4Code is empty");
    }

    std::list<string>& lines = a_parameter.lines;
    LPTDRMETA pstMeta = a_parameter.pstMeta;
    LPTDRMETAENTRY pstEntry = a_parameter.pstEntry;
    const string& variable = a_parameter.variable;
    Parameter::OperType operation = a_parameter.operType;

    const string& indent = getIndentStr(indentLevel);
    unsigned int furtherIndent = 0;
    string arrIndex;

    const string& entryName = getEntryName(pstEntry);

    string referPath;
    string loopVarType;
    if (pstEntry->stRefer.iUnitSize > 0)
    {
        offset2path(pstMeta, pstEntry->stRefer.iHOff, referPath);
        LPTDRMETAENTRY pstReferEntry = tdr_get_entry_refer(pstEntry);
        loopVarType = baseType.getType(pstReferEntry->idxType);
        if ("sbyte" == loopVarType)
        {
            /* compiler will give a warning to use char as array-index */
            loopVarType = "int";
        }
        if (1 != pstEntry->iCount)
        {
            if (TDR_TYPE_UCHAR != pstReferEntry->iType
                && TDR_TYPE_SMALLUINT != pstReferEntry->iType
                && TDR_TYPE_UINT != pstReferEntry->iType
                && TDR_TYPE_ULONG != pstReferEntry->iType
                && TDR_TYPE_ULONGLONG != pstReferEntry->iType)
            {
                lines.push_back(indent + "if (0 > " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);

                    lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_MINUS_REFER_VALUE;");
                }
                lines.push_back(indent + "}");
            }
            if ((unsigned int)pstEntry->iCount < getReferTypeTopValue(pstReferEntry))
            {
                lines.push_back(indent + "if (" + int2str(pstEntry->iCount) + " < " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);

                    lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_REFER_SURPASS_COUNT;");
                }
                lines.push_back(indent + "}");
            }
        }
    }


    loopVarType = "Int32";


    if (pstEntry->iType == TDR_TYPE_UNION)
    {
        const string selectName = string("selector4") + entryName;
        string selectPath;
        offset2path(pstMeta, pstEntry->stSelector.iHOff, selectPath);
        lines.push_back(indent + "Int64 " + selectName + " = " + selectPath + ";");
    }

    string loopCountVar;
    if (pstEntry->iCount != 1)
    {

        loopCountVar = int2str(pstEntry->iCount);
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            loopCountVar = referPath;
        }

        const string& loopVarName = getEntryName(pstEntry) + "_i";
        lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                        + loopVarName + " < " + loopCountVar + "; "
                        + loopVarName + "++)");
        lines.push_back(indent + "{");
        furtherIndent = 1;
        arrIndex ="[" + loopVarName + "]";

    }
    else
    {
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            lines.push_back(indent + "if (1 == " + referPath + ")");
            lines.push_back(indent + "{");
            furtherIndent = 1;
        }
    }

    Parameter parameter(lines, pstMeta, pstEntry, variable + arrIndex, operation);



    if (operation == Parameter::TYPE_ENCODE)
    {
        processEntryPack(indentLevel + furtherIndent, parameter);
    } else if (operation == Parameter::TYPE_DECODE)
    {
        processEntryUnpack(indentLevel + furtherIndent, parameter);
    }

    if (furtherIndent)
    {
        lines.push_back(indent + "}");
    }
}

void
TTransCSharp::processEntry4Construct(unsigned int indentLevel, const Parameter& a_parameter)
{
    if (!a_parameter.pstMeta)
    {
        throw string("'a_parameter.pstMeta' to processEntry4Construct is null");
    }
    if (!a_parameter.pstEntry)
    {
        throw string("'a_parameter.pstEntry' to processEntry4Construct is null");
    }
    if (a_parameter.variable.empty())
    {
        throw string("'a_parameter.variable' to processEntry4Construct is empty");
    }

    std::list<string>& lines = a_parameter.lines;
    LPTDRMETA pstMeta = a_parameter.pstMeta;
    LPTDRMETAENTRY pstEntry = a_parameter.pstEntry;

    const string& indent = getIndentStr(indentLevel);
    string arrIndex;

    string referPath;
    string loopVarType;
    if (pstEntry->stRefer.iUnitSize > 0)
    {
        offset2path(pstMeta, pstEntry->stRefer.iHOff, referPath);
        //LPTDRMETAENTRY pstReferEntry = tdr_get_entry_refer(pstEntry);

    }

    loopVarType = "Int32";


    string loopCountVar;
    string loopVarName;
    if (1 != pstEntry->iCount)
    {
        loopCountVar = int2str(pstEntry->iCount);
        loopVarName = getEntryName(pstEntry) + "_i";
    }


    if (TDR_TYPE_UNION == pstEntry->iType
        || TDR_TYPE_STRUCT == pstEntry->iType)
    {
        unsigned int furtherIndent = 0;

        if (1 != pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
            constructNonLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
        }
        else if (1== pstEntry->iCount)
        {
            constructNonLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
        }

        if (0 < furtherIndent)
        {
            lines.push_back(indent + "}");
        }
    } else if (TDR_TYPE_STRING != pstEntry->iType
               && TDR_TYPE_WSTRING != pstEntry->iType
               && TDR_TYPE_IP != pstEntry->iType
               && TDR_TYPE_DATE != pstEntry->iType
               && TDR_TYPE_TIME != pstEntry->iType
               && TDR_TYPE_DATETIME != pstEntry->iType)
    {
        if (pstEntry->iCount != 0)
        {
            constructCommLeafEntry(indentLevel, a_parameter, loopVarType, loopCountVar);
        }

    } else if (TDR_TYPE_STRING == pstEntry->iType)
    {
        unsigned int furtherIndent = 0;

        if (1 != pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
            constructStringEntry(indentLevel + furtherIndent, a_parameter, loopVarName);

        }
        else if (1 == pstEntry->iCount)
        {
            constructStringEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
        }

        if (0 < furtherIndent)
        {
            lines.push_back(indent + "}");
        }

    } else if (TDR_TYPE_IP == pstEntry->iType
               || TDR_TYPE_DATE == pstEntry->iType
               || TDR_TYPE_TIME == pstEntry->iType
               || TDR_TYPE_DATETIME == pstEntry->iType)
    {
        if (1 == pstEntry->iCount)
        {
            constructBinarySpecilEntry(indentLevel, a_parameter, loopVarName);
        } else
        {

            if (1 != pstEntry->iCount)
            {
                constructBinarySpecilEntry(indentLevel, a_parameter, "0");
                const string& variable = a_parameter.variable;
                lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 1; "
                                + loopVarName + " < " + loopCountVar + "; "
                                + loopVarName + "++)");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + variable + "[" + loopVarName + "] = " + variable + "[0];");
                }
                lines.push_back(indent + "}");
            }
        }
    }
}

void
TTransCSharp::constructNonLeafEntry(unsigned int indentLevel, const Parameter& parameter, const string& arrIndex)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to constructNonLeafEntry is null");
    }
    if (!parameter.pstMeta)
    {
        throw string("'parameter.pstMeta' to constructNonLeafEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to constructNonLeafEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    string variable = parameter.variable;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    LPTDRMETA pstMeta = parameter.pstMeta;
    const string entryName = getEntryName(parameter.pstEntry);
    string arrIndexPara;
    if (arrIndex.length() > 0)
    {
        variable = variable + "[" + arrIndex + "]";
        arrIndexPara = arrIndex + ", ";
    }

    const string& indent = getIndentStr(indentLevel);

    string selectParameter;
    if (pstEntry->iType == TDR_TYPE_UNION)
    {
        string entryName = getEntryName(pstEntry);
        const string selectName = string("selector4") + entryName;
        string selectPath;
        offset2path(pstMeta, pstEntry->stSelector.iHOff, selectPath);
        lines.push_back(indent + "Int64 " + selectName + " = " + selectPath + ";");
        selectParameter = selectName;
    }

    lines.push_back(indent + "ret = " + variable + ".construct(" + selectParameter + ");");
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}


void TTransCSharp::constructCommLeafEntry(unsigned int indentLevel, const Parameter& parameter,
                                          const string& loopVarType, const string& loopCountVar)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to constructCommLeafEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to constructCommLeafEntry is empty");
    }

    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    std::list<string>& lines = parameter.lines;
    const string& variable = parameter.variable;
    const string entryName = getEntryName(parameter.pstEntry);

    const string& indent = getIndentStr(indentLevel);

    if (loopCountVar.length() > 0)
    {
        const string& loopVarName = getEntryName(pstEntry) + "_i";

        if (hasSpecialDefaultValue(pstEntry))
        {
            string defaultValue;
            getEntryDefaultValue(pstEntry, defaultValue);


            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; " + loopVarName + "++)");
            lines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 1);

                lines.push_back(indent + variable + "[" + loopVarName + "]" + " = " + defaultValue + ";");
            }
            lines.push_back(indent + "}");

        }

    }
    else
    {
        if (hasSpecialDefaultValue(pstEntry))
        {
            string defaultValue;
            getEntryDefaultValue(pstEntry, defaultValue);

            if (pstEntry->iType == TDR_TYPE_FLOAT)
            {
                lines.push_back(indent + variable + " = " + defaultValue + "f;");
            }
            else
            {
                lines.push_back(indent + variable + " = " + defaultValue + ";");
            }

        } else
        {
            lines.push_back(indent + variable + " = 0;");
        }
    }

}

void
TTransCSharp::constructBinarySpecilEntry(unsigned int indentLevel, const Parameter& parameter, const string& arrIndex)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to constructStringEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to constructStringEntry is empty");
    }

    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    std::list<string>& lines = parameter.lines;
    string variable = parameter.variable;
    const string entryName = getEntryName(parameter.pstEntry);

    string arrIndexPara;
    if (arrIndex.length() > 0)
    {
        variable = variable + "[" + arrIndex + "]";
        arrIndexPara = arrIndex + ", ";
    }

    const string& indent = getIndentStr(indentLevel);

    if (TDR_INVALID_PTR == pstEntry->ptrDefaultVal)
    {
        lines.push_back(indent + variable + " = 0;");
        return;
    }

    if (TDR_TYPE_IP == pstEntry->iType)
    {
        char buffer[32] = {0};
        tdr_ip_t tdrIp = *(tdr_ip_t*)TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrDefaultVal);
        string strTdrIp = tdr_tdrip_ntop(tdrIp, buffer, sizeof(buffer));
        if (strTdrIp.length() == 0)
        {
            throw string("tdr_tdrip_ntop from defaultValue failed");
        }
        lines.push_back(indent + "ret = TdrTypeUtil.str2TdrIP(out " + variable + ", \"" + strTdrIp + "\");");
    } else if (TDR_TYPE_TIME == pstEntry->iType)
    {
        char buffer[16] = {0};
        tdr_time_t* tdrTime = (tdr_time_t*)TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrDefaultVal);
        string strTdrTime = tdr_tdrtime_to_str_r(tdrTime, buffer, sizeof(buffer));
        if (strTdrTime.length() == 0)
        {
            throw string("tdr_tdrtime_to_str_r from defaultValue failed");
        }
        lines.push_back(indent + "ret = TdrTypeUtil.str2TdrTime(out " + variable + ", \"" + strTdrTime + "\");");
    } else if (TDR_TYPE_DATE == pstEntry->iType)
    {
        char buffer[16] = {0};
        tdr_date_t* tdrDate = (tdr_date_t*)TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrDefaultVal);
        string strTdrDate = tdr_tdrdate_to_str_r(tdrDate, buffer, sizeof(buffer));
        if (strTdrDate.length() == 0)
        {
            throw string("tdr_tdrdate_to_str_r from defaultValue failed");
        }
        lines.push_back(indent + "ret = TdrTypeUtil.str2TdrDate(out " + variable + ", \"" + strTdrDate + "\");");
    } else if (TDR_TYPE_DATETIME == pstEntry->iType)
    {
        char buffer[32] = {0};
        tdr_datetime_t* tdrDateTime = (tdr_datetime_t*)TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrDefaultVal);
        string strTdrDateTime = tdr_tdrdatetime_to_str_r(tdrDateTime, buffer, sizeof(buffer));
        if (strTdrDateTime.length() == 0)
        {
            throw string("tdr_tdrdatetime_to_str_r from defaultValue failed");
        }
        lines.push_back(indent + "ret = TdrTypeUtil.str2TdrDateTime(out " + variable + ", \"" + strTdrDateTime + "\");");
    } else
    {
        throw string("unsupported type<") + int2str(pstEntry->iType) + "> for constructBinarySpecilEntry";
    }

    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

}

void
TTransCSharp::constructStringEntry(unsigned int indentLevel, const Parameter& parameter, const string& arrIndex)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to constructStringEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to constructStringEntry is empty");
    }

    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    std::list<string>& lines = parameter.lines;
    string variable = parameter.variable;
    const string entryName = getEntryName(parameter.pstEntry);

    string arrIndexPara;

    if (arrIndex.length() > 0)
    {
        variable = variable + "[" + arrIndex + "]";

    }

    const string& indent = getIndentStr(indentLevel);

    if (hasSpecialDefaultValue(pstEntry) && (pstEntry->iCustomHUnitSize > 0))
    {
        string defaultValue;
        getEntryDefaultValue(pstEntry, defaultValue);
        string entryDefaultName = getEntryName( pstEntry) + "_def";
        lines.push_back(indent + "string " + entryDefaultName + " = " + "\"" + defaultValue + "\"" + ";");

        lines.push_back(indent +"byte []" + getEntryName( pstEntry)+ "_tmp" + " = Encoding.ASCII.GetBytes(" + entryDefaultName + ");");
        lines.push_back(indent +"if (" + getEntryName( pstEntry)+ "_tmp.GetLength(0) > "  + variable + ".GetLength(0) - 1)");
        lines.push_back(indent +"{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
            lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_STR_LEN_TOO_BIG;");
        }
        lines.push_back(indent +"}");
        lines.push_back(indent + "for(int i = 0;i < " + getEntryName( pstEntry)+ "_tmp.GetLength(0); i++)");
        lines.push_back(indent +"{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + variable + "[i] = " + getEntryName( pstEntry)+ "_tmp[i];");
        }
        lines.push_back(indent +"}");


    }
    else
    {
        lines.push_back(indent +"/* " + variable + " have no defaultvalue */");
    }
}

void
TTransCSharp::processEntryPack(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to processEntryPack is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to processEntryPack is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;

    const string& indent = getIndentStr(indentLevel);
    const string& entryName = getEntryName(pstEntry);
    const string& entrySizePosVarName = string("sizePos4") + entryName;
    const string& entryBeginBufPosVarName = string("beginPos4") + entryName ;

    if (pstEntry->stSizeInfo.iUnitSize > 0)
    {
        lines.push_back(indent + "/* record sizeinfo position */");
        if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET)
        {
            lines.push_back(indent + "Int32 " + entrySizePosVarName + " = "
                            + "savedBeginPos + " + int2str((int)pstEntry->stSizeInfo.iNOff) + ";");
        } else
        {
            lines.push_back(indent + "Int32 " + entrySizePosVarName + " = destBuf.getUsedSize();");
            lines.push_back("");
            lines.push_back(indent + "/* reserve space for sizeinfo */");
            switch (pstEntry->stSizeInfo.iUnitSize)
            {
                case 1:
                    lines.push_back(indent + "ret = destBuf.reserve(sizeof(byte));");
                    break;
                case 2:
                    lines.push_back(indent + "ret = destBuf.reserve(sizeof(UInt16));");
                    break;
                case 4:
                    lines.push_back(indent + "ret = destBuf.reserve(sizeof(UInt32));");
                    break;
                default:
                    throw string("invalid sizeinfo iUnitSize, this is a tdr-lib error");
                    break;
            }

            lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
            lines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 1);
                TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
                lines.push_back(indent + "return ret;");
            }
            lines.push_back(indent + "}");
            lines.push_back("");
        }
        lines.push_back(indent + "/* record real-data's begin postion in buf */");
        lines.push_back(indent + "Int32 " + entryBeginBufPosVarName + " = destBuf.getUsedSize();");
        lines.push_back("");
    }

    if (TDR_TYPE_STRING == pstEntry->iType
        || TDR_TYPE_WSTRING == pstEntry->iType)
    {
        packStringEntry(indentLevel, parameter);
    } else if (baseType.has(pstEntry->idxType))
    {
        packBaseTypeEntry(indentLevel, parameter);
    } else // struct, union
    {
        packCallableEntry(indentLevel, parameter);
    }

    if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
        || pstEntry->stSizeInfo.idxSizeType != TDR_INVALID_INDEX)
    {
        lines.push_back("");
        lines.push_back(indent + "/* set sizeinfo for " + parameter.variable + " */");
        const string& sizeInfoVar = "size4" + getEntryName(pstEntry);
        lines.push_back(indent + "Int32 " + sizeInfoVar + " = "
                        + "destBuf.getUsedSize() - " + entryBeginBufPosVarName + ";");
        directWrite(indentLevel, lines, pstEntry->stSizeInfo.iUnitSize,
                    "destBuf", entrySizePosVarName, sizeInfoVar,
                    "invalid sizeinfo iUnitSize, this is a tdr-lib error");
    }
}


void
TTransCSharp::packStringEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to packStringEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to packStringEntry is empty");
    }

    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    std::list<string>& lines = parameter.lines;
    const string& variable = parameter.variable;
    const string& indent = getIndentStr(indentLevel);
    const string& strLenName = string("realSize4") + getEntryName(pstEntry);

    if (0 < pstEntry->iCustomHUnitSize)
    {
        lines.push_back(indent + "if (" + variable + ".GetLength(0) >= " + int2str((int)pstEntry->iCustomHUnitSize) + ")");
        lines.push_back(indent + "{");
        lines.push_back(getIndentStr(indentLevel+1) + "return TdrError.ErrorType.TDR_ERR_STR_LEN_TOO_BIG;");
        lines.push_back(indent + "}");

        lines.push_back("");
    }

    if (TDR_TYPE_STRING == pstEntry->iType)
    {
        lines.push_back(indent + "Int32 " + strLenName + " = TdrTypeUtil.cstrlen("  + variable + ");");
        lines.push_back("");
        lines.push_back(indent + "/* pack */");
        lines.push_back(indent + "ret = destBuf.writeCString(" + variable + ", " + strLenName + ");");


    } else
    {
        lines.push_back(indent + "Int32 " + strLenName + " = TdrTypeUtil.wstrlen("  + variable + ");");
        lines.push_back("");
        lines.push_back(indent + "/* pack */");
        lines.push_back(indent + "ret = destBuf.writeWString(" + variable + ", " + strLenName + ");");
    }

    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");
    lines.push_back(indent + "/* gurantee string or wstring terminated with null character */");
    lines.push_back(indent + "ret = destBuf.writeUInt8(0x00);");
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

}

void
TTransCSharp::packBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to packBaseTypeEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to packBaseTypeEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);
    if (TDR_TYPE_VOID != pstEntry->iType)
    {


        lines.push_back(indent + "ret = destBuf." + baseType.getEncFunc(pstEntry->idxType) + "(" + variable + ");");

        lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
    } else
    {
        lines.push_back(indent + "/* no need to pack pointer */");
    }
}

void
TTransCSharp::packCallableEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to packCallableEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to packCallableEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);
    if (pstEntry->iType == TDR_TYPE_UNION)
    {
        const string selectName = string("selector4") + getEntryName(pstEntry);
        lines.push_back(indent + "ret = " + variable +
                        ".pack(" + selectName + ", ref destBuf, cutVer);");
    } else
    {
        lines.push_back(indent + "ret = " + variable + ".pack(ref destBuf, cutVer);");
    }
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::directWrite(unsigned int indentLevel, list<string>& lines, int switchValue,
                          const string& bufName, const string& pos, const string& value,
                          const string& throwStr)
{
    if (bufName.empty())
    {
        throw string("'bufName' to directWrite is empty");
    }
    if (pos.empty())
    {
        throw string("'pos' to directWrite is empty");
    }
    if (value.empty())
    {
        throw string("'value' to directWrite is empty");
    }
    if (throwStr.empty())
    {
        throw string("'throwStr' to directWrite is empty");
    }

    const string& indent = getIndentStr(indentLevel);

    string packString;
    switch (switchValue)
    {
        case 1:
            packString = "ret = " + bufName + ".writeUInt8((byte)(" + value + "), " + pos + ");";
            break;
        case 2:
            packString = "ret = " + bufName + ".writeUInt16((UInt16)(" + value + "), " + pos + ");";
            break;
        case 4:
            packString = "ret = " + bufName + ".writeUInt32((UInt32)(" + value + "), " + pos + ");";
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + packString);
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::directRead(unsigned int indentLevel, std::list<string>& lines, int switchValue,
                         const string& leftValue, const string& bufName, const string& pos,
                         const string& throwStr)
{
    if (leftValue.empty())
    {
        throw string("'leftValue' to directRead is empty");
    }
    if (bufName.empty())
    {
        throw string("'bufName' to directRead is empty");
    }
    if (pos.empty())
    {
        throw string("'pos' to directRead is empty");
    }
    if (throwStr.empty())
    {
        throw string("'throwStr' to directRead is empty");
    }

    const string& indent = getIndentStr(indentLevel);

    string readTinyFunc;
    string readShortFunc;
    string readIntFunc;
    readTinyFunc  = "readUInt8";
    readShortFunc = "readUInt16";
    readIntFunc   = "readUInt32";

    string readFunc = readIntFunc;
    string tempType;
    switch (switchValue)
    {
        case 1:
            readFunc = readTinyFunc;
            tempType = "byte";
            break;
        case 2:
            readFunc = readShortFunc;
            tempType = "UInt16";
            break;
        case 4:
            readFunc = readIntFunc;
            tempType = "UInt32";
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + tempType + " " + leftValue + " = 0;");
    lines.push_back(indent + "ret = " + bufName + "." + readFunc + "(ref " + leftValue + ", " + pos + ");");
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}


void
TTransCSharp::serialRead(unsigned int indentLevel, list<string>& lines, int switchValue,
                         const string& leftValue, const string& bufName,
                         const string& throwStr)
{
    if (leftValue.empty())
    {
        throw string("'leftValue' to serialRead is empty");
    }
    if (bufName.empty())
    {
        throw string("'bufName' to serialRead is empty");
    }
    if (throwStr.empty())
    {
        throw string("'throwStr' to serialRead is empty");
    }

    const string& indent = getIndentStr(indentLevel);

    string readTinyFunc;
    string readShortFunc;
    string readIntFunc;

    readTinyFunc  = "readUInt8";
    readShortFunc = "readUInt16";
    readIntFunc   = "readUInt32";

    string readFunc;
    string tempType;
    switch (switchValue)
    {
        case 1:
            readFunc = readTinyFunc;
            tempType = "byte";
            break;
        case 2:
            readFunc = readShortFunc;
            tempType = "UInt16";
            break;
        case 4:
            readFunc = readIntFunc;
            tempType = "UInt32";
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + tempType + " " + leftValue + " = 0;");
    lines.push_back(indent + "ret = " + bufName + "." + readFunc + "(ref " + leftValue + ");");
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::processEntryLoad(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to processEntryLoad is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to processEntryLoad is empty");
    }

    LPTDRMETAENTRY pstEntry = parameter.pstEntry;

    if (TDR_TYPE_STRING == pstEntry->iType
        || TDR_TYPE_WSTRING == pstEntry->iType)
    {
        loadStringEntry(indentLevel, parameter);
    } else if (baseType.has(pstEntry->idxType))
    {
        loadBaseTypeEntry(indentLevel, parameter);
    } else // struct, union
    {
        loadCallableEntry(indentLevel, parameter);
    }
}

void
TTransCSharp::processEntryUnpack(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to processEntryUnpack is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to processEntryUnpack is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;

    const string& indent = getIndentStr(indentLevel);
    const string& entryName = getEntryName(pstEntry);

    if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
        || pstEntry->stSizeInfo.idxSizeType != TDR_INVALID_INDEX)
    {
        lines.push_back(indent + "/* get sizeinfo for " + parameter.variable + " */");
        const string entrySizeinfoVar = string("size4") + entryName;
        if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET)
        {
            directRead(indentLevel, lines, pstEntry->stSizeInfo.iUnitSize,
                       entrySizeinfoVar, "srcBuf",
                       "savedBeginPos + " + int2str((int)pstEntry->stSizeInfo.iHOff),
                       "invalid stSizeInfo iUnitSize, this is a tdr-lib error");
        } else
        {
            serialRead(indentLevel, lines, pstEntry->stSizeInfo.iUnitSize,
                       entrySizeinfoVar, "srcBuf",
                       "invalid stSizeInfo iUnitSize, this is a tdr-lib error");
        }
        lines.push_back("");
        lines.push_back(indent + "/* check whether data in buffer are enough */");
        lines.push_back(indent + "if ((int)" + entrySizeinfoVar + " > srcBuf.getLeftSize())");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_SHORT_BUF_FOR_READ;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
    }

    if (TDR_TYPE_STRING == pstEntry->iType
        || TDR_TYPE_WSTRING == pstEntry->iType)
    {
        unpackStringEntry(indentLevel, parameter);
    } else if (baseType.has(pstEntry->idxType))
    {
        unpackBaseTypeEntry(indentLevel, parameter);
    } else // struct, union
    {
        unpackCallableEntry(indentLevel, parameter);
    }

}
void
TTransCSharp::loadStringEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to loadStringEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to loadStringEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& strSizeName = string("size4") + getEntryName(pstEntry);
    const string& indent = getIndentStr(indentLevel);

    lines.push_back(indent + "Int32 " + strSizeName + " = " + int2str((int)tdr_get_entry_unitsize(pstEntry)) + ";");

    lines.push_back("");

    lines.push_back(indent + "/* load */");
    if (TDR_TYPE_STRING == pstEntry->iType)
    {
        lines.push_back(indent + "ret = srcBuf.readCString(ref " + variable + ", (int)" + strSizeName + ");");
    }
    else
    {
        lines.push_back(indent + "ret = srcBuf.readWString(ref " + variable + ", (int)" + strSizeName + "/2);");
    }
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");
}

void
TTransCSharp::loadBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to loadBaseTypeEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to loadBaseTypeEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);

    lines.push_back(indent + "ret = srcBuf." + baseType.getDecFunc(pstEntry->idxType) + "(ref " + variable + ");");

    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::loadCallableEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstMeta)
    {
        throw string("'parameter.pstMeta' to unpackCallableEntry is null");
    }
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry'' to unpackCallableEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to unpackCallableEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);

    if (pstEntry->iType == TDR_TYPE_UNION)
    {
        const string selectName = string("selector4") + getEntryName(pstEntry);
        lines.push_back(indent + "ret = " + variable +
                        ".load(" + selectName + ", ref srcBuf, cutVer);");
    } else
    {
        lines.push_back(indent + "ret = " + variable + ".load(ref srcBuf, cutVer);");
    }
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::unpackStringEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to unpackStringEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to unpackStringEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& strSizeName = string("size4") + getEntryName(pstEntry);
    const string& indent = getIndentStr(indentLevel);

    if (pstEntry->iCustomHUnitSize > 0)
    {
        lines.push_back(indent + "/* check whether sizeinfo is valid */");
        if (TDR_TYPE_STRING == pstEntry->iType)
        {
            lines.push_back(indent + "if ((int)" + strSizeName + " > " + variable +".GetLength(0))");
        }
        else
        {
            lines.push_back(indent + "if ((int)" + strSizeName + " > 2 * " + variable +".GetLength(0))");
        }
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
            lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_STR_LEN_TOO_BIG;");
        }
        lines.push_back(indent + "}");
    }

    string minStrLen;
    if (TDR_TYPE_STRING == pstEntry->iType)
    {
        minStrLen = "1";
    } else
    {
        minStrLen = "2";
    }

    lines.push_back("");
    lines.push_back(indent + "/* string or wstring must contains a null character */");
    lines.push_back(indent + "if (" + minStrLen + " > " + strSizeName + ")");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_STR_LEN_TOO_SMALL;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");

    lines.push_back(indent + "/* unpack */");
    if (TDR_TYPE_STRING == pstEntry->iType)
    {
        lines.push_back(indent + "ret = srcBuf.readCString(ref " + variable + ", (int)" + strSizeName + ");");
    }
    else
    {
        lines.push_back(indent + "ret = srcBuf.readWString(ref " + variable + ", (int)" + strSizeName + "/2);");
    }
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");
    if (0 < pstEntry->iCustomHUnitSize)
    {
        lines.push_back(indent + "/* check whether string-content is consistent with sizeinfo */");
        if (TDR_TYPE_STRING == pstEntry->iType)
        {
            lines.push_back(indent + "if (0 != " + variable + "[(int)" + strSizeName + " - 1])");
            lines.push_back(indent + "{");
        } else
        {
            lines.push_back(indent + "if (0 != " + variable + "[(int)" + strSizeName + "/2 - 1])");
            lines.push_back(indent + "{");
        }
        {
            const string& indent = getIndentStr(indentLevel + 1);
            TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
            lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_STR_LEN_CONFLICT;");
        }
        lines.push_back(indent + "}");
    }

    const string& strLenName = string("realSize4") + getEntryName(pstEntry);
    if (TDR_TYPE_STRING == pstEntry->iType)
    {
        lines.push_back(indent + "Int32 " + strLenName + " = TdrTypeUtil.cstrlen("  + variable + ") + 1;");
    } else
    {
        lines.push_back(indent + "Int32 " + strLenName + " = 2* (TdrTypeUtil.wstrlen("  + variable + ") + 1);");
    }
    lines.push_back(indent + "if (" + strSizeName + " != " + strLenName +")");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines, indentLevel +1);
        lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_STR_LEN_CONFLICT;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::unpackBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to unpackBaseTypeEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to unpackBaseTypeEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);

    lines.push_back(indent + "ret = srcBuf." + baseType.getDecFunc(pstEntry->idxType) + "(ref " + variable + ");");

    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCSharp::unpackCallableEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstMeta)
    {
        throw string("'parameter.pstMeta' to unpackCallableEntry is null");
    }
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry'' to unpackCallableEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to unpackCallableEntry is empty");
    }

    std::list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);

    if (pstEntry->iType == TDR_TYPE_UNION)
    {
        const string selectName = string("selector4") + getEntryName(pstEntry);
        lines.push_back(indent + "ret = " + variable +
                        ".unpack(" + selectName + ", ref srcBuf, cutVer);");
    } else
    {
        lines.push_back(indent + "ret = " + variable + ".unpack(ref srcBuf, cutVer);");
    }
    lines.push_back(indent + "if (TdrError.ErrorType.TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}




unsigned int
TTransCSharp::getReferTypeTopValue(LPTDRMETAENTRY pstEntry)
{
    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryName is null");
    }

    switch (pstEntry->iType)
    {
        case TDR_TYPE_CHAR:
            return 0x7f;
            break;
        case TDR_TYPE_UCHAR:
            return 0xff;
            break;
        case TDR_TYPE_SMALLINT:
            return 0x7fff;
            break;
        case TDR_TYPE_SMALLUINT:
            return 0xffff;
            break;
        case TDR_TYPE_INT:
            return 0x7fffffff;
            break;
        case TDR_TYPE_UINT:
            return 0xffffffffu;
        default:
            throw string("refer type error, this is a tdr internal error");
            return 0;
    }
}

void
TTransCSharp::offset2path(LPTDRMETA pstMeta, ptrdiff_t offset, string& path)
{
    int iRet = TDR_SUCCESS;
    char buffer[1024];

    iRet = tdr_hostoff_to_path_i(pstMeta, -1, offset, buffer, sizeof(buffer));
    if ((unsigned)iRet == TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_SPACE_TO_WRITE))
    {
        throw string("tdr-offset-to-path has no buffer to write path");
    } else if (TDR_ERR_IS_ERROR(iRet))
    {
        throw string("tdr-offset-to-path error, ") + tdr_error_string(iRet);
    }
    buffer[sizeof(buffer)-1] = '\0';

    path = "";
    string rawPath = buffer;

    string stepRawName;
    string::size_type stepHeadPos = 0;
    string::size_type stepTailPos = 0;

    LPTDRMETA pstStepMeta = pstMeta;
    LPTDRMETAENTRY pstStepEntry = NULL;
    while (string::npos != stepHeadPos)
    {
        stepTailPos = rawPath.find_first_of(".", stepHeadPos);

        stepRawName = rawPath.substr(stepHeadPos, stepTailPos - stepHeadPos);

        int entryIndex = 0;
        int iRet = tdr_get_entry_by_name(&entryIndex, pstStepMeta, stepRawName.c_str());
        if (TDR_ERR_IS_ERROR(iRet))
        {
            throw string("failed to get entry by name '") + stepRawName
                + "' from meta '" + tdr_get_meta_name(pstStepMeta);
        }
        pstStepEntry = tdr_get_entry_by_index(pstStepMeta, entryIndex);
        if (NULL == pstStepEntry)
        {
            throw string("failed to get entry by idx '") + int2str(entryIndex)
                + string("' from meta '") + tdr_get_meta_name(pstStepMeta);
        }

        if (string::npos != stepTailPos)
        {
            path += getEntryName(pstStepEntry) + ".";
        } else
        {
            path += getEntryName(pstStepEntry);
            break;
        }
        pstStepMeta = TDR_PTR_TO_META((TDR_META_TO_LIB(pstMeta)), (pstStepEntry->ptrMeta));
        stepHeadPos = stepTailPos + 1;
    }

    path = string("this.") + path;
}

string
TTransCSharp::getEntryVersion(LPTDRMETAENTRY pstEntry, LPTDRMETA pstMeta)
{
    if (pstMeta->iBaseVersion == pstEntry->iVersion)
    {
        return string(tdr_get_meta_name(pstMeta)) + ".BASEVERSION";
    } else
    {
        return string(tdr_get_meta_name(pstMeta)) + ".VERSION_" + getEntryName(pstEntry);
    }
}

void
TTransCSharp::adjustCutVersion(unsigned int indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to adjustCutVersion is null");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "/* adjust cutversion */");
    lines.push_back(indent + "if (0 == cutVer || "
                    + tdr_get_meta_name(pstMeta) + ".CURRVERSION < cutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "cutVer = " + tdr_get_meta_name(pstMeta) + ".CURRVERSION;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "/* check cutversion */");
    lines.push_back(indent + "if (" + tdr_get_meta_name(pstMeta) + ".BASEVERSION > cutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        TSF4G_TDR_CS_DEBUG_TRACE(lines,indentLevel + 1);
        lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_CUTVER_TOO_SMALL;");
    }
    lines.push_back(indent + "}");
    if (pstMeta->stVersionIndicator.iUnitSize > 0)
    {
        string version;
        offset2version(pstMeta, pstMeta->stVersionIndicator.iHOff, version);
        lines.push_back(indent + "if (" + version + " > cutVer)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            TSF4G_TDR_CS_DEBUG_TRACE(lines,indentLevel + 1);
            lines.push_back(indent + "return TdrError.ErrorType.TDR_ERR_CUTVER_CONFILICT;");
        }
        lines.push_back(indent + "}");
    }
}


string
TTransCSharp::getEntryNameWithThis(LPTDRMETAENTRY pstEntry)
{
    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryNameWithThis is null");
    }

    return string("this.") + getEntryName(pstEntry);

}

void
TTransCSharp::version(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to version is null");
    }


    const string& indent = getIndentStr(indentLevel);
    // pstMeta version info
    lines.push_back(indent + "/*  pstMeta version info */");
    lines.push_back(indent + "public static readonly uint BASEVERSION = "
                    + int2str(pstMeta->iBaseVersion) + ";");
    lines.push_back(indent + "public static readonly uint CURRVERSION = "
                    + int2str(pstMeta->iCurVersion) + ";");

    lines.push_back(indent + "/*  entry version info */");
    // entry version info
    LPTDRMETAENTRY pstEntry;
    string entryName;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        pstEntry = pstMeta->stEntries + i;
        if (pstMeta->iBaseVersion != pstEntry->iVersion)
        {
            entryName = getEntryName(pstEntry);
            lines.push_back(indent + "public static readonly uint VERSION_" + entryName + " = "
                            + int2str(pstEntry->iVersion) + ";");
        }
    }
}

void
TTransCSharp::offset2version(LPTDRMETA pstMeta, ptrdiff_t offset, string& version)
{
    int iRet = TDR_SUCCESS;
    char buffer[1024];

    iRet = tdr_hostoff_to_path_i(pstMeta, -1, offset, buffer, sizeof(buffer));
    if ((unsigned)iRet == TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_SPACE_TO_WRITE))
    {
        throw string("tdr-offset-to-version has no buffer to write path");
    } else if (TDR_ERR_IS_ERROR(iRet))
    {
        throw string("tdr-offset-to-version error, ") + tdr_error_string(iRet);
    }
    buffer[sizeof(buffer)-1] = '\0';

    string rawPath = buffer;

    string stepRawName;
    string::size_type stepHeadPos = 0;
    string::size_type stepTailPos = 0;

    LPTDRMETA pstStepMeta = pstMeta;
    LPTDRMETAENTRY pstStepEntry = NULL;
    while (string::npos != stepHeadPos)
    {
        stepTailPos = rawPath.find_first_of(".", stepHeadPos);

        stepRawName = rawPath.substr(stepHeadPos, stepTailPos - stepHeadPos);

        int entryIndex = 0;
        int iRet = tdr_get_entry_by_name(&entryIndex, pstStepMeta, stepRawName.c_str());
        if (TDR_ERR_IS_ERROR(iRet))
        {
            throw string("failed to get entry by name '") + stepRawName
                + "' from meta '" + tdr_get_meta_name(pstStepMeta);
        }
        pstStepEntry = tdr_get_entry_by_index(pstStepMeta, entryIndex);
        if (NULL == pstStepEntry)
        {
            throw string("failed to get entry by idx '") + int2str(entryIndex)
                + string("' from meta '") + string(tdr_get_meta_name(pstStepMeta));
        }

        if (string::npos == stepTailPos)
        {
            break;
        }
        pstStepMeta = TDR_PTR_TO_META((TDR_META_TO_LIB(pstMeta)), (pstStepEntry->ptrMeta));
        stepHeadPos = stepTailPos + 1;
    }

    version = getEntryVersion(pstStepEntry, pstStepMeta);
}

void
TTransCSharp::flagMeta4Default()
{
    for (int i = 0; i < pstMetaLib->iCurMetaNum; i++)
    {
        LPTDRMETA pstMeta = TDR_IDX_TO_META(pstMetaLib, i);

        for (int j = 0; j < pstMeta->iEntriesNum; j++)
        {
            LPTDRMETAENTRY pstEntry = tdr_get_entry_by_index(pstMeta, j);

            if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
            {
                LPTDRMETA pstEntryMeta = tdr_get_entry_type_meta(pstMetaLib, pstEntry);

                if (defaultSet.end() != defaultSet.find(tdr_get_meta_name(pstEntryMeta)))
                {
                    defaultSet.insert(tdr_get_meta_name(pstMeta));
                    break;
                }
            } else
            {
                if (hasSpecialDefaultValue(pstEntry))
                {
                    defaultSet.insert(tdr_get_meta_name(pstMeta));
                    break;
                }
            }
        }
    }
}


bool
TTransCSharp::needSetDefaultValue(LPTDRMETA pstMeta)
{
    if (NULL == pstMeta)
    {
        throw string("pstMeta to needSetDefaultValue is null");
    }

    const char* pszMetaName = tdr_get_meta_name(pstMeta);

    if (defaultSet.end() != defaultSet.find(pszMetaName))
    {
        return true;
    } else
    {
        return false;
    }
}

bool
TTransCSharp::needSetDefaultValue(LPTDRMETAENTRY pstEntry)
{
    if (NULL == pstEntry)
    {
        throw string("pstEntry to needSetDefaultValue is null");
    }

    if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
    {
        LPTDRMETA pstMeta = tdr_get_entry_type_meta(pstMetaLib, pstEntry);
        const char* pszMetaName = tdr_get_meta_name(pstMeta);

        if (defaultSet.end() != defaultSet.find(pszMetaName))
        {
            return true;
        }
    } else
    {
        if (hasSpecialDefaultValue(pstEntry))
        {
            return true;
        }
    }

    return false;
}

bool
TTransCSharp::hasSpecialDefaultValue(LPTDRMETAENTRY pstEntry)
{
    if (NULL == pstEntry)
    {
        throw string("pstEntry to hasSpecialDefaultValue is null");
    }

    if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
    {
        throw string("hasSpecialDefaultValue is only for atom type entry");
    }

    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryNameWithThis is null");
    }

    if (TDR_INVALID_PTR == pstEntry->ptrDefaultVal)
    {
        return false;
    }

    const char* pszDefault = TDR_GET_STRING_BY_PTR(TDR_META_TO_LIB(TDR_ENTRY_TO_META(pstEntry)), pstEntry->ptrDefaultVal);
    switch(pstEntry->iType)
    {
        case TDR_TYPE_STRING:
            if (strlen(pszDefault) > 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_CHAR:
            if (pszDefault[0] != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_UCHAR:
        case TDR_TYPE_BYTE:
            if ((unsigned char )pszDefault[0] != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_SMALLINT:
            if (*(short *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_SMALLUINT:
            if (*(unsigned short *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_INT:
        case TDR_TYPE_LONG:
            if (*(int *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_UINT:
        case TDR_TYPE_ULONG:
            if (*(uint32_t *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_LONGLONG:
            if (*(int64_t *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_ULONGLONG:
            if (*(int64_t *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_FLOAT:
            if (*(float *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_DOUBLE:
            if (*(double *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_WCHAR:
            if (*(tdr_wchar_t *)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_IP:
            if (*(tdr_ip_t*)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_TIME:
            if (*(tdr_time_t*)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_DATE:
            if (*(tdr_date_t*)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_DATETIME:
            if (*(tdr_datetime_t*)pszDefault != 0)
            {
                return true;
            }
            break;
        case TDR_TYPE_WSTRING:
            fprintf(stderr, "warning: for C#, defaultvalue for 'wstring' is not supported yet.\n");
            break;
    }

    return false;
}
void
TTransCSharp::getEntryDefaultValue(LPTDRMETAENTRY pstEntry, string& entryDefaultValue)
{
    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryNameWithThis is null");
    }

    if (TDR_INVALID_PTR == pstEntry->ptrDefaultVal)
    {
        throw string("getEntryNameWithThis, but entry<")
            + tdr_get_entry_name(pstEntry)+ "> has no defaultvalue";
    }

    char defaultValue[TDR_DEFAULT_VALUE_LEN] = {0};

    const char* pszDefault = TDR_GET_STRING_BY_PTR(TDR_META_TO_LIB(TDR_ENTRY_TO_META(pstEntry)), pstEntry->ptrDefaultVal);
    switch(pstEntry->iType)
    {
        case TDR_TYPE_STRING:
            sprintf(defaultValue, "%s", pszDefault);
            break;
        case TDR_TYPE_CHAR:
            sprintf(defaultValue, "%d", (int)pszDefault[0]);
            break;
        case TDR_TYPE_UCHAR:
        case TDR_TYPE_BYTE:
            sprintf(defaultValue, "%d", (int)(unsigned char )pszDefault[0]);
            break;
        case TDR_TYPE_SMALLINT:
            sprintf(defaultValue, "%d", (int)*(short *)pszDefault);
            break;
        case TDR_TYPE_SMALLUINT:
            sprintf(defaultValue, "%d", (int)*(unsigned short *)pszDefault);
            break;
        case TDR_TYPE_INT:
        case TDR_TYPE_LONG:
            sprintf(defaultValue, "%d", (int)*(int *)pszDefault);
            break;
        case TDR_TYPE_UINT:
        case TDR_TYPE_ULONG:
            sprintf(defaultValue, "%u", (uint32_t)*(uint32_t *)pszDefault);
            break;
        case TDR_TYPE_LONGLONG:
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
            sprintf(defaultValue, "%I64i", (int64_t)*(int64_t *)pszDefault);
#else
            sprintf(defaultValue, "%"TDRPRId64"", (int64_t)*(int64_t *)pszDefault);
#endif
            break;
        case TDR_TYPE_ULONGLONG:
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
            sprintf(defaultValue, "%I64u", (uint64_t)*(uint64_t *)pszDefault);
#else
            sprintf(defaultValue, "%"TDRPRIud64"", (uint64_t)*(uint64_t *)pszDefault);
#endif
            break;
        case TDR_TYPE_FLOAT:
            sprintf(defaultValue, "%f", *(float *)pszDefault);
            break;
        case TDR_TYPE_DOUBLE:
            sprintf(defaultValue, "%f", *(double *)pszDefault);
            break;
        case TDR_TYPE_WCHAR:
            {
                sprintf(defaultValue, "0x%04x", (uint32_t)*(tdr_wchar_t *)pszDefault);
                break;
            }
        case TDR_TYPE_WSTRING:
            break;
        default:
            throw string("unsupported TDR_TYPE<") + int2str(pstEntry->iType) + ">";
            break;
    }
    defaultValue[sizeof(defaultValue)-1] = '\0';

    entryDefaultValue = defaultValue;
}

