#include <ctime>
#include <cctype>
#include <string>
#include <cassert>
#include <iostream>
#include "tdr_cpp.h"
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
TTransCPP::makeMetaLibInfo()
{
    DEFINE_CONTAINER_HEAD();

    APPEND_CONTAINER_HEAD("#ifndef _" + this->packageName + "_metalib_h_");
    APPEND_CONTAINER_HEAD("#define _" + this->packageName + "_metalib_h_");
    APPEND_CONTAINER_HEAD("");

    APPEND_CONTAINER_HEAD("namespace " + this->packageName);

    APPEND_CONTAINER_HEAD("{");
    APPEND_CONTAINER_HEAD("");
    APPEND_CONTAINER_HEAD("");
    APPEND_CONTAINER_HEAD("class MetaLib");
    APPEND_CONTAINER_HEAD("{");
    {
        const unsigned int indentLevel = 1;
        const string& indent = getIndentStr(indentLevel);

        APPEND_CONTAINER_HEAD(indent + "public:");
        {
            const string& indent = getIndentStr(indentLevel + 1);

            APPEND_CONTAINER_HEAD(indent + "static int getVersion()");
            APPEND_CONTAINER_HEAD(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                APPEND_CONTAINER_HEAD(indent + "return " +
                                      int2str(tdr_get_metalib_version(pstMetaLib)) + ";");
            }
            APPEND_CONTAINER_HEAD(indent + "}");
            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD(indent + "static const char* getName()");
            APPEND_CONTAINER_HEAD(indent + "{");
            {
                const string& indent = getIndentStr(indentLevel + 2);
                APPEND_CONTAINER_HEAD(indent + "return \"" + this->packageName + "\";");
            }
            APPEND_CONTAINER_HEAD(indent + "}");
            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD(indent + "static const char* getMd5Sum()");
            APPEND_CONTAINER_HEAD(indent + "{");
            {
                unsigned char szMetalibHash[TDR_MD5_DIGEST_LENGTH];
                char szHash[TDR_MD5_DIGEST_LENGTH * 2 + 1] = {0};
                tdr_md5hash_buffer(szMetalibHash, (const unsigned char *)pstMetaLib,
                                   (unsigned int)pstMetaLib->iSize);
                tdr_md5hash2str(szMetalibHash, &szHash[0], sizeof(szHash));

                const string& indent = getIndentStr(indentLevel + 2);
                APPEND_CONTAINER_HEAD(indent + "return \"" + string(szHash) + "\";");
            }
            APPEND_CONTAINER_HEAD(indent + "}");
            APPEND_CONTAINER_HEAD("");
            APPEND_CONTAINER_HEAD(indent + "static const char* getTdrVersion()");
            APPEND_CONTAINER_HEAD(indent + "{");
            {
                string tdrVersion;
                tdr_get_version(tdrVersion);

                const string& indent = getIndentStr(indentLevel + 2);
                APPEND_CONTAINER_HEAD(indent + "return \"" + tdrVersion + "\";");
            }
            APPEND_CONTAINER_HEAD(indent + "}");
        }
    }
    APPEND_CONTAINER_HEAD("};");
    APPEND_CONTAINER_HEAD("");
    APPEND_CONTAINER_HEAD("");
    APPEND_CONTAINER_HEAD("}");
    APPEND_CONTAINER_HEAD("#endif");


    string fileName;
    fileName = string(tdr_get_metalib_name(this->pstMetaLib)) + "_metalib.h";

    OUTPUT_CONTAINER_HEAD(fileName);
}
