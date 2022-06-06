#include "tdr_csharp_baseType.h"
#include "tdr/tdr.h"
#include <iterator>

TTransCSharpBaseType::TTransCSharpBaseType()
{
    typeMap[0]  = CSharpTypeInfo("union"          ,"impossible"  ,"impossible"    ,"impossible"  ,"impossible"             ,TDR_TYPE_UNION      ); // TDR_TAG_UNION
    typeMap[1]  = CSharpTypeInfo("struct"         ,"impossible"  ,"impossible"    ,"impossible"  ,"impossible"             ,TDR_TYPE_STRUCT     ); // TDR_TAG_STRUCT
    typeMap[2]  = CSharpTypeInfo("int8_t"         ,"sbyte"       ,"writeInt8"     ,"readInt8"    ,"\"0x{0:x2}\""           ,TDR_TYPE_CHAR       ); // TDR_TAG_TINYINT
    typeMap[3]  = CSharpTypeInfo("uint8_t"        ,"byte"        ,"writeUInt8"    ,"readUInt8"   ,"\"0x{0:x2}\""           ,TDR_TYPE_UCHAR      ); // TDR_TAG_TINYUINT
    typeMap[4]  = CSharpTypeInfo("int16_t"        ,"Int16"       ,"writeInt16"    ,"readInt16"   ,"\"{0:d}\""              ,TDR_TYPE_SMALLINT   ); // TDR_TAG_SMALLINT
    typeMap[5]  = CSharpTypeInfo("uint16_t"       ,"UInt16"      ,"writeUInt16"   ,"readUInt16"  ,"\"{0:d}\""              ,TDR_TYPE_SMALLUINT  ); // TDR_TAG_SMALLUINT
    typeMap[6]  = CSharpTypeInfo("int32_t"        ,"Int32"       ,"writeInt32"    ,"readInt32"   ,"\"{0:d}\""              ,TDR_TYPE_INT        ); // TDR_TAG_INT
    typeMap[7]  = CSharpTypeInfo("uint32_t"       ,"UInt32"      ,"writeUInt32"   ,"readUInt32"  ,"\"{0:d}\""              ,TDR_TYPE_UINT       ); // TDR_TAG_UINT
    typeMap[8]  = CSharpTypeInfo("int64_t"        ,"Int64"       ,"writeInt64"    ,"readInt64"   ,"\"{0:d}\""              ,TDR_TYPE_LONGLONG   ); // TDR_TAG_BIGINT
    typeMap[9]  = CSharpTypeInfo("uint64_t"       ,"UInt64"      ,"writeUInt64"   ,"readUInt64"  ,"\"{0:d}\""              ,TDR_TYPE_ULONGLONG  ); // TDR_TAG_BIGUINT
    typeMap[10] = CSharpTypeInfo("int8_t"         ,"sbyte"       ,"writeInt8"     ,"readInt8"    ,"\"0x{0:x2}\""           ,TDR_TYPE_CHAR       ); // TDR_TAG_INT8
    typeMap[11] = CSharpTypeInfo("uint8_t"        ,"byte"        ,"writeUInt8"    ,"readUInt8"   ,"\"0x{0:x2}\""           ,TDR_TYPE_UCHAR      ); // TDR_TAG_UINT8
    typeMap[12] = CSharpTypeInfo("int16_t"        ,"Int16"       ,"writeInt16"    ,"readInt16"   ,"\"{0:d}\""              ,TDR_TYPE_SMALLINT   ); // TDR_TAG_INT16
    typeMap[13] = CSharpTypeInfo("uint16_t"       ,"UInt16"      ,"writeUInt16"   ,"readUInt16"  ,"\"{0:d}\""              ,TDR_TYPE_SMALLUINT  ); // TDR_TAG_UINT16
    typeMap[14] = CSharpTypeInfo("int32_t"        ,"Int32"       ,"writeInt32"    ,"readInt32"   ,"\"{0:d}\""              ,TDR_TYPE_INT        ); // TDR_TAG_INT32
    typeMap[15] = CSharpTypeInfo("uint32_t"       ,"UInt32"      ,"writeUInt32"   ,"readUInt32"  ,"\"{0:d}\""              ,TDR_TYPE_UINT       ); // TDR_TAG_UINT32
    typeMap[16] = CSharpTypeInfo("int64_t"        ,"Int64"       ,"writeInt64"    ,"readInt64"   ,"\"{0:d}\""              ,TDR_TYPE_LONGLONG   ); // TDR_TAG_INT64
    typeMap[17] = CSharpTypeInfo("uint64_t"       ,"UInt64"      ,"writeUInt64"   ,"readUInt64"  ,"\"{0:d}\""              ,TDR_TYPE_ULONGLONG  ); // TDR_TAG_UINT64
    typeMap[18] = CSharpTypeInfo("float"          ,"float"       ,"writeFloat"    ,"readFloat"   ,"\"{0:g}\""              ,TDR_TYPE_FLOAT      ); // TDR_TAG_FLOAT
    typeMap[19] = CSharpTypeInfo("double"         ,"double"      ,"writeDouble"   ,"readDouble"  ,"\"{0:g}\""              ,TDR_TYPE_DOUBLE     ); // TDR_TAG_DOUBLE
    typeMap[20] = CSharpTypeInfo("float"          ,"float"       ,"writeFloat"    ,"readFloat"   ,"\"{0:g}\""              ,TDR_TYPE_FLOAT      ); // TDR_TAG_DECIMAL
    typeMap[21] = CSharpTypeInfo("tdr_date_t"     ,"UInt32"      ,"writeUInt32"   ,"readUInt32"  ,"imposiible"             ,TDR_TYPE_DATE       ); // TDR_TAG_DATE
    typeMap[22] = CSharpTypeInfo("tdr_time_t"     ,"UInt32"      ,"writeUInt32"   ,"readUInt32"  ,"imposiible"             ,TDR_TYPE_TIME       ); // TDR_TAG_TIME
    typeMap[23] = CSharpTypeInfo("tdr_datetime_t" ,"UInt64"      ,"writeUInt64"   ,"readUInt64"  ,"imposiible"             ,TDR_TYPE_DATETIME   ); // TDR_TAG_DATETIME
    typeMap[24] = CSharpTypeInfo("char"           ,"byte"        ,"writeCString"  ,"readCString" ,"\"{0}\""                ,TDR_TYPE_STRING     ); // TDR_TAG_STRING
    typeMap[25] = CSharpTypeInfo("uint8_t"        ,"byte"        ,"writeUInt8"    ,"readUInt8"   ,"\"{0:d}\""              ,TDR_TYPE_UCHAR      ); // TDR_TAG_BYTE
    typeMap[26] = CSharpTypeInfo("tdr_ip_t"       ,"UInt32"      ,"writeUInt32"   ,"readUInt32"  ,"imposiible"             ,TDR_TYPE_IP         ); // TDR_TAG_IP
    typeMap[27] = CSharpTypeInfo("tdr_wchar_t"    ,"Int16"       ,"writeInt16"    ,"readInt16"   , "\"0x{0:x4}\""          ,TDR_TYPE_WCHAR      ); // TDR_TAG_WCHAR
    typeMap[28] = CSharpTypeInfo("tdr_wchar_t"    ,"Int16"       ,"writeWString"  ,"readWString" ,"\"{0}\""                ,TDR_TYPE_WSTRING    ); // TDR_TAG_WSTRING
    typeMap[29] = CSharpTypeInfo("void*"          ,"impossible"  ,"impossible"    ,"impossible"  ,"\"{0}\""                ,TDR_TYPE_VOID       ); // TDR_TAG_VOID
    typeMap[30] = CSharpTypeInfo("char"           ,"sbyte"       ,"writeInt8"     ,"readInt8"    ,"\"0x{0:x2}\""           ,TDR_TYPE_CHAR       ); // TDR_TAG_CHAR
    typeMap[31] = CSharpTypeInfo("unsigned char"  ,"byte"        ,"writeUInt8"    ,"readUInt8"   ,"\"0x{0:x2}\""           ,TDR_TYPE_UCHAR      ); // TDR_TAG_UCHAR
    typeMap[32] = CSharpTypeInfo("int16_t"        ,"Int16"       ,"writeInt16"    ,"readInt16"   ,"\"{0:d}\""              ,TDR_TYPE_SMALLINT   ); // TDR_TAG_SHORT
    typeMap[33] = CSharpTypeInfo("uint16_t"       ,"UInt16"      ,"writeUInt16"   ,"readUInt16"  ,"\"{0:d}\""              ,TDR_TYPE_SMALLUINT  ); // TDR_TAG_USHORT
    typeMap[34] = CSharpTypeInfo("int32_t"        ,"Int32"       ,"writeInt32"    ,"readInt32"   ,"\"{0:d}\""              ,TDR_TYPE_LONG       ); // TDR_TAG_LONG
    typeMap[35] = CSharpTypeInfo("uint32_t"       ,"UInt32"      ,"writeUInt32"   ,"readUInt32"  ,"\"{0:d}\""              ,TDR_TYPE_ULONG      ); // TDR_TAG_ULONG
    typeMap[36] = CSharpTypeInfo("int64_t"        ,"Int64"       ,"writeInt64"    ,"readInt64"   ,"\"{0:d}\""              ,TDR_TYPE_LONGLONG   ); // TDR_TAG_LONGLONG
    typeMap[37] = CSharpTypeInfo("uint64_t"       ,"UInt64"      ,"writeUInt64"   ,"readUInt64"  ,"\"{0:d}\""              ,TDR_TYPE_ULONGLONG  ); // TDR_TAG_ULONGLONG
    typeMap[38] = CSharpTypeInfo("int32_t"        ,"Int32"       ,"writeInt32"    ,"readInt32"   ,"\"{0:d}\""              ,TDR_TYPE_MONEY      ); // TDR_TAG_MONEY     .int iType = 0

}



