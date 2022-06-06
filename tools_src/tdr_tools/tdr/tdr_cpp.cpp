#include <ctime>
#include <cctype>
#include <string>
#include <cassert>
#include <iostream>
#include "tdr_cpp.h"
#include "tdr_debug.h"
#include "../tdr_tools.h"
#include "../../lib_src/tdr/tdr_md5.h"
#include "../../lib_src/tdr/tdr_ctypes_info_i.h"
#include "../../lib_src/tdr/tdr_metalib_kernel_i.h"
#include "../../lib_src/tdr/tdr_metalib_manage_i.h"


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::list;

string TTransCPP::ttranslatorVersion = "1.0";

TTransCPP::TTransCPP(const string& indentStr, const string& outputDir)
    : TTransBase(indentStr, outputDir, "// "), pstOption(NULL) {}

void
TTransCPP::checkMetaLib()
{
    bool checkFailed = false;
    string xmlTag;

    for (int i = 0; i < pstMetaLib->iCurMetaNum; i++)
    {
        LPTDRMETA pstMeta = TDR_IDX_TO_META(pstMetaLib, i);

        for (int j = 0; j < pstMeta->iEntriesNum; j++)
        {
            LPTDRMETAENTRY pstEntry = tdr_get_entry_by_index(pstMeta, j);

            if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
            {
                cerr << "error: for C++, reference type is NOT support yet, ";
                cerr << "invalid entry:" << endl;

                checkFailed = true;
            } else if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
            {
                cerr << "error: for C++, pointer type is NOT support yet, ";
                cerr << "invalid entry:" << endl;

                checkFailed = true;
            } else if (TDR_TYPE_WSTRING == pstEntry->iType
                       && TDR_INVALID_PTR != pstEntry->ptrDefaultVal)
            {
                cerr << "warning: for C++, defaultvalue for 'wstring' is NOT supported yet, ";
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
TTransCPP::makeComm() {
    /* meta-lib-independent files */
    makeReadMe();
    makeTdrErr();
    makeTdrPal();
    makeTdrBuf();
    makeTdrBufUtil();
    makeTdrTypeUtil();
    makeTdrThird();
    makeTdrTime();
}

void
TTransCPP::translate(LPTDRMETALIB pstMetaLib,
                     const std::list<XMLDesc>& xmlDescList,
                     TdrOption* pstOption)
{
    if (!pstMetaLib)
    {
        throw string("pstMetaLib to translate is null");
    }

    if (!pstOption)
    {
        throw string("pstOption to translate is null");
    }

    TDEBUG_PRINTLN(TTransCPP::ttranslatorVersion);

    this->pstOption = pstOption;
    this->pstMetaLib = pstMetaLib;
    this->packageName = tdr_get_metalib_name(pstMetaLib);

    checkMetaLib();

    /* meta-lib-dependent files */
    makeMetaLibInfo();

    flagMeta4Default();

    std::list<XMLDesc>::const_iterator descIter;
    for (descIter = xmlDescList.begin(); descIter != xmlDescList.end(); ++descIter)
    {
        this->headFileName = descIter->headFileName;
        this->cppFileName  = descIter->cppFileName;


        {
            DEFINE_CONTAINER();

            string defineGuard = tdr_filepath_to_guard(this->headFileName);
            APPEND_CONTAINER_HEAD("#ifndef " + defineGuard);
            APPEND_CONTAINER_HEAD("#define " + defineGuard);
            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD("#include \"TdrBuf.h\"");
            APPEND_CONTAINER_HEAD("#include \"TdrError.h\"");
            APPEND_CONTAINER_HEAD("#include \"TdrBufUtil.h\"");
            APPEND_CONTAINER_HEAD("#include \"TdrTypeUtil.h\"");
            APPEND_CONTAINER_HEAD("");

            std::list<string>::const_iterator iter;
            for (iter = descIter->includes.begin(); iter != descIter->includes.end(); ++iter)
            {
                APPEND_CONTAINER_HEAD("#include \"" + *iter + "\"");
            }

            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD("using tsf4g_tdr::TdrError;");
            APPEND_CONTAINER_HEAD("using tsf4g_tdr::TdrReadBuf;");
            APPEND_CONTAINER_HEAD("using tsf4g_tdr::TdrWriteBuf;");

            if (pstOption->unitedNameSpace || !descIter->nameSpace.empty())
            {
                APPEND_CONTAINER_HEAD("");
                APPEND_CONTAINER_HEAD("namespace " + this->packageName);
                APPEND_CONTAINER_HEAD("{");
            }

            /* if no meta, *.cpp is not needed */
            if (descIter->metas.size() > 0)
            {
                APPEND_CONTAINER_CPP("#include \"" + this->headFileName + "\"");
                APPEND_CONTAINER_CPP("#include \"TdrTypeUtil.h\"");
                APPEND_CONTAINER_CPP("");
                APPEND_CONTAINER_CPP("using tsf4g_tdr::TdrBufUtil;");
                APPEND_CONTAINER_CPP("using tsf4g_tdr::TdrTypeUtil;");

                if (pstOption->unitedNameSpace || !descIter->nameSpace.empty())
                {
                    APPEND_CONTAINER_CPP("");
                    APPEND_CONTAINER_CPP("namespace " + this->packageName);
                    APPEND_CONTAINER_CPP("{");
                }
            }

            OUTPUT_CONTAINER(headFileName, cppFileName);
        }


        translateMacros(descIter->macros);
        translateGroups(descIter->groups);

        {
            DEFINE_CONTAINER();
            APPEND_CONTAINER_HEAD("#pragma pack(1)");
            OUTPUT_CONTAINER(headFileName, cppFileName);
        }

        translateMetas(descIter->metas);

        {
            DEFINE_CONTAINER();

            APPEND_CONTAINER_HEAD("#pragma pack()");
            APPEND_CONTAINER_HEAD("");
            if (pstOption->unitedNameSpace || !descIter->nameSpace.empty())
            {
                APPEND_CONTAINER_HEAD("}");
            }
            APPEND_CONTAINER_HEAD("#endif");

            /* if no meta, *.cpp is not needed */
            if (descIter->metas.size() > 0)
            {
                if (pstOption->unitedNameSpace || !descIter->nameSpace.empty())
                {
                    APPEND_CONTAINER_CPP("}");
                }
            }

            OUTPUT_CONTAINER(headFileName, cppFileName);
        }
    }
}

void
TTransCPP::translateMetas(const std::list<string>& metas)
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
TTransCPP::translateGroups(const std::list<string>& groups)
{
    std::list<string> headLines;
    const unsigned indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);

    std::list<string>::const_iterator iter;
    for (iter = groups.begin(); iter != groups.end(); ++iter)
    {
        string desc;

        LPTDRMACROSGROUP pstGroup = tdr_get_macrosgroup_by_name(pstMetaLib, iter->c_str());
        if (NULL == pstGroup)
        {
            throw string("tdr_get_macrosgroup_by_name failed, by name ") + *iter;
        }

        if (TDR_INVALID_PTR != pstGroup->ptrDesc)
        {
            desc = string(" // ") + TDR_GET_STRING_BY_PTR(pstMetaLib, pstGroup->ptrDesc);
        }

        headLines.push_back(indent + "enum " + *iter + desc);
        headLines.push_back(indent + "{");
        {
            const unsigned indentLevel = 1;
            const string& indent = getIndentStr(indentLevel);

            int iCount = tdr_get_macrosgroup_macro_num(pstGroup);
            for (int i = 0; i < iCount; i++)
            {
                string desc;

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

                if (TDR_INVALID_PTR != pstMacro->ptrDesc)
                {
                    desc = string(" // ") + tdr_get_macro_desc_by_ptr(pstMetaLib, pstMacro);
                }

                headLines.push_back(indent + tdr_get_macro_name_by_ptr(pstMetaLib, pstMacro)
                                    + " = " + int2str(iValue) + "," + desc);
            }
        }
        headLines.push_back(indent + "};");
    }

    this->output(headLines, this->headFileName);
}

void
TTransCPP::translateMacros(const std::list<string>& macros)
{
    if (macros.empty())
    {
        return;
    }

    std::list<string> headLines;
    const unsigned indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);

    headLines.push_back(indent + "enum");
    headLines.push_back(indent + "{");
    {
        const unsigned indentLevel = 1;
        const string& indent = getIndentStr(indentLevel);

        std::list<string>::const_iterator iter;
        for (iter = macros.begin(); iter != macros.end(); ++iter)
        {
            string desc;

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

            if (TDR_INVALID_PTR != pstMacro->ptrDesc)
            {
                desc = string(" // ") + tdr_get_macro_desc_by_ptr(pstMetaLib, pstMacro);
            }

            headLines.push_back(indent + tdr_get_macro_name_by_ptr(pstMetaLib, pstMacro)
                                + " = " + int2str(iValue) + "," + desc);
        }
    }
    headLines.push_back(indent + "};");

    this->output(headLines, this->headFileName);
}

void
TTransCPP::translateStruct(LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to translateStruct is null");
    }

    DEFINE_CONTAINER();
    {
        const unsigned indentLevel = 0;
        const string& indent = getIndentStr(indentLevel);

        string desc;

        if (TDR_INVALID_PTR != pstMeta->ptrDesc)
        {
            desc = string(" // ") + tdr_get_meta_desc(pstMeta);
        }

        headLines.push_back(indent + "class " + tdr_get_meta_name(pstMeta) + desc);
        headLines.push_back(indent + "{");
        {
            const unsigned indentLevel = 1;
            const string& indent = getIndentStr(indentLevel);
            headLines.push_back(indent + "public: /* members */");
            {
                declaration(indentLevel + 1, headLines, pstMeta);
            }

            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD(indent + "public: /* methods */");
            {
                const unsigned indentLevel = 2;

                structConstruct(indentLevel, headLines, cppLines, pstMeta);

                APPEND_CONTAINER_CPP("");
                structPack(indentLevel, headLines, cppLines, pstMeta);

                APPEND_CONTAINER_CPP("");
                structUnpack(indentLevel, headLines, cppLines, pstMeta);

                if (this->pstOption->visualizeOn)
                {
                    APPEND_CONTAINER_CPP("");
                    structVisualize(indentLevel, headLines, cppLines, pstMeta);
                }
            }

            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD(indent + "public: /* version infomation */");
            {
                version(indentLevel + 1, headLines, pstMeta);
            }
        }
        headLines.push_back(indent + "};");

        headLines.push_back("");
        outputTypedef(headLines, indent, pstMeta);
    }

    {
        this->output(headLines, this->headFileName);
        this->output(cppLines, this->cppFileName);
    }
}

void
TTransCPP::translateUnion(LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to translateUnion is null");
    }

    DEFINE_CONTAINER();
    {
        // 1rt indent-level
        const string& indent = getIndentStr(0);

        string desc;

        if (TDR_INVALID_PTR != pstMeta->ptrDesc)
        {
            desc = string(" // ") + tdr_get_meta_desc(pstMeta);
        }

        headLines.push_back(indent + "class " + tdr_get_meta_name(pstMeta) + desc);
        headLines.push_back(indent + "{");
        {
            const unsigned indentLevel = 1;
            const string& indent = getIndentStr(indentLevel);
            headLines.push_back(indent + "public: /* members */");
            {
                declaration(indentLevel + 1, headLines, pstMeta);
            }

            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD(indent + "public: /* methods */");
            {
                const unsigned indentLevel = 2;

                processUnion(indentLevel, headLines, cppLines, pstMeta, Parameter::TYPE_INIT);

                APPEND_CONTAINER_CPP("");
                processUnion(indentLevel, headLines, cppLines, pstMeta, Parameter::TYPE_ENCODE);

                APPEND_CONTAINER_CPP("");
                processUnion(indentLevel, headLines, cppLines, pstMeta, Parameter::TYPE_DECODE);

                if (this->pstOption->visualizeOn)
                {
                    APPEND_CONTAINER_CPP("");
                    processUnion(indentLevel, headLines, cppLines, pstMeta, Parameter::TYPE_VISUALIZE);
                }
            }

            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD(indent + "public: /* version infomation */");
            {
                version(indentLevel + 1, headLines, pstMeta);
            }
        }
        headLines.push_back(indent + "};");

        headLines.push_back("");
        outputTypedef(headLines, indent, pstMeta);
    }

    {
        this->output(headLines, this->headFileName);
        this->output(cppLines, this->cppFileName);
    }
}

