#ifndef TTRANS_AS3_H
#define TTRANS_AS3_H

#include <list>
#include <string>
#include "tdr/tdr.h"
#include "tdr_base.h"
#include "tdr_as3_baseType.h"

using std::string;
using std::list;

class TTransAS3 : public TTransBase
{
    public:
        TTransAS3(const string& indentStr, const string& outputDir)
            : TTransBase(indentStr, outputDir, "// ") {}
              void translate(LPTDRMETALIB pstMetaLib);
    private:
              typedef void (TTransAS3::*EntryProcessFuncType)
                  (unsigned int indentLevel, const Parameter& parameter);
    private:
              LPTDRMETALIB pstMetaLib;
              string packageName;
              string metalibMd5;
              static string ttranslatorVersion;
              void output(list<string>& lines, const string& fileName);
              void adjustCutVersion(unsigned int indentLevel, list<string>& lines);

              void checkMetaLib();

              void makeErrorMap();
              void makeErrorMap_i(unsigned int indentLevel, list<string>& lines);
              void makeTdrUtil();
              void makeTdrUtil_i(unsigned int indentLevel, list<string>& lines);
              void makeInt64();
              void makeInt64_i(unsigned int indentLevel, list<string>& lines);
              void makeUint64();
              void makeUint64_i(unsigned int indentLevel, list<string>& lines);

              void translateMacros();
              void translateStruct(LPTDRMETA pstMeta);
              void translateUnion(LPTDRMETA pstMeta);

              void declaration(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void structConstruct(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void structEncode(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void structDecode(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void version(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

              void prepare4sizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void encodeVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void encodeSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void encodeEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void encodeBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter);
              void encodeStringEntry(unsigned int indentLevel, const Parameter& parameter);
              void encodeCallableEntry(unsigned int indentLevel, const Parameter& parameter);

              void decodeVersionIndicator(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void decodeSizeinfo(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void decodeEntry(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void decodeBaseTypeEntry(unsigned int indentLevel, const Parameter& parameter);
              void decodeStringEntry(unsigned int indentLevel, const Parameter& parameter);
              void decodeCallableEntry(unsigned int indentLevel, const Parameter& parameter);

              void unionConstruct(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void unionEncode(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);
              void unionDecode(unsigned int indentLevel, list<string>& lines, LPTDRMETA pstMeta);

              void processEntry(unsigned int indentLevel, const Parameter& parameter);
              void processEntryEncode(unsigned int indentLevel, const Parameter& parameter);
              void processEntryDecode(unsigned int indentLevel, const Parameter& parameter);

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
              void getEntryType(LPTDRMETAENTRY pstEntry, string& entryType);
    private:
              TTransAS3BaseType baseType;
};

#endif
