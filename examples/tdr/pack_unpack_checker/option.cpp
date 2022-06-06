#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "option.h"
#include "getopt.h"

#if defined(_WIN32) || defined(_WIN64)
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#define OPT_ONLY_COMM_FILE 0x101
#define OPT_NO_COMM_FILE   0x102

using std::cout;
using std::endl;
using std::string;

Option::Option()
{
    this->enID = OPER_UNDEFINED;
    this->srcDataType = TYPE_HEX_STR;
    this->srcDataFile = NULL;
}

int
Option::parse(int argc, char** argv)
{
    assert(0 != argc);
    assert(NULL != argv);
    assert(NULL != *argv);

    int opt;
    static struct option s_astLongOptions[] = {
        {"hton", 0, NULL, 'P'},
        {"ntoh", 0, NULL, 'U'},
        {"binary", 0, NULL, 'b'},
        {"data-file", 1, NULL, 'f'},
        {"help", 0, 0, 'h'},

        {0, 0, 0, 0}
    };

    while (true)
    {
        int option_index = 0;

        opt = getopt_long (argc, argv, "PUb:f:h",
                           s_astLongOptions, &option_index);
        if (opt == -1)
            break;

        switch(opt)
        {
            case 'P':
                this->enID = OPER_HTON;
                break;
            case 'U':
                this->enID = OPER_NTOH;
                break;
            case 'b':
                this->srcDataType = TYPE_BINARY;
                break;
            case 'f':
                this->srcDataFile = optarg;
                break;
            case 'h':
            case '?':
                {
                    help(argv[0]);
                    exit(0);
                }
                break;

            default:
                break;
        }/*switch(opt)*/
    }/*while (1)*/

    return check();
}

int
Option::check()
{
    if (OPER_UNDEFINED == this->enID)
    {
        printf("error: 必须通过 -P(--hton) 或 -U(--ntoh) 指定操作。\n");
        return -1;
    }

    if (NULL == this->srcDataFile)
    {
        printf("error: 必须通过 -f(--data-file)=FILE 指定数据源文件。\n");
        return -1;
    }

    if (0 == strlen(this->srcDataFile))
    {
        printf("error: 选项 -f(--data-file)=FILE 的值不能为空字符串。\n");
        return -1;
    }

    return 0;
}

void
Option::help(const char* pszApp)
{
    assert(NULL != pszApp);

    printf ("\n- 辅助定位 tdr_hton / tdr_ntoh 打解包过程中出现的错误。\n");

    printf ("\n用法: \n");
    printf (" %s  (-P --hton [-b --binary] [-f --data-file=FILE])\n", pszApp);
    printf ("\t  | (-U --ntoh [-b --binary] (-f --data-file=FILE))\n\n");

    printf ("\n操作选项：\n"
            "    -P, --hton            定位 tdr_hton 过程中出现的错误。\n"
            "    -U, --ntoh            定位 tdr_ntoh 过程中出现的错误。\n\n");

    printf ("\n必选选项：\n");
    printf ("    -f, --data-file=FILE  指定数据源所在的文件。\n");

    printf ("\n可选选项：\n");
    printf ("    -b, --binary          指示--data-file中的数据是二进制数据。\n"
            "                          如不指定，则认为--data-file中的数据为16进制字符串。\n");

    printf ("\n使用示例:\n");
    printf ("%s --hton --data-file host_data.txt\n", pszApp);
    printf ("%s --ntoh --data-file net_data.txt\n", pszApp);

    printf ("\n");
}