void
TTransCPP::output(std::list<string>& lines, const string& fileName, bool careMeta)
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
TTransCPP::addTdrInfo(std::list<string>& lines, bool careMeta)
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
                           (unsigned)pstMetaLib->iSize);
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
TTransCPP::declaration(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to declaration is null");
    }

    const string& indent = getIndentStr(indentLevel);
    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryType;

    int furtherIndent = 0;
    if (TDR_TYPE_UNION == pstMeta->iType)
    {
        lines.push_back(indent + "union");
        lines.push_back(indent + "{");
        furtherIndent++;
    }
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        string desc;

        const string& indent = getIndentStr(indentLevel + furtherIndent);

        pstEntry = pstMeta->stEntries + i;
        entryName = TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrName);
        entryName = getEntryName(pstEntry);
        TDEBUG_PRINTLN(entryName);
        getEntryType(pstEntry, entryType);

        string strLen;
        if (TDR_TYPE_STRING == pstEntry->iType || TDR_TYPE_WSTRING == pstEntry->iType)
        {
            if( TDR_INVALID_INDEX != pstEntry->idxCustomHUnitSize )
            {
                TDRPTR ptrMacro = (TDR_GET_MACRO_TABLE(this->pstMetaLib))[pstEntry->idxCustomHUnitSize].ptrMacro;
                strLen = string("[") + TDR_GET_STRING_BY_PTR(this->pstMetaLib, ptrMacro) + "]";
            } else if (0 < pstEntry->iCustomHUnitSize)
            {
                strLen = "[" + int2str(pstEntry->iCustomHUnitSize) + "]";
            } else
            {
                strLen = "[1]";
            }
        }

        TDEBUG_PRINTLN(entryType);

        if (TDR_INVALID_PTR != pstEntry->ptrDesc)
        {
            desc = string(" // ") + tdr_get_entry_desc(pstEntry);
        }

        if (0 == pstEntry->iCount)
        {
            lines.push_back(indent + entryType + " " + getEntryName(pstEntry)
                            + "[1]" + strLen + ";" + desc);
        } else if (1 == pstEntry->iCount)
        {
            lines.push_back(indent + entryType + " " + getEntryName(pstEntry)
                            + strLen + ";" + desc);
        } else
        {
            lines.push_back(indent + entryType + " " + getEntryName(pstEntry)
                            + "[" + getEntryCount(pstEntry) + "]" + strLen + ";" + desc);
        }
    }
    if (TDR_TYPE_UNION == pstMeta->iType)
    {
        lines.push_back(indent + "};");
    }
}

void
TTransCPP::structConstruct(unsigned indentLevel, std::list<string>& headLines,
                           std::list<string>& cppLines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structConstruct is null");
    }

    headLines.push_back(getIndentStr(indentLevel) + "TdrError::ErrorType construct();");
    headLines.push_back("");

    indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);
    cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta) + "::construct()");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        cppLines.push_back(indent + "TdrError::ErrorType ret = TdrError::TDR_NO_ERROR;");

        cppLines.push_back("");
        if (needSetDefaultValue(pstMeta))
        {
            constructStructEntry(indentLevel + 1, cppLines, pstMeta);
        } else
        {
            const string& indent = getIndentStr(indentLevel + 1);
            cppLines.push_back(indent + "memset(this, 0, sizeof(*this));");
        }

        cppLines.push_back("");
        cppLines.push_back(indent + "return ret;");
    }
    cppLines.push_back(indent + "}");
}

