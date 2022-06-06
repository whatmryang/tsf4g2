#ifndef TTRANS_AS3_BASETYPE_H
#define TTRANS_AS3_BASETYPE_H

#include <map>
#include <string>

using std::map;
using std::string;

struct TypeInfo
{
    TypeInfo(const string& ct="", const string& size="", const string& type="",
             const string& encode="", const string& decode="")
        : ctype(ct), netSize(size), targetType(type),
        encodeFunction(encode), decodeFunction(decode) {}
    TypeInfo(const TypeInfo& ti) :
        ctype(ti.ctype),
        netSize(ti.netSize),
        targetType(ti.targetType),
        encodeFunction(ti.encodeFunction),
        decodeFunction(ti.decodeFunction) {}
    TypeInfo& operator = (const TypeInfo& ti)
    {
        ctype = ti.ctype;
        netSize = ti.netSize;
        targetType = ti.targetType;
        encodeFunction = ti.encodeFunction;
        decodeFunction = ti.decodeFunction;
        return *this;
    }
    string ctype;
    string netSize;
    string targetType;
    string encodeFunction;
    string decodeFunction;
};

class TTransAS3BaseType
{
    public:
        TTransAS3BaseType();
        bool has(unsigned int idxType);
        const string& getNetSize(unsigned int idxType);
        const string& getType(unsigned int idxType);
        const string& getEncFunc(unsigned int idxType);
        const string& getDecFunc(unsigned int idxType);
        const string& getCType(unsigned int idxType);
    private:
        static void typeCheck(unsigned int idxType);
        map<unsigned int, TypeInfo> typeMap;
};

#endif
