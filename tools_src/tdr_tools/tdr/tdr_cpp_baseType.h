#ifndef TTRANS_CPP_BASETYPE_H
#define TTRANS_CPP_BASETYPE_H

#include <map>
#include <string>

using std::map;
using std::string;

struct CPPTypeInfo
{
    CPPTypeInfo(const string& type="", const string& encode="",
                const string& decode="", const string& print="", int iType = 0)
        : targetType(type), encodeFunction(encode),
        decodeFunction(decode), printInfo(print), tdrType(iType) {}
    CPPTypeInfo(const CPPTypeInfo& ti) :
        targetType(ti.targetType),
        encodeFunction(ti.encodeFunction),
        decodeFunction(ti.decodeFunction),
        printInfo(ti.printInfo),
        tdrType(ti.tdrType) {}
    CPPTypeInfo& operator = (const CPPTypeInfo& ti)
    {
        targetType = ti.targetType;
        encodeFunction = ti.encodeFunction;
        decodeFunction = ti.decodeFunction;
        printInfo = ti.printInfo;
        tdrType = ti.tdrType;
        return *this;
    }
    string targetType;
    string encodeFunction;
    string decodeFunction;
    string printInfo;
    int tdrType;
};

class TTransCPPBaseType
{
    public:
        TTransCPPBaseType();
        bool has(unsigned int idxType);
        const string& getType(unsigned int idxType);
        const string& getEncFunc(unsigned int idxType);
        const string& getDecFunc(unsigned int idxType);
        const string& getPrintInfo(unsigned int idxType);
    private:
        static void typeCheck(unsigned int idxType);
        map<unsigned int, CPPTypeInfo> typeMap;
};

#endif