void
TTransCPP::structVisualize(unsigned indentLevel, std::list<string>& headLines,
                           std::list<string>& cppLines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structVisualize is null");
    }

    {
        const string& indent = getIndentStr(indentLevel);
        headLines.push_back(indent + "/* set indent = -1 to disable indent */");
        headLines.push_back(indent + "TdrError::ErrorType visualize"
                            "(TdrWriteBuf& destBuf, int indent = 0, char separator = '\\n') const;");
        headLines.push_back(indent + "TdrError::ErrorType visualize(char* buffer, size_t size, size_t* usedSize = NULL,");
        headLines.push_back(indent + "                              int indent = 0, char separator = '\\n') const;");
        headLines.push_back(indent + "const char* visualize_ex"
                            "(TdrWriteBuf& destBuf, int indent = 0, char separator = '\\n') const;");
        headLines.push_back(indent + "const char* visualize_ex(char* buffer, size_t size, size_t* usedSize = NULL,");
        headLines.push_back(indent + "                         int indent = 0, char separator = '\\n') const;");
        headLines.push_back("");
    }

    indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);

    cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                       + "::visualize(char* buffer, size_t size, size_t* usedSize, int indent, char separator) const");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        cppLines.push_back(indent + "if (NULL == buffer)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);

            cppLines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            cppLines.push_back(indent + "return TdrError::TDR_ERR_NULL_POINTER_PARAMETER;");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
        cppLines.push_back(indent + "TdrWriteBuf destBuf(buffer, size);");
        cppLines.push_back(indent + "TdrError::ErrorType ret = visualize(destBuf, indent, separator);");
        cppLines.push_back(indent + "if (NULL != usedSize)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            cppLines.push_back(indent + "*usedSize = destBuf.getUsedSize();");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
        cppLines.push_back(indent + "return ret;");
    }
    cppLines.push_back(indent + "}");
    cppLines.push_back("");

    cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                       + "::visualize(TdrWriteBuf& destBuf, int indent, char separator) const");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        cppLines.push_back(indent + "TdrError::ErrorType ret = TdrError::TDR_NO_ERROR;");

        cppLines.push_back("");
        visualizeEntry(indentLevel + 1, cppLines, pstMeta);

        cppLines.push_back("");
        cppLines.push_back(indent + "return ret;");
    }
    cppLines.push_back(indent + "}");
    cppLines.push_back("");

    cppLines.push_back(indent + "const char*\n" + tdr_get_meta_name(pstMeta)
                       + "::visualize_ex(char* buffer, size_t size, size_t* usedSize, int indent, char separator) const");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        cppLines.push_back(indent + "if (NULL == buffer || 0 == size)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            cppLines.push_back(indent + "return \"\";");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");

        cppLines.push_back(indent + "TdrWriteBuf destBuf(buffer, size);");
        cppLines.push_back(indent + "visualize(destBuf, indent, separator);");
        cppLines.push_back("");

        cppLines.push_back(indent + "size_t targetPos = destBuf.getUsedSize();");
        cppLines.push_back(indent + "const size_t totalSize = size;");
        cppLines.push_back(indent + "if (totalSize <= targetPos)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            cppLines.push_back(indent + "targetPos = totalSize - 1;");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back(indent + "buffer[targetPos] = '\\0';");
        cppLines.push_back("");

        cppLines.push_back(indent + "if (NULL != usedSize)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            cppLines.push_back(indent + "*usedSize = destBuf.getUsedSize();");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");

        cppLines.push_back(indent + "return destBuf.getBeginPtr();");
    }
    cppLines.push_back(indent + "}");
    cppLines.push_back("");

    cppLines.push_back(indent + "const char*\n" + tdr_get_meta_name(pstMeta)
                       + "::visualize_ex(TdrWriteBuf& destBuf, int indent, char separator) const");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        cppLines.push_back(indent + "if (NULL == destBuf.getBeginPtr() || 0 == destBuf.getTotalSize())");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            cppLines.push_back(indent + "return \"\";");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");

        cppLines.push_back(indent + "visualize(destBuf, indent, separator);");
        cppLines.push_back("");

        cppLines.push_back(indent + "size_t targetPos = destBuf.getUsedSize();");
        cppLines.push_back(indent + "const size_t totalSize = destBuf.getTotalSize();");
        cppLines.push_back(indent + "if (totalSize <= targetPos)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            cppLines.push_back(indent + "targetPos = totalSize - 1;");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back(indent + "destBuf.writeChar('\\0', targetPos);");
        cppLines.push_back("");

        cppLines.push_back(indent + "return destBuf.getBeginPtr();");
    }
    cppLines.push_back(indent + "}");
}

void
TTransCPP::structPack(unsigned indentLevel, std::list<string>& headLines,
                      std::list<string>& cppLines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structPack is null");
    }

    {
        const string& indent = getIndentStr(indentLevel);
        headLines.push_back(indent + "TdrError::ErrorType pack"
                            "(TdrWriteBuf& destBuf, unsigned cutVer = 0) const;");
        headLines.push_back(indent + "TdrError::ErrorType pack"
                            "(char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0) const;");
        headLines.push_back("");
    }

    indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);

    cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                       + "::pack(char* buffer, size_t size, size_t* usedSize, unsigned cutVer) const");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        cppLines.push_back(indent + "if (NULL == buffer)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);

            cppLines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            cppLines.push_back(indent + "return TdrError::TDR_ERR_NULL_POINTER_PARAMETER;");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
        cppLines.push_back(indent + "TdrWriteBuf destBuf(buffer, size);");
        cppLines.push_back(indent + "TdrError::ErrorType ret = pack(destBuf, cutVer);");
        cppLines.push_back(indent + "if (NULL != usedSize)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            cppLines.push_back(indent + "*usedSize = destBuf.getUsedSize();");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
        cppLines.push_back(indent + "return ret;");
    }
    cppLines.push_back(indent + "}");
    cppLines.push_back("");

    cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                       + "::pack(TdrWriteBuf& destBuf, unsigned cutVer) const");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        cppLines.push_back(indent + "TdrError::ErrorType ret = TdrError::TDR_NO_ERROR;");

        cppLines.push_back("");
        adjustCutVersion(indentLevel + 1, cppLines, pstMeta);

        // save space for sizeinfo
        if (pstMeta->stSizeType.iUnitSize > 0)
        {
            cppLines.push_back("");
            prepare4sizeinfo(indentLevel + 1, cppLines, pstMeta);
        }

        // records destBuf.getUsedSize()
        if (0 < pstMeta->stVersionIndicator.iUnitSize
            || 0 < pstMeta->stSizeType.iUnitSize)
        {
            cppLines.push_back("");
            cppLines.push_back(indent + "const size_t savedBeginPos = destBuf.getUsedSize();");
        } else
        {
            for (int i = 0; i < pstMeta->iEntriesNum; i++)
            {
                LPTDRMETAENTRY pstEntry = pstMeta->stEntries + i;
                if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
                    && pstEntry->stSizeInfo.iUnitSize > 0)
                {
                    cppLines.push_back("");
                    cppLines.push_back(indent + "const size_t savedBeginPos = destBuf.getUsedSize();");
                    break;
                }
            }
        }

        cppLines.push_back("");
        packStructEntry(indentLevel + 1, cppLines, pstMeta);

        packVersionIndicator(indentLevel + 1, cppLines, pstMeta);

        packSizeinfo(indentLevel + 1, cppLines, pstMeta);

        cppLines.push_back("");
        cppLines.push_back(indent + "return ret;");
    }
    cppLines.push_back(indent + "}");
}

void
TTransCPP::structUnpack(unsigned indentLevel, std::list<string>& headLines,
                        std::list<string>& cppLines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structUnpack is null");
    }

    {
        const string& indent = getIndentStr(indentLevel);
        headLines.push_back(indent + "TdrError::ErrorType unpack"
                            "(TdrReadBuf& srcBuf, unsigned cutVer = 0);");
        headLines.push_back(indent + "TdrError::ErrorType unpack"
                            "(const char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0);");
        headLines.push_back("");
    }

    indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);

    cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta) +
                       "::unpack(const char* buffer, size_t size, size_t* usedSize, unsigned cutVer)");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        cppLines.push_back(indent + "if (NULL == buffer)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);

            cppLines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            cppLines.push_back(indent + "return TdrError::TDR_ERR_NULL_POINTER_PARAMETER;");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
        cppLines.push_back(indent + "TdrReadBuf srcBuf(buffer, size);");
        cppLines.push_back(indent + "TdrError::ErrorType ret = unpack(srcBuf, cutVer);");
        cppLines.push_back(indent + "if (NULL != usedSize)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            cppLines.push_back(indent + "*usedSize = srcBuf.getUsedSize();");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
        cppLines.push_back(indent + "return ret;");
    }
    cppLines.push_back(indent + "}");
    cppLines.push_back("");

    cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                       + "::unpack(TdrReadBuf& srcBuf, unsigned cutVer)");
    cppLines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        cppLines.push_back(indent + "TdrError::ErrorType ret = TdrError::TDR_NO_ERROR;");

        cppLines.push_back("");

        if (pstMeta->stVersionIndicator.iUnitSize > 0)
        {
            unpackVersionIndicator(indentLevel + 1, cppLines, pstMeta);
        } else
        {
            adjustCutVersion(indentLevel + 1, cppLines, pstMeta);
        }

        if (pstMeta->stSizeType.iUnitSize > 0)
        {
            cppLines.push_back("");
            unpackSizeinfo(indentLevel + 1, cppLines, pstMeta);
        }

        // records srcBuf.getUsedSize()
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            LPTDRMETAENTRY pstEntry = pstMeta->stEntries + i;
            if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
                && pstEntry->stSizeInfo.iUnitSize > 0)
            {
                cppLines.push_back("");
                cppLines.push_back(indent + "const size_t savedBeginPos = srcBuf.getUsedSize();");
                break;
            }
        }

        cppLines.push_back("");
        unpackStructEntry(indentLevel + 1, cppLines, pstMeta);

        cppLines.push_back("");
        cppLines.push_back(indent + "return ret;");
    }
    cppLines.push_back(indent + "}");
}

