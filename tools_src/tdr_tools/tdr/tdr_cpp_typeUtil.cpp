#include "tdr_cpp.h"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

const char*
TTransCPP::typeUtilHead[] = {
    "class TdrTypeUtil",
    "{",
    "    public:",
    "        static TdrError::ErrorType str2TdrIP(tdr_ip_t& dest, const char *pszIP);",
    "        static TdrError::ErrorType str2TdrTime(tdr_time_t& dest, const char *pszTime);",
    "        static TdrError::ErrorType str2TdrDate(tdr_date_t& dest, const char *pszDate);",
    "        static TdrError::ErrorType str2TdrDateTime(tdr_datetime_t& dest, const char *pszDateTime);",
    "        static TdrError::ErrorType UTC2TdrDateTime(tdr_datetime_t& dest, time_t utcTime);",
    "",
    "",
    "        static TdrError::ErrorType tdrIp2Str(TdrWriteBuf& dest, tdr_ip_t src);",
    "",
    "        static TdrError::ErrorType tdrTime2Str(TdrWriteBuf& dest, tdr_time_t src)",
    "        {",
    "            return TdrTime(src).print2Buf(dest);",
    "        }",
    "",
    "        static TdrError::ErrorType tdrDate2Str(TdrWriteBuf& dest, tdr_date_t src)",
    "        {",
    "            return TdrDate(src).print2Buf(dest);",
    "        }",
    "",
    "        static TdrError::ErrorType tdrDateTime2Str(TdrWriteBuf& dest, tdr_datetime_t src)",
    "        {",
    "            return TdrDateTime(src).print2Buf(dest);",
    "        }",
    "",
    "        static TdrError::ErrorType tdrDateTime2UTC(time_t& dest, tdr_datetime_t src);",
    "",
    "    public:",
    "        static int compareTdrTime(tdr_time_t left, tdr_time_t right);",
    "        static int compareTdrDate(tdr_date_t left, tdr_date_t right);",
    "        static int compareTdrDateTime(tdr_datetime_t left, tdr_datetime_t right);",
    "",
    "    public:",
    "        static size_t wstrlen(const tdr_wchar_t* wstr);",
    "};",
};

