#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "pal/pal.h"
#include "tbus_option.h"
#include "tbus_tools.h"
#include "version.h"

using std::cout;
using std::endl;
using std::string;

TbusOption::TbusOption()
{
    this->eOperID = TBUS_OPER_UNDEFINE;
    this->eComType = TBUS_COM_TYPE_UNDEFINE;
    this->pszDescFile = NULL;
    this->pszOutDir = NULL;
    this->bNeedExit = false;
}

int
TbusOption::parse(int argc, char** argv)
{
    if (argc == 0)
    {
        fprintf(stderr, "Error: 'argc' to TbusOption::parse is 0\n");
        return -1;
    }
    if (argv == NULL)
    {
        fprintf(stderr, "Error: 'argv' to TbusOption::parse is NULL\n");
    }

    static struct option s_astLongOptions[] = {
        {"gen-tbus-conf", 0, NULL, 't'},
        {"gen-desc-file", 0, NULL, 'g'},
        {"single-machine", 0, NULL, 's'},
        {"multi-machines", 0, NULL, 'm'},
        {"desc-file", 1, NULL, 'd'},
        {"out-dir", 1, NULL, 'o'},

        {0, 0, 0, 0}
    };

    while (true)
    {
        int opt = 0;
        int option_index = 0;

        opt = getopt_long(argc, argv, "tgsmd:o:vh", s_astLongOptions, &option_index);
        if (opt == -1)
            break;

        switch(opt)
        {
            case 't':
                this->eOperID = TBUS_OPER_GEN_CONF;
                break;
            case 'g':
                this->eOperID = TBUS_OPER_GEN_DESC;
                break;
            case 's':
                this->eComType = TBUS_COM_TYPE_SINGLE;
                break;
            case 'm':
                this->eComType = TBUS_COM_TYPE_MULTI;
                break;
            case 'd':
                {
                    if (!strlen(optarg))
                    {
                        fprintf(stderr, "Error: 指定的通信关系描述文件名为空，请使用有效的文件名\n");
                        return -1;
                    }
                    this->pszDescFile = optarg;
                }
                break;
            case 'o':
                {
                    if (!strlen(optarg))
                    {
                        fprintf(stderr, "Error: 指定的输出目录为空，请使用有效的目录名\n");
                        return -1;
                    }
                    this->pszOutDir = optarg;
                }
                break;
            case 'v':
                {
                    printf("%s:version %d.%d.%d  build at %d.\n",
                           TbusTools::tbus_basename(argv[0]), MAJOR, MINOR, REV, BUILD);
                    this->bNeedExit = true;
                    return 0;
                }
                break;
            case 'h':
            case '?':
                {
                    help(argv[0]);
                    this->bNeedExit = true;
                    return 0;
                }
                break;

            default:
                break;
        }/*switch(opt)*/
    }/*while (1)*/

    /*检查参数的有效性*/
    if (TBUS_OPER_UNDEFINE == this->eOperID)
    {
        fprintf(stderr, "Error: 未通过参数(-t,或-g)指定操作类型\n");
        return -1;
    }
    if (TBUS_COM_TYPE_UNDEFINE == this->eComType )
    {
        fprintf(stderr, "Error: 未通过参数(-s,或-m)指定通信类型\n");
        return -1;
    }
    if (NULL == this->pszDescFile)
    {
        if (TBUS_OPER_GEN_CONF == this->eOperID)
        {
            fprintf(stderr, "Error: 指定操作为:生成TBUS配置，但未通过参数(-d)指定通信关系描述文件\n");
        } else if (TBUS_OPER_GEN_DESC == this->eOperID)
        {
            fprintf(stderr, "Error: 指定操作为:生成通信关系描述文件模板，但未通过参数(-d)指定生成的通信关系描述文件名\n");
        }
        return -1;
    }


    return 0;
}

void
TbusOption::help(const char* pszPathName)
{
    assert(NULL != pszPathName);
    const char* pszApp = TbusTools::tbus_basename(pszPathName);

    printf ("\n%s: 根据指定的通信关系描述文件，生成TBUS通信配置文件。\n", pszApp);

    printf ("\n用法: \n");
    printf ("%s (-t --gen-tbus-conf  | -g --gen-desc-file)\n", pszApp);
    printf ("         (-s --single-machine | -m --multi-machines)\n"
            "         (-d --desc-file)  [-o --out-dir]\n");

    printf ("\n主要转换操作选项：\n"
            "    -t, --gen-tbus-conf  根据指定的通信类型、指定的通信关系描述文件，生成TBUS配置文件。\n"
            "    -g, --gen-desc-file  根据指定的通信类型，生成通信关系描述文件模板。\n");

    printf ("\n转换公共选项：\n"
            "-s, --single-machine     指定通信类型为单机通信。\n"
            "-m, --multi-machines     指定通信类型为多机通信。\n");

    printf ("\n-t, --gen-tbus-conf的选项：\n"
            "-d, --desc-file,         [必选] 指定通信关系描述文件\n"
            "-o, --out-dir=DIRECOTRY  [可选] 指定TBUS配置文件的输出目录。\n"
            "                                如果不指定此选项：\n"
            "                                对于 -s 选项，输出到目录 localhost/ 中。\n"
            "                                对于 -m 选项，输出到目录 multi_machines/ 中。");

    printf ("\n-g, --gen-desc-file的选项：\n"
            "-d, --desc-file,         [必选] 指定生成的通信关系描述文件名\n"
            "-o, --out-dir=DIRECOTRY  [可选] 指定保存生成的通信关系描述文件的目录。\n"
            "                                如果不指定此选项，输出到时当前工作目录。\n");

    printf ("\n"
            "-?, -h, --help           输出此工具的帮助列表\n"
            "-v, --version            输出此工具的构建版本\n");

    printf ("\n使用示例: \n"
            "      %s --gen-tbus-conf --single-machine --desc-file tbus_single_desc.xml\n"
            "      %s --gen-tbus-conf --multi-machines --desc-file tbus_multi_desc.xml\n"
            "      %s --gen-desc-file --single-machine --desc-file tbus_single_desc_tmplt.xml\n"
            "      %s --gen-desc-file --multi-machines --desc-file tbus_multi_desc_tmplt.xml\n",
            pszPathName, pszPathName, pszPathName, pszPathName);

    printf ("\n");
}
