#ifndef TTRANS_BASE_H
#define TTRANS_BASE_H

#include <map>
#include <list>
#include <string>
#include <iostream>
#include "tdr/tdr.h"

using std::ios_base;
using std::string;
using std::list;
using std::map;

class Parameter
{
    public:
        enum OperType {TYPE_INIT, TYPE_ENCODE, TYPE_DECODE, TYPE_VISUALIZE, TYPE_LOAD};
        Parameter(list<string>& li, LPTDRMETA pm,
                  LPTDRMETAENTRY pe, const string& var, OperType ot)
            :lines(li), pstMeta(pm), pstEntry(pe), variable(var), operType(ot){};
    public:
        list<string>& lines;
        LPTDRMETA pstMeta;
        LPTDRMETAENTRY pstEntry;
        const string variable;
        OperType operType;

        /* added to enhance performance to encode/decode bit8-array */
        string byteArrLen;
};

class TTransBase
{
    public:
        TTransBase(const string& indentStr, const string& outputDir, const string& commentStr);
        virtual ~TTransBase();
        const string& getIndentStr(unsigned int indentLevel=0);
        const string& getCommentStr(unsigned int indentLevel=0);
        const char* getEntryType(LPTDRMETALIB pstMetaLib, LPTDRMETAENTRY pstEntry);

    protected:
        void output(const list<string>& lines, const string& file,
                    ios_base::openmode mode = ios_base::trunc);

    private:
        string indentStr;
        string outputDir;
        string commentStr;
        map<unsigned int, string> indentsMap;
        map<unsigned int, string> commentsMap;
};

#endif
