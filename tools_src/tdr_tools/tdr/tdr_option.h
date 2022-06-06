#ifndef TTRANS_OPTION_H
#define TTRANS_OPTION_H

#include "tdr/tdr.h"
#include <cstring>
#include <string>

#define TDR_MAX_FILE_LEN 1024
#define TDR_MAX_META_NAME_LEN 128
#define TDR_DEFAULT_INDENT_SIZE 4

using std::string;

class TdrOption
{
    public:
        // main-operation-type
        enum TDROPERID
        {
            TDR_OPER_UNDEFINE = 0,	        /**<未定义*/
            TDR_OPER_XML2DR,		        /**<转化成二进制库*/
            TDR_OPER_XML2C,			        /**<xml2c*/
            TDR_OPER_XML2H,			        /**<xml2h*/
            TDR_OPER_GEN_TMPLT,             /**<gen_xmltmplt*/
            TDR_OPER_XML2AS3,               /**<xml2as3*/
            TDR_OPER_EXPORT_PREFIX_FILE,    /**<export_prefix*/
            TDR_OPER_XML2CPP,               /**<xml2c++*/
            TDR_OPER_XML2CSHARP,
        };

    public:
        TdrOption();
        int parse(int argc, char** argv);

    private:
        void help(const char* pszApp);

    public:

    public:
        // common options
        bool needExit;
        bool needDumpLib;
        int iTagSetVersion;
        TDROPERID enID;
        const char* pszOutFile;
        const char* pszMetaName;
        char szOutFile[TDR_MAX_FILE_LEN];
        char szOutPath[TDR_MAX_FILE_LEN];
        unsigned char outputXmlFileFormat;
        int iVersion;
        TDRHPPRULE stRule;
        const char* pszPrefixFile;
        unsigned int uiIndentSize;
        char* pszEncoding;
        int iXMLFileNum;
        char *paszXMLFile[TDR_MAX_FILES_IN_ONE_PARSE];
        bool needCommFile;
        bool onlyCommFile;
        bool visualizeOn;
        bool unitedNameSpace;
};

#endif
