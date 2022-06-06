#include <ctime>
#include <string>
#include <cassert>
#include <iostream>
#include "tdr/tdr.h"
#include "tdr_as3.h"
#include "../tdr_tools.h"
#include "tdr_debug.h"
#include "../../lib_src/tdr/tdr_md5.h"
#include "../../lib_src/tdr/tdr_metalib_kernel_i.h"
#include "../../lib_src/tdr/tdr_metalib_manage_i.h"

#define TDR_MD5_DIGEST_LENGTH 16

using std::cout;
using std::cerr;
using std::endl;
using std::string;

string TTransAS3::ttranslatorVersion = "1.0";

void
TTransAS3::checkMetaLib()
{
    bool checkFailed = false;
    string xmlTag;

    for (int i = 0; i < pstMetaLib->iCurMetaNum; i++)
    {
        LPTDRMETA pstMeta = TDR_IDX_TO_META(pstMetaLib, i);

        for (int j = 0; j < pstMeta->iEntriesNum; j++)
        {
            LPTDRMETAENTRY pstEntry = tdr_get_entry_by_index(pstMeta, j);

            if (TDR_TYPE_COMPOSITE < pstEntry->iType)
            {
                string unsupported;

                if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
                {
                    unsupported = "reference";

                    checkFailed = true;
                } else if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
                {
                    unsupported = "pointer";

                    checkFailed = true;
                } else if (TDR_TYPE_DATETIME == pstEntry->iType)
                {
                    unsupported = "datetime";

                    checkFailed = true;
                } else if (TDR_TYPE_WSTRING == pstEntry->iType)
                {
                    unsupported = "wstring";

                    checkFailed = true;
                } else if (TDR_TYPE_WCHAR == pstEntry->iType)
                {
                    unsupported = "wchar";

                    checkFailed = true;
                } else
                {
                    continue;
                }

                cerr << "error: for AS3, " << unsupported << " type is NOT support yet, ";
                cerr << "invalid entry:" << endl;
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
    }

    if (checkFailed)
    {
        throw string("failed to check metalib");
    }
}

void
TTransAS3::translate(LPTDRMETALIB pstMetaLib)
{
    if (!pstMetaLib)
    {
        throw string("pstMetaLib to translate is null");
    }

    TDEBUG_PRINTLN(TTransAS3::ttranslatorVersion);
    this->pstMetaLib = pstMetaLib;
    this->packageName = tdr_get_metalib_name(pstMetaLib);

    checkMetaLib();

    makeErrorMap();
    makeTdrUtil();
    makeInt64();
    makeUint64();

    translateMacros();

    for (int i = 0; i < pstMetaLib->iCurMetaNum; i++)
    {
        LPTDRMETA pstMeta = TDR_IDX_TO_META(pstMetaLib, i);
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
TTransAS3::translateMacros()
{

    list<string> lines;
    lines.push_back("package " + this->packageName + " {");
    {
        const unsigned int indentLevel = 1;
        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "public class Macros {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            LPTDRMACRO pstMacro = TDR_GET_MACRO_TABLE(pstMetaLib);
            for (int i = 0; i< pstMetaLib->iCurMacroNum; i++)
            {
                lines.push_back(indent + "public static const " +
                                TDR_GET_STRING_BY_PTR(pstMetaLib, pstMacro[i].ptrMacro) +
                                " : int = " + int2str(pstMacro[i].iValue) + ";");
            }
        }
        lines.push_back(indent + "}");
    }
    lines.push_back("}");
    this->output(lines, string("Macros.as"));
}

void
TTransAS3::translateStruct(LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to translateStruct is null");
    }

    list<string> lines;
    lines.push_back("package " + this->packageName + " {");
    {
        const unsigned int indentLevel = 1;
        const string& indent = getIndentStr(indentLevel);
        lines.push_back(indent + "import flash.utils.ByteArray;");
        lines.push_back("");
        lines.push_back(indent + "public class " + tdr_get_meta_name(pstMeta) + " {");
        {
            const unsigned int indentLevel = 2;
            declaration(indentLevel, lines, pstMeta);

            lines.push_back("");
            structConstruct(indentLevel, lines, pstMeta);

            lines.push_back("");
            structEncode(indentLevel, lines, pstMeta);

            lines.push_back("");
            structDecode(indentLevel, lines, pstMeta);

            lines.push_back("");
            version(indentLevel, lines, pstMeta);
        }
        lines.push_back(indent + "}");
    }
    lines.push_back("}");
    this->output(lines, string(tdr_get_meta_name(pstMeta)) + ".as");
}

void
TTransAS3::translateUnion(LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to translateUnion is null");
    }

    list<string> lines;
    lines.push_back("package " + this->packageName + " {");
    {
        // 1rt indent-level
        const string& indent = getIndentStr(1);
        lines.push_back(indent + "import flash.utils.ByteArray;");
        lines.push_back("");
        lines.push_back(indent + "public class " + tdr_get_meta_name(pstMeta) + " {");
        {
            // 2nd indent-level
            const unsigned int indentLevel = 2;
            declaration(indentLevel, lines, pstMeta);

            lines.push_back("");
            unionConstruct(indentLevel, lines, pstMeta);

            lines.push_back("");
            unionEncode(indentLevel, lines, pstMeta);

            lines.push_back("");
            unionDecode(indentLevel, lines, pstMeta);

            lines.push_back("");
            version(indentLevel, lines, pstMeta);
        }
        lines.push_back(indent + "}");
    }
    lines.push_back("}");
    this->output(lines, string(tdr_get_meta_name(pstMeta)) + ".as");
}

void
TTransAS3::output(list<string>& lines, const string& fileName)
{
    if (fileName.empty())
    {
        throw string("fileName to output is empty");
    }

    lines.push_front("");

    // tdr version
    string tdrVersion;
    tdr_get_version(tdrVersion);
    lines.push_front(getCommentStr(0) + "tdr version: " + tdrVersion);

    // create time
    time_t curTime;
    time(&curTime);
    string strTime = ctime(&curTime);
    strTime = strTime.substr(0, strTime.length()-1);
    lines.push_front(getCommentStr(0) + "creation time: " + strTime);

    // metalib md5sum
    unsigned char szMetalibHash[TDR_MD5_DIGEST_LENGTH];
    char szHash[TDR_MD5_DIGEST_LENGTH * 2 + 1] = {0};
    tdr_md5hash_buffer(szMetalibHash, (const unsigned char *)pstMetaLib,
                       (unsigned int)pstMetaLib->iSize);
    tdr_md5hash2str(szMetalibHash, &szHash[0], sizeof(szHash));
    lines.push_front(getCommentStr(0) + "metalib md5sum: " + string(szHash));

    // metalib version
    lines.push_front(getCommentStr(0) + "metalib version: "
                     + int2str(tdr_get_metalib_version(pstMetaLib)));

    lines.push_front("");
    lines.push_front(getCommentStr(0) + "No manual modification is permitted.");
    lines.push_front(getCommentStr(0) + "This source file is generated by tdr.");

    TTransBase::output(lines, fileName);
}

void
TTransAS3::declaration(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to declaration is null");
    }

    const string& indent = getIndentStr(indentLevel);
    // data-members
    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryType;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        pstEntry = pstMeta->stEntries + i;
        entryName = TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrName);
        entryName = tdr_get_entry_name(pstEntry);
        TDEBUG_PRINTLN(entryName);
        getEntryType(pstEntry, entryType);
        TDEBUG_PRINTLN(entryType);
        string entryComment;
        if (baseType.has(pstEntry->iType))
        {
            entryComment = " // " + baseType.getCType(pstEntry->iType);
        }
        if (pstEntry->iCount != 1)
        {
            lines.push_back(indent + "public var " + tdr_get_entry_name(pstEntry)
                            + " : Vector.<" + entryType + ">;" + entryComment);
        } else
        {
            lines.push_back(indent + "public var " + entryName
                            + " : " + entryType + ";" + entryComment);
        }
    }
}

