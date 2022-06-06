/** @file $RCSfile: tbus_mgr.c,v $
  general description of this module
  $Id: tbus_mgr.c,v 1.16 2009-07-13 01:10:30 jacky Exp $
  @author $Author: jacky $
  @date $Date: 2009-07-13 01:10:30 $
  @version $Revision: 1.16 $
  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
  @note Platform: Linux
  */



#include "tdr/tdr.h"
#include "tbus/tbus.h"
#include "tbus/tbus_config_mng.h"
#include "tbus_mgr.h"
#include "version.h"
#include "tbus_view_channel.h"
#include "tbus/tbus_log.h"
#include "tbus/tbus_kernel.h"

#define TBUSMGR_CONFINFO "TbusMgr"
#define TBUS_GCIM_CONF_INFO "TbusGCIM"

#define TBUS_OPT_SIMPLE_CHANNEL 257
#define TBUS_OPT_SEND_BUF_SIZE  258
#define TBUS_OPT_RECV_BUF_SIZE  259
#define TBUS_OPT_ALIGN_SIZE     260
#define TBUS_OPT_BACKUP_FILE    261
#define TBUS_OPT_DUMP_BINARY    262
#define TBUS_OPT_WITH_SEPERATOR 263
#define TBUS_OPT_START_MSG_IDX  264
#define TBUS_OPT_INCRIMENTAL    265
#define TBUS_OPT_RECREATE       266
#define TBUS_OPT_DISABLE_CHNNL  267
#define TBUS_OPT_ENABLE_CHNNL   268

#define TBUS_MAX_TEST_DATA_LEN 1024

#define TBUS_MGR_CONF_XML "tbusmgr.xml"

#define TBUS_MGR_STRING_ERROR 1024

#if defined(_WIN32) || defined(_WIN64)
#define PTR_OR_INT "%p"
#else
#define PTR_OR_INT "%d"
#endif

extern unsigned char g_szMetalib_TbusConf[] ;
int tbus_is_channel_in_use(IN LPTBUSSHMCHANNELCNF a_pstShmChl,
                           LPTBUSSHMGCIMHEAD a_pstHead, INOUT int* a_iIsInUse);

/**
  various internal function defined
  */

static void tbus_print_help(char* a_pszFileName);
static int tbus_list_gcim(IN LPTBUSSHMGCIM a_pShmGCIM, OUT FILE *a_fpOut);
static int tbus_delete_all_channles(IN LPTBUSSHMGCIM a_pShmGCIM);

#if !defined(_WIN32) && !defined(_WIN64)
static int tbus_cal_channel_size(IN LPTBUSMGROPTIONS a_pstOption);
#endif

static int del(IN LPTBUSSHMGCIM a_pShmGCIM, int a_idx);
static int clean(IN LPTBUSSHMGCIM a_pShmGCIM, int a_idx);
static int setDefaultOptions(IN LPTBUSMGROPTIONS a_pstOption);
static int set(IN LPTBUSGCIM a_pstGCIM, IN LPTBUSSHMGCIM a_pShmGCIM, IN int a_iFlag);
static int commandLine(int argc,  char *argv[], LPTBUSMGROPTIONS a_pstOption);
static int SendTestMsg(IN LPTBUSSHMGCIM a_pShmGCIM, LPTBUSMGROPTIONS a_pstOption);
static int load_config_of_old_format(LPTBUSMGROPTIONS a_pstOption, OUT LPTBUSGCIM a_pstGCIM);
static int backChannelInfo(IN LPTBUSSHMGCIM a_pstShmGCIM, LPTBUSGCIM a_pstGCIM, LPTBUSMGROPTIONS a_pstOption);
static int initialize (LPTBUSMGROPTIONS a_pstOption, OUT LPTBUSGCIM a_pstGCIM, OUT LPTBUSSHMGCIM *a_ppShmGCIM);
int AttachChannel(IN TBUSSHMCHANNELCNF *a_ptRoute, INOUT TBUSCHANNEL *a_ptChannel, IN LPTBUSSHMGCIMHEAD a_pstHead);