const string&
TTransCSharpBaseType::getCType(unsigned int idxType)
{
    typeCheck(idxType);

	std::map<unsigned int, CSharpTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.ctype;
}

const string&
TTransCSharpBaseType::getType(unsigned int idxType)
{
    typeCheck(idxType);

	std::map<unsigned int, CSharpTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.targetType;
}

const string&
TTransCSharpBaseType::getEncFunc(unsigned int idxType)
{
    typeCheck(idxType);

	std::map<unsigned int, CSharpTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.encodeFunction;
}

const string&
TTransCSharpBaseType::getDecFunc(unsigned int idxType)
{
    typeCheck(idxType);

	std::map<unsigned int, CSharpTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.decodeFunction;
}


bool
TTransCSharpBaseType::has(unsigned int idxType)
{
    if (0 == idxType || 1 == idxType)
    {
        return false;
    }
    
    typeCheck(idxType);
	std::map<unsigned int, CSharpTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        return false;
    return true;
}

void TTransCSharpBaseType::typeCheck(unsigned int idxType)
{

    //if (idxType == TDR_TYPE_VOID)
    //    throw string("type=\"void*\" is not supported for c#");
    if (idxType == 0)
        throw string("type=\"union\" is not basic type");
    if (idxType == 1)
        throw string("type=\"struct\" is not basic type");
}

const string& TTransCSharpBaseType::getPrintInfo(unsigned int idxType)
{
    typeCheck(idxType);

    std::map<unsigned int, CSharpTypeInfo>::iterator iter;
    iter = typeMap.find(idxType);
    if (iter == typeMap.end())
        throw string("unexpected tdr-base-type meeted, this is tdr-lib error");

    return iter->second.printInfo;
}