void
TTransAS3::unionConstruct(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to unionConstruct is null");
    }

    const string& indent = getIndentStr(indentLevel);
    // data-members
    LPTDRMETAENTRY pstEntry;
    string entryName;
    lines.push_back(indent + "public function "
                    + TDR_GET_STRING_BY_PTR(pstMetaLib, pstMeta->ptrName) + "() {");
    {
        // 3rd indent-level
        const string& indent = getIndentStr(3);
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            pstEntry = pstMeta->stEntries + i;
            entryName = TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrName);
            if (pstEntry->iCount != 1 || !baseType.has(pstEntry->iType))
            {
                lines.push_back(indent + entryName + " = null;");
            }
        }
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::structConstruct(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structConstruct is null");
    }

    const string& indent = getIndentStr(indentLevel);
    // data-members
    LPTDRMETAENTRY pstEntry;
    string entryName;
    string entryType;
    lines.push_back(indent + "public function "
                    + TDR_GET_STRING_BY_PTR(pstMetaLib, pstMeta->ptrName) + "() {");
    {
        // 3rd indent-level
        const string& indent = getIndentStr(indentLevel + 1);
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            pstEntry = pstMeta->stEntries + i;
            entryName = TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrName);
            getEntryType(pstEntry, entryType);
            if (pstEntry->iCount == 0)
            {
                lines.push_back(indent + entryName + " = new Vector.<" + entryType + ">();");
            } else if (pstEntry->iCount > 1)
            {
                lines.push_back(indent + entryName + " = new Vector.<" + entryType + ">();"
                                " //max-count: " + int2str(pstEntry->iCount));
                /* qqpet require this feature
                lines.push_back(indent + entryName + " = new Vector.<" + entryType + ">("
                                + int2str(pstEntry->iCount) + ");");
                string entryLoopVarName = entryName + "_i";
                if (!baseType.has(pstEntry->iType))
                {
                    lines.push_back(indent + "for (var " + entryLoopVarName + " : uint = 0; "
                                    + entryLoopVarName + " < " + int2str(pstEntry->iCount) + "; "
                                    + entryLoopVarName + "++) {");
                    {
                        const string& indent = getIndentStr(indentLevel + 2);
                        lines.push_back(indent + entryName + "[" + entryLoopVarName + "]"
                                        + " = new " + entryType + "();");
                    }
                    lines.push_back(indent + "}");
                }
                */
            } else if (!baseType.has(pstEntry->iType))
            {
                lines.push_back(indent + entryName + " = new " + entryType + "();");
            }
        }
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::structEncode(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structEncode is null");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "public function encode(buf:ByteArray, cutVersion:uint = 0):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        adjustCutVersion(indentLevel + 1, lines);

        // save space for sizeinfo
        if (pstMeta->stSizeType.iUnitSize > 0)
        {
            lines.push_back("");
            prepare4sizeinfo(indentLevel + 1, lines, pstMeta);
        }

        // records buf.position
        lines.push_back("");
        lines.push_back(indent + "const savedBufBeginPos : uint = buf.position;");

        lines.push_back("");
        lines.push_back(indent + "var ret : int = ErrorMap.NO_ERROR;");

        lines.push_back("");
        encodeEntry(indentLevel + 1, lines, pstMeta);

        // lines.push_back("");
        encodeVersionIndicator(indentLevel + 1, lines, pstMeta);

        lines.push_back("");
        encodeSizeinfo(indentLevel + 1, lines, pstMeta);

        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::structDecode(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to structDecode is null");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "public function decode(buf:ByteArray, cutVersion:uint = 0):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);

        adjustCutVersion(indentLevel + 1, lines);

        lines.push_back("");
        decodeSizeinfo(indentLevel + 1, lines, pstMeta);

        // records buf.position
        lines.push_back("");
        lines.push_back(indent + "const savedBufBeginPos : uint = buf.position;");

        lines.push_back("");
        decodeVersionIndicator(indentLevel + 1, lines, pstMeta);

        lines.push_back("");
        lines.push_back(indent + "var ret : int = ErrorMap.NO_ERROR;");

        lines.push_back("");
        decodeEntry(indentLevel + 1, lines, pstMeta);

        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::version(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to version is null");
    }

    const string& indent = getIndentStr(indentLevel);

    // pstMeta version info
    lines.push_back(indent + "static public const BASEVERSION : uint = "
                    + int2str(pstMeta->iBaseVersion) + ";");
    lines.push_back(indent + "static public const CURRVERSION : uint = "
                    + int2str(pstMeta->iCurVersion) + ";");

    // entry version info
    LPTDRMETAENTRY pstEntry;
    string entryName;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        pstEntry = pstMeta->stEntries + i;
        entryName = TDR_GET_STRING_BY_PTR(pstMetaLib, pstEntry->ptrName);
        lines.push_back(indent + "static public const VERSION_" + entryName + " : uint = "
                        + int2str(pstEntry->iVersion) + ";");
    }
}