void tbus_print_help (char* a_pszFileName)
{
    printf ("\n") ;
    printf ("- tbusͨ������\n\n");

    printf ("�÷�:\n");
    printf ("tbusmgr\t([-W,--write] | [-D,--delete=INDEX] | [-X,--delete-all] | [-T,--send-test-msg] |\n"
            "\t  [--simple] | [-R,--clean=INDEX] | [-S,--see=TbusID] | [-A,--see-all] | [-L,--list] |\n"
            "\t  [-E,--errorlookup=code]"
#if !defined(_WIN32) && !defined(_WIN64)
            " | [-I,--channel=shmID] | [-U,--channel-size]) |\n"
#endif
            "\t  [--disable=INDEX] | [--enable=INDEX]\n\n"
            "\t  [[--incrimentally] [--recreate-channels]] [[--enable] [--disable]]\n"
            "\t  [[-m,--meta-name=NAME] [-B,--meta-lib=FILE] [--dump-binary] [--no-seperator]\n"
            "\t   [--start-msg-index=IDX] [-o,--out-file] [-n,--see-msg-num]]\n"
            "\t  [[--send-buf-size=SIZE] [--recv-buf-size=SIZE] [--align-size=SIZE]]\n"
            "\t  [-C,--conf-file=FILE] [-k,--bus-key=KEY] [-f,--old-conf] [--backup-file=FILE]\n"
            "\t  [[--src=TbusdID] [--dst=TbusID] [--data-len=LEN]]\n"
            "\t  [--debug] [-v] [-h]\n");

    printf ("\n��Ҫ����ѡ��:\n");
    printf ("-W,--write         дGCIM(ȫ��ͨ����Ϣ)\n"
            "                   �������ý�Tbusϵͳ��ͨ������Ϣд�������ڴ��С�\n"
            "                   ����������ͬʱʹ��--conf-fileָ��Tbusϵͳ��ͨ����Ϣ�������ļ���\n" );
    printf ("-D,--delete=INDEX  ɾ��ͨ��\n"
            "                   ɾ��INDEXָ����ͨ��������ֵINDEX��1��ʼ���롣\n"
            "                   ����������ָ��--conf-file��--bus-keyѡ�\n");
    printf ("-R,--clean=INDEX   ���ͨ���е�����\n"
            "                   ���INDEXָ����ͨ���е����ݣ�����ֵINDEX��1��ʼ���롣\n"
            "                   ����������ָ��--conf-file��--bus-keyѡ�\n");
    printf ("-X,--delete-all    ɾ��GCIM(ȫ��ͨ����Ϣ)���������ͨ��\n"
            "                   ����������ָ��--conf-file��--bus-keyѡ�\n" );
    printf ("-L,--list          �鿴GCIM(ȫ��ͨ����Ϣ)\n"
            "                   �鿴�����ڴ��е�ͨ��������������ָ��--conf-file��--bus-keyѡ�\n");
    printf ("-S,--see=TbusID    �鿴��Tbus��ַ��ص�ͨ���ĸ�ò\n"
            "                   �鿴��ĳ��Tbus��ַ��ص�����Tbusͨ������Ϣ��ͨ��TbusIDָ����Tbus��ַ��\n"
            "                   ����������ָ��--conf-file��--bus-keyѡ�\n" );
    printf ("-A,--see-all       �鿴��ָ��Shmkey��ص�����ͨ���ĸ�ò\n"
            "                   ����������ָ��--conf-file��--bus-keyѡ�\n");
#if !defined(_WIN32) && !defined(_WIN64)
    printf ("-I,--channel=shmID �鿴shmID��Ӧ��ͨ���Ļ�����Ϣ��\n");
    printf ("-U,--channel-size  ����ͨ���Ĺ����ڴ��С��\n");
    printf ("                   ����������ָ��--send-buf-sizeѡ�\n");
    printf ("                   ����������ָ��--recv-buf-sizeѡ�\n");
#endif

    printf ("-T,--send-test-msg ���Ͳ������ݵ�ָ��ͨ��\n"
            "                   ����������ָ��--conf-file��--bus-keyѡ�\n"
            "                   ����������ָ��--src=TbusID --dst=TbusID --data-len=LENѡ�\n");
    printf ("-E,--errorlookup=code  �鿴�������Ӧ�Ĵ����ַ���\n");
    printf ("--simple           �鿴ֱ��ͨ��ģʽ��ͨ����ò\n"
            "                   ����������ָ��--bus-keyѡ�\n");
    printf ("--enable=INDEX     ����ָ����ͨ����\n"
            "                   ����������ָ��--conf-file��--bus-keyѡ�\n" );
    printf ("--disable=INDEX    ����ָ����ͨ����\n"
            "                   ����������ָ��--conf-file��--bus-keyѡ�\n" );

    printf ("\n����������:\n");
    printf ("    -C,--conf-file=FILE\n"
            "                ָ������Tbusͨ����Ϣ�������ļ���\n"
            "                ���û��ָ����ѡ���ʹ��Ĭ��ֵ%s��\n", TBUS_MGR_CONF_XML);
    printf ("    -k,--bus-key=key\n"
            "                ָ���洢Tbusͨ������Ϣ�Ĺ����ڴ�keyֵ��\n"
            "                ���ָ����ѡ���ͬʱҲָ����--conf-fileѡ���洢ͨ����Ĺ����ڴ�key�Ա�ѡ��Ϊ׼��\n"
            "                ����--simple��������ѡ��ȿ���ָ�������ڴ��key��Ҳ����ָ����������key���ļ�·����\n");
    printf ("    -f,--old-conf\n"
            "                ָ��xml�����ļ�Ϊ�ϵĸ�ʽ����ͨ����ַͨ��UpAddr,DownAddr�����ֵġ�\n"
            "                �µ������ļ��������������֣���ͨ���Ĺ����ڴ�keyҲ�������ã����Զ����ɡ�\n");
    printf ("    -g,--debug\n"
            "                ��ӡһЩ������Ϣ��\n" ) ;
    printf ("    -v,--version, -v\n"
            "                �鿴���߰汾��\n" ) ;
    printf ("    -h,--help, -h\n"
            "                �鿴�����ߵİ�����\n" ) ;
    printf ("\n--write����������:\n");
    printf ("    --incrimentally\n"
            "                ���������ļ���û�����ã������Ѿ������ڹ����ڴ��е�ͨ��������������\n");
    printf ("    --recreate-channels\n"
            "                ��������ļ������õ�ͨ���Ѿ������ڹ����ڴ��У���ɾ���ٴ�����\n");
    printf ("\n--simple����������:\n");
    printf ("    -i,--busi-id\n"
            "                ���--bus-keyѡ��ָ����������key���ļ�·����--busi-id������ָ��ҵ��id, Ĭ��ҵ��idΪ0��\n");
    printf ("\n--see����������:\n");
    printf ("    -B,--meta-lib\n"
            "                ָ��tbusͨ������Ϣ��Ӧ���ݽṹ��Ԫ���������ļ���\n");
    printf ("    -m,--meta-name\n"
            "                ָ��tbusͨ������Ϣ��Ӧ���ݽṹ��Ԫ�����������е����֡�\n") ;
    printf ("    --dump-binary\n"
            "                �Զ����Ƶ���ʽ���tbusͨ���е�Ӧ�����ݡ�\n");
    printf ("    --start-msg-index\n"
            "                ָ����ʼ�鿴��tbusͨ���е���Ϣ�������š�\n");
    printf ("    --no-seperator\n"
            "                �Զ����Ƶ���ʽ���tbusͨ���е�Ӧ������ʱ��ÿ���ֽ�֮�䲻ʹ�ÿո�ָ���\n");
    printf ("    -o,--out-file\n"
            "                ָ��tbusͨ������Ϣ��Ϣ������ָ���ļ��С�\n") ;
    printf ("    -n,--see-msg-num\n"
            "                ��ϸ�쿴����Ϣ������\n");

#if !defined(_WIN32) && !defined(_WIN64)
    printf ("\n--channel-size����������:\n");
    printf ("    --send-buf-size=SIZE\n"
            "                ָ�����ͻ���Ĵ�С��\n");
    printf ("    --recv-buf-size=SIZE\n"
            "                ָ�����ջ���Ĵ�С��\n");
    printf ("\n--channel-size/--channel�Ŀ�ѡ������:\n");
    printf ("    --align-size=SIZE\n"
            "                ָ��ͨ�����ݶ����С��\tһ�㲻��Ҫ���ô�ѡ�\n"
            "                ���ָ����ѡ���ֵ�����빲���ڴ������ļ���ָ����ChannelDataAlignֵ��ͬ��\n"
            "                ��������ļ���û��ָ��ChannelDataAlign��ֵ������Ҫ���ô�ѡ�\n");
#endif

    printf ("\n--write/--delete�Ŀ�ѡ������:\n");
    printf ("    --backup-file=FILE\n"
            "                ָ�������ڴ�ͨ���������ļ��������ڴ�ͨ�����������׼�����ͬʱ����������ļ��С�\n"
            "                �粻ָ���������Ĭ�ϻ�������ļ���Ϊtbus_channels_#KEY.bak���ļ��С�\n");

    printf ("\n--send-test-msg����������:\n");
    printf ("    --src=TbusID\n"
            "                ָ�����Ͳ������ݵ�ͨ����Դ��ַ��\n");
    printf ("    --dst=TbusID\n"
            "                ָ�����Ͳ������ݵ�ͨ����Ŀ�ĵ�ַ��\n");
    printf ("    --data-len=TestDataLen\n"
            "                ָ�����͵Ĳ������ݳ��ȡ�\n");

    printf ("\nʹ��ʾ��:\n");
    printf ("%s --conf-file=./tbusmgr.xml --write\n", a_pszFileName);
    printf ("%s -C tbusmgr.xml -X\n", a_pszFileName);
    printf ("%s -C tbusmgr.xml -A\n", a_pszFileName);
    printf ("%s --see=1.1.1.1 --bus-key=1688\n", a_pszFileName);
    printf ("%s --see=1.1.1.1 --bus-key=1688 --dump-binary --start-msg-index=11\n", a_pszFileName);
    printf ("%s -S 1.1.1.1 -k 1688 -B cusprotocol.tdr -m CusPkg\n", a_pszFileName);
    printf ("%s -C tbusmgr.xml --delete=2\n", a_pszFileName);
    printf ("%s -C tbusmgr.xml -R 2\n", a_pszFileName);
    printf ("%s --list --conf-file=./tbusmgr.xml\n", a_pszFileName);
    printf ("%s -C tbusmgr.xml -T --data-len 10 --src 1.1.1.1 --dst 1.1.2.1\n", a_pszFileName);
    printf ("%s --simple --bus-key /tmp/example.sock\n", a_pszFileName);

#if !defined(_WIN32) && !defined(_WIN64)
    printf ("%s --channel 33521786\n", a_pszFileName);
    printf ("%s --channel-size --send-buf-size 1024000 --recv-buf-size 1024000\n", a_pszFileName);
#endif

    printf ("%s --errorlookup 0\n", a_pszFileName);
    printf ( "\nAuthors: ARCH,IERD,Tencent\n" );
    printf ( "Bugs:    g_IERD_Rnd_Architecture@tencent.com\n" );
    printf ( "\n" ) ;

    return;
}


