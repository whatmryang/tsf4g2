#include "tdr_as3.h"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

void
TTransAS3::makeErrorMap()
{
    list<string> lines;
    lines.push_back("package " + this->packageName + " {");
    {
        const string& indent = getIndentStr(1);
        lines.push_back(indent + "public class ErrorMap {");
        {
            makeErrorMap_i(2, lines);
        }
        lines.push_back(indent + "}");
    }
    lines.push_back("}");
    this->output(lines, string("ErrorMap.as"));
}

void
TTransAS3::makeErrorMap_i(unsigned int indentLevel, list<string>& lines)
{
    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "private static var errors:Array = new Array();");
    lines.push_back("");
    lines.push_back(indent + "public static const NO_ERROR:int                        =   0;");
    lines.push_back(indent + "public static const ERR_PARTIAL_MSG:int                 =  -1;");
    lines.push_back(indent + "public static const ERR_INVALIED_CUTOFF_VERSION:int     =  -2;");
    lines.push_back(indent + "public static const ERR_REFERENCE_IS_NULL:int           =  -3;");
    lines.push_back(indent + "public static const ERR_NO_BUF_FOR_SIZEINFO:int         =  -4;");
    lines.push_back(indent + "public static const ERR_NO_BUF_FOR_INDICATOR:int        =  -5;");
    lines.push_back(indent + "public static const ERR_STR_SURPASS_DEFINED_SIZE:int    =  -6;");
    lines.push_back(indent + "public static const ERR_ARRAY_REFER_IS_NULL:int         =  -7;");
    lines.push_back(indent + "public static const ERR_REFER_BIGGER_THAN_COUNT:int     =  -8;");
    lines.push_back(indent + "public static const ERR_ARRAY_ELEMENT_UNDEFIND:int      =  -9;");
    lines.push_back(indent + "public static const ERR_MINUS_REFER_VALUE:int           = -10;");
    lines.push_back("");
    lines.push_back(indent + "errors[NO_ERROR]                            = 'no error';");
    lines.push_back(indent + "errors[ERR_PARTIAL_MSG]                     = "
                    "'data in buffer do not cover a complete package';");
    lines.push_back(indent + "errors[ERR_INVALIED_CUTOFF_VERSION]         = "
                    "'version-indicator is invalid';");
    lines.push_back(indent + "errors[ERR_REFERENCE_IS_NULL]               = "
                    "'reference is null';");
    lines.push_back(indent + "errors[ERR_NO_BUF_FOR_SIZEINFO]             = "
                    "'refer to an invalid position, when set sizeinfo';");
    lines.push_back(indent + "errors[ERR_NO_BUF_FOR_INDICATOR]            = "
                    "'refer to an invalid position, when set version-indicator';");
    lines.push_back(indent + "errors[ERR_STR_SURPASS_DEFINED_SIZE]        = "
                    "'length of string surpass customed-size';");
    lines.push_back(indent + "errors[ERR_ARRAY_REFER_IS_NULL]             = "
                    "'array-type reference is null';");
    lines.push_back(indent + "errors[ERR_REFER_BIGGER_THAN_COUNT]         = "
                    "'value of refer is bigger than that of count';");
    lines.push_back(indent + "errors[ERR_ARRAY_ELEMENT_UNDEFIND]          = "
                    "'element of array is undefined';");
    lines.push_back(indent + "errors[ERR_MINUS_REFER_VALUE]               = "
                    "'value of refer is minus';");
    lines.push_back("");
    lines.push_back(indent + "public static function getErrorString(error:int):String");
    lines.push_back(indent + "{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var ret : * = ErrorMap.errors[error];");
        lines.push_back(indent + "if (ret == undefined) {");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            lines.push_back(indent + "return ErrorMap.errors[NO_ERROR];");
        }
        lines.push_back(indent + "}");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}
