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
        printf("error: ����ͨ�� -P(--hton) �� -U(--ntoh) ָ��������\n");
        return -1;
    }

    if (NULL == this->srcDataFile)
    {
        printf("error: ����ͨ�� -f(--data-file)=FILE ָ������Դ�ļ���\n");
        return -1;
    }

    if (0 == strlen(this->srcDataFile))
    {
        printf("error: ѡ�� -f(--data-file)=FILE ��ֵ����Ϊ���ַ�����\n");
        return -1;
    }

    return 0;
}

void
Option::help(const char* pszApp)
{
    assert(NULL != pszApp);

    printf ("\n- ������λ tdr_hton / tdr_ntoh ���������г��ֵĴ���\n");

    printf ("\n�÷�: \n");
    printf (" %s  (-P --hton [-b --binary] [-f --data-file=FILE])\n", pszApp);
    printf ("\t  | (-U --ntoh [-b --binary] (-f --data-file=FILE))\n\n");

    printf ("\n����ѡ�\n"
            "    -P, --hton            ��λ tdr_hton �����г��ֵĴ���\n"
            "    -U, --ntoh            ��λ tdr_ntoh �����г��ֵĴ���\n\n");

    printf ("\n��ѡѡ�\n");
    printf ("    -f, --data-file=FILE  ָ������Դ���ڵ��ļ���\n");

    printf ("\n��ѡѡ�\n");
    printf ("    -b, --binary          ָʾ--data-file�е������Ƕ��������ݡ�\n"
            "                          �粻ָ��������Ϊ--data-file�е�����Ϊ16�����ַ�����\n");

    printf ("\nʹ��ʾ��:\n");
    printf ("%s --hton --data-file host_data.txt\n", pszApp);
    printf ("%s --ntoh --data-file net_data.txt\n", pszApp);

    printf ("\n");
}