TBUSGCIM    gs_stGCIM;

int main ( int argc, char ** argv )
{
    TBUSMGROPTIONS    stOption;
    LPTBUSSHMGCIM pstShmGCIM = NULL;
    int iRet = 0;

    /* get options from command line */
    memset(&stOption, 0, sizeof(TBUSMGROPTIONS));
    setDefaultOptions(&stOption);
    if (commandLine (argc, argv , &stOption) != 0)
    {
        fprintf(stdout, "failed to parse commandline, please check your args\n");
        return -1 ;
    }

    if (TBUS_MGR_OP_SIMPLE_CHNNL != stOption.iMgrOP
#if !defined(_WIN32) && !defined(_WIN64)
        && TBUS_MGR_OP_VIEW_CHNNL != stOption.iMgrOP
        && TBUS_MGR_OP_CAL_CHNNL_SIZE != stOption.iMgrOP
#endif
        && TBUS_MGR_OP_STRING_ERROR != stOption.iMgrOP)
    {
        /* load config and attach shm*/
        if (0 != initialize (&stOption, &gs_stGCIM, &pstShmGCIM))
            return -1 ;
    }

    switch(stOption.iMgrOP)
    {
        case TBUS_MGR_OP_WRITE:
            {
                iRet = set(&gs_stGCIM, pstShmGCIM, stOption.iSetGCIMFlag);
                backChannelInfo(pstShmGCIM, &gs_stGCIM, &stOption);
            }
            break;
        case TBUS_MGR_OP_SIMPLE_CHNNL:
            {
                iRet = ViewSimpleChannel(&stOption);
            }
            break;
#if !defined(_WIN32) && !defined(_WIN64)
        case TBUS_MGR_OP_VIEW_CHNNL:
            {
                iRet = ViewChannelByShmID(&stOption);
            }
            break;
        case TBUS_MGR_OP_CAL_CHNNL_SIZE:
            {
                iRet = tbus_cal_channel_size(&stOption);
            }
            break;
#endif
        case TBUS_MGR_OP_STRING_ERROR:
            {
                fprintf(stdout, "\nCode %d: %s\n\n", stOption.iDelID, tbus_error_string(stOption.iDelID));
            }
            break;
        case TBUS_MGR_OP_DELETE:
            {
                if ( 0 < stOption.iDelID )
                {
                    iRet = del(pstShmGCIM, stOption.iDelID) ;
                    if ( TBUS_SUCCESS == iRet )
                    {
                        tbus_list_gcim(pstShmGCIM, stdout);
                    }
                    backChannelInfo(pstShmGCIM, &gs_stGCIM, &stOption);
                }else
                {
                    fprintf(stdout, "\nError: channel<index: %d> nonexists\n\n", stOption.iDelID);
                }
            }
            break;
        case TBUS_MGR_OP_DISABLE_CHNNL:
            {
                if ( 0 < stOption.iDelID )
                {
                    iRet = tbus_disable_channel_by_index(pstShmGCIM, stOption.iDelID-1);
                    if ( TBUS_SUCCESS == iRet )
                    {
                        tbus_list_gcim(pstShmGCIM, stdout);
                    }
                    backChannelInfo(pstShmGCIM, &gs_stGCIM, &stOption);
                }else
                {
                    fprintf(stdout, "\nError: channel<index: %d> nonexists\n\n", stOption.iDelID);
                }
            }
            break;
        case TBUS_MGR_OP_ENABLE_CHNNL:
            {
                if ( 0 < stOption.iDelID )
                {
                    iRet = tbus_enable_channel_by_index(pstShmGCIM, stOption.iDelID-1);
                    if ( TBUS_SUCCESS == iRet )
                    {
                        tbus_list_gcim(pstShmGCIM, stdout);
                    }
                    backChannelInfo(pstShmGCIM, &gs_stGCIM, &stOption);
                }else
                {
                    fprintf(stdout, "\nError: channel<index: %d> nonexists\n\n", stOption.iDelID);
                }
            }
            break;
        case TBUS_MGR_OP_CLEAN:
            {
                if ( 0 < stOption.iDelID )
                {
                    iRet = clean(pstShmGCIM, stOption.iDelID) ;
                    if ( TBUS_SUCCESS == iRet )
                    {
                        ViewAllChannels(pstShmGCIM, &stOption) ;
                    }
                }
                else
                {
                    fprintf(stdout, "\nError: channel<index: %d> nonexists\n\n", stOption.iDelID);
                }
            }
            break;
        case TBUS_MGR_OP_DELETE_ALL:
            {
                int iNum = 0;
                char cChoice = 0;
                int iShmID = -1;

#if !defined(_WIN32) && !defined(_WIN64)
                iRet = tbus_get_attached_num(stOption.szShmKey, gs_stGCIM.iBussinessID, &iNum, &iShmID);
                if (iRet < 0)
                {
                    fprintf(stdout, "\nErrror: unable to acquire how many processes attached to shm<key: %s, bussnissID: %d>\n",
                            stOption.szShmKey, gs_stGCIM.iBussinessID);
                    break;
                }
                if (1 < iNum)
                {
                    fprintf(stdout, "\nWarnning:\nThere are other processes still attached to this shared memory.\n");
                    fprintf(stdout, "Are you sure to go on? [y|n]");
                    scanf("%c", &cChoice);
                    if ('y' != cChoice && 'Y' != cChoice)
                    {
                        fprintf(stdout, "\n! Operation aborted by user\n\n");
                        break;
                    }
                }
#else
                // ��windowsϵͳ�£�û�л��ƿ��Լ��attach��ӳ���ļ��Ľ�������
#endif
                iRet = tbus_delete_all_channles(pstShmGCIM);
                if (0 > iRet)
                {
                    fprintf(stdout, "\nError: failed to remove all channels\n\n");
                    break;
                }

#if !defined(_WIN32) && !defined(_WIN64)
                {
                    struct shmid_ds stShmidDs;
                    iRet = shmctl(iShmID, IPC_RMID, &stShmidDs);
                    if (0 > iRet)
                        fprintf(stdout, "\nError:\nfailed to remove shared memory, shmctl error: %s\n\n", strerror(errno));
                    else
                        fprintf(stdout, "\nShm and relevant channles were sccessfully removed.\n\n");
                }
#else
                // ��windowsϵͳ�£����ڵ�ǰ�����Ѿ������˻��ߴ����ڴ�ӳ���ļ���
                // ����û�аѾ����������Ҫɾ��ӳ���ļ��Ƚ����ѣ���������û�����κδ���
                // ������Ȼͨ���Ѿ�����������GCIM��ϢҲ��֤��һ�£��Ѿ�û�б�Ҫɾ����ӦGCIM���ļ���
#endif
            }
            break;
        case TBUS_MGR_OP_VIEW:
            {
                iRet = ViewChannels(pstShmGCIM, &stOption);
            }
            break;
        case TBUS_MGR_OP_VIEW_ALL:
            {
                iRet = ViewAllChannels(pstShmGCIM, &stOption);
            }
            break;
        case TBUS_MGR_OP_SEND_TEST_MSG:
            {
                iRet = SendTestMsg(pstShmGCIM, &stOption);
            }
            break;
        default:
            iRet = tbus_list_gcim(pstShmGCIM, stdout);
    }

    return iRet;
}