void
TTransAS3::unionEncode(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to unionEncode is null");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "public function encode(select:int, "
                    "buf:ByteArray, cutVersion:uint = 0):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        adjustCutVersion(indentLevel + 1, lines);
        lines.push_back("");
        lines.push_back(indent + "var ret : int = ErrorMap.NO_ERROR;");

        LPTDRMETAENTRY pstEntry;
        LPTDRMETAENTRY pstDefaultEntry = NULL;
        string entryName;
        string entryType;
        string elseString;
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            pstEntry = pstMeta->stEntries + i;

            if (TDR_ENTRY_DO_HAVE_ID(pstEntry))
            {
                lines.push_back(indent + elseString + "if (select == " + int2str(pstEntry->iID) + ") {");
            } else if (TDR_ENTRY_DO_HAVE_MAXMIN_ID(pstEntry))
            {
                lines.push_back(indent + elseString + "if (" + int2str(pstEntry->iMinId) + " <= select"
                                + " && select <= " + int2str(pstEntry->iMaxId) + "){");
            } else
            {
                pstDefaultEntry = pstEntry;
                continue;
            }

            if (elseString.empty())
            {
                elseString = "else ";
            }

            entryName = string(tdr_get_entry_name(pstEntry));
            Parameter parameter(lines, pstMeta, pstEntry, entryName, Parameter::TYPE_ENCODE);
            processEntry(indentLevel + 2, parameter);
            lines.push_back(indent + "}");
        }

        if (pstDefaultEntry != NULL)
        {
            int furtherIndent = 0;
            if (1 < pstMeta->iEntriesNum)
            {
                furtherIndent++;
                lines.push_back(indent + "else {");
            }
            entryName = string(tdr_get_entry_name(pstDefaultEntry));
            Parameter parameter(lines, pstMeta, pstDefaultEntry, entryName, Parameter::TYPE_ENCODE);
            processEntry(indentLevel + 1 + furtherIndent, parameter);
            if (1 < pstMeta->iEntriesNum)
            {
                lines.push_back(indent + "}");
            }
        }

        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::unionDecode(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to unionDecode is null");
    }

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "public function decode(select:int,"
                    "buf:ByteArray, cutVersion:uint = 0):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        adjustCutVersion(indentLevel + 1, lines);
        lines.push_back("");

        lines.push_back(indent + "var ret : int = ErrorMap.NO_ERROR;");
        LPTDRMETAENTRY pstEntry;
        LPTDRMETAENTRY pstDefaultEntry = NULL;
        string entryType;
        string entryName;
        string elseString;
        for (int i = 0; i < pstMeta->iEntriesNum; i++)
        {
            pstEntry = pstMeta->stEntries + i;

            if (TDR_ENTRY_DO_HAVE_ID(pstEntry))
            {
                lines.push_back(indent + elseString + "if (select == " + int2str(pstEntry->iID) + ") {");
            } else if (TDR_ENTRY_DO_HAVE_MAXMIN_ID(pstEntry))
            {
                lines.push_back(indent + elseString + "if (" + int2str(pstEntry->iMinId) + " <= select"
                                + " && select <= " + int2str(pstEntry->iMaxId) + "){");
            } else
            {
                pstDefaultEntry = pstEntry;
                continue;
            }

            {
                entryName = string(tdr_get_entry_name(pstEntry));
                Parameter parameter(lines, pstMeta, pstEntry, entryName, Parameter::TYPE_DECODE);
                processEntry(indentLevel + 2, parameter);
            }
            lines.push_back(indent + "}");

            if (elseString.empty())
            {
                elseString = "else ";
            }
        }

        if (pstDefaultEntry != NULL)
        {
            int furtherIndent = 0;
            if (1 < pstMeta->iEntriesNum)
            {
                furtherIndent++;
                lines.push_back(indent + "else {");
            }
            entryName = string(tdr_get_entry_name(pstDefaultEntry));
            Parameter parameter(lines, pstMeta, pstDefaultEntry, entryName, Parameter::TYPE_DECODE);
            processEntry(indentLevel + 1 + furtherIndent, parameter);
            if (1 < pstMeta->iEntriesNum)
            {
                lines.push_back(indent + "}");
            }
        }

        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}"); // end of decode
}

