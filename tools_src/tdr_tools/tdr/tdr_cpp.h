#ifndef TTRANS_CPP_H
#define TTRANS_CPP_H

#include <set>
#include <list>
#include <string>
#include "tdr/tdr.h"
#include "tdr_base.h"
#include "tdr_option.h"
#include "tdr_xml_desc.h"
#include "tdr_as3_baseType.h"
#include "tdr_cpp_baseType.h"

#define TDR_MD5_DIGEST_LENGTH 16

#define DEFINE_CONTAINER_HEAD() \
    list<string> headLines;

#define DEFINE_CONTAINER() \
    list<string> headLines, cppLines;

#define APPEND_CONTAINER(CONTENT) \
    (headLines).push_back(CONTENT); \
(cppLines).push_back(CONTENT);

#define APPEND_CONTAINER_HEAD(CONTENT) \
    (headLines).push_back(CONTENT);

#define APPEND_CONTAINER_CPP(CONTENT) \
    (cppLines).push_back(CONTENT);

#define OUTPUT_CONTAINER_HEAD(headFile) \
    this->output(headLines, headFile);

#define OUTPUT_CONTAINER(headFile, cppFile) \
    this->output(headLines, headFile);\
this->output(cppLines, cppFile);

#define OUTPUT_CONTAINER_HEAD_NOMETA(headFile) \
    this->output(headLines, headFile, false);

#define OUTPUT_CONTAINER_NOMETA(headFile, cppFile) \
    this->output(headLines, headFile, false);\
this->output(cppLines, cppFile, false);

using std::string;
using std::list;
using std::set;

class TTransCPP : public TTransBase
{
    public:
        TTransCPP(const string& indentStr, const string& outputDir);
        void translate(LPTDRMETALIB pstMetaLib,
                       const list<XMLDesc>& xmlDescList,
                       TdrOption* pstOption);

        void makeComm();
    private:
        typedef void (TTransCPP::*EntryProcessFuncType)
            (unsigned int indentLevel, const Parameter& parameter);
    private:
        LPTDRMETALIB pstMetaLib;
        string packageName;
        string metalibMd5;
        static string ttranslatorVersion;