int commandLine ( int argc,  char * argv[] , LPTBUSMGROPTIONS a_pstOption)
{
    static struct option s_astLongOptions[] = {
        {"conf-file", 1, NULL, 'C'},
        {"write", 0, NULL, 'W'},
        {"see", 1, NULL, 'S'},
        {"see-all", 0, NULL, 'A'},
        {"simple", 0, NULL, TBUS_OPT_SIMPLE_CHANNEL},
#if !defined(_WIN32) && !defined(_WIN64)
        {"channel", 1, NULL, 'I'},
        {"channel-size", 0, NULL, 'U'},
        {"disable", 1, NULL, TBUS_OPT_DISABLE_CHNNL},
        {"enable", 1, NULL, TBUS_OPT_ENABLE_CHNNL},
        {"send-buf-size", 1, NULL, TBUS_OPT_SEND_BUF_SIZE},
        {"recv-buf-size", 1, NULL, TBUS_OPT_RECV_BUF_SIZE},
        {"align-size", 1, NULL, TBUS_OPT_ALIGN_SIZE},
#endif
        {"backup-file", 1, NULL, TBUS_OPT_BACKUP_FILE},
        {"send-test-msg", 0, NULL, 'T'},
        {"dump-binary", 0, NULL, TBUS_OPT_DUMP_BINARY},
        {"start-msg-index", 1, NULL, TBUS_OPT_START_MSG_IDX},
        {"no-seperator", 0, NULL, TBUS_OPT_WITH_SEPERATOR},
        {"incrimentally", 0, NULL, TBUS_OPT_INCRIMENTAL},
        {"recreate-channels", 0, NULL, TBUS_OPT_RECREATE},
        {"src", 1, NULL, 'Y'},
        {"dst", 1, NULL, 'Z'},
        {"data-len", 1, NULL, 'p'},
        {"delete", 1, NULL, 'D'},
        {"delete-all", 0, NULL, 'X'},
        {"clean", 1, NULL, 'R'},
        {"errorlookup", 1, NULL, 'E'},
        {"list", 0, NULL, 'L'},
        {"bus-key", 1, NULL, 'k'},
        {"meta-lib", 1, NULL, 'B'},
        {"meta-name", 1, NULL, 'm'},
        {"out-file", 1, NULL, 'o'},
        {"see-msg-num", 1, NULL, 'n'},
        {"old-conf", 0, NULL, 'f'},
        {"busi-id", 1, NULL, 'i'},
        {"debug", 0, NULL, 'g'},
        {"version", 0, 0, 'v'},
        {"help", 0, 0, 'h'},

        {0, 0, 0, 0}
    };
    int opt = 0;
    char* pszApp = NULL ;

    assert(NULL != a_pstOption);

    pszApp = basename ( argv[0] ) ;

    while (1)
    {
        int option_index = 0;

        opt = getopt_long (argc, argv, "n:C:c:S:s:D:d:R:r:k:K:B:b:E:e:m:M:o:O:Y:Z:I:UtTaAxXgLlwWfvh",
                           s_astLongOptions, &option_index);

        if (opt == -1)
            break;

        switch(opt)
        {
            case 'C':
            case 'c':
                STRNCPY(a_pstOption->szConfFile, optarg, sizeof(a_pstOption->szConfFile) ) ;
                break;
            case 'W':
            case 'w':
                a_pstOption->iMgrOP = TBUS_MGR_OP_WRITE ;
                break;
            case 'D':
            case 'd':
                a_pstOption->iMgrOP = TBUS_MGR_OP_DELETE;
                a_pstOption->iDelID = atoi(optarg) ;
                break;
            case TBUS_OPT_DISABLE_CHNNL:
                a_pstOption->iMgrOP = TBUS_MGR_OP_DISABLE_CHNNL;
                a_pstOption->iDelID = atoi(optarg) ;
                break;
            case TBUS_OPT_ENABLE_CHNNL:
                a_pstOption->iMgrOP = TBUS_MGR_OP_ENABLE_CHNNL;
                a_pstOption->iDelID = atoi(optarg) ;
                break;
            case 'X':
            case 'x':
                a_pstOption->iMgrOP = TBUS_MGR_OP_DELETE_ALL;
                break;
            case 'R':
            case 'r':
                a_pstOption->iMgrOP = TBUS_MGR_OP_CLEAN;
                a_pstOption->iDelID = atoi(optarg) ;
                break;
            case 'E':
            case 'e':
                a_pstOption->iMgrOP = TBUS_MGR_OP_STRING_ERROR;
                a_pstOption->iDelID = strtol(optarg, NULL, 0);
                break;
            case 'A':
            case 'a':
                a_pstOption->iMgrOP = TBUS_MGR_OP_VIEW_ALL;
                break;
            case TBUS_OPT_SIMPLE_CHANNEL:
                a_pstOption->iMgrOP = TBUS_MGR_OP_SIMPLE_CHNNL;
                break;
            case 'T':
            case 't':
                a_pstOption->iMgrOP = TBUS_MGR_OP_SEND_TEST_MSG;
                break;
            case 'Y':
                STRNCPY(a_pstOption->szTbusSrcAddr, optarg, sizeof(a_pstOption->szTbusSrcAddr));
                break;
            case 'Z':
                STRNCPY(a_pstOption->szTbusDstAddr, optarg, sizeof(a_pstOption->szTbusDstAddr));
                break;
#if !defined(_WIN32) && !defined(_WIN64)
            case 'I':
                a_pstOption->iMgrOP = TBUS_MGR_OP_VIEW_CHNNL;
                a_pstOption->iShmID = atoi(optarg);
                break;
            case 'U':
                a_pstOption->iMgrOP = TBUS_MGR_OP_CAL_CHNNL_SIZE;
                break;
            case TBUS_OPT_SEND_BUF_SIZE:
                a_pstOption->iSendBufSize = atoi(optarg);
                break;
            case TBUS_OPT_RECV_BUF_SIZE:
                a_pstOption->iRecvBufSize = atoi(optarg);
                break;
            case TBUS_OPT_ALIGN_SIZE:
                a_pstOption->iAlignSize = atoi(optarg);
                break;
#endif
            case TBUS_OPT_BACKUP_FILE:
                a_pstOption->pszBackFile = optarg;
                break;
            case TBUS_OPT_INCRIMENTAL:
                a_pstOption->iSetGCIMFlag |= TBUS_SET_GCIM_FLAG_RELATIVE;
                break;
            case TBUS_OPT_RECREATE:
                a_pstOption->iSetGCIMFlag |= TBUS_SET_GCIM_FLAG_RECREATE;
                break;
            case TBUS_OPT_DUMP_BINARY:
                a_pstOption->iDumpBinary = 1;
                break;
            case TBUS_OPT_WITH_SEPERATOR:
                a_pstOption->iWithSep = 0;
                break;
            case TBUS_OPT_START_MSG_IDX:
                a_pstOption->iStartMsgIndex = atoi(optarg);
                if (0 >= a_pstOption->iStartMsgIndex)
                {
                    fprintf(stderr, "Error: ָ������ʼ��Ϣ������Ч����Чֵ���� > 0\n");
                    exit(1);
                }
                break;
            case 'p':
                a_pstOption->iTestDataLen = atoi(optarg);
                break;
            case 'S':
            case 's':
                {
                    STRNCPY(a_pstOption->szProcID, optarg, sizeof(a_pstOption->szProcID));
                    a_pstOption->iMgrOP = TBUS_MGR_OP_VIEW;
                }
                break;
            case 'L':
            case 'l':
                {
                    a_pstOption->iMgrOP = TBUS_MGR_OP_LIST ;
                    break;
                }
            case 'k':
            case 'K':
                {
                    STRNCPY(a_pstOption->szShmKey, optarg, sizeof(a_pstOption->szShmKey)) ;
                    break;
                }
            case 'n':
                {
                    a_pstOption->iMaxSeeMsg = atoi(optarg) ;
                    break;
                }
            case 'B':
            case 'b':
                {
                    STRNCPY(a_pstOption->szMetalibFile, optarg, sizeof(a_pstOption->szMetalibFile));
                    break;
                }
            case 'm':
            case 'M':
                {
                    STRNCPY(a_pstOption->szMetaName, optarg, sizeof(a_pstOption->szMetaName));
                    break;
                }
            case 'o':
                {
                    STRNCPY(a_pstOption->szOutFile, optarg, sizeof(a_pstOption->szOutFile));
                    break;
                }
            case 'f':
                {
                    a_pstOption->iIsOldCnf = 1; /*�ϸ�ʽ*/
                    break;
                }
            case 'i':
                {
                    a_pstOption->iBusiID = strtol(optarg, NULL, 0);
                    break;
                }
            case 'g':
                {
                    a_pstOption->iIsDebug= 1; /*debug*/
                    break;
                }
            case 'v': /* show version. */
                fprintf(stdout, "%s:version %d.%d.%d, build at %d.\n", pszApp, MAJOR, MINOR, REV, BUILD);
                exit(0);

            case 'h':
            case '?':
                tbus_print_help(argv[0]);
                exit(0);
            default:
                break;
        }/*switch(opt)*/
    }

    return 0 ;
}