void
TTransCPP::version(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to version is null");
    }

    const string& indent = getIndentStr(indentLevel);

    lines.push_back(indent + "enum");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        // pstMeta version info
        lines.push_back(indent + "BASEVERSION = "
                        + int2str(pstMeta->iBaseVersion) + ",");
        lines.push_back(indent + "CURRVERSION = "
                        + int2str(pstMeta->iCurVersion) + ",");

        // entry version info
        LPTDRMETAENTRY pstEntry;
        string entryName;
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            pstEntry = pstMeta->stEntries + i;
            if (pstMeta->iBaseVersion != pstEntry->iVersion)
            {
                entryName = getEntryName(pstEntry);
                lines.push_back(indent + "VERSION_" + entryName + " = "
                                + int2str(pstEntry->iVersion) + ",");
            }
        }
    }
    lines.push_back(indent + "};");
}

void
TTransCPP::generateUnionMethods(unsigned indentLevel, std::list<string>& lines,
                                LPTDRMETA pstMeta, Parameter::OperType oper)
{
    const string& indent = getIndentStr(indentLevel);
    if (Parameter::TYPE_ENCODE == oper || Parameter::TYPE_DECODE == oper)
    {
        adjustCutVersion(indentLevel, lines, pstMeta);
        lines.push_back("");
    }
    lines.push_back(indent + "TdrError::ErrorType ret = TdrError::TDR_NO_ERROR;");
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
            string entryIdName = tdr_get_entry_id_name(TDR_META_TO_LIB(pstMeta), pstEntry);
            if (entryIdName.length() == 0)
            {
                entryIdName = int2str(pstEntry->iID);
            } else
            {
                entryIdName = entryIdName;
            }
            lines.push_back(indent + elseString + "if (" + entryIdName + " == selector)");
            lines.push_back(indent + "{");
        } else if (TDR_ENTRY_DO_HAVE_MAXMIN_ID(pstEntry))
        {
            string entryMaxIdName = tdr_get_entry_maxid_name(TDR_META_TO_LIB(pstMeta), pstEntry);
            if (entryMaxIdName.length() == 0)
            {
                entryMaxIdName = int2str(pstEntry->iMaxId);
            } else
            {
                entryMaxIdName = entryMaxIdName;
            }
            string entryMinIdName = tdr_get_entry_minid_name(TDR_META_TO_LIB(pstMeta), pstEntry);
            if (entryMinIdName.length() == 0)
            {
                entryMinIdName = int2str(pstEntry->iMinId);
            } else
            {
                entryMinIdName = entryMinIdName;
            }
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
TTransCPP::processUnionEntry(unsigned indentLevel, std::list<string>& lines, const Parameter& parameter)
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
    if (Parameter::TYPE_ENCODE == parameter.operType || Parameter::TYPE_DECODE == parameter.operType)
    {
        if (pstEntry->iVersion != pstMeta->iBaseVersion)
        {
            string entryVersion = getEntryVersion(pstEntry, pstMeta);
            lines.push_back(indent + "if (" + entryVersion + " <= cutVer)");
            lines.push_back(indent + "{");
            {
                processEntry4Code(indentLevel + 1, parameter);
            }
            lines.push_back(indent + "}");
        } else
        {
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
    } else if (Parameter::TYPE_VISUALIZE == parameter.operType)
    {
        processEntry4Visualize(indentLevel, parameter);
    } else if (Parameter::TYPE_INIT == parameter.operType)
    {
        processEntry4Construct(indentLevel, parameter);
    }
}

void
TTransCPP::processUnion(unsigned indentLevel, std::list<string>& headLines,
                        std::list<string>& cppLines, LPTDRMETA pstMeta, Parameter::OperType oper)
{
    if (!pstMeta)
    {
        throw string("pstMeta to processUnion is null");
    }

    if (Parameter::TYPE_INIT == oper)
    {
        headLines.push_back(getIndentStr(indentLevel) + "TdrError::ErrorType construct(int64_t selector);");
        headLines.push_back("");
    } else if (Parameter::TYPE_ENCODE == oper)
    {
        const string& indent = getIndentStr(indentLevel);
        headLines.push_back(indent + "TdrError::ErrorType pack"
                            "(int64_t selector, TdrWriteBuf& destBuf, unsigned cutVer = 0) const;");
        headLines.push_back(indent + "TdrError::ErrorType pack(int64_t selector, char* buffer, size_t size,");
        headLines.push_back(indent + "                         size_t* usedSize = NULL, unsigned cutVer = 0) const;");
        headLines.push_back("");
    } else if (Parameter::TYPE_DECODE == oper)
    {
        const string& indent = getIndentStr(indentLevel);
        headLines.push_back(indent + "TdrError::ErrorType unpack"
                            "(int64_t selector, TdrReadBuf& srcBuf, unsigned cutVer = 0);");
        headLines.push_back(indent + "TdrError::ErrorType unpack(int64_t selector, const char* buffer, size_t size,");
        headLines.push_back(indent + "                           size_t* usedSize = NULL, unsigned cutVer = 0);");
        headLines.push_back("");
    } else if (Parameter::TYPE_VISUALIZE == oper)
    {
        const string& indent = getIndentStr(indentLevel);
        headLines.push_back(indent + "TdrError::ErrorType visualize(int64_t selector, TdrWriteBuf& destBuf,");
        headLines.push_back(indent + "                              int indent = 0, char separator = '\\n') const;");
        headLines.push_back(indent + "TdrError::ErrorType visualize(int64_t selector, char* buffer, size_t size,");
        headLines.push_back(indent + "                              size_t* usedSize = NULL, int indent = 0, char separator = '\\n') const;");
        headLines.push_back("");
    } else
    {
        throw string("unsupported operation to processUnion");
    }

    indentLevel = 0;
    const string& indent = getIndentStr(indentLevel);
    if (Parameter::TYPE_ENCODE == oper)
    {
        const string& indent = getIndentStr(indentLevel);
        cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                           + "::pack(int64_t selector, char* buffer, size_t size, size_t* usedSize, unsigned cutVer) const");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            cppLines.push_back(indent + "if (NULL == buffer)");
            cppLines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);

                cppLines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                cppLines.push_back(indent + "return TdrError::TDR_ERR_NULL_POINTER_PARAMETER;");
            }
            cppLines.push_back(indent + "}");
            cppLines.push_back("");
            cppLines.push_back(indent + "TdrWriteBuf destBuf(buffer, size);");
            cppLines.push_back(indent + "TdrError::ErrorType ret = pack(selector, destBuf, cutVer);");
            cppLines.push_back(indent + "if (NULL != usedSize)");
            cppLines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                cppLines.push_back(indent + "*usedSize = destBuf.getUsedSize();");
            }
            cppLines.push_back(indent + "}");
            cppLines.push_back("");
            cppLines.push_back(indent + "return ret;");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
    } else if (Parameter::TYPE_DECODE == oper)
    {
        const string& indent = getIndentStr(indentLevel);
        cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                           + "::unpack(int64_t selector, const char* buffer, size_t size, size_t* usedSize, unsigned cutVer)");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            cppLines.push_back(indent + "if (NULL == buffer)");
            cppLines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);

                cppLines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                cppLines.push_back(indent + "return TdrError::TDR_ERR_NULL_POINTER_PARAMETER;");
            }
            cppLines.push_back(indent + "}");
            cppLines.push_back("");
            cppLines.push_back(indent + "TdrReadBuf srcBuf(buffer, size);");
            cppLines.push_back(indent + "TdrError::ErrorType ret = unpack(selector, srcBuf, cutVer);");
            cppLines.push_back(indent + "if (NULL != usedSize)");
            cppLines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                cppLines.push_back(indent + "*usedSize = srcBuf.getUsedSize();");
            }
            cppLines.push_back(indent + "}");
            cppLines.push_back("");
            cppLines.push_back(indent + "return ret;");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
    } else if (Parameter::TYPE_VISUALIZE == oper)
    {
        const string& indent = getIndentStr(indentLevel);
        cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                           + "::visualize(int64_t selector, char* buffer, size_t size, size_t* usedSize, int indent, char separator) const");
        cppLines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            cppLines.push_back(indent + "if (NULL == buffer)");
            cppLines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);

                cppLines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                cppLines.push_back(indent + "return TdrError::TDR_ERR_NULL_POINTER_PARAMETER;");
            }
            cppLines.push_back(indent + "}");
            cppLines.push_back("");
            cppLines.push_back(indent + "TdrWriteBuf destBuf(buffer, size);");
            cppLines.push_back(indent + "TdrError::ErrorType ret = visualize(selector, destBuf, indent, separator);");
            cppLines.push_back(indent + "if (NULL != usedSize)");
            cppLines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                cppLines.push_back(indent + "*usedSize = destBuf.getUsedSize();");
            }
            cppLines.push_back(indent + "}");
            cppLines.push_back("");
            cppLines.push_back(indent + "return ret;");
        }
        cppLines.push_back(indent + "}");
        cppLines.push_back("");
    }

    if (Parameter::TYPE_INIT == oper)
    {
        cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                           + "::construct(int64_t selector)");
    } else if (Parameter::TYPE_ENCODE == oper)
    {
        cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                           + "::pack(int64_t selector, TdrWriteBuf& destBuf, unsigned cutVer) const");
    } else if (Parameter::TYPE_DECODE == oper)
    {
        cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                           + "::unpack(int64_t selector, TdrReadBuf& srcBuf, unsigned cutVer)");
    } else if (Parameter::TYPE_VISUALIZE == oper)
    {
        cppLines.push_back(indent + "TdrError::ErrorType\n" + tdr_get_meta_name(pstMeta)
                           + "::visualize(int64_t selector, TdrWriteBuf& destBuf, int indent, char separator) const");
    }

    cppLines.push_back(indent + "{");

    generateUnionMethods(indentLevel + 1, cppLines, pstMeta, oper);

    cppLines.push_back(indent + "}");
}

