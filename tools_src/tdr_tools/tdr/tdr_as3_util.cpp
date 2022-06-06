#include "tdr_as3.h"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

void
TTransAS3::makeTdrUtil()
{
    list<string> lines;
    lines.push_back("package " + this->packageName + " {");
    {
        const string& indent = getIndentStr(1);
        lines.push_back(indent + "import flash.utils.ByteArray;");
        lines.push_back("");
        lines.push_back(indent + "public class TdrUtil {");
        {
            makeTdrUtil_i(2, lines);
        }
        lines.push_back(indent + "}");
    }
    lines.push_back("}");
    this->output(lines, string("TdrUtil.as"));
}

void
TTransAS3::makeTdrUtil_i(unsigned int indentLevel, list<string>& lines)
{
    const string& indent = getIndentStr(indentLevel);
    lines.push_back(indent + "public static function directWriteInt (buf:ByteArray, position:uint, value:int):void {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "buf.writeInt(value);");
        lines.push_back(indent + "buf.position = pos;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directWriteUint (buf:ByteArray, position:uint, value:uint):void {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "buf.writeUnsignedInt(value);");
        lines.push_back(indent + "buf.position = pos;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directWriteSmallint (buf:ByteArray, position:uint, value:int):void {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "buf.writeShort(value);");
        lines.push_back(indent + "buf.position = pos;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directWriteSmalluint (buf:ByteArray, position:uint, value:int):void {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "buf.writeShort(value);");
        lines.push_back(indent + "buf.position = pos;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directWriteTinyint (buf:ByteArray, position:uint, value:uint):void {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "buf.writeByte(value);");
        lines.push_back(indent + "buf.position = pos;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directWriteTinyuint (buf:ByteArray, position:uint, value:uint):void {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "buf.writeByte(value);");
        lines.push_back(indent + "buf.position = pos;");
    }
    lines.push_back(indent + "}");
    lines.push_back(indent + "");
    lines.push_back("");
    lines.push_back(indent + "public static function directReadInt (buf:ByteArray, position:uint):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var ret:int;");
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "ret = buf.readInt();");
        lines.push_back(indent + "buf.position = pos;");
        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directReadUint (buf:ByteArray, position:uint):uint {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var ret:uint;");
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "ret = buf.readUnsignedInt();");
        lines.push_back(indent + "buf.position = pos;");
        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directReadSmallint (buf:ByteArray, position:uint):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var ret:int;");
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "ret = buf.readShort();");
        lines.push_back(indent + "buf.position = pos;");
        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directReadSmalluint (buf:ByteArray, position:uint):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var ret:int;");
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "ret = buf.readUnsignedShort();");
        lines.push_back(indent + "buf.position = pos;");
        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directReadTinyint (buf:ByteArray, position:uint):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var ret:int;");
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "ret = buf.readByte();");
        lines.push_back(indent + "buf.position = pos;");
        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
    lines.push_back("");
    lines.push_back(indent + "public static function directReadTinyuint (buf:ByteArray, position:uint):int {");
    {
        const string& indent = getIndentStr(indentLevel + 1);
        lines.push_back(indent + "var ret:int;");
        lines.push_back(indent + "var pos:uint = buf.position;");
        lines.push_back(indent + "buf.position = position;");
        lines.push_back(indent + "ret = buf.readUnsignedByte();");
        lines.push_back(indent + "buf.position = pos;");
        lines.push_back("");
        lines.push_back(indent + "return ret;");
    }
    lines.push_back(indent + "}");
}
