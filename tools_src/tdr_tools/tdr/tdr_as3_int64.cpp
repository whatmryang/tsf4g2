#include "tdr_as3.h"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

void
TTransAS3::makeInt64()
{
    list<string> lines;
    lines.push_back("package " + this->packageName + " {");
    {
        const string& indent = getIndentStr(1);
        lines.push_back(indent + "import flash.utils.ByteArray;");
        lines.push_back("");
        lines.push_back(indent + "public class Int64 {");
        {
            makeInt64_i(2, lines);
        }
        lines.push_back(indent + "}");
    }
    lines.push_back("}");
    this->output(lines, string("Int64.as"));
}

void
TTransAS3::makeInt64_i(unsigned int indentLevel, list<string>& lines)
{
    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "private var _high : int = 0;");
    lines.push_back(indent + "private var _low : uint = 0;");
    lines.push_back(indent + "private var _strValue : String = \"0\";");
    lines.push_back(indent + "public static const size:uint = 8;");
    lines.push_back("");
    lines.push_back(indent + "public static const COMPARE_RESULT_UNKOWN  : int     = -2;");
    lines.push_back(indent + "public static const COMPARE_RESULT_SMALLER : int     = -1;");
    lines.push_back(indent + "public static const COMPARE_RESULT_EQUAL   : int     =  0;");
    lines.push_back(indent + "public static const COMPARE_RESULT_LARGER  : int     =  1;");
    lines.push_back("");
    lines.push_back(indent + "private static const INT32 : Number = 4294967296;");
    lines.push_back(indent + "private static const INT32_REMAINDER : uint = 7296;");
    lines.push_back("");
    lines.push_back(indent + "private static const PARAM : uint = 10000;");

    lines.push_back("");

    lines.push_back(indent + "public function Int64(high:int = 0, low:uint = 0) {");
    lines.push_back(indent + "    setValue(high, low);");
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function decode(buf:ByteArray):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "if (buf.bytesAvailable < Int64.size) {");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            lines.push_back(indent + "return ErrorMap.ERR_PARTIAL_MSG;");
        }
        lines.push_back(indent + "}");
        lines.push_back(indent + "var high:int = buf.readInt();");
        lines.push_back(indent + "var low:uint = buf.readUnsignedInt();");
        lines.push_back(indent + "setValue(high, low);");
        lines.push_back("");
        lines.push_back(indent + "return 0;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function encode(buf:ByteArray):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "buf.writeInt(_high);");
        lines.push_back(indent + "buf.writeUnsignedInt(_low);");
        lines.push_back("");
        lines.push_back(indent + "return 0;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function getSize():uint{");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return Int64.size;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function get high():int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return _high;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function get low():uint {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return _low;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function get string():String {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return _strValue;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function toString():String {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "return _strValue;");
    }
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function setValue(high:int, low:uint):void {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "_high = high;");
        lines.push_back(indent + "_low = low;");
        lines.push_back(indent + "_strValue = (high * INT32 + low).toString();");
        lines.push_back("");
        lines.push_back(indent + "var len : uint = _strValue.length;");
        lines.push_back(indent + "if (len > 16) {");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            lines.push_back(indent + "var strTemp : String = "
                            "(Math.abs((high % PARAM) * INT32_REMAINDER + "
                            "(low % PARAM)) % PARAM).toString();;");
            lines.push_back(indent + "while (strTemp.length < 4) {");
            {
                const string& indent = getIndentStr(indentLevel + 3);
                lines.push_back(indent + "strTemp = '0' + strTemp;");
            }
            lines.push_back(indent + "}");
            lines.push_back(indent + "_strValue = _strValue.substr(0, len - 4) + strTemp;");
        }
        lines.push_back(indent + "}");
    }
    lines.push_back(indent + "}");

    lines.push_back("");

    lines.push_back(indent + "public function compare(nAnother:Int64):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "if (nAnother == null) {");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            lines.push_back(indent + "return COMPARE_RESULT_UNKOWN;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
        lines.push_back(indent + "if (_high < nAnother.high) {");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            lines.push_back(indent + "return COMPARE_RESULT_SMALLER;");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
        lines.push_back(indent + "if (_high == nAnother.high) {");
        {
            const string& indent = getIndentStr(indentLevel + 2);
            lines.push_back(indent + "if (_low < nAnother.low) {");
            {
                const string& indent = getIndentStr(indentLevel + 3);
                lines.push_back(indent + "return COMPARE_RESULT_SMALLER;");
            }
            lines.push_back(indent + "}");
            lines.push_back(indent + "else if (_low == nAnother.low) {");
            {
                const string& indent = getIndentStr(indentLevel + 3);
                lines.push_back(indent + "return COMPARE_RESULT_EQUAL;");
            }
            lines.push_back(indent + "}");
            lines.push_back(indent + "else {");
            {
                const string& indent = getIndentStr(indentLevel + 3);
                lines.push_back(indent + "return COMPARE_RESULT_LARGER;");
            }
            lines.push_back(indent + "}");
        }
        lines.push_back(indent + "}");
        lines.push_back("");
        lines.push_back(indent + "return COMPARE_RESULT_LARGER;");
    }
    lines.push_back(indent + "}");
}