void
TTransCPP::adjustCutVersion(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to adjustCutVersion is null");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "/* adjust cutversion */");
    lines.push_back(indent + "if (0 == cutVer || "
                    + tdr_get_meta_name(pstMeta) + "::CURRVERSION < cutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "cutVer = " + tdr_get_meta_name(pstMeta) + "::CURRVERSION;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "/* check cutversion */");
    lines.push_back(indent + "if (" + tdr_get_meta_name(pstMeta) + "::BASEVERSION > cutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return TdrError::TDR_ERR_CUTVER_TOO_SMALL;");
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
            lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            lines.push_back(indent + "return TdrError::TDR_ERR_CUTVER_CONFILICT;");
        }
        lines.push_back(indent + "}");
    }
}

void
TTransCPP::packVersionIndicator(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
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
TTransCPP::unpackVersionIndicator(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
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
                    + "::BASEVERSION > netCutVer || netCutVer > "
                    + tdr_get_meta_name(pstMeta) + "::CURRVERSION)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return TdrError::TDR_ERR_NET_CUTVER_INVALID;");
    }
    lines.push_back(indent + "}");

    string version;
    offset2version(pstMeta, pstMeta->stVersionIndicator.iHOff, version);
    lines.push_back(indent + "if (" + version + " > netCutVer)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return TdrError::TDR_ERR_CUTVER_CONFILICT;");
    }
    lines.push_back(indent + "}");

    lines.push_back(indent + "cutVer = netCutVer;");
}

void
TTransCPP::packSizeinfo(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
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
        lines.push_back(indent + "const size_t sizeInBuf = destBuf.getUsedSize() - savedBeginPos;");
        directWrite(indentLevel, lines, pstMeta->stSizeType.iUnitSize,
                    "destBuf", "metaSizePos",
                    "sizeInBuf", "invalid sizeinfo iUnitSize, this is a tdr-lib error");
    }
}

void
TTransCPP::prepare4sizeinfo(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to prepare4sizeinfo is null");
    }

    const string& indent = getIndentStr(indentLevel);
    TDEBUG_PRINTLN(pstMeta->stSizeType.iUnitSize);
    TDEBUG_PRINTLN(pstMeta->stSizeType.iNOff);

    unsigned offset = 0;
    string position = "destBuf.getUsedSize()";
    if (pstMeta->stSizeType.iNOff > 0)
    {
        offset = pstMeta->stSizeType.iNOff;
        position += " + " + int2str((int)offset);
    }
    lines.push_back(indent + "const size_t metaSizePos = " + position + ";");

    if (pstMeta->stSizeType.idxSizeType != TDR_INVALID_INDEX)
    {
        lines.push_back("");
        lines.push_back(indent + "/* reserve space for struct's sizeinfo */");
        switch (pstMeta->stSizeType.iUnitSize)
        {
            case 1:
                lines.push_back(indent + "ret = destBuf.reserve(sizeof(uint8_t));");
                break;
            case 2:
                lines.push_back(indent + "ret = destBuf.reserve(sizeof(uint16_t));");
                break;
            case 4:
                lines.push_back(indent + "ret = destBuf.reserve(sizeof(uint32_t));");
                break;
            default:
                throw string("invalid sizeinfo iUnitSize, this is a tdr-lib error");
                break;
        }
        lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
    }
}

void
TTransCPP::unpackSizeinfo(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
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

        unsigned offset = 0;
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
            lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            lines.push_back(indent + "return TdrError::TDR_ERR_SHORT_BUF_FOR_READ;");
        }
        lines.push_back(indent + "}");
    }
}

void
TTransCPP::constructStringEntry(unsigned indentLevel, const Parameter& parameter, const string& arrIndex)
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

    if (hasSpecialDefaultValue(pstEntry))
    {
        string defaultValue;
        getEntryDefaultValue(pstEntry, defaultValue);
        lines.push_back(indent + "strncpy(" + variable + ", \"" + defaultValue + "\", sizeof(" + variable + "));");
    } else
    {
        lines.push_back(indent + variable + "[0] = '\\0';");
    }
}

