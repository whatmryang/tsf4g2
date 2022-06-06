#include <ctime>
#include <cctype>
#include <string>
#include <cassert>
#include <iostream>
#include "tdr_csharp.h"
#include "tdr_debug.h"
#include "../tdr_tools.h"
#include "../../lib_src/tdr/tdr_md5.h"
#include "../../lib_src/tdr/tdr_metalib_kernel_i.h"


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::list;

void
TTransCSharp::makeMetaLibInfo()
{
    DEFINE_CSHARP_CONTAINER();

    APPEND_CSHARP_CONTAINER("");

    APPEND_CSHARP_CONTAINER("namespace " + this->packageName);

    APPEND_CSHARP_CONTAINER("{");
    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("public class MetaLib");
    APPEND_CSHARP_CONTAINER("{");
    {
        const unsigned int indentLevel = 1;
        {
            const string& indent = getIndentStr(indentLevel);

            APPEND_CSHARP_CONTAINER(indent + "public static int getVersion()");
            APPEND_CSHARP_CONTAINER(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 1);
                APPEND_CSHARP_CONTAINER(indent + "return " +
                                      int2str(tdr_get_metalib_version(pstMetaLib)) + ";");
            }
            APPEND_CSHARP_CONTAINER(indent + "}");
            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER(indent + "public static string getName()");
            APPEND_CSHARP_CONTAINER(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 1);
                APPEND_CSHARP_CONTAINER(indent + "return \"" + this->packageName + "\";");
            }
            APPEND_CSHARP_CONTAINER(indent + "}");
            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER(indent + "public static string getMd5Sum()");
            APPEND_CSHARP_CONTAINER(indent + "{");
            {
                unsigned char szMetalibHash[TDR_MD5_DIGEST_LENGTH];
                char szHash[TDR_MD5_DIGEST_LENGTH * 2 + 1] = {0};
                tdr_md5hash_buffer(szMetalibHash, (const unsigned char *)pstMetaLib,
                                   (unsigned int)pstMetaLib->iSize);
                tdr_md5hash2str(szMetalibHash, &szHash[0], sizeof(szHash));

                const string& indent = getIndentStr(indentLevel + 1);
                APPEND_CSHARP_CONTAINER(indent + "return \"" + string(szHash) + "\";");
            }
            APPEND_CSHARP_CONTAINER(indent + "}");
            APPEND_CSHARP_CONTAINER("");
            APPEND_CSHARP_CONTAINER(indent + "public static string getTdrVersion()");
            APPEND_CSHARP_CONTAINER(indent + "{");
            {
                string tdrVersion;
                tdr_get_version(tdrVersion);

                const string& indent = getIndentStr(indentLevel + 1);
                APPEND_CSHARP_CONTAINER(indent + "return \"" + tdrVersion + "\";");
            }
            APPEND_CSHARP_CONTAINER(indent + "}");
        }
    }
    APPEND_CSHARP_CONTAINER("}");
    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("}");
   
    string fileName;
    fileName = string(tdr_get_metalib_name(this->pstMetaLib)) + "_metalib.cs";

    OUTPUT_CSHARP_CONTAINER(fileName);
}

