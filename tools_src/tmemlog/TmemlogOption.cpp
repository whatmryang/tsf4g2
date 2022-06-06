/*
 * TmemlogOption.cpp
 *
 *  Created on: 2010-7-13
 *      Author: mickeyxu
 */

#include "TmemlogOption.h"

Option::Option()
{
    pszLogConf = NULL;
    pszUri = NULL;
    pszOutPutFile = NULL;
    iFollow = 0;
    iNeedExit = 0;
}

int Option::Parse(int argc, char **argv)
{
    if (argc == 0)
    {
        printf("error: 'argc' to TdrOption::parse is 0\n");
        return -1;
    }
    if (argv == NULL)
    {
        printf("error: 'argv' to TdrOption::parse is NULL\n");
    }

    int opt;

    static struct option s_astLongOptions[] =
    {
    { "tlog-conf", 1, NULL, 'B' },
    { "uri", 1, NULL, 'C' },
    { "out", 1, NULL, 'A' },
    { "help", 0, NULL, 'H' },
    { 0, 0, 0, 0 } };

    while (true)
    {
        int option_index = 0;

        opt
                = getopt_long(argc, argv, "o:fvhH", s_astLongOptions,
                        &option_index);
        if (opt == -1)
            break;

        switch (opt)
        {
        case 'B':
        {
            if (!strlen(optarg))
            {
                printf("error:指定的日志配置文件名不能为空，请使用有效的文件名\n");
                return -1;
            }
            this->pszLogConf = optarg;
        }
            break;
        case 'C':
        {
            if (!strlen(optarg))
            {
                printf("error:指定的URI不能为空，请使用有效的文件名\n");
                return -1;
            }
            this->pszUri = optarg;
        }
            break;
        case 'f':
            this->iFollow = 1;
            break;
        case 'A':
        case 'o':
        {
            if (!strlen(optarg))
            {
                printf("error: 指定的输出文件名不能为空，请使用有效的文件名\n");
                return -1;
            }
            this->pszOutPutFile = optarg;
        }
            break;
        case 'v':
        {
            printf("%s version:%d.%d.%d build at %d\n", argv[0], MAJOR, MINOR,
                    REV, BUILD);
            this->iNeedExit = 1;
            return 0;
        }
            break;
        case 'H':
        case 'h':
        case '?':
        {
            this->Help(argc, argv);
            this->iNeedExit = 1;
            return 0;
        }
            break;

        default:
            break;
        }/*switch(opt)*/
    }/*while (1)*/

    return 0;
}

int Option::Help(int argc, char **argv)
{
    printf("usage:\n");
    printf("    %s --tlog-conf=TlogConfFile [-f]\n", argv[0]);
    printf("    %s --uri=TSM_uri -o outFileName [-f]\n", argv[0]);
    return 0;
}