void
TTransCPP::constructBinarySpecilEntry(unsigned indentLevel, const Parameter& parameter, const string& arrIndex)
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
        lines.push_back(indent + "ret = TdrTypeUtil::str2TdrIP(" + variable + ", \"" + strTdrIp + "\");");
    } else if (TDR_TYPE_TIME == pstEntry->iType)
    {
        char buffer[16] = {0};
        tdr_time_t* tdrTime = (tdr_time_t*)TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrDefaultVal);
        string strTdrTime = tdr_tdrtime_to_str_r(tdrTime, buffer, sizeof(buffer));
        if (strTdrTime.length() == 0)
        {
            throw string("tdr_tdrtime_to_str_r from defaultValue failed");
        }
        lines.push_back(indent + "ret = TdrTypeUtil::str2TdrTime(" + variable + ", \"" + strTdrTime + "\");");
    } else if (TDR_TYPE_DATE == pstEntry->iType)
    {
        char buffer[16] = {0};
        tdr_date_t* tdrDate = (tdr_date_t*)TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrDefaultVal);
        string strTdrDate = tdr_tdrdate_to_str_r(tdrDate, buffer, sizeof(buffer));
        if (strTdrDate.length() == 0)
        {
            throw string("tdr_tdrdate_to_str_r from defaultValue failed");
        }
        lines.push_back(indent + "ret = TdrTypeUtil::str2TdrDate(" + variable + ", \"" + strTdrDate + "\");");
    } else if (TDR_TYPE_DATETIME == pstEntry->iType)
    {
        char buffer[32] = {0};
        tdr_datetime_t* tdrDateTime = (tdr_datetime_t*)TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrDefaultVal);
        string strTdrDateTime = tdr_tdrdatetime_to_str_r(tdrDateTime, buffer, sizeof(buffer));
        if (strTdrDateTime.length() == 0)
        {
            throw string("tdr_tdrdatetime_to_str_r from defaultValue failed");
        }
        lines.push_back(indent + "ret = TdrTypeUtil::str2TdrDateTime(" + variable + ", \"" + strTdrDateTime + "\");");
    } else
    {
        throw string("unsupported type<") + int2str(pstEntry->iType) + "> for constructBinarySpecilEntry";
    }

    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::visualizeSpecialLeafEntry(unsigned indentLevel, const Parameter& parameter, const string& arrIndex)
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
        printFunc = "TdrBufUtil::printString";
    } else if (TDR_TYPE_WSTRING == pstEntry->iType)
    {
        printFunc = "TdrBufUtil::printWString";
    } else if (TDR_TYPE_IP == pstEntry->iType)
    {
        printFunc = "TdrBufUtil::printTdrIP";
    } else if (TDR_TYPE_TIME == pstEntry->iType)
    {
        printFunc = "TdrBufUtil::printTdrTime";
    } else if (TDR_TYPE_DATE == pstEntry->iType)
    {
        printFunc = "TdrBufUtil::printTdrDate";
    } else if (TDR_TYPE_DATETIME == pstEntry->iType)
    {
        printFunc = "TdrBufUtil::printTdrDateTime";
    }

    assert(printFunc.length() > 0);

    lines.push_back(indent + "ret = " + printFunc + "(destBuf, indent, separator, \"["
                    + entryName + "]\", " + arrIndexPara + variable + ");");
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::constructNonLeafEntry(unsigned indentLevel, const Parameter& parameter, const string& arrIndex)
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
        lines.push_back(indent + "const int64_t " + selectName + " = " + selectPath + ";");
        selectParameter = selectName;
    }

    lines.push_back(indent + "ret = " + variable + ".construct(" + selectParameter + ");");
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}
void
TTransCPP::visualizeNonLeafEntry(unsigned indentLevel, const Parameter& parameter, const string& arrIndex)
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
    lines.push_back(indent + "ret = TdrBufUtil::printVariable(destBuf, indent, separator, \"["
                    + entryName + "]\", " + arrIndexPara + "true);");
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
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
        lines.push_back(indent + "const int64_t " + selectName + " = " + selectPath + ";");
        selectParameter = selectName + ", ";
    }

    lines.push_back(indent + "if (0 > indent)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "ret = " + variable + ".visualize("
                        + selectParameter + "destBuf, indent, separator);");
    }
    lines.push_back(indent + "} else");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "ret = " + variable + ".visualize("
                        + selectParameter + "destBuf, indent + 1, separator);");
    }
    lines.push_back(indent + "}");
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::packCallableEntry(unsigned indentLevel, const Parameter& parameter)
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
                        ".pack(" + selectName + ", destBuf, cutVer);");
    } else
    {
        lines.push_back(indent + "ret = " + variable + ".pack(destBuf, cutVer);");
    }
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::packStructEntry(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
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

void
TTransCPP::constructStructEntry(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
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
TTransCPP::visualizeEntry(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
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

void
TTransCPP::unpackCallableEntry(unsigned indentLevel, const Parameter& parameter)
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
                        ".unpack(" + selectName + ", srcBuf, cutVer);");
    } else
    {
        lines.push_back(indent + "ret = " + variable + ".unpack(srcBuf, cutVer);");
    }
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::unpackStructEntry(unsigned indentLevel, std::list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to unpackStructEntry is null");
    }

    //    TDEBUG_PRINT_STR("unpackStructEntry......");
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

void
TTransCPP::unpackBaseTypeEntry(unsigned indentLevel, const Parameter& parameter)
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

    if (1 != pstEntry->iCount && 1 == pstEntry->iHUnitSize)
    {
        assert(0 < parameter.byteArrLen.length());
        lines.push_back(indent + "ret = srcBuf.readBytes(&" + variable + ", " + parameter.byteArrLen + ");");
    } else
    {
        lines.push_back(indent + "ret = srcBuf." + baseType.getDecFunc(pstEntry->idxType) + "(" + variable + ");");
    }
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::packBaseTypeEntry(unsigned indentLevel, const Parameter& parameter)
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
        if (1 != pstEntry->iCount && 1 == pstEntry->iHUnitSize)
        {
            assert(0 < parameter.byteArrLen.length());
            lines.push_back(indent + "ret = destBuf.writeBytes(&" + variable + ", " + parameter.byteArrLen + ");");
        } else
        {
            lines.push_back(indent + "ret = destBuf." + baseType.getEncFunc(pstEntry->idxType) + "(" + variable + ");");
        }
        lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");
    } else
    {
        lines.push_back(indent + "/* no need to pack pointer */");
    }
}

void
TTransCPP::constructCommLeafEntry(unsigned indentLevel, const Parameter& parameter,
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

            if (1 == pstEntry->iHUnitSize)
            {
                lines.push_back(indent + "memset(&" + variable + ", " + defaultValue + ", " + loopCountVar + ");");
            } else
            {
                lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                                + loopVarName + " < " + loopCountVar + "; " + loopVarName + "++)");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);

                    lines.push_back(indent + variable + "[" + loopVarName + "]" + " = " + defaultValue + ";");
                }
                lines.push_back(indent + "}");
            }
        } else
        {
            lines.push_back(indent + "memset(&" + variable + ", 0, " + loopCountVar + ");");
        }
    } else if (0 == pstEntry->iCount)
    {
        if (hasSpecialDefaultValue(pstEntry))
        {
            string defaultValue;
            getEntryDefaultValue(pstEntry, defaultValue);

            lines.push_back(indent + variable + "[0] = " + defaultValue + ";");
        } else
        {
            lines.push_back(indent + variable + "[0] = 0;");
        }
    } else
    {
        if (hasSpecialDefaultValue(pstEntry))
        {
            string defaultValue;
            getEntryDefaultValue(pstEntry, defaultValue);

            lines.push_back(indent + variable + " = " + defaultValue + ";");
        } else
        {
            lines.push_back(indent + variable + " = 0;");
        }
    }
}
void
TTransCPP::visualizeCommLeafEntry(unsigned indentLevel, const Parameter& parameter,
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

        lines.push_back(indent + "ret = TdrBufUtil::printArray(destBuf, indent, separator, \"["
                        + entryName + "]\", " + loopCountVar + ");");
        lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            lines.push_back(indent + "return ret;");
        }
        lines.push_back(indent + "}");

        lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                        + loopVarName + " < " + loopCountVar + "; " + loopVarName + "++)");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "ret = destBuf.textize(\" \"" + baseType.getPrintInfo(pstEntry->idxType)
                            + ", " + variable + "[" + loopVarName + "]);");
            lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
            lines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                lines.push_back(indent + "return ret;");
            }
            lines.push_back(indent + "}");
        }
        lines.push_back(indent + "}");

        lines.push_back(indent + "ret = destBuf.writeCharWithNull(separator);");
    } else
    {
        lines.push_back(indent + "ret = TdrBufUtil::printVariable(destBuf, indent, separator, \"["
                        + entryName + "]\", " + baseType.getPrintInfo(pstEntry->idxType) + ", " + variable + ");");
    }

    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::packStringEntry(unsigned indentLevel, const Parameter& parameter)
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
        lines.push_back(indent + "/* gurantee string or wstring terminated with null character */");
        if (TDR_TYPE_STRING == pstEntry->iType)
        {
            lines.push_back(indent + "*(char*)&" + variable + "[sizeof(" + variable + ")-1] = \'\\0\';");
        } else
        {
            lines.push_back(indent + "*(tdr_wchar_t*)&" + variable + "[sizeof(" + variable + ")/"
                            + "sizeof(" + variable + "[0])" + "-1] = 0;");
        }
        lines.push_back("");
    }

    if (TDR_TYPE_STRING == pstEntry->iType)
    {
        lines.push_back(indent + "size_t " + strLenName + " = strlen("  + variable + ") + 1;");
    } else
    {
        lines.push_back(indent + "size_t " + strLenName + " = 2 * (TdrTypeUtil::wstrlen("  + variable + ") + 1);");
    }

    lines.push_back("");
    lines.push_back(indent + "/* pack */");
    lines.push_back(indent + "ret = destBuf.writeBytes(" + variable + ", " + strLenName + ");");
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::unpackStringEntry(unsigned indentLevel, const Parameter& parameter)
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

    if (0 < pstEntry->iCustomHUnitSize)
    {
        lines.push_back(indent + "/* check whether sizeinfo is valid */");
        lines.push_back(indent + "if (" + strSizeName + " > sizeof(" + variable +"))");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            lines.push_back(indent + "return TdrError::TDR_ERR_STR_LEN_TOO_BIG;");
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
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return TdrError::TDR_ERR_STR_LEN_TOO_SMALL;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");

    lines.push_back(indent + "/* unpack */");
    lines.push_back(indent + "ret = srcBuf.readBytes(" + variable + ", " + strSizeName + ");");
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");
    if (0 < pstEntry->iCustomHUnitSize)
    {
        lines.push_back(indent + "/* check whether string-content is consistent with sizeinfo */");
        if (TDR_TYPE_STRING == pstEntry->iType)
        {
            lines.push_back(indent + "if (\'\\0\' != " + variable + "[" + strSizeName + " - 1])");
            lines.push_back(indent + "{");
        } else
        {
            lines.push_back(indent + "if (0 != " + variable + "[" + strSizeName + "/2 - 1])");
            lines.push_back(indent + "{");
        }
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            lines.push_back(indent + "return TdrError::TDR_ERR_STR_LEN_CONFLICT;");
        }
        lines.push_back(indent + "}");
    }

    const string& strLenName = string("realSize4") + getEntryName(pstEntry);
    if (TDR_TYPE_STRING == pstEntry->iType)
    {
        lines.push_back(indent + "size_t " + strLenName + " = strlen("  + variable + ") + 1;");
    } else
    {
        lines.push_back(indent + "size_t " + strLenName + " = 2 * (TdrTypeUtil::wstrlen("  + variable + ") + 1);");
    }
    lines.push_back(indent + "if (" + strSizeName + " != " + strLenName +")");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return TdrError::TDR_ERR_STR_LEN_CONFLICT;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::processEntry4Construct(unsigned indentLevel, const Parameter& a_parameter)
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
        LPTDRMETAENTRY pstReferEntry = tdr_get_entry_refer(pstEntry);
        loopVarType = baseType.getType(pstReferEntry->idxType);
        if ("char" == loopVarType)
        {
            /* compiler will give a warning to use char as array-index */
            loopVarType = "int";
        }
    } else
    {
        loopVarType = "unsigned";
    }

    string loopCountVar;
    string loopVarName;
    if (1 < pstEntry->iCount)
    {
        loopCountVar = getEntryCount(pstEntry);
        loopVarName = getEntryName(pstEntry) + "_i";
    }

    if (TDR_TYPE_UNION == pstEntry->iType
        || TDR_TYPE_STRUCT == pstEntry->iType)
    {
        unsigned furtherIndent = 0;

        if (1 < pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
        } else if (0 == pstEntry->iCount)
        {
            loopVarName = "0";
        }

        constructNonLeafEntry(indentLevel + furtherIndent, a_parameter, loopVarName);

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
        constructCommLeafEntry(indentLevel, a_parameter, loopVarType, loopCountVar);
    } else if (TDR_TYPE_STRING == pstEntry->iType)
    {
        unsigned furtherIndent = 0;

        if (1 < pstEntry->iCount)
        {
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent++;
        } else if (0 == pstEntry->iCount)
        {
            loopVarName = "0";
        }
        constructStringEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
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
            constructBinarySpecilEntry(indentLevel, a_parameter, "0");
            if (1 < pstEntry->iCount)
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
}