void
TTransAS3::adjustCutVersion(unsigned int indentLevel, list<string>& lines)
{
    const string& indent = getIndentStr(indentLevel);
    lines.push_back(getCommentStr(indentLevel) + "adjust cutversion");
    lines.push_back(indent + "if (cutVersion == 0 || cutVersion > CURRVERSION) {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "cutVersion = CURRVERSION;");
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::encodeVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to encodeVersionIndicator is null");
    }

    const string& indent = getIndentStr(indentLevel);

    lines.push_back(getCommentStr(indentLevel) + "set versionindicator");
    if (pstMeta->stVersionIndicator.iUnitSize > 0)
    {
        lines.push_back(indent + "if (buf.position < savedBufBeginPos + "
                        + int2str(pstMeta->stVersionIndicator.iNOff + pstMeta->stVersionIndicator.iUnitSize)
                        + ") {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_NO_BUF_FOR_INDICATOR;");
        }
        lines.push_back(indent + "}");

        lines.push_back(indent + "// set versionindicator");
        directWrite(indentLevel, lines, pstMeta->stVersionIndicator.iUnitSize,
                    "buf", "savedBufBeginPos + " + int2str((int)pstMeta->stVersionIndicator.iNOff),
                    "cutVersion", "invalid versionindicator iUnitSize, this is a tdr-lib error");
    } else
    {
        lines.push_back(getCommentStr(indentLevel) + "no versionindicator to set");
    }
}

void
TTransAS3::decodeVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to decodeVersionIndicator is null");
    }

    const string& indent = getIndentStr(indentLevel);

    lines.push_back(getCommentStr(indentLevel) + "get versionindicator");
    if (pstMeta->stVersionIndicator.iUnitSize > 0)
    {
        lines.push_back(indent + "if (buf.bytesAvailable < "
                        + int2str(pstMeta->stVersionIndicator.iNOff + pstMeta->stVersionIndicator.iUnitSize)
                        + ") {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_PARTIAL_MSG;");
        }
        lines.push_back(indent + "}");

        lines.push_back(indent + "// get cutVersion-value");
        directRead(indentLevel, lines, pstMeta->stVersionIndicator.iUnitSize,
                   "cutVersion", "buf", "buf.position + " + int2str((int)pstMeta->stVersionIndicator.iNOff),
                   "invalid versionindicator iUnitSize, this is a tdr-lib error");

        lines.push_back(indent + "if (BASEVERSION > cutVersion || cutVersion > CURRVERSION) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_INVALIED_CUTOFF_VERSION;");
        }
        lines.push_back(indent + "}");
    } else
    {
        lines.push_back(getCommentStr(indentLevel) + "no versionindicator to get");
    }
}

void
TTransAS3::encodeSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to encodeSizeinfo is null");
    }

    const string& indent = getIndentStr(indentLevel);

    lines.push_back(indent + "// set sizeinfo");
    if (pstMeta->stSizeType.iUnitSize > 0)
    {
        TDEBUG_PRINTLN(pstMeta->stSizeType.iUnitSize);
        TDEBUG_PRINTLN(pstMeta->stSizeType.iNOff);

        lines.push_back(indent + "if (buf.position  < (metaSizeinfoPos + "
                        + int2str(pstMeta->stSizeType.iUnitSize)
                        + ")) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_NO_BUF_FOR_SIZEINFO;");
        }
        lines.push_back(indent + "}");

        lines.push_back(indent + "const size : uint = buf.position - savedBufBeginPos;");
        directWrite(indentLevel, lines, pstMeta->stSizeType.iUnitSize,
                    "buf", "metaSizeinfoPos",
                    "size", "invalid sizeinfo iUnitSize, this is a tdr-lib error");
    } else
    {
        lines.push_back(indent + "// no sizeinfo to set");
    }
}

