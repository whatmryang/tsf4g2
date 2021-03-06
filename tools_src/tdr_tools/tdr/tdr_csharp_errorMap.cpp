#include "tdr_csharp.h"

const char*
TTransCSharp::errCode[] = {
    "",
    "",
    "public class TdrError",
    "{"
    "    public enum ErrorType",
    "    {",
    "        TDR_NO_ERROR = 0,",
    "        TDR_ERR_SHORT_BUF_FOR_WRITE = -1,",
    "        TDR_ERR_SHORT_BUF_FOR_READ = -2,",
    "        TDR_ERR_STR_LEN_TOO_BIG = -3,",
    "        TDR_ERR_STR_LEN_TOO_SMALL = -4,",
    "        TDR_ERR_STR_LEN_CONFLICT = -5,",
    "        TDR_ERR_MINUS_REFER_VALUE = -6,",
    "        TDR_ERR_REFER_SURPASS_COUNT = -7,",
    "        TDR_ERR_ARG_IS_NULL = -8,",
    "        TDR_ERR_CUTVER_TOO_SMALL = -9,",
    "        TDR_ERR_CUTVER_CONFILICT = -10,",
    "        TDR_ERR_PARSE_TDRIP_FAILED = -11,",
    "        TDR_ERR_INVALID_TDRIP_VALUE = -12,",
    "        TDR_ERR_INVALID_TDRTIME_VALUE = -13,",
    "        TDR_ERR_INVALID_TDRDATE_VALUE = -14,",
    "        TDR_ERR_INVALID_TDRDATETIME_VALUE = -15,",
    "        TDR_ERR_FUNC_LOCALTIME_FAILED = -16,",
    "        TDR_ERR_INVALID_HEX_STR_LEN = -17,",
    "        TDR_ERR_INVALID_HEX_STR_FORMAT = -18,",
    "        TDR_ERR_INVALID_BUFFER_PARAMETER = -19,",
    "        TDR_ERR_NET_CUTVER_INVALID = -20,",
    "        TDR_ERR_ACCESS_VILOATION_EXCEPTION = -21,",
    "        TDR_ERR_ARGUMENT_NULL_EXCEPTION = -22,",
    "        TDR_ERR_USE_HAVE_NOT_INIT_VARIABLE_ARRAY = -23,",
    "        TDR_ERR_INVALID_FORMAT = -24,",
    "        TDR_ERR_HAVE_NOT_SET_SIZEINFO = -25,",
    "        TDR_ERR_VAR_STRING_LENGTH_CONFILICT = -26,",
    "    }",
    "",
    "    private static string[] errorTab = new string[]",
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
    "        /*19*/\"NULL array as parameter\",",
    "        /*20*/\"cutVer from net-msg not in [BASEVERSION, CURRVERSION]\",",
    "        /*21*/\"assess voliation exception cause by ptr is null , or bad formation\",",
    "        /*22*/\"argument null exception cause by arguments is null\",",
    "        /*23*/\"variable array have not alloc memory,you must alloc memory before use\",",
    "        /*24*/\"invalid string format cause FORMATEXCETPION\",",
    "        /*25*/\"the meta have not set sizeinfo attribute \",",
    "        /*26*/\"string/wstring length confilict with meta file\",",
    "    };",
    "",
    "    public static string getErrorString(ErrorType errorCode)",
    "    {",
    "        int errorIndex = -1 * (int)errorCode;",
    "        if (0 > errorIndex || errorIndex >= errorTab.GetLength(0))",
    "        {",
    "            return errorTab[0];",
    "        }else",
    "        {",
    "            return errorTab[errorIndex];",
    "        }",
    "    }",
    "}"
};



void
TTransCSharp::makeTdrErr()
{
    DEFINE_CSHARP_CONTAINER()
    APPEND_CSHARP_CONTAINER("");

    APPEND_CSHARP_CONTAINER("namespace tsf4g_tdr_csharp");
    APPEND_CSHARP_CONTAINER("{");
    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("");
    {
        loadCodeRes(0, Lines, errCode, sizeof(TTransCSharp::errCode)/sizeof(TTransCSharp::errCode[0]));
    }

    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("}");
    OUTPUT_CSHARP_CONTAINER_NOMETA("TdrError.cs");
}