void
TTransCPP::processEntry4Default(unsigned indentLevel, const Parameter& a_parameter)
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
        if ("char" == loopVarType)
        {
            /* compiler will give a warning to use char as array-index */
            loopVarType = "int";
        }

        if (pstEntry->iCount > 0)
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
                    lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                    lines.push_back(indent + "return TdrError::TDR_ERR_MINUS_REFER_VALUE;");
                }
                lines.push_back(indent + "}");
            }
            if ((unsigned)pstEntry->iCount < getReferTypeTopValue(pstReferEntry))
            {
                lines.push_back(indent + "if (" + getEntryCount(pstEntry) + " < " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                    lines.push_back(indent + "return TdrError::TDR_ERR_REFER_SURPASS_COUNT;");
                }
                lines.push_back(indent + "}");
            }
        }
    } else
    {
        loopVarType = "unsigned";
    }

    string loopCountVar;
    string loopVarName;
    if (1 != pstEntry->iCount)
    {
        loopCountVar = getEntryCount(pstEntry);
        loopVarName = getEntryName(pstEntry) + "_i";
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            loopCountVar = referPath;
        }
    }

    unsigned furtherIndent = 0;

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
        constructStringEntry(indentLevel + furtherIndent, a_parameter, loopVarName);
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
TTransCPP::processEntry4Visualize(unsigned indentLevel, const Parameter& a_parameter)
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
        if ("char" == loopVarType)
        {
            /* compiler will give a warning to use char as array-index */
            loopVarType = "int";
        }

        if (pstEntry->iCount > 0)
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
                    lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                    lines.push_back(indent + "return TdrError::TDR_ERR_MINUS_REFER_VALUE;");
                }
                lines.push_back(indent + "}");
            }
            if ((unsigned)pstEntry->iCount < getReferTypeTopValue(pstReferEntry))
            {
                lines.push_back(indent + "if (" + getEntryCount(pstEntry) + " < " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                    lines.push_back(indent + "return TdrError::TDR_ERR_REFER_SURPASS_COUNT;");
                }
                lines.push_back(indent + "}");
            }
        }
    } else
    {
        loopVarType = "unsigned";
    }

    unsigned furtherIndent = 0;

    string loopCountVar;
    string loopVarName;
    if (1 != pstEntry->iCount)
    {
        loopCountVar = getEntryCount(pstEntry);
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

void
TTransCPP::processEntry4Code(unsigned indentLevel, const Parameter& a_parameter)
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
    unsigned furtherIndent = 0;
    string arrIndex;

    const string& entryName = getEntryName(pstEntry);

    string referPath;
    string loopVarType;
    if (pstEntry->stRefer.iUnitSize > 0)
    {
        offset2path(pstMeta, pstEntry->stRefer.iHOff, referPath);
        LPTDRMETAENTRY pstReferEntry = tdr_get_entry_refer(pstEntry);
        loopVarType = baseType.getType(pstReferEntry->idxType);
        if ("char" == loopVarType)
        {
            /* compiler will give a warning to use char as array-index */
            loopVarType = "int";
        }
        if (pstEntry->iCount > 0)
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
                    lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                    lines.push_back(indent + "return TdrError::TDR_ERR_MINUS_REFER_VALUE;");
                }
                lines.push_back(indent + "}");
            }
            if ((unsigned)pstEntry->iCount < getReferTypeTopValue(pstReferEntry))
            {
                lines.push_back(indent + "if (" + getEntryCount(pstEntry) + " < " + referPath + ")");
                lines.push_back(indent + "{");
                {
                    const string& indent = getIndentStr(indentLevel + 1);
                    lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                    lines.push_back(indent + "return TdrError::TDR_ERR_REFER_SURPASS_COUNT;");
                }
                lines.push_back(indent + "}");
            }
        }
    } else
    {
        loopVarType = "unsigned";
    }

    if (pstEntry->iType == TDR_TYPE_UNION)
    {
        const string selectName = string("selector4") + entryName;
        string selectPath;
        offset2path(pstMeta, pstEntry->stSelector.iHOff, selectPath);
        lines.push_back(indent + "const int64_t " + selectName + " = " + selectPath + ";");
    }

    string loopCountVar;
    if (pstEntry->iCount != 1)
    {
        loopCountVar = getEntryCount(pstEntry);
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            loopCountVar = referPath;
        }

        if (1 != pstEntry->iHUnitSize || TDR_TYPE_STRING == pstEntry->iType)
        {
            const string& loopVarName = getEntryName(pstEntry) + "_i";
            lines.push_back(indent + "for (" + loopVarType + " " + loopVarName + " = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++)");
            lines.push_back(indent + "{");
            furtherIndent = 1;
            arrIndex = string("[") + loopVarName + "]";
        }
    } else
    {
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            lines.push_back(indent + "if (1 == " + referPath + ")");
            lines.push_back(indent + "{");
            furtherIndent = 1;
        }
    }

    Parameter parameter(lines, pstMeta, pstEntry, variable + arrIndex, operation);

    /* added to enhance performance to pack/unpack bit8-array */
    if (loopCountVar.length() > 0)
    {
        parameter.byteArrLen = loopCountVar;
    }

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
TTransCPP::directWrite (unsigned indentLevel, std::list<string>& lines, int switchValue,
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
            packString = "ret = " + bufName + ".writeUInt8((uint8_t)(" + value + "), " + pos + ");";
            break;
        case 2:
            packString = "ret = " + bufName + ".writeUInt16((uint16_t)(" + value + "), " + pos + ");";
            break;
        case 4:
            packString = "ret = " + bufName + ".writeUInt32((uint32_t)(" + value + "), " + pos + ");";
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + packString);
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::directRead(unsigned indentLevel, std::list<string>& lines, int switchValue,
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
            tempType = "uint8_t";
            break;
        case 2:
            readFunc = readShortFunc;
            tempType = "uint16_t";
            break;
        case 4:
            readFunc = readIntFunc;
            tempType = "uint32_t";
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + tempType + " " + leftValue + ";");
    lines.push_back(indent + "ret = " + bufName + "." + readFunc + "(" + leftValue + ", " + pos + ");");
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::serialRead(unsigned indentLevel, std::list<string>& lines, int switchValue,
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
            tempType = "uint8_t";
            break;
        case 2:
            readFunc = readShortFunc;
            tempType = "uint16_t";
            break;
        case 4:
            readFunc = readIntFunc;
            tempType = "uint32_t";
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + tempType + " " + leftValue + ";");
    lines.push_back(indent + "ret = " + bufName + "." + readFunc + "(" + leftValue + ");");
    lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransCPP::processEntryPack(unsigned indentLevel, const Parameter& parameter)
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
            lines.push_back(indent + "const size_t " + entrySizePosVarName + " = "
                            + "savedBeginPos + " + int2str((int)pstEntry->stSizeInfo.iNOff) + ";");
        } else
        {
            lines.push_back(indent + "const size_t " + entrySizePosVarName + " = destBuf.getUsedSize();");
            lines.push_back("");
            lines.push_back(indent + "/* reserve space for sizeinfo */");
            switch (pstEntry->stSizeInfo.iUnitSize)
            {
                case 1:
                    lines.push_back(indent + "ret = destBuf.reserve(sizeof(uint8_t));");
                    break;
                case 2:
                    lines.push_back(indent + "ret = destBuf.reserve(sizeof(uint16_t));");
                    break;
                case 4:
                    lines.push_back(indent + "ret = destBuf.reserve(sizeof(uint32_t));");
                    break;
                default:
                    throw string("invalid sizeinfo iUnitSize, this is a tdr-lib error");
                    break;
            }

            lines.push_back(indent + "if (TdrError::TDR_NO_ERROR != ret)");
            lines.push_back(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 1);
                lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
                lines.push_back(indent + "return ret;");
            }
            lines.push_back(indent + "}");
            lines.push_back("");
        }
        lines.push_back(indent + "/* record real-data's begin postion in buf */");
        lines.push_back(indent + "const size_t " + entryBeginBufPosVarName + " = destBuf.getUsedSize();");
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
        lines.push_back(indent + "const size_t " + sizeInfoVar + " = "
                        + "destBuf.getUsedSize() - " + entryBeginBufPosVarName + ";");
        directWrite(indentLevel, lines, pstEntry->stSizeInfo.iUnitSize,
                    "destBuf", entrySizePosVarName, sizeInfoVar,
                    "invalid sizeinfo iUnitSize, this is a tdr-lib error");
    }
}

