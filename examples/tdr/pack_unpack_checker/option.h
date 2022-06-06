#ifndef CHECKER_OPTION_H
#define CHECKER_OPTION_H

#include <string>

#define MAX_FILE_LEN 1024
#define MAX_META_NAME_LEN 128
#define DEFAULT_INDENT_SIZE 4

using std::string;

class Option
{
    public:
        // main-operation-type
        enum OPERID
        {
            OPER_UNDEFINED = 0,	        /**<未定义*/
            OPER_HTON,			        /**<pack*/
            OPER_NTOH,			        /**<unpack*/
        };

        // source-data-type
        enum DATATYPE
        {
            TYPE_UNDEFINED = 0,	        /**<未定义*/
            TYPE_HEX_STR,               /**<hex-str*/
            TYPE_BINARY,                /**<binary*/
        };

    public:
        Option();
        int parse(int argc, char** argv);

    private:
        int check();
        void help(const char* pszApp);

    public:
        OPERID enID;
        DATATYPE srcDataType;
        const char* srcDataFile;
};

#endif