        void addTdrInfo(list<string>& lines, bool careMeta = true);
        void output(list<string>& lines, const string& fileName, bool careMeta = true);
        void adjustCutVersion(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void checkMetaLib();

        /* meta-lib-independent files */
        void makeReadMe();
        void makeTdrErr();
        void makeTdrBuf();
        void makeTdrPal();
        void makeTdrBufUtil();
        void makeTdrTypeUtil();
        void makeTdrThird();
        void makeTdrTime();

        void makeMetaLibInfo();

        void translateMacros(const list<string>& macros);
        void translateGroups(const list<string>& groups);
        void translateMetas(const list<string>& metas);

        void translateStruct(LPTDRMETA pstMeta);
        void translateUnion(LPTDRMETA pstMeta);

        void declaration(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void structPack(unsigned int indentLevel, list<string>& headLines,
                        list<string>& cppLines, LPTDRMETA pstMeta);

        void structUnpack(unsigned int indentLevel, list<string>& headLines,
                          list<string>& cppLines, LPTDRMETA pstMeta);
        void version(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void prepare4sizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void packVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void packSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void packStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void packBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter);
        void packStringEntry(unsigned int indentLevel, const Parameter& parameter);
        void packCallableEntry(unsigned int indentLevel, const Parameter& parameter);

        void unpackVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void unpackSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void unpackStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void unpackBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter);
        void unpackStringEntry(unsigned int indentLevel, const Parameter& parameter);
        void unpackCallableEntry(unsigned int indentLevel, const Parameter& parameter);

        void visualizeNonLeafEntry(unsigned int indentLevel, const Parameter& parameter, const string& arrIndex);

        void constructNonLeafEntry(unsigned int indentLevel, const Parameter& parameter, const string& arrIndex);

        void visualizeCommLeafEntry(unsigned int indentLevel, const Parameter& parameter,
                                    const string& loopVarType, const string& loopCountVar);

        void constructCommLeafEntry(unsigned int indentLevel, const Parameter& parameter,
                                  const string& loopVarType, const string& loopCountVar);

        void visualizeSpecialLeafEntry(unsigned int indentLevel, const Parameter& parameter, const string& arrIndex);

        void constructStringEntry(unsigned int indentLevel, const Parameter& parameter, const string& arrIndex);

        void constructBinarySpecilEntry(unsigned int indentLevel, const Parameter& parameter, const string& arrIndex);

        void processUnion(unsigned int indentLevel, list<string>& headLines,
                          list<string>& cppLines, LPTDRMETA pstMeta, Parameter::OperType oper);

        void generateUnionMethods(unsigned int indentLevel, list<string>& lines,
                                  LPTDRMETA pstMeta, Parameter::OperType oper);

        void processUnionEntry(unsigned int indentLevel, list<string>& lines, const Parameter& parameter);

        void structVisualize(unsigned int indentLevel, list<string>& headLines,
                             list<string>& cppLines, LPTDRMETA pstMeta);

        void structConstruct(unsigned int indentLevel, list<string>& headLines,
                           list<string>& cppLines, LPTDRMETA pstMeta);

        void visualizeEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void constructStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void processEntry4Code(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntry4Visualize(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntry4Construct(unsigned int indentLevel, const Parameter& a_parameter);
        void processEntry4Default(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntryPack(unsigned int indentLevel, const Parameter& parameter);
        void processEntryUnpack(unsigned int indentLevel, const Parameter& parameter);

        void directWrite(unsigned int indentLevel, list<string>& lines, int switchValue,
                         const string& bufName, const string& pos, const string& value,
                         const string& throwStr);
        void directRead(unsigned int indentLevel, list<string>& lines, int switchValue,
                        const string& leftValue, const string& bufName, const string& pos,
                        const string& throwStr);
        void serialRead(unsigned int indentLevel, list<string>& lines, int switchValue,
                        const string& leftValue, const string& bufName,
                        const string& throwStr);

        void offset2path(LPTDRMETA pstMeta, ptrdiff_t offset, string& path);
        void offset2version(LPTDRMETA pstMeta, ptrdiff_t offset, string& path);
        void getEntryType(LPTDRMETAENTRY pstEntry, string& entryType);
        void loadCodeRes(unsigned int indentLevel, list<string>& lines,
                         const char *pszSrc[], size_t lineCount);

        string getEntryName(LPTDRMETAENTRY pstEntry);
        string getEntryNameWithThis(LPTDRMETAENTRY pstEntry);
        string getEntryVersion(LPTDRMETAENTRY pstEntry, LPTDRMETA pstMeta);
        string getEntryCount(LPTDRMETAENTRY pstEntry);

        static void getEntryDefaultValue(LPTDRMETAENTRY pstEntry, string& entryDefaultValue);
        static unsigned int getReferTypeTopValue(LPTDRMETAENTRY pstEntry);
        static string getUpperString(const string& source);

        void outputTypedef(list<string>& lines, const string& indent, LPTDRMETA pstMeta);

        void flagMeta4Default();
        bool needSetDefaultValue(LPTDRMETA pstMeta);
        bool needSetDefaultValue(LPTDRMETAENTRY pstEntry);
        bool hasSpecialDefaultValue(LPTDRMETAENTRY pstEntry);

    private:
        TTransCPPBaseType baseType;
        set<string> touchedFiles;
        string headFileName;
        string cppFileName;
        set<string> defaultSet;

        TdrOption* pstOption;

        static const char* errHead[];
        static const char* errCpp[];
        static const char* bufHead[];
        static const char* bufCpp[];
        static const char* palHead[];
        static const char* bufUtilHead[];
        static const char* bufUtilCpp[];
        static const char* typeUtilHead[];
        static const char* typeUtilCpp[];
        static const char* thirdHead[];
        static const char* thirdCpp[];
        static const char* timeHead[];
        static const char* timeCpp[];

        static const char* readme[];
};

#endif