void
TTransAS3::prepare4sizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to prepare4sizeinfo is null");
    }

    const string& indent = getIndentStr(indentLevel);
    TDEBUG_PRINTLN(pstMeta->stSizeType.iUnitSize);
    TDEBUG_PRINTLN(pstMeta->stSizeType.iNOff);

    unsigned int offset = 0;
    string position = "buf.position";
    if (pstMeta->stSizeType.iNOff > 0)
    {
        offset = pstMeta->stSizeType.iNOff;
        position += " + " + int2str((int)offset);
    }
    lines.push_back(indent + "var metaSizeinfoPos : uint = " + position + ";");

    if (pstMeta->stSizeType.idxSizeType != TDR_INVALID_INDEX)
    {
        const string& indent = getIndentStr(indentLevel);
        lines.push_back("");
        lines.push_back(indent + "// reserve space for struct's sizeinfo");
        switch (pstMeta->stSizeType.iUnitSize)
        {
            case 1:
            case 2:
            case 4:
                lines.push_back(indent + "buf.position += "
                                + int2str(pstMeta->stSizeType.iUnitSize) + ";");
                break;
            default:
                throw string("invalid sizeinfo iUnitSize, this is a tdr-lib error");
                break;
        }
    }
}

void
TTransAS3::decodeSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to decodeSizeinfo is null");
    }

    const string& indent = getIndentStr(indentLevel);

    lines.push_back(indent + "// get sizeinfo");
    if (pstMeta->stSizeType.iUnitSize > 0)
    {
        TDEBUG_PRINTLN(pstMeta->stSizeType.iUnitSize);
        TDEBUG_PRINTLN(pstMeta->stSizeType.iNOff);

        unsigned int offset = 0;
        string position = "buf.position";
        if (pstMeta->stSizeType.iNOff >= 0)
        {
            offset = pstMeta->stSizeType.iNOff;
            position = "buf.position + " + int2str((int)offset);
        }

        lines.push_back(indent + "if (buf.bytesAvailable < "
                        + int2str(pstMeta->stSizeType.iUnitSize + offset)
                        + ") {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_PARTIAL_MSG;");
        }
        lines.push_back(indent + "}");

        lines.push_back(indent + "var size : uint;");
        if (pstMeta->stSizeType.idxSizeType != TDR_INVALID_INDEX)
        {
            // sizeinfo refers to a base-type
            serialRead(indentLevel, lines, pstMeta->stSizeType.iUnitSize,
                       "size", "buf", "invalid sizeinfo iUnitSize, this is a tdr-lib error");
        } else
        {
            lines.push_back(indent + "// get meta's sizeinfo-value");
            directRead(indentLevel, lines, pstMeta->stSizeType.iUnitSize,
                       "size", "buf", position, "invalid sizeinfo iUnitSize, this is a tdr-lib error");
        }

        // check available bytes are enough
        lines.push_back("");
        lines.push_back(indent + "// check buf contains enough bytes specified by sizeinfo");
        lines.push_back(indent + "if (buf.bytesAvailable < size) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_PARTIAL_MSG;");
        }
        lines.push_back(indent + "}");
    } else
    {
        lines.push_back(indent + "// no sizeinfo to get");
    }
}

void
TTransAS3::encodeCallableEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to encodeCallableEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to encodeCallableEntry is empty");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);
    if (pstEntry->iCount != 1)
    {
        lines.push_back(indent + "if (" + variable + " == undefined) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_ARRAY_ELEMENT_UNDEFIND;");
        }
        lines.push_back(indent + "}");
    } else
    {
        lines.push_back(indent + "if (" + variable + " == null) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_REFERENCE_IS_NULL;");
        }
        lines.push_back(indent + "}");
    }

    if (pstEntry->iType == TDR_TYPE_LONGLONG)
    {
        lines.push_back(indent + "ret = " + variable + ".encode(buf);");
    } else if (pstEntry->iType == TDR_TYPE_ULONGLONG)
    {
        lines.push_back(indent + "ret = " + variable + ".encode(buf);");
    } else if (pstEntry->iType == TDR_TYPE_UNION)
    {
        const string selectName = string(tdr_get_entry_name(pstEntry)) + "Select";
        lines.push_back(indent + "ret = " + variable +
                        ".encode(" + selectName + ", buf, cutVersion);");
    } else
    {
        lines.push_back(indent + "ret = " + variable + ".encode(buf, cutVersion);");
    }
    lines.push_back(indent + "if (ret != ErrorMap.NO_ERROR) {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::encodeEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to encodeEntry is null");
    }

    //    TDEBUG_PRINT_STR("encodeEntry......");
    //    TDEBUG_PRINT_STRLN(tdr_get_meta_name(pstMeta));
    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "// encode entries");
    // encode entries of current meta
    LPTDRMETAENTRY pstEntry;
    string entryName;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        pstEntry = pstMeta->stEntries + i;
        entryName = tdr_get_entry_name(pstEntry);
        Parameter parameter(lines, pstMeta, pstEntry, entryName, Parameter::TYPE_ENCODE);
        processEntry(indentLevel, parameter);

        lines.push_back("");
    }
}