int backChannelInfo(IN LPTBUSSHMGCIM a_pstShmGCIM, LPTBUSGCIM a_pstGCIM, LPTBUSMGROPTIONS a_pstOption)
{
    int iRet = 0;
    FILE *fpBack = NULL;
    char szBackFile[512];

    assert(NULL != a_pstGCIM);
    assert(NULL != a_pstShmGCIM);
    assert(NULL != a_pstOption);

    if (NULL == a_pstOption->pszBackFile)
    {
        sprintf(szBackFile, "tbus_channels_%s.bak", a_pstGCIM->szGCIMShmKey);
        szBackFile[sizeof(szBackFile)-1] = '\0';
        a_pstOption->pszBackFile = szBackFile;
    }

    fpBack = fopen(a_pstOption->pszBackFile, "wb");
    if (NULL == fpBack)
    {
        fprintf(stderr, "warning: unable to open \"%s\" to write channels info\n",
                a_pstOption->pszBackFile);
        return -1;
    }
    tbus_list_gcim(a_pstShmGCIM, fpBack);
    fclose(fpBack);

    return iRet;
}

int initialize (LPTBUSMGROPTIONS a_pstOption, OUT LPTBUSGCIM a_pstGCIM, OUT LPTBUSSHMGCIM *a_ppShmGCIM)
{
    int iRet = TBUS_SUCCESS ;
    unsigned int iSize = 0 ;
    LPTDRMETALIB ptLib = NULL ;
    LPTDRMETA ptMeta = NULL ;
    TDRDATA stDataTemp ;

    assert(NULL != a_pstOption);
    assert(NULL != a_pstGCIM);
    assert(NULL != a_ppShmGCIM);

    /*��ʼ����־ϵͳ*/
    tbus_init_log();
    tbus_set_logpriority(TLOG_PRIORITY_TRACE);

    /*û��ָ��ͨ����Ĺ����ڴ�key������ҪдGCIM�����Զ�ȡ�����ļ�*/
    if (('\0' == a_pstOption->szShmKey[0]) || (TBUS_MGR_OP_WRITE == a_pstOption->iMgrOP))
    {
        if (!a_pstOption->iIsOldCnf)
        {
            ptLib = (LPTDRMETALIB) g_szMetalib_TbusConf ;
            ptMeta = tdr_get_meta_by_name ( ptLib, TBUS_GCIM_CONF_INFO ) ;
            if ( NULL == ptMeta )
            {
                fprintf ( stdout, "ERROR: initialize, tdr_get_meta_by_name(%s) failed\n",
                         TBUS_GCIM_CONF_INFO) ;
                return -1 ;
            }

            stDataTemp.iBuff = sizeof(TBUSGCIM) ;
            stDataTemp.pszBuff = (char *)a_pstGCIM ;
            iRet = tdr_input_file ( ptMeta, &stDataTemp, a_pstOption->szConfFile,
                                   0, TDR_IO_NEW_XML_VERSION ) ;
            if ( 0 > iRet )
            {
                fprintf ( stdout, "initialize, tdr_input_file() failed %s, error %s\n",
                         a_pstOption->szConfFile, tdr_error_string(iRet) ) ;
                return -1 ;
            }

            if (a_pstOption->iIsDebug)
                tdr_fprintf(ptMeta, stdout, &stDataTemp, 0);
        }else
        {
            iRet = load_config_of_old_format(a_pstOption, a_pstGCIM);
            if (0 != iRet)
            {
                return -1;
            }
        }/*if (!a_pstOption->iIsOldCnf)*/
        STRNCPY(a_pstOption->szShmKey, a_pstGCIM->szGCIMShmKey,
                sizeof(a_pstOption->szShmKey));
    }/*if ((0 > a_pstOption->iShmKey) || (TBUS_MGR_OP_WRITE == a_pstOption->iMgrOP))*/


    /* attach routes share memory */
    if (TBUS_MGR_OP_WRITE == a_pstOption->iMgrOP)
    {
        iSize = sizeof(TBUSSHMGCIM);
        iRet = tbus_create_gcimshm(a_ppShmGCIM, a_pstOption->szShmKey,
                                   a_pstGCIM->iBussinessID, iSize, NULL);
    }else
    {
        iRet = tbus_get_gcimshm(a_ppShmGCIM, a_pstOption->szShmKey,
                                a_pstGCIM->iBussinessID, iSize, 0, NULL);
    }
    if (0 != iRet)
    {
        fprintf(stdout, "failed to get GCIM share memory by shmkey(%s) and bussinessid %d\n",
                a_pstOption->szShmKey, a_pstGCIM->iBussinessID);
    }

    return iRet ;
}