void
TTransCPP::processEntryUnpack(unsigned indentLevel, const Parameter& parameter)
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
        lines.push_back(indent + "if (" + entrySizeinfoVar + " > srcBuf.getLeftSize())");
        lines.push_back(indent + "{");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "TSF4G_TDR_DEBUG_TRACE();");
            lines.push_back(indent + "return TdrError::TDR_ERR_SHORT_BUF_FOR_READ;");
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
TTransCPP::offset2path(LPTDRMETA pstMeta, ptrdiff_t offset, string& path)
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

    path = string("this->") + path;
}

void
TTransCPP::offset2version(LPTDRMETA pstMeta, ptrdiff_t offset, string& version)
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
TTransCPP::getEntryType(LPTDRMETAENTRY pstEntry, string& entryType)
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
TTransCPP::loadCodeRes(unsigned indentLevel, std::list<string>& lines,
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

string TTransCPP::getEntryNameWithThis(LPTDRMETAENTRY pstEntry)
{
    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryNameWithThis is null");
    }

    return string("this->") + getEntryName(pstEntry);
}

void
TTransCPP::getEntryDefaultValue(LPTDRMETAENTRY pstEntry, string& entryDefaultValue)
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

string
TTransCPP::getEntryName(LPTDRMETAENTRY pstEntry)
{
    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryName is null");
    }

    string cumtomPrefix;
    if (TDR_HPPRULE_ADD_CUSTOM_PREFIX & this->pstOption->stRule.iRule)
    {
        cumtomPrefix = this->pstOption->stRule.szCustomNamePrefix;
    }

    if (!(TDR_HPPRULE_NO_TYPE_PREFIX & this->pstOption->stRule.iRule))
    {
        if (TDR_ENTRY_IS_REFER_TYPE(pstEntry) || TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
        {
            cumtomPrefix = "p" + cumtomPrefix;
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

        cumtomPrefix += pszPrefix;
    }

    string entryName = tdr_get_entry_name(pstEntry);

    if (!cumtomPrefix.empty())
    {
        return cumtomPrefix + (char)toupper(entryName[0]) + entryName.substr(1);
    } else if (!(TDR_HPPRULE_NO_LOWERCASE_PREFIX & this->pstOption->stRule.iRule))
    {
        return (char)tolower(entryName[0]) + entryName.substr(1);
    } else
    {
        return entryName;
    }
}

unsigned
TTransCPP::getReferTypeTopValue(LPTDRMETAENTRY pstEntry)
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

string
TTransCPP::getUpperString(const string& source)
{
    string ret = source;

    for (size_t i = 0; i < ret.length(); i++)
    {
        if (islower(ret[i]))
        {
            ret[i] = toupper(ret[i]);
        }
    }

    return ret;
}

void
TTransCPP::outputTypedef(list<string>& lines, const string& indent,  LPTDRMETA pstMeta)
{
    if (!(TDR_HPPRULE_NO_TYPE_DECLARE & this->pstOption->stRule.iRule))
    {
        lines.push_back(indent + "typedef       " + tdr_get_meta_name(pstMeta)
                        + "          tag" + tdr_get_meta_name(pstMeta) + ";");

        string upMetaName = getUpperString(string(tdr_get_meta_name(pstMeta)));
        lines.push_back(indent + "typedef       " + tdr_get_meta_name(pstMeta)
                        + "             " + upMetaName + ";");
        lines.push_back(indent + "typedef       " + tdr_get_meta_name(pstMeta)
                        + "*          LP" + upMetaName + ";");
        lines.push_back(indent + "typedef const " + tdr_get_meta_name(pstMeta)
                        + "*         LCP" + upMetaName + ";");
    }
}

string
TTransCPP::getEntryVersion(LPTDRMETAENTRY pstEntry, LPTDRMETA pstMeta)
{
    if (pstMeta->iBaseVersion == pstEntry->iVersion)
    {
        return string(tdr_get_meta_name(pstMeta)) + "::BASEVERSION";
    } else
    {
        return string(tdr_get_meta_name(pstMeta)) + "::VERSION_" + getEntryName(pstEntry);
    }
}

string
TTransCPP::getEntryCount(LPTDRMETAENTRY pstEntry)
{
    if (0 == pstEntry->iCount)
    {
        return int2str(0);
    } else if (1 == pstEntry->iCount)
    {
        return int2str(1);
    } else if( TDR_INVALID_INDEX != pstEntry->idxCount)
    {
        TDRPTR ptrMacro = (TDR_GET_MACRO_TABLE(this->pstMetaLib))[pstEntry->idxCount].ptrMacro;
        return string(TDR_GET_STRING_BY_PTR(this->pstMetaLib, ptrMacro));
    } else
    {
        return int2str(pstEntry->iCount);
    }
}

void
TTransCPP::flagMeta4Default()
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
TTransCPP::needSetDefaultValue(LPTDRMETA pstMeta)
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
TTransCPP::needSetDefaultValue(LPTDRMETAENTRY pstEntry)
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
TTransCPP::hasSpecialDefaultValue(LPTDRMETAENTRY pstEntry)
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
            fprintf(stderr, "warning: for C++, defaultvalue for 'wstring' is not supported yet.\n");
            break;
    }

    return false;
}
