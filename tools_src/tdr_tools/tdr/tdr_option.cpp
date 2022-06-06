#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "tdr_option.h"
#include "../tdr_tools.h"
#include "../tgetopt.h"
#include "tdr/tdr.h"
#include "../../lib_src/tdr/tdr_metalib_kernel_i.h"
#include "pal/pal.h"
#include "tdr_as3.h"

#if defined(_WIN32) || defined(_WIN64)
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#define OPT_ONLY_COMM_FILE      0x101
#define OPT_NO_COMM_FILE        0x102
#define OPT_NO_VISUALIZE        0x103
#define OPT_NO_UNIT_NAME        0x104

using std::cout;
using std::endl;
using std::string;

TdrOption::TdrOption()
{
    this->needExit = false;
    this->needDumpLib = false;
    this->iTagSetVersion = TDR_XML_TAGSET_VERSION_1;
    this->enID = TDR_OPER_UNDEFINE;
    this->pszOutFile = NULL;
    this->pszMetaName = NULL;
    this->szOutFile[0] = '\0';
    this->szOutPath[0] = '\0';
    this->outputXmlFileFormat = TDR_XML_DATA_FORMAT_LIST_MATA_NAME;
    this->iVersion = 0;
    this->stRule.iRule = TDR_HPPRULE_DEFAULT;
    this->pszPrefixFile = NULL;
    this->uiIndentSize = TDR_DEFAULT_INDENT_SIZE;
    this->pszEncoding = NULL;
    this->iXMLFileNum = 0;
    this->needCommFile = true;
    this->onlyCommFile = false;
    this->visualizeOn = true;
    this->unitedNameSpace = true;
}