int load_config_of_old_format(LPTBUSMGROPTIONS a_pstOption, OUT LPTBUSGCIM a_pstGCIM)
{
    TBUSMGR stTbusMgr;
    LPTDRMETALIB ptLib = NULL ;
    LPTDRMETA ptMeta = NULL ;
    TDRDATA stDataTemp ;
    int iRet = 0;
    unsigned int i;

    assert(NULL != a_pstOption);
    assert(NULL != a_pstGCIM);

    ptLib = (LPTDRMETALIB) g_szMetalib_TbusConf ;
    ptMeta = tdr_get_meta_by_name ( ptLib, TBUSMGR_CONFINFO ) ;
    if ( NULL == ptMeta )
    {
        fprintf ( stdout, "ERROR: initialize, tdr_get_meta_by_name(%s) failed\n",
                 TBUSMGR_CONFINFO) ;
        return -1 ;
    }

    memset(&stTbusMgr, 0, sizeof(stTbusMgr));
    stDataTemp.iBuff = sizeof(stTbusMgr) ;
    stDataTemp.pszBuff = (char *)&stTbusMgr ;
    iRet = tdr_input_file ( ptMeta, &stDataTemp, a_pstOption->szConfFile,
                           0, TDR_IO_NEW_XML_VERSION ) ;
    if ( 0 > iRet )
    {
        fprintf ( stdout, "initialize, tdr_input_file() failed %s, error %s\n",
                 a_pstOption->szConfFile, tdr_error_string(iRet) ) ;
        return -1 ;
    }

    /*convert old format data to new format*/
    a_pstGCIM->iBussinessID = TBUS_DEFAUL_BUSSINESS_ID;
    snprintf(a_pstGCIM->szGCIMShmKey, sizeof(a_pstGCIM->szGCIMShmKey), "%u", stTbusMgr.dwRoutesShmKey);
    STRNCPY(a_pstGCIM->szAddrTemplet, TBUS_DEFAULT_ADDRESS_TEMPLET, sizeof(a_pstGCIM->szAddrTemplet));
    for (i = 0; i < stTbusMgr.dwUsedCnt; i++)
    {
        LPCHANNELCNF pstChannel = &a_pstGCIM->astChannels[i];
        LPROUTEINFO pstRoute = &stTbusMgr.astRoutes[i];
        pstChannel->dwPriority = pstRoute->dwPriority;
        STRNCPY(pstChannel->aszAddress[0], pstRoute->szUpAddr, sizeof(pstChannel->aszAddress[0]));
        STRNCPY(pstChannel->aszAddress[1], pstRoute->szDownAddr, sizeof(pstChannel->aszAddress[1]));
        pstChannel->dwRecvSize = pstRoute->dwUpSize;
        pstChannel->dwSendSize = pstRoute->dwDownSize;
        pstChannel->dwAddressCount = 2;
        STRNCPY(pstChannel->szDesc, pstRoute->szDesc, sizeof(pstChannel->szDesc));
        pstChannel->iShmID = pstRoute->iShmID;
    }/*for (i = 0; i < stTbusMgr.dwUsedCnt; i++)*/
    a_pstGCIM->dwCount = i;

    return 0;
}



