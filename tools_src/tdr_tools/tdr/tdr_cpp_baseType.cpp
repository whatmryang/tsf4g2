#include "tdr_cpp_baseType.h"
#include "tdr/tdr.h"
#include <iterator>

TTransCPPBaseType::TTransCPPBaseType()
{
    typeMap[0]  = CPPTypeInfo("union"          ,"impossible"    ,"impossible"  ,"impossible"             ,TDR_TYPE_UNION      ); // TDR_TAG_UNION
    typeMap[1]  = CPPTypeInfo("struct"         ,"impossible"    ,"impossible"  ,"impossible"             ,TDR_TYPE_STRUCT     ); // TDR_TAG_STRUCT
    typeMap[2]  = CPPTypeInfo("int8_t"         ,"writeInt8"     ,"readInt8"    ,"\"0x%02x\""             ,TDR_TYPE_CHAR       ); // TDR_TAG_TINYINT
    typeMap[3]  = CPPTypeInfo("uint8_t"        ,"writeUInt8"    ,"readUInt8"   ,"\"0x%02x\""             ,TDR_TYPE_UCHAR      ); // TDR_TAG_TINYUINT
    typeMap[4]  = CPPTypeInfo("int16_t"        ,"writeInt16"    ,"readInt16"   ,"\"%d\""                 ,TDR_TYPE_SMALLINT   ); // TDR_TAG_SMALLINT
    typeMap[5]  = CPPTypeInfo("uint16_t"       ,"writeUInt16"   ,"readUInt16"  ,"\"%d\""                 ,TDR_TYPE_SMALLUINT  ); // TDR_TAG_SMALLUINT
    typeMap[6]  = CPPTypeInfo("int32_t"        ,"writeInt32"    ,"readInt32"   ,"\"%d\""                 ,TDR_TYPE_INT        ); // TDR_TAG_INT
    typeMap[7]  = CPPTypeInfo("uint32_t"       ,"writeUInt32"   ,"readUInt32"  ,"\"%u\""                 ,TDR_TYPE_UINT       ); // TDR_TAG_UINT
    typeMap[8]  = CPPTypeInfo("int64_t"        ,"writeInt64"    ,"readInt64"   ,"TDR_CPP_INT64_FORMAT"   ,TDR_TYPE_LONGLONG   ); // TDR_TAG_BIGINT
    typeMap[9]  = CPPTypeInfo("uint64_t"       ,"writeUInt64"   ,"readUInt64"  ,"TDR_CPP_UINT64_FORMAT"  ,TDR_TYPE_ULONGLONG  ); // TDR_TAG_BIGUINT
    typeMap[10] = CPPTypeInfo("int8_t"         ,"writeInt8"     ,"readInt8"    ,"\"0x%02x\""             ,TDR_TYPE_CHAR       ); // TDR_TAG_INT8
    typeMap[11] = CPPTypeInfo("uint8_t"        ,"writeUInt8"    ,"readUInt8"   ,"\"0x%02x\""             ,TDR_TYPE_UCHAR      ); // TDR_TAG_UINT8
    typeMap[12] = CPPTypeInfo("int16_t"        ,"writeInt16"    ,"readInt16"   ,"\"%d\""                 ,TDR_TYPE_SMALLINT   ); // TDR_TAG_INT16
    typeMap[13] = CPPTypeInfo("uint16_t"       ,"writeUInt16"   ,"readUInt16"  ,"\"%d\""                 ,TDR_TYPE_SMALLUINT  ); // TDR_TAG_UINT16
    typeMap[14] = CPPTypeInfo("int32_t"        ,"writeInt32"    ,"readInt32"   ,"\"%d\""                 ,TDR_TYPE_INT        ); // TDR_TAG_INT32
    typeMap[15] = CPPTypeInfo("uint32_t"       ,"writeUInt32"   ,"readUInt32"  ,"\"%u\""                 ,TDR_TYPE_UINT       ); // TDR_TAG_UINT32
    typeMap[16] = CPPTypeInfo("int64_t"        ,"writeInt64"    ,"readInt64"   ,"TDR_CPP_INT64_FORMAT"   ,TDR_TYPE_LONGLONG   ); // TDR_TAG_INT64
    typeMap[17] = CPPTypeInfo("uint64_t"       ,"writeUInt64"   ,"readUInt64"  ,"TDR_CPP_UINT64_FORMAT"  ,TDR_TYPE_ULONGLONG  ); // TDR_TAG_UINT64
    typeMap[18] = CPPTypeInfo("float"          ,"writeFloat"    ,"readFloat"   ,"\"%f\""                 ,TDR_TYPE_FLOAT      ); // TDR_TAG_FLOAT
    typeMap[19] = CPPTypeInfo("double"         ,"writeDouble"   ,"readDouble"  ,"\"%f\""                 ,TDR_TYPE_DOUBLE     ); // TDR_TAG_DOUBLE
    typeMap[20] = CPPTypeInfo("float"          ,"writeFloat"    ,"readFloat"   ,"\"%f\""                 ,TDR_TYPE_FLOAT      ); // TDR_TAG_DECIMAL
    typeMap[21] = CPPTypeInfo("tdr_date_t"     ,"writeUInt32"   ,"readUInt32"  ,"imposiible"             ,TDR_TYPE_DATE       ); // TDR_TAG_DATE
    typeMap[22] = CPPTypeInfo("tdr_time_t"     ,"writeUInt32"   ,"readUInt32"  ,"imposiible"             ,TDR_TYPE_TIME       ); // TDR_TAG_TIME
    typeMap[23] = CPPTypeInfo("tdr_datetime_t" ,"writeUInt64"   ,"readUInt64"  ,"imposiible"             ,TDR_TYPE_DATETIME   ); // TDR_TAG_DATETIME
    typeMap[24] = CPPTypeInfo("char"           ,"impossible"    ,"impossible"  ,"\"%s\""                 ,TDR_TYPE_STRING     ); // TDR_TAG_STRING
    typeMap[25] = CPPTypeInfo("uint8_t"        ,"writeUInt8"    ,"readUInt8"   ,"\"%d\""                 ,TDR_TYPE_UCHAR      ); // TDR_TAG_BYTE
    typeMap[26] = CPPTypeInfo("tdr_ip_t"       ,"writeUInt32"   ,"readUInt32"  ,"imposiible"             ,TDR_TYPE_IP         ); // TDR_TAG_IP
    typeMap[27] = CPPTypeInfo("tdr_wchar_t"    ,"writeUInt16"   ,"readUInt16"  ,"\"0x%04x\""             ,TDR_TYPE_WCHAR      ); // TDR_TAG_WCHAR
    typeMap[28] = CPPTypeInfo("tdr_wchar_t"    ,"impossible"    ,"impossible"  ,"imposiible"             ,TDR_TYPE_WSTRING    ); // TDR_TAG_WSTRING
    typeMap[29] = CPPTypeInfo("void*"          ,"impossible"    ,"impossible"  ,"TDR_CPP_PTR_FORMAT"     ,TDR_TYPE_VOID       ); // TDR_TAG_VOID
    typeMap[30] = CPPTypeInfo("char"           ,"writeChar"     ,"readChar"    ,"\"0x%02x\""             ,TDR_TYPE_CHAR       ); // TDR_TAG_CHAR
    typeMap[31] = CPPTypeInfo("unsigned char"  ,"writeUChar"    ,"readUChar"   ,"\"0x%02x\""             ,TDR_TYPE_UCHAR      ); // TDR_TAG_UCHAR
    typeMap[32] = CPPTypeInfo("int16_t"        ,"writeInt16"    ,"readInt16"   ,"\"%d\""                 ,TDR_TYPE_SMALLINT   ); // TDR_TAG_SHORT
    typeMap[33] = CPPTypeInfo("uint16_t"       ,"writeUInt16"   ,"readUInt16"  ,"\"%d\""                 ,TDR_TYPE_SMALLUINT  ); // TDR_TAG_USHORT
    typeMap[34] = CPPTypeInfo("int32_t"        ,"writeInt32"    ,"readInt32"   ,"\"%d\""                 ,TDR_TYPE_LONG       ); // TDR_TAG_LONG
    typeMap[35] = CPPTypeInfo("uint32_t"       ,"writeUInt32"   ,"readUInt32"  ,"\"%u\""                 ,TDR_TYPE_ULONG      ); // TDR_TAG_ULONG
    typeMap[36] = CPPTypeInfo("int64_t"        ,"writeInt64"    ,"readInt64"   ,"TDR_CPP_INT64_FORMAT"   ,TDR_TYPE_LONGLONG   ); // TDR_TAG_LONGLONG
    typeMap[37] = CPPTypeInfo("uint64_t"       ,"writeUInt64"   ,"readUInt64"  ,"TDR_CPP_UINT64_FORMAT"  ,TDR_TYPE_ULONGLONG  ); // TDR_TAG_ULONGLONG
    typeMap[38] = CPPTypeInfo("int32_t"        ,"writeInt32"    ,"readInt32"   ,"\"%d\""                 ,TDR_TYPE_MONEY      ); // TDR_TAG_MONEY     .
}

const string&
TTransCPPBaseType::getType(unsigned int idxType)
{
    typeCheck(idxType);

    std::map<unsigned int, CPPTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.targetType;
}

const string&
TTransCPPBaseType::getEncFunc(unsigned int idxType)
{
    typeCheck(idxType);

    std::map<unsigned int, CPPTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.encodeFunction;
}

const string&
TTransCPPBaseType::getDecFunc(unsigned int idxType)
{
    typeCheck(idxType);

    std::map<unsigned int, CPPTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.decodeFunction;
}

const string&
TTransCPPBaseType::getPrintInfo(unsigned int idxType)
{
    typeCheck(idxType);

    std::map<unsigned int, CPPTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.printInfo;
}

bool
TTransCPPBaseType::has(unsigned int idxType)
{
    if (0 == idxType || 1 == idxType)
    {
        return false;
    }
    std::map<unsigned int, CPPTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        return false;
    return true;
}

void
TTransCPPBaseType::typeCheck(unsigned int idxType)
{
    if (idxType == 0)
        throw string("type=\"union\" is not basic type");
    if (idxType == 1)
        throw string("type=\"struct\" is not basic type");
}