int
TdrOption::parse(int argc, char** argv)
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

    int iFile;
    int i;
    int opt;
    int iWriteLen;
    static struct option s_astLongOptions[] = {
        {"xml2dr", 0, NULL, 'B'},
        {"xml2c", 0, NULL, 'C'},
        {"xml2h", 0, NULL, 'H'},
        {"gen_xmltmplt", 0, NULL, 'G'},
        {"xml2as3", 0, NULL, 'A'},
        {"xml2cpp", 0, NULL, 'P'},
        {"xml2csharp", 0, NULL, 'S'},
        {"errorlookup", 1, NULL, 'E'},
        {"out_file", 1, NULL, 'o'},
        {"dump_lib", 0, NULL, 'd'},
        {"no_type_prefix", 0, NULL, 'p'},
        {"add_custom_prefix", 1, NULL, 's'},
        {"no_lowercase_prefix", 0, NULL, 'l'},
        {"no_type_declare", 0, NULL, 'c'},
        {"meta_name", 1, NULL, 'm'},
        {"meta_version", 1, NULL, 'e'},
        {"xml_format", 1, NULL, 'x'},
        {"out_path", 1, NULL, 'O'},
        {"no_comm_files", 0, NULL, OPT_NO_COMM_FILE},
        {"only_comm_files", 0, NULL, OPT_ONLY_COMM_FILE},
        {"no_visualize", 0, NULL, OPT_NO_VISUALIZE},
        {"no_united_name", 0, NULL, OPT_NO_UNIT_NAME},
        {"old_xml_tagset", 0, NULL, 'f'},
        {"indent-size", 1, NULL, 'i'},
        {"prefix_file", 1, NULL, 'u'},
        {"export_prefix", 0, NULL, 'T'},
        {"encoding", 1, NULL, 'n'},
        {"version", 0, 0, 'v'},
        {"help", 0, 0, 'h'},

        {0, 0, 0, 0}
    };

    while (true)
    {
        int option_index = 0;

        opt = getopt_long (argc, argv, "APSBCHGTE:o:dps:lcm:e:x:O:i:u:fvh",
                           s_astLongOptions, &option_index);
        if (opt == -1)
            break;

        switch(opt)
        {
            case 'B':
                this->enID = TDR_OPER_XML2DR;
                break;
            case 'C':
                this->enID = TDR_OPER_XML2C;
                break;
            case 'H':
                this->enID = TDR_OPER_XML2H;
                break;
            case 'G':
                this->enID = TDR_OPER_GEN_TMPLT;
                break;
            case 'A':
                this->enID = TDR_OPER_XML2AS3;
                break;
            case 'P':
                this->enID = TDR_OPER_XML2CPP;
                break;
            case 'S':
                this->enID = TDR_OPER_XML2CSHARP;
                break;
            case 'T':
                this->enID = TDR_OPER_EXPORT_PREFIX_FILE;
                break;
            case 'E':
                {
                    int ierrno = strtol(optarg,NULL,0);
                    printf("%s\n", tdr_error_string(ierrno));
                    this->needExit = true;
                    return 0;
                }
                break;
            case 'o':
                {
                    if (!strlen(optarg))
                    {
                        printf("error: 指定的目标文件名不能为空，请使用有效的文件名\n");
                        return -1;
                    }
                    this->pszOutFile = optarg;
                }
                break;
            case 'm':
                {
                    if (!strlen(optarg))
                    {
                        printf("error: 元数据名不能为空，请输入正确的元数据名\n");
                        return -1;
                    }
                    this->pszMetaName = optarg;
                }
                break;
            case 'e':
                {
                    this->iVersion = atoi(optarg);
                }
                break;
            case 'i':
                {
                    int iSize = atoi(optarg);
                    if (iSize <= 0)
                    {
                        printf("warning: 指定的缩进大小<%d>不合法，使用默认值<4个空格>\n", iSize);
                        this->uiIndentSize = TDR_DEFAULT_INDENT_SIZE;
                    } else
                    {
                        this->uiIndentSize = iSize;
                    }
                }
                break;
            case 'x':
                {
                    if (0 == strcmp(optarg, "ATTR"))
                    {
                        this->outputXmlFileFormat = TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME;
                    }
                }
                break;
            case 'd':
                this->needDumpLib = true;
                break;
            case 'p':
                this->stRule.iRule |= TDR_HPPRULE_NO_TYPE_PREFIX;
                break;
            case 's':
                {
                    this->stRule.iRule |= TDR_HPPRULE_ADD_CUSTOM_PREFIX;
                    iWriteLen = snprintf(this->stRule.szCustomNamePrefix,
                                         sizeof(this->stRule.szCustomNamePrefix), "%s", optarg);
                    if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(this->stRule.szCustomNamePrefix))))
                    {
                        printf("error: 指定自定义前缀太长，目前最多支持%d个字符的自定义前缀\n",
                               (int)(sizeof(this->stRule.szCustomNamePrefix) -1));
                        return -1;
                    }
                }
                break;
            case 'l':
                this->stRule.iRule |= TDR_HPPRULE_NO_LOWERCASE_PREFIX;
                break;
            case 'c':
                this->stRule.iRule |= TDR_HPPRULE_NO_TYPE_DECLARE;
                break;
            case 'u':
                {
                    if (strlen(optarg) <= 0)
                    {
                        printf("error: 指定的前缀配置表文件名无效\n");
                        return -1;
                    }
                    this->pszPrefixFile = optarg;
                    /* this is right, when prefix-from-file is specified, all other flags are cleared */
                    this->stRule.iRule = TDR_HPPRULE_CUSTOMED_PREFIX;
                }
                break;
            case 'O':
                {
                    iWriteLen = snprintf(this->szOutPath, sizeof(this->szOutPath), "%s", optarg);
                    if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(this->szOutPath))))
                    {
                        printf("error: 指定输出路径信息太长，目前最多支持%d个字符的路径信息\n",
                               (int)(sizeof(this->szOutPath) -1));
                        return -1;
                    }
                }
                break;
            case OPT_NO_COMM_FILE:
                {
                    this->needCommFile = false;
                    if (this->onlyCommFile)
                    {
                        printf("error: 不能同时指定 --no_comm_files 与 --only_comm_files\n");
                        return -1;
                    }
                }
                break;
            case OPT_ONLY_COMM_FILE:
                {
                    this->onlyCommFile = true;
                    if (!this->needCommFile)
                    {
                        printf("error: 不能同时指定 --no_comm_files 与 --only_comm_files\n");
                        return -1;
                    }
                }
                break;
            case OPT_NO_VISUALIZE:
                {
                    this->visualizeOn = false;
                }
                break;
            case OPT_NO_UNIT_NAME:
                {
                    this->unitedNameSpace = false;
                }
                break;
            case 'n':
                if (0 == strlen(optarg))
                {
                    printf("warning: 指定的编码字符串为空(\"\"), 使用默认值 encoding=\"GBK\"\n");
                } else
                {
                    this->pszEncoding = optarg;
                }
                break;
            case 'f':
                this->iTagSetVersion  = TDR_XML_TAGSET_VERSION_0;
                break;
            case 'v':
                {
                    tdr_output_version_info(basename(argv[0]));
                    this->needExit = true;
                    return 0;
                }
                break;
            case 'h':
            case '?':
                {
                    help(tdr_basename(argv[0]));
                    this->needExit = true;
                    return 0;
                }
                break;

            default:
                break;
        }/*switch(opt)*/
    }/*while (1)*/

    /*检查参数的有效性*/
    if (TDR_OPER_UNDEFINE == this->enID )
    {
        printf("必须通过参数(-B,-C,-H,-A,-P,-S或-G)指定特定的操作。\n请通过参数 -H 查看帮助信息。\n");
        return -2;
    }

    /*通过参数获取xml元数据库文件*/
    iFile = 0;
    for (i = optind; i < argc; i++)
    {
        this->paszXMLFile[iFile] = argv[i];
        iFile++;
        if (iFile >= TDR_MAX_FILES_IN_ONE_PARSE)
        {
            printf("本工具目前最多只支持使用%d个XML文件来描述一个元数据描述库\n", TDR_MAX_FILES_IN_ONE_PARSE);
            break;
        }
    }

    this->iXMLFileNum = iFile;

    return 0;
}