const char*
TTransCPP::typeUtilCpp[] = {
    "TdrError::ErrorType",
    "TdrTypeUtil::tdrIp2Str(TdrWriteBuf& dest, tdr_ip_t src)",
    "{",
    "    struct in_addr addr;",
    "",
    "    memset(&addr, 0, sizeof(addr));",
    "    addr.s_addr = src;",
    "",
    "#if defined(_WIN32) || defined(_WIN64)",
    "    return dest.textize(\"%u.%u.%u.%u\", addr.s_net, addr.s_host, addr.s_lh, addr.s_impno);",
    "#else",
    "    char dotIP[32];",
    "    if (NULL == inet_ntop(AF_INET, &addr, dotIP, sizeof(dotIP)))",
    "    {",
    "        return TdrError::TDR_ERR_SHORT_BUF_FOR_WRITE;",
    "    }",
    "    dotIP[sizeof(dotIP)-1] = '\\0';",
    "    return dest.textize(\"%s\", dotIP);",
    "#endif",
    "}",
    "",
    "TdrError::ErrorType",
    "TdrTypeUtil::UTC2TdrDateTime(tdr_datetime_t& dest, time_t utcTime)",
    "{",
    "    struct tm* pstTm;",
    "",
    "#if defined(_WIN32) || defined(_WIN64)",
    "    pstTm = localtime(&utcTime);",
    "#else",
    "    struct tm stTm;",
    "    memset(&stTm, 0, sizeof(stTm));",
    "    pstTm = localtime_r(&utcTime, &stTm);",
    "#endif",
    "",
    "    if (NULL == pstTm)",
    "    {",
    "        return TdrError::TDR_ERR_FUNC_LOCALTIME_FAILED;",
    "    }",
    "",
    "    TdrDateTime tdrDateTime(*pstTm);",
    "    if (!tdrDateTime.isValid())",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRDATETIME_VALUE;",
    "    }",
    "",
    "    dest = tdrDateTime;",
    "",
    "    return TdrError::TDR_NO_ERROR;",
    "}",
    "",
    "TdrError::ErrorType",
    "TdrTypeUtil::str2TdrIP(tdr_ip_t& dest, const char *pszIP)",
    "{",
    "    assert(NULL != pszIP);",
    "",
    "    struct in_addr addr;",
    "",
    "#if defined(_WIN32) || defined(_WIN64)",
    "    addr.s_addr = inet_addr(pszIP);",
    "    if (INADDR_NONE == addr.s_addr)",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRIP_VALUE;",
    "    }",
    "#else",
    "    if (0 == inet_aton(pszIP, &addr))",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRIP_VALUE;",
    "    }",
    "#endif",
    "",
    "    dest = addr.s_addr;",
    "",
    "    return TdrError::TDR_NO_ERROR;",
    "}",
    "",
    "TdrError::ErrorType",
    "TdrTypeUtil::str2TdrTime(tdr_time_t& dest, const char *pszTime)",
    "{",
    "    assert(NULL != pszTime);",
    "",
    "    struct tm stTm;",
    "    memset(&stTm, 0, sizeof(stTm));",
    "    if (NULL == tsf4g_tdr_third_party::strptime(pszTime, \"%H:%M:%S\", &stTm))",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRTIME_VALUE;",
    "    }",
    "",
    "    TdrTime tdrTime(stTm);",
    "    if (!tdrTime.isValid())",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRTIME_VALUE;",
    "    }",
    "",
    "    dest = tdrTime;",
    "",
    "    return TdrError::TDR_NO_ERROR;",
    "}",
    "",
    "TdrError::ErrorType",
    "TdrTypeUtil::str2TdrDate(tdr_date_t& dest, const char *pszDate)",
    "{",
    "    assert(NULL != pszDate);",
    "",
    "    struct tm stTm;",
    "    memset(&stTm, 0, sizeof(stTm));",
    "    if (NULL == tsf4g_tdr_third_party::strptime(pszDate, \"%Y-%m-%d\", &stTm))",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRDATE_VALUE;",
    "    }",
    "",
    "    TdrDate tdrDate(stTm);",
    "    if (!tdrDate.isValid())",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRDATE_VALUE;",
    "    }",
    "",
    "    dest = tdrDate;",
    "",
    "    return TdrError::TDR_NO_ERROR;",
    "}",
    "",
    "TdrError::ErrorType",
    "TdrTypeUtil::str2TdrDateTime(tdr_datetime_t& dest, const char *pszDateTime)",
    "{",
    "    assert(NULL != pszDateTime);",
    "",
    "    struct tm stTm;",
    "    memset(&stTm, 0, sizeof(stTm));",
    "    if (NULL == tsf4g_tdr_third_party::strptime(pszDateTime, \"%Y-%m-%d %H:%M:%S\", &stTm))",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRDATETIME_VALUE;",
    "    }",
    "",
    "    TdrDateTime tdrDateTime(stTm);",
    "    if (!tdrDateTime.isValid())",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRDATETIME_VALUE;",
    "    }",
    "",
    "    dest = tdrDateTime;",
    "",
    "    return TdrError::TDR_NO_ERROR;",
    "}",
    "",
    "TdrError::ErrorType",
    "TdrTypeUtil::tdrDateTime2UTC(time_t& dest, tdr_datetime_t src)",
    "{",
    "    struct tm stTm;",
    "    memset(&stTm, 0, sizeof(stTm));",
    "",
    "    TdrDateTime tdrDateTime(src);",
    "    if (!tdrDateTime.isValid())",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRDATETIME_VALUE;",
    "    }",
    "",
    "    tdrDateTime.toTm(stTm);",
    "    time_t utcTime = mktime(&stTm);",
    "    if (-1 == utcTime)",
    "    {",
    "        return TdrError::TDR_ERR_INVALID_TDRDATETIME_VALUE;",
    "    } else",
    "    {",
    "        dest = utcTime;",
    "    }",
    "",
    "    return TdrError::TDR_NO_ERROR;",
    "}",
    "",
    "int TdrTypeUtil::compareTdrTime(tdr_time_t left, tdr_time_t right)",
    "{",
    "    if (TdrTime(left) < TdrTime(right))",
    "    {",
    "        return -1;",
    "    } else if (TdrTime(right) < TdrTime(left))",
    "    {",
    "        return 1;",
    "    } else",
    "    {",
    "        return 0;",
    "    }",
    "}",
    "",
    "int TdrTypeUtil::compareTdrDate(tdr_date_t left, tdr_date_t right)",
    "{",
    "    if (TdrDate(left) < TdrDate(right))",
    "    {",
    "        return -1;",
    "    } else if (TdrDate(right) < TdrDate(left))",
    "    {",
    "        return 1;",
    "    } else",
    "    {",
    "        return 0;",
    "    }",
    "}",
    "",
    "int TdrTypeUtil::compareTdrDateTime(tdr_datetime_t left, tdr_datetime_t right)",
    "{",
    "    if (TdrDateTime(left) < TdrDateTime(right))",
    "    {",
    "        return -1;",
    "    } else if (TdrDateTime(right) < TdrDateTime(left))",
    "    {",
    "        return 1;",
    "    } else",
    "    {",
    "        return 0;",
    "    }",
    "}",
    "",
    "size_t TdrTypeUtil::wstrlen(const tdr_wchar_t* wstr)",
    "{",
    "    assert(NULL != wstr);",
    "",
    "    size_t len = 0;",
    "    while (true)",
    "    {",
    "        if (*wstr == (tdr_wchar_t)0)",
    "        {",
    "            break;",
    "        }",
    "        wstr++;",
    "        len++;",
    "    }",
    "",
    "    return len;",
    "}",
};