void
TTransAS3::decodeCallableEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstMeta)
    {
        throw string("'parameter.pstMeta' to decodeCallableEntry is null");
    }
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry'' to decodeCallableEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to decodeCallableEntry is empty");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETA pstMeta = parameter.pstMeta;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);

    if (pstMeta->idxType == TDR_TYPE_UNION || pstEntry->iCount != 1)
    {
        string typeName;
        if (pstEntry->iType == TDR_TYPE_LONGLONG)
        {
            typeName = "Int64";
        } else if (pstEntry->iType == TDR_TYPE_ULONGLONG)
        {
            typeName = "UInt64";
        } else
        {
            typeName = tdr_get_meta_name(TDR_PTR_TO_META(pstMetaLib, pstEntry->ptrMeta));
        }
        lines.push_back(indent + variable + " = new " + typeName + "();");
    } else
    {
        lines.push_back(indent + "if (" + variable + " == null) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_REFERENCE_IS_NULL;");
        }
        lines.push_back(indent + "}");
    }

    if (pstEntry->iType == TDR_TYPE_LONGLONG)
    {
        lines.push_back(indent + "ret = " + variable + ".decode(buf);");
    } else if (pstEntry->iType == TDR_TYPE_ULONGLONG)
    {
        lines.push_back(indent + "ret = " + variable + ".decode(buf);");
    } else if (pstEntry->iType == TDR_TYPE_UNION)
    {
        const string selectName = string(tdr_get_entry_name(pstEntry)) + "Select";
        lines.push_back(indent + "ret = " + variable +
                        ".decode(" + tdr_get_entry_name(pstEntry)+ "Select, buf, cutVersion);");
    } else
    {
        lines.push_back(indent + "ret = " + variable + ".decode(buf, cutVersion);");
    }
    lines.push_back(indent + "if (ret != ErrorMap.NO_ERROR) {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::decodeEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta)
{
    if (!pstMeta)
    {
        throw string("pstMeta to decodeEntry is null");
    }

    //    TDEBUG_PRINT_STR("decodeEntry......");
    //    TDEBUG_PRINT_STRLN(tdr_get_meta_name(pstMeta));
    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "// decode entries");

    // decode entries of current meta
    LPTDRMETAENTRY pstEntry;
    string entryName;
    for (int i = 0; i < pstMeta->iEntriesNum; i++)
    {
        pstEntry = pstMeta->stEntries + i;
        entryName = tdr_get_entry_name(pstEntry);

        Parameter parameter(lines, pstMeta, pstEntry, entryName, Parameter::TYPE_DECODE);
        processEntry(indentLevel, parameter);

        lines.push_back("");
    }
}

void
TTransAS3::decodeBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to decodeBaseTypeEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to decodeBaseTypeEntry is empty");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "if (buf.bytesAvailable < " + int2str(pstEntry->iNUnitSize) + ") {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ErrorMap.ERR_PARTIAL_MSG;");
    }
    lines.push_back(indent + "}");
    lines.push_back(indent + variable + " = buf." + baseType.getDecFunc(pstEntry->iType) + "();");
}

void
TTransAS3::encodeBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to encodeBaseTypeEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to encodeBaseTypeEntry is empty");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);
    if (pstEntry->iCount != 1)
    {
        lines.push_back(indent + "if (" + variable + " == undefined) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_ARRAY_ELEMENT_UNDEFIND;");
        }
        lines.push_back(indent + "}");
    }
    lines.push_back(indent + "buf." + baseType.getEncFunc(pstEntry->iType) + "(" + variable + ");");
}

void
TTransAS3::encodeStringEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to encodeStringEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to encodeStringEntry is empty");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& indent = getIndentStr(indentLevel);
    if (pstEntry->iCount != 1)
    {
        lines.push_back(indent + "if (" + variable + " == undefined) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_ARRAY_ELEMENT_UNDEFIND;");
        }
        lines.push_back(indent + "}");
    } else
    {
        lines.push_back(indent + "if (" + variable + " == null) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_REFERENCE_IS_NULL;");
        }
        lines.push_back(indent + "}");
    }

    if (pstEntry->iCustomHUnitSize > 0)
    {
        const string& maxSize = int2str(pstEntry->iCustomHUnitSize);
        lines.push_back(indent + "if (" + variable + ".length + 1 > " + maxSize +") {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_STR_SURPASS_DEFINED_SIZE;");
        }
        lines.push_back(indent + "}");
    }

    //    lines.push_back(indent + "buf.writeUnsignedInt(" + variable + ".length + 1);");
    lines.push_back(indent + "buf.writeUTFBytes(" + variable + ");");
    lines.push_back(indent + "buf.writeByte(0);");
}

void
TTransAS3::decodeStringEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to decodeStringEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to decodeStringEntry is empty");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;

    const string& strLenName = string(tdr_get_entry_name(pstEntry)) + "Size";
    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "if (buf.bytesAvailable < " + strLenName + ") {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return ErrorMap.ERR_PARTIAL_MSG;");
    }
    lines.push_back(indent + "}");
    lines.push_back(indent + variable + " = buf.readUTFBytes(" + strLenName + " - 1);");
    lines.push_back(indent + "buf.readByte();");
}