void
TdrOption::help(const char* pszApp)
{
    printf ("\n- 将多个XML格式的元数据描述库转换成二进制格式、C语言定义格式描述及生成数据定义的C语言头文件。\n");
    printf ("  也可以把多个XML格式的元数据描述库转换成AS3、C++、C#的类文件，这些类文件实现了打解包接接口。\n");
    printf ("  如果多个xml文件之间存在依赖关系，则被依赖的xml文件必须放在参数表的前面。\n");

    printf ("\n用法: \n");
    printf ("%s (-B --xml2dr [-o --out_file=FILE] )\n", pszApp);
    printf ("  | (-C --xml2c [-o --out_file=FILE])\n");
    printf ("  | (-H --xml2h [-p --no_type_prefix] [-s --add_custom_prefix=PREFIX]\n"
            "                [-l --no_lowercase_prefix] [-c --no_type_declare] [-u --prefix_file=FILE])\n");
    printf ("  | (-A --as3   [-i --indent-size=TABSIZE])\n");
    printf ("  | (-P --xml2cpp [-i --indent-size=TABSIZE] [--no_comm_files]\n"
            "                  [--only_comm_files] [--no_visualize] [-p --no_type_prefix]\n"
            "                  [-s --add_custom_prefix=PREFIX] [-l --no_lowercase_prefix]\n"
            "                  [-c --no_type_declare] [-u --prefix_file=FILE] [--no_united_name]\n");
    printf ("  | (-S --xml2csharp [-i --indent-size=TABSIZE])\n");
    printf ("  | (-G --gen_xmltmplt --meta_name=NAME [-e --meta_version=VERSION]\n"
            "                [-x --xml_format=formatId] [-n, --encoding=ENCODING] [-o --out_file=FILE])\n");
    printf ("  | (-T --export_prefix --prefix_file=FILE\n");
    printf ("  | (-E --errorlookup=ERRORCODE)\n");
    printf ("   [-O --out_path=PATH] [-f --old_xml_tagset] [-d --dump_lib ] [-h -? --help] [-v -version] xmlfile... \n\n");

    printf ("主要转换操作选项：\n"
            "    -B, --xml2dr         指定使用将多个xml格式的元数据描述库转换成二进制个数描述库的功能（xml2dr）。\n"
            "    -C, --xml2c          指定使用将多个xml格式的元数据描述库转换成C语言定义格式描述库的功能（xml2c）。\n"
            "    -H, --xml2h          指定使用将多个xml格式的元数据描述库转换为数据的C语言头文件的功能（xml2h）。\n"
            "                         每个xml文件会转换为一个相应头文件；除文件后缀外，头文件与xml文件名相同。\n"
            "    -A, --xml2as3        指定使用将多个xml格式的元数据描述库转换为ActionScript3语言的类文件的功能。\n"
            "    -P, --xml2cpp        指定使用将多个xml格式的元数据描述库转换为C++语言类文件的功能。\n"
            "    -S, --xml2csharp     指定使用将多个xml格式的元数据描述库转换为C#语言类文件的功能。\n"
            "    -G, --gen_xmltmplt   为指定元数据生成xml格式的配置文件模板（gen_xmltmplt）。\n"
            "    -T, --export_prefix  导出tdr生成.h文件或C++类文件时使用的数据成员前缀表到文件中。\n"
            "    -E, --errorlookup    根据tdr API接口返回的错误号，查找具体的错误信息。\n\n");

    printf ("\nxml2dr的可选项：\n");
    printf ("    -o, --out_file=FILE\n"
            "                指定转换出来的二进制描述库的文件名，如果没有指定此选择，则缺省以a.tdr为文件名。\n");

    printf ("\nxml2c的可选项：\n");
    printf ("    -o, --out_file=FILE\n"
            "                指定转换出来的C语言定义格式描述库的文件名，如果没有指定此选择，则缺省以a.c为文件名。\n");

    printf ("\nxml2h的可选项：\n");
    printf ("    -p, --no_type_prefix\n"
            "                生成的结构体（struct）/联合体（union）成员名不添加表示此成员类型的前缀。\n"
            "                如果不指定此选择，缺省会添加表示成员类型的前缀。\n");
    printf ("    -s, --add_custom_prefix=PREFIX\n"
            "                生成的结构体（struct）/联合体（union）成员名添加自定义的前缀。\n"
            "                PREFIX为要添加的前缀串，目前其最多只能包含%d个字符。\n",
            TDR_MAX_CUSTOM_NAME_PREFIX_LEN-1);
    printf ("    -l, --no_lowercase_prefix\n"
            "                如果指定此选项，则生成的结构体（struct）/联合体（union）成员名不强制将首字母改成小写。\n");
    printf ("    -c, --no_type_declare\n"
            "                如果指定了此选项则在元数据的C语句头文件中不生成结构体（struct）/联合体（union）的类型声明。\n");
    printf ("    -u, --prefix_file=FILE\n"
            "                如果指定了此选项则使用文件FILE中的前缀规则生成C语言头文件的数据成员前缀。\n"
            "                注意，如果指定了此选项，其他关于前缀的选项均被设置为无效。\n");
    printf ("\nxml2as3的可选项：\n");
    printf ("    -i, --indent-size\n"
            "                指定生成的ActionScript3类文件的缩进大小(必须大于0)，默认为 4 个空格。\n");

    printf ("\nxml2cpp的可选项(同时支持所有xml2h的可选项)：\n");
    printf ("    -i, --indent-size\n"
            "                指定生成的c++类文件的缩进大小(必须大于0)，默认为 4 个空格。\n");
    printf ("    --no_comm_files\n"
            "                如果指定了此选项则不生成与协议无关的、公用的C++类文件。\n");
    printf ("    --only_comm_files\n"
            "                如果指定了此选项则只生成与协议无关的、公用C++类文件, 此时不需要指定xml描述文件。\n");
    printf ("    --no_visualize\n"
            "                如果指定了此选项则不生成visualize成员函数, 默认生成visualize成员函数。\n");
    printf ("    --no_united_name\n"
            "                如果指定了此选项，根节点的 name 属性值为 \"\" 的xml描述文件, 生成的代码不指定namespace。\n");

    printf ("\nxml2csharp的可选项：\n");
    printf ("    -i, --indent-size\n"
            "                指定生成的c++类文件的缩进大小(必须大于0)，默认为 4 个空格。\n");
    printf ("    -u, --prefix_file=FILE\n"
            "                如果指定了此选项则使用文件FILE中的前缀规则生成C#类文件的数据成员前缀。\n");

    printf ("\ngen_xmltmplt的必选项：\n");
    printf ("    -m, --meta_name=NAME  指定要生成模板的元数据名称。\n");

    printf ("\ngen_xmltmplt的可选项：\n");
    printf ("    -e, --meta_version=VERSION   如果指定此选项，则只输出版本号小于或等于此值的元数据或子成员数据。\n"
            "                                 VERSION必须大于元数据的基础版本，且小于TDR支持的最大版本（%d）。\n"
            "                                 如果没有指定此选项，或指定的值为0，则取指定元数据的最大版本值。\n",
            TDR_MAX_VERSION);
    printf ("    -x, --xml_formatID=FORMATID  指定生成的xml文件的格式, 目前支持如下格式：\n"
            "                                 ATTR: 简单非数组成员以属性方式输出。\n"
            "                                 LIST: 简单非数组成员以子节点方式输出。\n"
            "                                 如果没有指定 -x 选项，则缺省输出格式为 LIST。\n"
            "    -n, --encoding=ENCODING      指定输出的xml模板文件的头部的 encoding 属性的值。\n"
            "                                 如果没有指定 -n 选项，则缺省输出 encoding=\"GBK\"。\n"
            "    -o, --out_file=FILE          指定生成的配置文件，如果没有指定此选择，则缺省以a.xml为文件名。\n");

    printf ("\nexport_prefix的必选项：\n");
    printf ("    -u, --prefix_file=FILE  指定用于保存导出的数据成员前缀表的文件名。\n");

    printf ("\n转换公共选项：\n"
            "-O, --out_path=PATH    指定输出文件的保存路径。\n"
            "                       如果不指定此选项：\n"
            "                       对于xml2dr和xml2c其输出目录缺省为当前工作目录;\n"
            "                       对于xml2h其输出目录为对应xml所在目录。\n"
            "-f, --old_xml_tagset   指明xml元数据描述库是采用FO定义的老的XML标签进行描述的，转换工具必须按照老的格式去解析。\n"
            "                       如果没有指定此选项，则转换工具会按照TDR定义的xml标签去解析xml元数据描述库。\n"
            "-d, --dump_lib         以可视化的方式打印出元数据描述库,并输出到\"dump_lib.txt\"文件中。\n"
            "\n其他选项：\n"
            "-?, -h, --help   输出此工具的帮助列表\n"
            "-v, --version    输出此工具的构建版本\n");

    printf ("\n使用示例:\n");
    printf ("%s -B -o ov_res.tdr ov_res.xml\n"
            "        #xml格式元数据库生成.tdr二进制库\n"
            "%s -C -o ov_res.c --old_xml_tagset  ov_res.xml\n"
            "        #使用老标签集的xml格式元数据库生成.c文件\n"
            "%s -H -O \"include\" --add_custom_prefix=\"m_\" --no_type_prefix\n"
            "        #xml元数据库生成.h文件，生成的文件保存在include目录,\n"
            "        #结构体（struct）/联合体（union）成员名添加前缀\"m_\"，但不添加类型前缀\n"
            "%s -G -m Pkg -x ATTR -o Pkg.xml net_protocol.xml\n"
            "        #为Pkg生成xml格式的配置文件, 剪切版本为Pkg的最大版本， 文件名为Pkg.xml\n"
            "%s -T -u prefixfile\n"
            "        #导出生成.h文件时使用的数据成员前缀表到文件prefixfile中\n"
            "%s -A --indent-size=8 net_protocol.xml\n"
            "        #根据net_protocol.xml中描述的协议生成ActionScript3语言的类文件，缩进大小为8个空格\n"
            "%s -P --indent-size=8 net_protocol.xml\n"
            "        #根据net_protocol.xml中描述的协议生成C++语言的类文件，缩进大小为8个空格\n"
            "%s -S --indent-size=8 net_protocol.xml\n"
            "        #根据net_protocol.xml中描述的协议生成C#语言的类文件，缩进大小为8个空格\n",
            pszApp, pszApp, pszApp, pszApp, pszApp, pszApp, pszApp,pszApp);
    printf ("%s -E 0x83010404\n"
            "        #查询错误号0x83010404对应的错误信息\n",pszApp);
    printf ("\n");
}
