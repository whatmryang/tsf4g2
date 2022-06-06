#include "tdr_cpp.h"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

const char*
TTransCPP::errHead[] = {
    "class TdrError",
    "{",
    "    public:",
    "        enum ErrorType",
    "        {",
    "            TDR_NO_ERROR = 0,",
    "            TDR_ERR_SHORT_BUF_FOR_WRITE        =  -1,",
    "            TDR_ERR_SHORT_BUF_FOR_READ         =  -2,",
    "            TDR_ERR_STR_LEN_TOO_BIG            =  -3,",
    "            TDR_ERR_STR_LEN_TOO_SMALL          =  -4,",
    "            TDR_ERR_STR_LEN_CONFLICT           =  -5,",
    "            TDR_ERR_MINUS_REFER_VALUE          =  -6,",
    "            TDR_ERR_REFER_SURPASS_COUNT        =  -7,",
    "            TDR_ERR_ARG_POINTER_IS_NULL        =  -8,",
    "            TDR_ERR_CUTVER_TOO_SMALL           =  -9,",
    "            TDR_ERR_CUTVER_CONFILICT           = -10,",
    "            TDR_ERR_PARSE_TDRIP_FAILED         = -11,",
    "            TDR_ERR_INVALID_TDRIP_VALUE        = -12,",
    "            TDR_ERR_INVALID_TDRTIME_VALUE      = -13,",
    "            TDR_ERR_INVALID_TDRDATE_VALUE      = -14,",
    "            TDR_ERR_INVALID_TDRDATETIME_VALUE  = -15,",
    "            TDR_ERR_FUNC_LOCALTIME_FAILED      = -16,",
    "            TDR_ERR_INVALID_HEX_STR_LEN        = -17,",
    "            TDR_ERR_INVALID_HEX_STR_FORMAT     = -18,",
    "            TDR_ERR_NULL_POINTER_PARAMETER     = -19,",
    "            TDR_ERR_NET_CUTVER_INVALID         = -20,",
    "        };",
    "",
    "    public:",
    "        static const char* getErrorString(ErrorType errorCode);",
    "};",
};

const char*
TTransCPP::errCpp[] = {
    "const char* TdrError::getErrorString(ErrorType errorCode)",
    "{",
    "    static const char* errorTab[] =",
    "    {",
    "        /* 0*/\"no error\",",
    "        /* 1*/\"available free space in buffer is not enough\",",
    "        /* 2*/\"available data in buffer is not enough\",",
    "        /* 3*/\"string length surpass defined size\",",
    "        /* 4*/\"string length smaller than min string length\",",
    "        /* 5*/\"string sizeinfo inconsistent with real length\",",
    "        /* 6*/\"reffer value can not be minus\",",
    "        /* 7*/\"reffer value bigger than count or size\",",
    "        /* 8*/\"pointer-type argument is NULL\",",
    "        /* 9*/\"cut-version is smaller than base-version\",",
    "        /*10*/\"cut-version not covers entry refered by versionindicator\",",
    "        /*11*/\"inet_ntoa failed when parse tdr_ip_t\",",
    "        /*12*/\"value variable of tdr_ip_t is invalid\",",
    "        /*13*/\"value variable of tdr_time_t is invalid\",",
    "        /*14*/\"value variable of tdr_date_t is invalid\",",
    "        /*15*/\"value variable of tdr_datetime_t is invalid\",",
    "        /*16*/\"function 'localtime' or 'localtime_r' failed\",",
    "        /*17*/\"invalid hex-string length, must be an even number\",",
    "        /*18*/\"invalid hex-string format, each character must be a hex-digit\",",
    "        /*19*/\"NULL pointer as parameter\",",
    "        /*20*/\"cutVer from net-msg not in [BASEVERSION, CURRVERSION]\",",
    "    };",
    "",
    "    int errorIndex = -1 * (int)errorCode;",
    "    if (0 > errorIndex || errorIndex >= (int)(sizeof(errorTab)/sizeof(errorTab[0])))",
    "    {",
    "        return errorTab[0];",
    "    } else",
    "    {",
    "        return errorTab[errorIndex];",
    "    }",
    "}",
};

void
TTransCPP::makeTdrErr()
{
    DEFINE_CONTAINER();

    APPEND_CONTAINER_HEAD("#ifndef _TDR_ERROR_H_");
    APPEND_CONTAINER_HEAD("#define _TDR_ERROR_H_");
    APPEND_CONTAINER_HEAD("");

    APPEND_CONTAINER_CPP("#include \"TdrError.h\"");
    APPEND_CONTAINER_CPP("");

    APPEND_CONTAINER("namespace tsf4g_tdr");
    APPEND_CONTAINER("{");
    APPEND_CONTAINER("");
    APPEND_CONTAINER("");
    {
        loadCodeRes(0, headLines, errHead, sizeof(TTransCPP::errHead)/sizeof(TTransCPP::errHead[0]));
        loadCodeRes(0, cppLines, errCpp, sizeof(TTransCPP::errCpp)/sizeof(TTransCPP::errCpp[0]));
    }
    APPEND_CONTAINER("");
    APPEND_CONTAINER("}");

    APPEND_CONTAINER_HEAD("#endif");

    OUTPUT_CONTAINER_NOMETA(string("TdrError.h"), string("TdrError.cpp"));
}