void
TTransAS3::processEntry(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstMeta)
    {
        throw string("'parameter.pstMeta' to processEntry is null");
    }
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to processEntry is null");
    }
    if (parameter.variable.empty())
    {
        throw string("'parameter.variable' to processEntry is empty");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETA pstMeta = parameter.pstMeta;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;
    const string& variable = parameter.variable;
    Parameter::OperType operation = parameter.operType;

    const string& indent = getIndentStr(indentLevel);
    unsigned int furtherIndent = 0;
    string arrIndex;

    const string& entryName = tdr_get_entry_name(pstEntry);
    lines.push_back(indent + "if (VERSION_" + entryName + " <= cutVersion) {");
    {
        if (pstEntry->iCount < 0)
        {
            throw string(tdr_get_entry_name(pstEntry)) + ": invalid iCount value<"
                + int2str(pstEntry->iCount) + ">";
        }

        const string& indent = getIndentStr(indentLevel + 1);

        string entryRefer;
        if (pstEntry->stRefer.iUnitSize > 0)
        {
            entryRefer = string(tdr_get_entry_name(pstEntry)) + "Refer";
            string referPath;
            offset2path(pstMeta, pstEntry->stRefer.iHOff, referPath);
            lines.push_back(indent + "const " + entryRefer + " : int = " + referPath + ";");
            lines.push_back(indent + "if (" + entryRefer + " < 0) {");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                lines.push_back(indent + "return ErrorMap.ERR_MINUS_REFER_VALUE;");
            }
            lines.push_back(indent + "}");
            if (pstEntry->iCount != 0)
            {
                lines.push_back(indent + "if (" + entryRefer + " > " + int2str(pstEntry->iCount) + ") {");
                {
                    const string& indent = getIndentStr(indentLevel + 2);
                    lines.push_back(indent + "return ErrorMap.ERR_REFER_BIGGER_THAN_COUNT;");
                }
                lines.push_back(indent + "}");
            }
        }

        if (pstEntry->iType == TDR_TYPE_UNION)
        {
            const string selectName = entryName + "Select";
            string selectPath;
            offset2path(pstMeta, pstEntry->stSelector.iHOff, selectPath);
            lines.push_back(indent + "const " + selectName + " : int = " + selectPath + ";");
        }

        if (pstEntry->iCount != 1)
        {
            string loopCountVar = int2str(pstEntry->iCount);
            if (pstEntry->stRefer.iUnitSize > 0)
            {
                loopCountVar = entryRefer;
            }
            if (operation == Parameter::TYPE_ENCODE && !baseType.has(pstEntry->iType))
            {
                if (pstEntry->stRefer.iUnitSize > 0)
                {
                    lines.push_back(indent + "if (" + loopCountVar + " > 0 && "
                                    + entryName + " == null) {");
                } else
                {
                    lines.push_back(indent + "if (" + entryName + " == null) {");
                }
                {
                    const string& indent = getIndentStr(indentLevel + 2);
                    lines.push_back(indent + "return ErrorMap.ERR_ARRAY_REFER_IS_NULL;");
                }
                lines.push_back(indent + "}");
            }

            if (operation == Parameter::TYPE_DECODE && pstMeta->iType == TDR_TYPE_UNION)
            {
                string entryType;
                getEntryType(pstEntry, entryType);
                lines.push_back(indent + entryName + " = new Vector.<" + entryType + ">();");
            }

            const string& loopVarName = string(tdr_get_entry_name(pstEntry)) + "_i";
            lines.push_back(indent + "for (var " + loopVarName + " : uint = 0; "
                            + loopVarName + " < " + loopCountVar + "; "
                            + loopVarName + "++) {");
            furtherIndent = 1;
            arrIndex = string("[") + loopVarName + "]";
        } else
        {
            if (pstEntry->stRefer.iUnitSize > 0)
            {
                lines.push_back(indent + "if (" + entryRefer + " == 1) {");
                furtherIndent = 1;
            }
        }

        Parameter parameter(lines, pstMeta, pstEntry, variable + arrIndex, operation);
        if (operation == Parameter::TYPE_ENCODE)
        {
            processEntryEncode(indentLevel + 1 + furtherIndent, parameter);
        } else if (operation == Parameter::TYPE_DECODE)
        {
            processEntryDecode(indentLevel + 1 + furtherIndent, parameter);
        }

        if (furtherIndent)
        {
            lines.push_back(indent + "}");
        }
    }
    lines.push_back(indent + "}");
}

void
TTransAS3::directWrite (unsigned int indentLevel, list<string>& lines, int switchValue,
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

    string writeTinyFunc;
    string writeShortFunc;
    string writeIntFunc;

    writeTinyFunc  = "TdrUtil.directWriteTinyuint";
    writeShortFunc = "TdrUtil.directWriteSmalluint";
    writeIntFunc   = "TdrUtil.directWriteUint";

    string writeFunc = writeIntFunc;
    switch (switchValue)
    {
        case 1:
            writeFunc = writeTinyFunc;
            break;
        case 2:
            writeFunc = writeShortFunc;
            break;
        case 4:
            writeFunc = writeIntFunc;
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + writeFunc + "("
                    + bufName + ", "
                    + pos + ", "
                    + value + ");");
}

void
TTransAS3::directRead(unsigned int indentLevel, list<string>& lines, int switchValue,
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
    readTinyFunc  = "TdrUtil.directReadTinyuint";
    readShortFunc = "TdrUtil.directReadSmalluint";
    readIntFunc   = "TdrUtil.directReadUint";

    string readFunc = readIntFunc;
    switch (switchValue)
    {
        case 1:
            readFunc = readTinyFunc;
            break;
        case 2:
            readFunc = readShortFunc;
            break;
        case 4:
            readFunc = readIntFunc;
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + leftValue + " = " + readFunc + "("
                    + bufName + ", "
                    + pos + ");");
}

void
TTransAS3::serialRead(unsigned int indentLevel, list<string>& lines, int switchValue,
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

    readTinyFunc  = "readUnsignedByte";
    readShortFunc = "readUnsignedShort";
    readIntFunc   = "readUnsignedInt";

    string readFunc = readIntFunc;
    switch (switchValue)
    {
        case 1:
            readFunc = readTinyFunc;
            break;
        case 2:
            readFunc = readShortFunc;
            break;
        case 4:
            readFunc = readIntFunc;
            break;
        default:
            throw throwStr;
            break;
    }
    lines.push_back(indent + leftValue + " = " + bufName + "." + readFunc + "()");
}

