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
                        fprintf(stderr, "Error: ָ����ͨ�Ź�ϵ�����ļ���Ϊ�գ���ʹ����Ч���ļ���\n");
                        return -1;
                    }
                    this->pszDescFile = optarg;
                }
                break;
            case 'o':
                {
                    if (!strlen(optarg))
                    {
                        fprintf(stderr, "Error: ָ�������Ŀ¼Ϊ�գ���ʹ����Ч��Ŀ¼��\n");
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

    /*����������Ч��*/
    if (TBUS_OPER_UNDEFINE == this->eOperID)
    {
        fprintf(stderr, "Error: δͨ������(-t,��-g)ָ����������\n");
        return -1;
    }
    if (TBUS_COM_TYPE_UNDEFINE == this->eComType )
    {
        fprintf(stderr, "Error: δͨ������(-s,��-m)ָ��ͨ������\n");
        return -1;
    }
    if (NULL == this->pszDescFile)
    {
        if (TBUS_OPER_GEN_CONF == this->eOperID)
        {
            fprintf(stderr, "Error: ָ������Ϊ:����TBUS���ã���δͨ������(-d)ָ��ͨ�Ź�ϵ�����ļ�\n");
        } else if (TBUS_OPER_GEN_DESC == this->eOperID)
        {
            fprintf(stderr, "Error: ָ������Ϊ:����ͨ�Ź�ϵ�����ļ�ģ�壬��δͨ������(-d)ָ�����ɵ�ͨ�Ź�ϵ�����ļ���\n");
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

    printf ("\n%s: ����ָ����ͨ�Ź�ϵ�����ļ�������TBUSͨ�������ļ���\n", pszApp);

    printf ("\n�÷�: \n");
    printf ("%s (-t --gen-tbus-conf  | -g --gen-desc-file)\n", pszApp);
    printf ("         (-s --single-machine | -m --multi-machines)\n"
            "         (-d --desc-file)  [-o --out-dir]\n");

    printf ("\n��Ҫת������ѡ�\n"
            "    -t, --gen-tbus-conf  ����ָ����ͨ�����͡�ָ����ͨ�Ź�ϵ�����ļ�������TBUS�����ļ���\n"
            "    -g, --gen-desc-file  ����ָ����ͨ�����ͣ�����ͨ�Ź�ϵ�����ļ�ģ�塣\n");

    printf ("\nת������ѡ�\n"
            "-s, --single-machine     ָ��ͨ������Ϊ����ͨ�š�\n"
            "-m, --multi-machines     ָ��ͨ������Ϊ���ͨ�š�\n");

    printf ("\n-t, --gen-tbus-conf��ѡ�\n"
            "-d, --desc-file,         [��ѡ] ָ��ͨ�Ź�ϵ�����ļ�\n"
            "-o, --out-dir=DIRECOTRY  [��ѡ] ָ��TBUS�����ļ������Ŀ¼��\n"
            "                                �����ָ����ѡ�\n"
            "                                ���� -s ѡ������Ŀ¼ localhost/ �С�\n"
            "                                ���� -m ѡ������Ŀ¼ multi_machines/ �С�");

    printf ("\n-g, --gen-desc-file��ѡ�\n"
            "-d, --desc-file,         [��ѡ] ָ�����ɵ�ͨ�Ź�ϵ�����ļ���\n"
            "-o, --out-dir=DIRECOTRY  [��ѡ] ָ���������ɵ�ͨ�Ź�ϵ�����ļ���Ŀ¼��\n"
            "                                �����ָ����ѡ������ʱ��ǰ����Ŀ¼��\n");

    printf ("\n"
            "-?, -h, --help           ����˹��ߵİ����б�\n"
            "-v, --version            ����˹��ߵĹ����汾\n");

    printf ("\nʹ��ʾ��: \n"
            "      %s --gen-tbus-conf --single-machine --desc-file tbus_single_desc.xml\n"
            "      %s --gen-tbus-conf --multi-machines --desc-file tbus_multi_desc.xml\n"
            "      %s --gen-desc-file --single-machine --desc-file tbus_single_desc_tmplt.xml\n"
            "      %s --gen-desc-file --multi-machines --desc-file tbus_multi_desc_tmplt.xml\n",
            pszPathName, pszPathName, pszPathName, pszPathName);

    printf ("\n");
}
