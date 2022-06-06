#ifndef TBUS_OPTION_H
#define TBUS_OPTION_H

class TbusOption
{
    public:
        enum TBUSOPERID
        {
            TBUS_OPER_UNDEFINE = 0,
            TBUS_OPER_GEN_CONF,
            TBUS_OPER_GEN_DESC,
        };
        enum TBUSCOMTYPE
        {
            TBUS_COM_TYPE_UNDEFINE = 0,
            TBUS_COM_TYPE_SINGLE,
            TBUS_COM_TYPE_MULTI,
        };

    public:
        TbusOption();
        int parse(int argc, char** argv);

    private:
        void help(const char* pszPathName);

    public:
        bool bNeedExit;

    public:
        // common options
        TBUSOPERID eOperID;
        TBUSCOMTYPE eComType;
        const char* pszDescFile;
        const char* pszOutDir;
};

#endif