void
TTransAS3::processEntryEncode(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to processEntryEncode is null");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;

    const string& indent = getIndentStr(indentLevel);
    const string& entryName = tdr_get_entry_name(pstEntry);
    const string& entrySizePosVarName = entryName + "SizeInfoPos";
    const string& entryBeginBufPosVarName = entryName + "BeginPos";

    if (pstEntry->stSizeInfo.iUnitSize > 0)
    {
        lines.push_back(indent + "// record the place to set sizeinfo");
        if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET)
        {
            lines.push_back(indent + "const " + entrySizePosVarName + " : uint = "
                            + "savedBufBeginPos + " + int2str((int)pstEntry->stSizeInfo.iNOff) + ";");
        } else
        {
            lines.push_back(indent + "const " + entrySizePosVarName + " : uint = buf.position;");
            lines.push_back(indent + "// reserve space for entry's sizeinfo");
            switch (pstEntry->stSizeInfo.iUnitSize)
            {
                case 1:
                case 2:
                case 4:
                    lines.push_back(indent + "buf.position += "
                                    + int2str(pstEntry->stSizeInfo.iUnitSize) + ";");
                    break;
                default:
                    throw string("invalid sizeinfo iUnitSize, this is a tdr-lib error");
                    break;
            }
        }
        lines.push_back(indent + "// record entry's begin position in buf");
        lines.push_back(indent + "var " + entryBeginBufPosVarName + " : uint = buf.position;");
    }

    if (baseType.has(pstEntry->iType))
    {
        encodeBaseTypeEntry(indentLevel, parameter);
    } else if (pstEntry->iType == TDR_TYPE_STRING)
    {
        encodeStringEntry(indentLevel, parameter);
    } else // struct, union, int64, uint64
    {
        encodeCallableEntry(indentLevel, parameter);
    }

    if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
        || pstEntry->stSizeInfo.idxSizeType != TDR_INVALID_INDEX)
    {
        lines.push_back(indent + "// set sizeinfo");
        directWrite(indentLevel, lines, pstEntry->stSizeInfo.iUnitSize,
                    "buf", entrySizePosVarName,
                    "buf.position - " + entryBeginBufPosVarName,
                    "invalid sizeinfo iUnitSize, this is a tdr-lib error");
    }
}

void
TTransAS3::processEntryDecode(unsigned int indentLevel, const Parameter& parameter)
{
    if (!parameter.pstEntry)
    {
        throw string("'parameter.pstEntry' to processEntryDecode is null");
    }

    list<string>& lines = parameter.lines;
    LPTDRMETAENTRY pstEntry = parameter.pstEntry;

    const string& indent = getIndentStr(indentLevel);
    const string& entryName = tdr_get_entry_name(pstEntry);

    if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET
        || pstEntry->stSizeInfo.idxSizeType != TDR_INVALID_INDEX)
    {
        const string entrySizeinfoVar = string("const ") + entryName + "Size : uint";
        if (pstEntry->stSizeInfo.iNOff != TDR_INVALID_OFFSET)
        {
            lines.push_back(indent + "// get entry's sizeinfo");
            directRead(indentLevel, lines, pstEntry->stSizeInfo.iUnitSize,
                       entrySizeinfoVar, "buf",
                       "savedBufBeginPos + " + int2str((int)pstEntry->stSizeInfo.iHOff),
                       "invalid stSizeInfo iUnitSize, this is a tdr-lib error");
        } else
        {
            serialRead(indentLevel, lines, pstEntry->stSizeInfo.iUnitSize,
                       entrySizeinfoVar, "buf",
                       "invalid stSizeInfo iUnitSize, this is a tdr-lib error");
        }
        lines.push_back(indent + "if (" + entryName + "Size > buf.bytesAvailable) {");
        {
            const string& indent = getIndentStr(indentLevel + 1);
            lines.push_back(indent + "return ErrorMap.ERR_PARTIAL_MSG;");
        }
        lines.push_back(indent + "}");
    }

    if (baseType.has(pstEntry->iType))
    {
        decodeBaseTypeEntry(indentLevel, parameter);
    } else if (pstEntry->iType == TDR_TYPE_STRING)
    {
        decodeStringEntry(indentLevel, parameter);
    } else // struct, union, int64, uint64
    {
        decodeCallableEntry(indentLevel, parameter);
    }
}

void
TTransAS3::offset2path(LPTDRMETA pstMeta, ptrdiff_t offset, string& path)
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
    path = buffer;
}

void
TTransAS3::getEntryType(LPTDRMETAENTRY pstEntry, string& entryType)
{
    if (!pstEntry)
    {
        throw string("'pstEntry' to getEntryType is null");
    }

    if (baseType.has(pstEntry->iType))
    {
        entryType = baseType.getType(pstEntry->iType);
    } else if (pstEntry->iType == TDR_TYPE_LONGLONG)
    {
        entryType = "Int64";
    } else if (pstEntry->iType == TDR_TYPE_ULONGLONG)
    {
        entryType = "UInt64";
    } else if (pstEntry->iType == TDR_TYPE_STRING)
    {
        entryType = "String";
    } else
    {
        LPTDRMETA pstEntryMeta = TDR_PTR_TO_META(pstMetaLib, pstEntry->ptrMeta);
        entryType = tdr_get_meta_name(pstEntryMeta);
    }
}