int tbus_list_gcim(IN LPTBUSSHMGCIM a_pShmGCIM, OUT FILE *a_fpOut)
{
    int iRet = TBUS_SUCCESS ;
    unsigned int i = 0 ;
    LPTBUSSHMGCIMHEAD pstHead ;
    LPTBUSSHMCHANNELCNF pstShmChl;

    assert(NULL != a_pShmGCIM);
    assert(NULL != a_fpOut);

    pstHead = &a_pShmGCIM->stHead;
    fprintf (a_fpOut, "\n========================\n");

    tbus_rdlock(&pstHead->stRWLock);

    /* share memory header list */
    fprintf (a_fpOut, "Shm version:\t%i\n", pstHead->dwVersion);
    fprintf (a_fpOut, "Shm key:\t%i\n", pstHead->dwShmKey);
    fprintf (a_fpOut, "Shm size:\t%i\n", pstHead->dwShmSize);
    fprintf (a_fpOut, "DataAlign:\t%d  \nAlignLevel:%d\n", pstHead->iAlign, pstHead->iAlignLevel);
    fprintf (a_fpOut, "Routes max count:\t%i\n", pstHead->dwMaxCnt);
    fprintf (a_fpOut, "Routes used count:\t%i\n", pstHead->dwUsedCnt);
    tbus_dump_addrtemplet(&pstHead->stAddrTemplet, a_fpOut);
    fprintf (a_fpOut, "\n");


    /* share memory routes information list */
    for ( i=0; i<pstHead->dwUsedCnt; i++ )
    {
        pstShmChl = &a_pShmGCIM->astChannels[i];
        fprintf (a_fpOut, "Channel %i information:\n", i+1);

        if (!TBUS_GCIM_CHANNEL_IS_ENABLE(pstShmChl))
        {
            fprintf (a_fpOut, "\tStatus:0x%x -- disable\n",  pstShmChl->dwFlag);
        }
        else
        {
            fprintf (a_fpOut, "\tStatus:0x%x -- enable\n", pstShmChl->dwFlag);
        }

        fprintf (a_fpOut, "\tPriority -- %i\n", pstShmChl->dwPriority);
        fprintf (a_fpOut, "\tShmID -- "PTR_OR_INT"\n", pstShmChl->iShmID);
        fprintf (a_fpOut, "\tAddress -- %s\n",
                 tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmChl->astAddrs[0]));
        fprintf (a_fpOut, "\tAddress -- %s\n",
                 tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmChl->astAddrs[1]));
        fprintf (a_fpOut, "\t[%s] RecvQueueSize -- %u\n",
                 tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmChl->astAddrs[0]),
                 pstShmChl->dwRecvSize);
        fprintf (a_fpOut, "\t[%s] SendQueueSize -- %u\n",
                 tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmChl->astAddrs[0]),
                 pstShmChl->dwSendSize);
        fprintf (a_fpOut, "\tCreateTime -- %s", ctime(&pstShmChl->dwCTime));
        if (!TBUS_GCIM_CHANNEL_IS_ENABLE(pstShmChl))
        {
            fprintf (a_fpOut, "\tDisableTime -- %s\n",  ctime(&pstShmChl->dwInvalidTime));
        }
        else
        {
            fprintf (a_fpOut, "\tDisableTime -- N/A\n");
        }
    }

    tbus_unlock(&pstHead->stRWLock);



    return iRet ;
}


int set (IN LPTBUSGCIM a_pstGCIM, IN LPTBUSSHMGCIM a_pShmGCIM, IN int a_iFlag)
{
    int iRet = TBUS_SUCCESS;

    iRet = tbus_set_gcim_ex(a_pShmGCIM, a_pstGCIM, a_iFlag);
    if (TBUS_SUCCESS != iRet)
    {
        fprintf (stdout, "\nChannels writing failed, for %s\n\n", tbus_error_string(iRet)) ;
    }else
    {
        tbus_list_gcim(a_pShmGCIM, stdout);
        fprintf (stdout, "\nChannels writing...... done\n\n");
    }

    return iRet ;
}

int clean ( IN LPTBUSSHMGCIM a_pShmGCIM, int a_idx )
{
    int iRet = TBUS_SUCCESS;
    LPTBUSSHMGCIMHEAD pstHead = NULL;
    LPTBUSSHMCHANNELCNF pstShmChl = NULL;
    TBUSCHANNEL stChannel;
    int iIsInUse = 0;

    assert(NULL != a_pShmGCIM);

    pstHead = &a_pShmGCIM->stHead;
    if ((1 > a_idx) || (a_idx > (int)pstHead->dwUsedCnt))
    {
        fprintf(stdout, "\nError: channel<index: %d> nonexists\n\n", a_idx);
        tbus_log(TLOG_PRIORITY_ERROR,"invalid channel index(%d), it must be in 0~%d",
                 a_idx, pstHead->dwUsedCnt);
        return TBUS_ERR_ARG;
    }

    pstShmChl = &a_pShmGCIM->astChannels[a_idx-1];
    iRet = tbus_is_channel_in_use(pstShmChl, pstHead, &iIsInUse);
    if (0 > iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_is_channel_in_use failed, shmid "PTR_OR_INT", %s", pstShmChl->iShmID,
                 strerror(errno));
        fprintf(stdout, "\nError: shmid IPC_STAT failed\n\n");
        return iRet;
    }

    if (iIsInUse)
    {
        fprintf(stdout, "\nError:\nThe channel<index:%d> is using by some processes, "
                "before clean this channel, please stop the processes which use it\n\n", a_idx);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_DELETE_USED_CHANNEL);
    }

    iRet = AttachChannel(pstShmChl, &stChannel, &a_pShmGCIM->stHead);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "failed to attach shm<iShmID:"PTR_OR_INT"> for channel<index:%d>\n",
                 pstShmChl->iShmID, a_idx);
        return iRet;
    }

    iRet = tbus_channel_clean(&stChannel);
    if (TBUS_SUCCESS != iRet)
    {
        fprintf ( stdout, "\nfailed to clean Specified 'channel(index: %i)' \n\n", a_idx) ;
        tbus_log(TLOG_PRIORITY_ERROR, "failed to clean channel<index:%d, iShmID:%d>\n",
                 a_idx, pstShmChl->iShmID);
        return iRet;
    }
    else
    {
        fprintf ( stdout, "\nClean specified 'channel(index: %i)'...... done\n\n", a_idx) ;
    }

    return iRet;
}

