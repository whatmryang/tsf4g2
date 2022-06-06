#include "tdr_as3_baseType.h"
#include "tdr/tdr.h"
#include <iterator>

TTransAS3BaseType::TTransAS3BaseType()
{
    typeMap[ TDR_TYPE_CHAR ]       = TypeInfo("int8_t",   "1", "int",    "writeByte",        "readByte");
    typeMap[ TDR_TYPE_UCHAR ]      = TypeInfo("uint8_t",  "1", "uint",   "writeByte",        "readUnsignedByte");
    typeMap[ TDR_TYPE_BYTE ]       = TypeInfo("uint8_t",  "1", "int",    "writeByte",        "readByte");
    typeMap[ TDR_TYPE_SMALLINT ]   = TypeInfo("int16_t",  "2", "int",    "writeShort",       "readShort");
    typeMap[ TDR_TYPE_SMALLUINT ]  = TypeInfo("uint16_t", "2", "uint",   "writeShort",       "readUnsignedShort");
    typeMap[ TDR_TYPE_INT ]        = TypeInfo("int32_t",  "4", "int",    "writeInt",         "readInt");
    typeMap[ TDR_TYPE_UINT ]       = TypeInfo("uint32_t", "4", "uint",   "writeUnsignedInt", "readUnsignedInt");
    typeMap[ TDR_TYPE_LONG ]       = TypeInfo("int32_t",  "4", "int",    "writeInt",         "readInt");
    typeMap[ TDR_TYPE_ULONG ]      = TypeInfo("int32_t",  "4", "uint",   "writeUnsignedInt", "readUnsignedInt");
    typeMap[ TDR_TYPE_DATE ]       = TypeInfo("uint32_t", "4", "uint",   "writeUnsignedInt", "readUnsignedInt");
    typeMap[ TDR_TYPE_TIME ]       = TypeInfo("uint32_t", "4", "uint",   "writeUnsignedInt", "readUnsignedInt");
    typeMap[ TDR_TYPE_MONEY ]      = TypeInfo("int32_t",  "4", "int",    "writeUnsignedInt", "readUnsignedInt");
    typeMap[ TDR_TYPE_FLOAT ]      = TypeInfo("float",    "4", "Number", "writeFloat",       "readFloat");
    typeMap[ TDR_TYPE_DOUBLE ]     = TypeInfo("double",   "8", "Number", "writeDouble",      "readDouble");
    typeMap[ TDR_TYPE_IP ]         = TypeInfo("int32_t",  "4", "uint",   "writeUnsignedInt", "readUnsignedInt");
}

const string&
TTransAS3BaseType::getCType(unsigned int idxType)
{
    typeCheck(idxType);

	std::map<unsigned int, TypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.ctype;
}

const string&
TTransAS3BaseType::getType(unsigned int idxType)
{
    typeCheck(idxType);

	std::map<unsigned int, TypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.targetType;
}

const string&
TTransAS3BaseType::getEncFunc(unsigned int idxType)
{
    typeCheck(idxType);

	std::map<unsigned int, TypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.encodeFunction;
}

const string&
TTransAS3BaseType::getDecFunc(unsigned int idxType)
{
    typeCheck(idxType);

	std::map<unsigned int, TypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.decodeFunction;
}

const string&
TTransAS3BaseType::getNetSize(unsigned int idxType)
{
    typeCheck(idxType);
	std::map<unsigned int, TypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.netSize;
}

bool
TTransAS3BaseType::has(unsigned int idxType)
{
    typeCheck(idxType);
	std::map<unsigned int, TypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        return false;
    return true;
}

void
TTransAS3BaseType::typeCheck(unsigned int idxType)
{
    if (idxType == TDR_TYPE_DATETIME)
        throw string("type=\"datetime\" is not supported for AS3");
    if (idxType == TDR_TYPE_WCHAR)
        throw string("type=\"wchar\" is not supported for AS3");
    if (idxType == TDR_TYPE_WSTRING)
        throw string("type=\"wstring\" is not supported for AS3");
    if (idxType == TDR_TYPE_VOID)
        throw string("type=\"void*\" is not supported for AS3");
}
