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
                        printf("error: ָ����Ŀ���ļ�������Ϊ�գ���ʹ����Ч���ļ���\n");
                        return -1;
                    }
                    this->pszOutFile = optarg;
                }
                break;
            case 'm':
                {
                    if (!strlen(optarg))
                    {
                        printf("error: Ԫ����������Ϊ�գ���������ȷ��Ԫ������\n");
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
                        printf("warning: ָ����������С<%d>���Ϸ���ʹ��Ĭ��ֵ<4���ո�>\n", iSize);
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
                        printf("error: ָ���Զ���ǰ׺̫����Ŀǰ���֧��%d���ַ����Զ���ǰ׺\n",
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
                        printf("error: ָ����ǰ׺���ñ��ļ�����Ч\n");
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
                        printf("error: ָ�����·����Ϣ̫����Ŀǰ���֧��%d���ַ���·����Ϣ\n",
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
                        printf("error: ����ͬʱָ�� --no_comm_files �� --only_comm_files\n");
                        return -1;
                    }
                }
                break;
            case OPT_ONLY_COMM_FILE:
                {
                    this->onlyCommFile = true;
                    if (!this->needCommFile)
                    {
                        printf("error: ����ͬʱָ�� --no_comm_files �� --only_comm_files\n");
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
                    printf("warning: ָ���ı����ַ���Ϊ��(\"\"), ʹ��Ĭ��ֵ encoding=\"GBK\"\n");
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

    /*����������Ч��*/
    if (TDR_OPER_UNDEFINE == this->enID )
    {
        printf("����ͨ������(-B,-C,-H,-A,-P,-S��-G)ָ���ض��Ĳ�����\n��ͨ������ -H �鿴������Ϣ��\n");
        return -2;
    }

    /*ͨ��������ȡxmlԪ���ݿ��ļ�*/
    iFile = 0;
    for (i = optind; i < argc; i++)
    {
        this->paszXMLFile[iFile] = argv[i];
        iFile++;
        if (iFile >= TDR_MAX_FILES_IN_ONE_PARSE)
        {
            printf("������Ŀǰ���ֻ֧��ʹ��%d��XML�ļ�������һ��Ԫ����������\n", TDR_MAX_FILES_IN_ONE_PARSE);
            break;
        }
    }

    this->iXMLFileNum = iFile;

    return 0;
}

void
TdrOption::help(const char* pszApp)
{
    printf ("\n- �����XML��ʽ��Ԫ����������ת���ɶ����Ƹ�ʽ��C���Զ����ʽ�������������ݶ����C����ͷ�ļ���\n");
    printf ("  Ҳ���԰Ѷ��XML��ʽ��Ԫ����������ת����AS3��C++��C#�����ļ�����Щ���ļ�ʵ���˴����ӽӿڡ�\n");
    printf ("  ������xml�ļ�֮�����������ϵ����������xml�ļ�������ڲ������ǰ�档\n");

    printf ("\n�÷�: \n");
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

    printf ("��Ҫת������ѡ�\n"
            "    -B, --xml2dr         ָ��ʹ�ý����xml��ʽ��Ԫ����������ת���ɶ����Ƹ���������Ĺ��ܣ�xml2dr����\n"
            "    -C, --xml2c          ָ��ʹ�ý����xml��ʽ��Ԫ����������ת����C���Զ����ʽ������Ĺ��ܣ�xml2c����\n"
            "    -H, --xml2h          ָ��ʹ�ý����xml��ʽ��Ԫ����������ת��Ϊ���ݵ�C����ͷ�ļ��Ĺ��ܣ�xml2h����\n"
            "                         ÿ��xml�ļ���ת��Ϊһ����Ӧͷ�ļ������ļ���׺�⣬ͷ�ļ���xml�ļ�����ͬ��\n"
            "    -A, --xml2as3        ָ��ʹ�ý����xml��ʽ��Ԫ����������ת��ΪActionScript3���Ե����ļ��Ĺ��ܡ�\n"
            "    -P, --xml2cpp        ָ��ʹ�ý����xml��ʽ��Ԫ����������ת��ΪC++�������ļ��Ĺ��ܡ�\n"
            "    -S, --xml2csharp     ָ��ʹ�ý����xml��ʽ��Ԫ����������ת��ΪC#�������ļ��Ĺ��ܡ�\n"
            "    -G, --gen_xmltmplt   Ϊָ��Ԫ��������xml��ʽ�������ļ�ģ�壨gen_xmltmplt����\n"
            "    -T, --export_prefix  ����tdr����.h�ļ���C++���ļ�ʱʹ�õ����ݳ�Աǰ׺���ļ��С�\n"
            "    -E, --errorlookup    ����tdr API�ӿڷ��صĴ���ţ����Ҿ���Ĵ�����Ϣ��\n\n");

    printf ("\nxml2dr�Ŀ�ѡ�\n");
    printf ("    -o, --out_file=FILE\n"
            "                ָ��ת�������Ķ�������������ļ��������û��ָ����ѡ����ȱʡ��a.tdrΪ�ļ�����\n");

    printf ("\nxml2c�Ŀ�ѡ�\n");
    printf ("    -o, --out_file=FILE\n"
            "                ָ��ת��������C���Զ����ʽ��������ļ��������û��ָ����ѡ����ȱʡ��a.cΪ�ļ�����\n");

    printf ("\nxml2h�Ŀ�ѡ�\n");
    printf ("    -p, --no_type_prefix\n"
            "                ���ɵĽṹ�壨struct��/�����壨union����Ա������ӱ�ʾ�˳�Ա���͵�ǰ׺��\n"
            "                �����ָ����ѡ��ȱʡ����ӱ�ʾ��Ա���͵�ǰ׺��\n");
    printf ("    -s, --add_custom_prefix=PREFIX\n"
            "                ���ɵĽṹ�壨struct��/�����壨union����Ա������Զ����ǰ׺��\n"
            "                PREFIXΪҪ��ӵ�ǰ׺����Ŀǰ�����ֻ�ܰ���%d���ַ���\n",
            TDR_MAX_CUSTOM_NAME_PREFIX_LEN-1);
    printf ("    -l, --no_lowercase_prefix\n"
            "                ���ָ����ѡ������ɵĽṹ�壨struct��/�����壨union����Ա����ǿ�ƽ�����ĸ�ĳ�Сд��\n");
    printf ("    -c, --no_type_declare\n"
            "                ���ָ���˴�ѡ������Ԫ���ݵ�C���ͷ�ļ��в����ɽṹ�壨struct��/�����壨union��������������\n");
    printf ("    -u, --prefix_file=FILE\n"
            "                ���ָ���˴�ѡ����ʹ���ļ�FILE�е�ǰ׺��������C����ͷ�ļ������ݳ�Աǰ׺��\n"
            "                ע�⣬���ָ���˴�ѡ���������ǰ׺��ѡ���������Ϊ��Ч��\n");
    printf ("\nxml2as3�Ŀ�ѡ�\n");
    printf ("    -i, --indent-size\n"
            "                ָ�����ɵ�ActionScript3���ļ���������С(�������0)��Ĭ��Ϊ 4 ���ո�\n");

    printf ("\nxml2cpp�Ŀ�ѡ��(ͬʱ֧������xml2h�Ŀ�ѡ��)��\n");
    printf ("    -i, --indent-size\n"
            "                ָ�����ɵ�c++���ļ���������С(�������0)��Ĭ��Ϊ 4 ���ո�\n");
    printf ("    --no_comm_files\n"
            "                ���ָ���˴�ѡ����������Э���޹صġ����õ�C++���ļ���\n");
    printf ("    --only_comm_files\n"
            "                ���ָ���˴�ѡ����ֻ������Э���޹صġ�����C++���ļ�, ��ʱ����Ҫָ��xml�����ļ���\n");
    printf ("    --no_visualize\n"
            "                ���ָ���˴�ѡ��������visualize��Ա����, Ĭ������visualize��Ա������\n");
    printf ("    --no_united_name\n"
            "                ���ָ���˴�ѡ����ڵ�� name ����ֵΪ \"\" ��xml�����ļ�, ���ɵĴ��벻ָ��namespace��\n");

    printf ("\nxml2csharp�Ŀ�ѡ�\n");
    printf ("    -i, --indent-size\n"
            "                ָ�����ɵ�c++���ļ���������С(�������0)��Ĭ��Ϊ 4 ���ո�\n");
    printf ("    -u, --prefix_file=FILE\n"
            "                ���ָ���˴�ѡ����ʹ���ļ�FILE�е�ǰ׺��������C#���ļ������ݳ�Աǰ׺��\n");

    printf ("\ngen_xmltmplt�ı�ѡ�\n");
    printf ("    -m, --meta_name=NAME  ָ��Ҫ����ģ���Ԫ�������ơ�\n");

    printf ("\ngen_xmltmplt�Ŀ�ѡ�\n");
    printf ("    -e, --meta_version=VERSION   ���ָ����ѡ���ֻ����汾��С�ڻ���ڴ�ֵ��Ԫ���ݻ��ӳ�Ա���ݡ�\n"
            "                                 VERSION�������Ԫ���ݵĻ����汾����С��TDR֧�ֵ����汾��%d����\n"
            "                                 ���û��ָ����ѡ���ָ����ֵΪ0����ȡָ��Ԫ���ݵ����汾ֵ��\n",
            TDR_MAX_VERSION);
    printf ("    -x, --xml_formatID=FORMATID  ָ�����ɵ�xml�ļ��ĸ�ʽ, Ŀǰ֧�����¸�ʽ��\n"
            "                                 ATTR: �򵥷������Ա�����Է�ʽ�����\n"
            "                                 LIST: �򵥷������Ա���ӽڵ㷽ʽ�����\n"
            "                                 ���û��ָ�� -x ѡ���ȱʡ�����ʽΪ LIST��\n"
            "    -n, --encoding=ENCODING      ָ�������xmlģ���ļ���ͷ���� encoding ���Ե�ֵ��\n"
            "                                 ���û��ָ�� -n ѡ���ȱʡ��� encoding=\"GBK\"��\n"
            "    -o, --out_file=FILE          ָ�����ɵ������ļ������û��ָ����ѡ����ȱʡ��a.xmlΪ�ļ�����\n");

    printf ("\nexport_prefix�ı�ѡ�\n");
    printf ("    -u, --prefix_file=FILE  ָ�����ڱ��浼�������ݳ�Աǰ׺����ļ�����\n");

    printf ("\nת������ѡ�\n"
            "-O, --out_path=PATH    ָ������ļ��ı���·����\n"
            "                       �����ָ����ѡ�\n"
            "                       ����xml2dr��xml2c�����Ŀ¼ȱʡΪ��ǰ����Ŀ¼;\n"
            "                       ����xml2h�����Ŀ¼Ϊ��Ӧxml����Ŀ¼��\n"
            "-f, --old_xml_tagset   ָ��xmlԪ�����������ǲ���FO������ϵ�XML��ǩ���������ģ�ת�����߱��밴���ϵĸ�ʽȥ������\n"
            "                       ���û��ָ����ѡ���ת�����߻ᰴ��TDR�����xml��ǩȥ����xmlԪ���������⡣\n"
            "-d, --dump_lib         �Կ��ӻ��ķ�ʽ��ӡ��Ԫ����������,�������\"dump_lib.txt\"�ļ��С�\n"
            "\n����ѡ�\n"
            "-?, -h, --help   ����˹��ߵİ����б�\n"
            "-v, --version    ����˹��ߵĹ����汾\n");

    printf ("\nʹ��ʾ��:\n");
    printf ("%s -B -o ov_res.tdr ov_res.xml\n"
            "        #xml��ʽԪ���ݿ�����.tdr�����ƿ�\n"
            "%s -C -o ov_res.c --old_xml_tagset  ov_res.xml\n"
            "        #ʹ���ϱ�ǩ����xml��ʽԪ���ݿ�����.c�ļ�\n"
            "%s -H -O \"include\" --add_custom_prefix=\"m_\" --no_type_prefix\n"
            "        #xmlԪ���ݿ�����.h�ļ������ɵ��ļ�������includeĿ¼,\n"
            "        #�ṹ�壨struct��/�����壨union����Ա�����ǰ׺\"m_\"�������������ǰ׺\n"
            "%s -G -m Pkg -x ATTR -o Pkg.xml net_protocol.xml\n"
            "        #ΪPkg����xml��ʽ�������ļ�, ���а汾ΪPkg�����汾�� �ļ���ΪPkg.xml\n"
            "%s -T -u prefixfile\n"
            "        #��������.h�ļ�ʱʹ�õ����ݳ�Աǰ׺���ļ�prefixfile��\n"
            "%s -A --indent-size=8 net_protocol.xml\n"
            "        #����net_protocol.xml��������Э������ActionScript3���Ե����ļ���������СΪ8���ո�\n"
            "%s -P --indent-size=8 net_protocol.xml\n"
            "        #����net_protocol.xml��������Э������C++���Ե����ļ���������СΪ8���ո�\n"
            "%s -S --indent-size=8 net_protocol.xml\n"
            "        #����net_protocol.xml��������Э������C#���Ե����ļ���������СΪ8���ո�\n",
            pszApp, pszApp, pszApp, pszApp, pszApp, pszApp, pszApp,pszApp);
    printf ("%s -E 0x83010404\n"
            "        #��ѯ�����0x83010404��Ӧ�Ĵ�����Ϣ\n",pszApp);
    printf ("\n");
}
