#ifndef TTRANS_CSHARP_BASETYPE_H
#define TTRANS_CSHARP_BASETYPE_H

#include <map>
#include <string>

using std::map;
using std::string;

struct CSharpTypeInfo
{
    CSharpTypeInfo(const string& ct="", const string& type="",
             const string& encode="", const string& decode="",const string &strPrint="",int iType = 0)
        : ctype(ct), targetType(type),
        encodeFunction(encode), decodeFunction(decode),
        printInfo(strPrint),tdrType(iType) {}
    CSharpTypeInfo(const CSharpTypeInfo& ti) :
        ctype(ti.ctype),
        targetType(ti.targetType),
        encodeFunction(ti.encodeFunction),
        decodeFunction(ti.decodeFunction),
        printInfo(ti.printInfo),
        tdrType(ti.tdrType){}
    CSharpTypeInfo& operator = (const CSharpTypeInfo& ti)
    {
        ctype = ti.ctype;
        targetType = ti.targetType;
        encodeFunction = ti.encodeFunction;
        decodeFunction = ti.decodeFunction;
        printInfo = ti.printInfo;
        return *this;
    }
    string ctype;
    string targetType;
    string encodeFunction;
    string decodeFunction;
    string printInfo;
    int tdrType;
};

class TTransCSharpBaseType
{
    public:
        TTransCSharpBaseType();
        bool has(unsigned int idxType);
        const string& getNetSize(unsigned int idxType);
        const string& getType(unsigned int idxType);
        const string& getEncFunc(unsigned int idxType);
        const string& getDecFunc(unsigned int idxType);
        const string& getCType(unsigned int idxType);
        const string& getPrintInfo(unsigned int idxType);
    private:
        static void typeCheck(unsigned int idxType);
        map<unsigned int, CSharpTypeInfo> typeMap;
};

#endif //TTRANS_CSHARP_BASETYPE_H