int del ( IN LPTBUSSHMGCIM a_pShmGCIM, int a_iID )
{
    int iRet = TBUS_SUCCESS ;

    iRet = tbus_delete_channel_by_index(a_pShmGCIM, a_iID-1);
    if (TBUS_SUCCESS != iRet)
    {
        if (TBUS_ERR_GET_ERROR_CODE(iRet) == TBUS_ERROR_DELETE_USED_CHANNEL)
        {
            fprintf ( stdout, "\nError:\nThe channel<index:%d> is using by some processes, before "
                     "delete this channel, please stop the processes which use it\n\n", a_iID) ;
        }else
        {
            fprintf ( stdout, "\nError:\nfailed to delete Specified 'channel(index: %i)' \n\n", a_iID) ;
        }

    }else
    {
        fprintf ( stdout, "\nSpecified 'channel(index: %i)' deleted...... done\n\n", a_iID) ;
    }

    return iRet ;
}

int tbus_delete_all_channles (IN LPTBUSSHMGCIM a_pShmGCIM)
{
    int iRet = TBUS_SUCCESS ;
    int iIndex = 0;
    int iDelCount = 0;
    int iCount = 0;
    int iInUseCount = 0;

    assert(NULL != a_pShmGCIM);

    iCount = (int)a_pShmGCIM->stHead.dwUsedCnt;

    for (iIndex = 1; iIndex <= iCount; iIndex++)
    {
        iRet = tbus_delete_channel_by_index(a_pShmGCIM, iInUseCount);
        if (TBUS_SUCCESS != iRet)
        {
            if (TBUS_ERR_GET_ERROR_CODE(iRet) == TBUS_ERROR_DELETE_USED_CHANNEL)
            {
                fprintf ( stdout, "\nError:\nThe channel<index:%d> is using by some processes, before "
                         "delete this channel, please stop the processes which use it\n", iIndex) ;
            }else
            {
                fprintf ( stdout, "\nError:\nfailed to delete Specified 'channel(index: %i)' \n\n", iIndex) ;
            }
            iInUseCount++;
            continue;
        }
        iDelCount++;
    }

    // not all channels have been removed
    if (iDelCount < iCount)
        return -1;
    else
        return 0;

}

#if !defined(_WIN32) && !defined(_WIN64)
int tbus_cal_channel_size(IN LPTBUSMGROPTIONS a_pstOption)
{
    int iRet = 0;
    int iChannelSize = 0;
    int iChannelHeadLen = 0;

    assert(NULL != a_pstOption);

    if (0 >= a_pstOption->iSendBufSize)
    {
        printf("Error: --send-buf-size=SIZE not specified, or specified value invalid\n");
        return -1;
    }

    if (0 >= a_pstOption->iRecvBufSize)
    {
        printf("Error: --recv-buf-size=SIZE not specified, or specified value invalid\n");
        return -1;
    }

    iChannelHeadLen = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, a_pstOption->iAlignSize);
    iChannelSize = iChannelHeadLen + a_pstOption->iRecvBufSize + a_pstOption->iSendBufSize;

    iChannelSize += TBUS_SHM_CTRL_CHNNL_SIZE * 2;

    printf("\nCHANNEL SIZE: %d\n\n", iChannelSize);

    return iRet;
}
#endif

int SendTestMsg(IN LPTBUSSHMGCIM a_pShmGCIM, LPTBUSMGROPTIONS a_pstOption)
{
    int iRet =0;
    int iBusHandle;
    TBUSADDR iSrcAddr;
    TBUSADDR iDstAddr;
    char szBuff[TBUS_MAX_TEST_DATA_LEN];

    assert(NULL != a_pShmGCIM);
    assert(NULL != a_pstOption);

    if ('\0' == a_pstOption->szTbusSrcAddr[0])
    {
        printf("Error: --src=TbusId not specified, or specified value invalid.\n");
        return -1;
    }

    if ('\0' == a_pstOption->szTbusDstAddr[0])
    {
        printf("Error: --dst=TbusId not specified, or specified value invalid.\n");
        return -1;
    }

    if (0 >= a_pstOption->iTestDataLen || TBUS_MAX_TEST_DATA_LEN < a_pstOption->iTestDataLen)
    {
        fprintf(stdout, "Error: --data-len=LEN not specified, or specified value beyond [1-%d]\n",
                TBUS_MAX_TEST_DATA_LEN);
        return -1;
    }

    iRet = tbus_initialize(a_pstOption->szShmKey, a_pShmGCIM->stHead.dwBusiID);
    if (TBUS_SUCCESS != iRet)
    {
        printf("tbus init error\n");
        return -1;
    }

    iRet = tbus_new(&iBusHandle);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_fini();
        printf("tbus new error, for %s\n", tbus_error_string(iRet));
        return -1;
    }

    iRet = tbus_addr_aton(&a_pstOption->szTbusSrcAddr[0], &iSrcAddr);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_delete(&iBusHandle);
        tbus_fini();
        printf("failed to convert %s to tbus address, for %s\n", a_pstOption->szTbusSrcAddr, tbus_error_string(iRet));
        return -1;
    }

    iRet = tbus_addr_aton(&a_pstOption->szTbusDstAddr[0], &iDstAddr);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_delete(&iBusHandle);
        tbus_fini();
        printf("failed to convert %s to tbus address, for %s\n", a_pstOption->szTbusDstAddr, tbus_error_string(iRet));
        return -1;
    }

    iRet = tbus_bind(iBusHandle, iSrcAddr);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_delete(&iBusHandle);
        tbus_fini();
        printf("tbus bind error by addr %s\n", a_pstOption->szTbusSrcAddr);
        return -1;
    }

    memset(szBuff, 'T', a_pstOption->iTestDataLen);
    szBuff[a_pstOption->iTestDataLen-1] = 0;
    iRet = tbus_send(iBusHandle, &iSrcAddr, &iDstAddr, szBuff, a_pstOption->iTestDataLen, TBUS_FLAG_SET_TEST_MSG);
    if (TBUS_SUCCESS != iRet)
    {
        fprintf(stdout, "tbus_send error, for %s\n", tbus_error_string(iRet));
        tbus_delete(&iBusHandle);
        tbus_fini();
    }

    tbus_delete(&iBusHandle);
    tbus_fini();

    return 0;
}

int setDefaultOptions(IN LPTBUSMGROPTIONS a_pstOption)
{
    int iRet = 0;

    assert(NULL != a_pstOption);

    a_pstOption->iMgrOP = TBUS_MGR_OP_LIST;
    STRNCPY(a_pstOption->szConfFile, TBUS_MGR_CONF_XML, sizeof(a_pstOption->szConfFile));
    a_pstOption->iDelID = -1;
    a_pstOption->iMaxSeeMsg = TBUS_MGR_DEFAULT_SEE_MAX_NUM;
    a_pstOption->iAlignSize = TBUS_DEFAULT_CHANNEL_DATA_ALIGN;
    a_pstOption->iStartMsgIndex = 1;
    a_pstOption->iWithSep = 1;

    return iRet;
}