void
TTransCPP::makeTdrTypeUtil()
{
    DEFINE_CONTAINER();

    APPEND_CONTAINER_HEAD("#ifndef _TDR_TYPE_H_");
    APPEND_CONTAINER_HEAD("#define _TDR_TYPE_H_");
    APPEND_CONTAINER_HEAD("");
    APPEND_CONTAINER_HEAD("#include \"TdrPal.h\"");
    APPEND_CONTAINER_HEAD("#include \"TdrBuf.h\"");
    APPEND_CONTAINER_HEAD("#include \"TdrError.h\"");
    APPEND_CONTAINER_HEAD("#include \"TdrTime.h\"");
    APPEND_CONTAINER_HEAD("");

    APPEND_CONTAINER_CPP("");
    APPEND_CONTAINER_CPP("#include <cassert>");
    APPEND_CONTAINER_CPP("#include <cstring>");
    APPEND_CONTAINER_CPP("");
    APPEND_CONTAINER_CPP("#include \"TdrTypeUtil.h\"");
    APPEND_CONTAINER_CPP("#include \"TdrThirdParty.h\"");
    APPEND_CONTAINER_CPP("");

    APPEND_CONTAINER("namespace tsf4g_tdr");
    APPEND_CONTAINER("{");
    APPEND_CONTAINER("");
    APPEND_CONTAINER("");
    {
        loadCodeRes(0, headLines, typeUtilHead, sizeof(TTransCPP::typeUtilHead)/sizeof(TTransCPP::typeUtilHead[0]));
        loadCodeRes(0, cppLines, typeUtilCpp, sizeof(TTransCPP::typeUtilCpp)/sizeof(TTransCPP::typeUtilCpp[0]));
    }
    APPEND_CONTAINER("");
    APPEND_CONTAINER("}");
    APPEND_CONTAINER_HEAD("#endif");

    OUTPUT_CONTAINER_NOMETA(string("TdrTypeUtil.h"), string("TdrTypeUtil.cpp"));
}
