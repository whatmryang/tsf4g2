#ifndef TTRANS_CSHARP_H
#define TTRANS_CSHARP_H


#include "tdr/tdr.h"
#include "tdr_base.h"
#include "tdr_option.h"
#include "tdr_csharp_baseType.h"
#include "tdr_xml_desc.h"

#include <list>
#include <string>
#include <set>

using std::string;
using std::list;
using std::set;


#define DEFINE_CSHARP_CONTAINER() \
    list<string> Lines;

#define APPEND_CSHARP_CONTAINER(CONTENT) \
    (Lines).push_back(CONTENT);

#define OUTPUT_CSHARP_CONTAINER(File) \
    this->output(Lines, File);

#define OUTPUT_CSHARP_CONTAINER_NOMETA(File) \
    this->output(Lines, File, false);


class TTransCSharp : public TTransBase
{
    public:
        TTransCSharp(const string& indentStr, const string& outputDir);
        void translate(LPTDRMETALIB pstMetaLib,const list<XMLDesc>& xmlDescList,TdrOption* pstOption);
    private:
        typedef void (TTransCSharp::*EntryProcessFuncType)
            (unsigned int indentLevel, const Parameter& parameter);
    private:
        LPTDRMETALIB pstMetaLib;
        TdrOption* pstOption;
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

        void makeTdrBufUtil();
        void makeTdrTypeUtil();
        void makeTdrTime();

        void makeMetaLibInfo();

        void translateMacros(const list<XMLDesc>& xmlDescList);
        void translateInterface();
        void translateGroups(const list<string>& groups);
        void translateMetas(const list<string>& metas);

        void translateStruct(LPTDRMETA pstMeta);
        void translateUnion(LPTDRMETA pstMeta);

        void structConstruct(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void unionConstruct(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void declaration(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void structPack(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void structUnpack(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void structLoad(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void structSizeInfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        //void unionSizeInfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void version(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void prepare4sizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void packVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void packSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void packStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void packBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter);
        void packStringEntry(unsigned int indentLevel, const Parameter& parameter);
        void packCallableEntry(unsigned int indentLevel, const Parameter& parameter);
        void loadVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void loadSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void loadStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
        void loadBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter);
        void loadStringEntry(unsigned int indentLevel, const Parameter& parameter);
        void loadCallableEntry(unsigned int indentLevel, const Parameter& parameter);

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

        void processUnion(unsigned int indentLevel, list<string>& csLines,
                           LPTDRMETA pstMeta, Parameter::OperType oper);

        void generateUnionMethods(unsigned int indentLevel, list<string>& lines,
                                  LPTDRMETA pstMeta, Parameter::OperType oper);

        void processUnionEntry(unsigned int indentLevel, list<string>& lines, const Parameter& parameter);

        void structVisualize(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void constructStruct(unsigned int indentLevel, list<string>& lines,LPTDRMETA pstMeta);

        void visualizeEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void constructStructEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

        void processEntry4Load(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntry4Code(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntry4Visualize(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntry4Construct(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntry4DefaultLoad(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntry4Default(unsigned int indentLevel, const Parameter& a_parameter);

        void processEntryPack(unsigned int indentLevel, const Parameter& parameter);
        void processEntryUnpack(unsigned int indentLevel, const Parameter& parameter);
        void processEntryLoad(unsigned int indentLevel, const Parameter& parameter);

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

        static void getEntryDefaultValue(LPTDRMETAENTRY pstEntry, string& entryDefaultValue);
        static unsigned int getReferTypeTopValue(LPTDRMETAENTRY pstEntry);

        void flagMeta4Default();
        bool needSetDefaultValue(LPTDRMETA pstMeta);
        bool needSetDefaultValue(LPTDRMETAENTRY pstEntry);
        bool hasSpecialDefaultValue(LPTDRMETAENTRY pstEntry);

    private:
        TTransCSharpBaseType baseType;
        set<string> touchedFiles;
        string  csFileName;
        set<string> defaultSet;

        static const char* errCode[];
        static const char* bufCode[];
        static const char* typeUtilCode[];
        static const char* bufUtilCode[];
        static const char* timeCode[];
        static const char* readme[];

};

#endif

