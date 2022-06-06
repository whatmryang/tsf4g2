/***********************************************************
 * FileName: tapp_proc_ctrlpkg.c
 * Author: jackyai, flyma
 * Version: 1.0
 * Date: 2010-01-29
 * Function List:
 *      tapp_get_controller_env
 *      tapp_controller_init
 *      tapp_controller_proc
 *      tapp_controller_fini
 *      tapp_controller_authen_req
 *      tapp_controller_parse_input
 *      tapp_controller_parse_cmdline
 *      tapp_controller_dispatch_input
 *      tapp_controller_dispatch_msg
 *      tapp_controller_help
 *      tapp_controller_cmdline_proc
 *      tapp_controller_sendbin_proc
 *      tapp_controller_show_bus_handle_proc
 *      tapp_controller_show_bus_channel_proc
 *      tapp_controller_show_by_addrs_proc
 *      tapp_controller_show_by_key_proc
 *      tapp_controller_init_meta
 *      tapp_controller_auth
 *      tapp_controller_proc_bus_res
 * Description:
 *      this source file implements logic of tapp-ctrl's console
 * History:
 *      <time>        <author>      <version>   <desc>
 *      2010-01-26     jackyai       1.0
 *      2010-01-29     flyma         1.0
 * ***********************************************************/

#include "tapp_controller.h"
#include "tappctrl_kernel_i.h"

#define _CRT_SECURE_NO_WARNINGS

#define TAPPCTRL_CONN_TIME_LIMIT 3000

#define TAPP_CTRL_TBUS_GLOBAL_META  "CITbusGlobal"
#define TAPP_CTRL_TBUS_HANDLE_META  "CITbusHandle"
#define TAPP_CTRL_TBUS_CHANNEL_META "CIShowChnnlRes"
#define TAPP_CTRL_TBUS_CHNNL_HEAD_META "CICHANNELHEAD"

struct tagTappCtrlTempVar
{
    LPTDRMETALIB pstMetaLib;
    LPTDRMETA pstMeta;
    int argc;
    const char* argv[TAPPCTRL_MAX_ARGC];
    char pLineBuf[TAPPCTRL_INPUT_LINE_BUF_LEN];
};

extern LPTLOGCATEGORYINST g_pstTappCtrlLogCat;
extern LPTDRMETALIB g_pstTappProtoMetaLib;
static struct tagTappCtrlTempVar gs_stTempVar;

static int tapp_controller_authen_req(const char* a_pszCIAddr);
static void tapp_controller_parse_cmdline(char* pLine, int* piArgc, const char **argv);
static void tapp_controller_parse_input(TAPPCTX *a_pstCtx);
static int tapp_controller_dispatch_input(short a_nCmdID);
static void tapp_controller_dispatch_msg(TAPPCTX *a_pstCtx, LPCOMMANDINJECTPKG a_pstCIMsg);
static void tapp_controller_help();
static int tapp_controller_cmdline_proc();
static int tapp_controller_sendbin_proc();
static int tapp_controller_show_bus_handle_proc();
static int tapp_controller_show_bus_channel_proc();
static int tapp_controller_show_by_addrs_proc();
static int tapp_controller_show_by_key_proc();
static int tapp_controller_init_meta();
static void tapp_controller_proc_bus_res(char * a_pBuf, unsigned a_uLen, const char* a_pszMetaName);
int tapp_controller_auth(const char* a_pszCIAddr);
int tapp_controller_init(TAPPCTX *a_pstCtx, void* a_pvArg);

/////////////////////////////////////////////////////
#define DECLA


static int tapp_controller_init_meta()
{
    int iRet = 0;

    iRet = tdr_load_metalib(&gs_stTempVar.pstMetaLib, gs_stTempVar.argv[1]);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "tdr_load_metalib failed, for %s\n", tdr_error_string(iRet));
        return -1;
    }

    if (TAPPCTRL_MAX_META_NAME_LEN <= strlen(gs_stTempVar.argv[2]))
    {
        tdr_free_lib(&gs_stTempVar.pstMetaLib);
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "meta name \'%s\' is longer than %d", gs_stTempVar.argv[2], TAPPCTRL_MAX_META_NAME_LEN-1);
        return -1;
    }

    gs_stTempVar.pstMeta = tdr_get_meta_by_name(gs_stTempVar.pstMetaLib, gs_stTempVar.argv[2]);
    if (NULL == gs_stTempVar.pstMeta)
    {
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "tdr_get_meta_by_name failed, name<%s>\n", gs_stTempVar.argv[2]);
        tdr_free_lib(&gs_stTempVar.pstMetaLib);
        return -1;
    }

    return iRet;
}


int tapp_controller_init(TAPPCTX *a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    if (NULL != a_pstCtx->pfnControllerInit)
    {
        iRet = a_pstCtx->pfnControllerInit(a_pstCtx, a_pvArg);
        if (0 > iRet)
        {
            printf("Error: controller init function(a_pstCtx->pfnControllerInit) failed");
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "a_pstCtx->pfnControllerInit return %d", iRet);
            return iRet;
        }
    }

    printf("# max input line length: %d characters\n", TAPPCTRL_INPUT_LINE_BUF_LEN);
    printf("# input \'help\' for usage information\n");
    printf("\n> ");
    tlog_info(g_pstTappCtrlLogCat, 0, 0, "tappctrl start");
	return iRet;
}

static void tapp_controller_help()
{
    printf("\nCOMMAND LIST:\n");
    printf("quit\t\t\t# exit this program\n");
    printf("help\t\t\t# show this information\n");
    printf("techo\t\t\t# can be used to test connection to server\n");
    printf("tgetconf\t\t# get and display server's conf data\n");
    printf("tgetcumulate\t\t# get and display runtime cumulate data\n");
    printf("tgetstatus\t\t# get and display runtime status data\n");
    printf("treload\t\t\t# reload conf data\n");
    printf("tbusglobal\t\t# get and display tbus global data\n");
    printf("tshowhandle\t\t# get and display a tbus handle's data\n");
    printf("\t\t\t# format:  tshowhandle #handle_index\n");
    printf("\t\t\t# example: tshowhandle 1\n");
    printf("tshowchannelbyindex\t# get and display a tbus channel's data by indexes\n");
    printf("\t\t\t# format:  tshowchannelbyindex #handle_index #channel_index\n");
    printf("\t\t\t# example: tshowchannelbyindex  1  1\n");
    printf("tshowchannelbyaddrs\t# get and display a tbus channel's data by addrs\n");
    printf("\t\t\t# format:  tshowchannelbyaddrs #handle_index #channel_addr1 #channel_addr2\n");
    printf("\t\t\t# example: tshowchannelbyaddrs  1  1.1.1.1  1.1.2.1\n");
    printf("tshowsimplechannel\t# get and display simple channel mode's data\n");
    printf("\t\t\t# format:  tshowsimplechannel key [business_id]\n");
    printf("\t\t\t# example: tshowsimplechannel 16688\n");
    printf("tsendbin\t\t# send binary pkg\n");
    printf("\t\t\t# format:  tsendbin tdr_file meta_name data_file [ATTR | LIST]\n");
    printf("\t\t\t# example: tsendbin desc.tdr Pkg data.xml\n");
    printf("tshowlogconf\t\t# show tlog config\n");
}

static int tapp_controller_cmdline_proc()
{
    int i = 0;
    size_t tLen;
    COMMANDINJECTPKG stMsg;

    if (TAPPCTRL_MAX_ARGC < gs_stTempVar.argc)
    {
        printf("argument numbers if more than %d\n", gs_stTempVar.argc);
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "argument numbers if more than %d\n", gs_stTempVar.argc);
        return -1;
    }

    for (i = 0; i < gs_stTempVar.argc; i++)
    {
        tLen = strlen(gs_stTempVar.argv[i]);
        if (TAPPCTRL_MAX_ARGV_LEN <= tLen)
        {
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "arg: \'%s\' is too long<size: %"PRIdPTR">, max arg len is %d\n",
                    gs_stTempVar.argv[i], tLen, TAPPCTRL_MAX_ARGV_LEN-1);
            printf("arg: \'%s\' is too long<size: %"PRIdPTR">, max arg len is %d\n", gs_stTempVar.argv[i], tLen, TAPPCTRL_MAX_ARGV_LEN-1);
            return -1;
        }
        strcpy(stMsg.stBody.stCmdLineReq.aszArgv[i], gs_stTempVar.argv[i]);
    }
    tappctrl_fill_pkghead(&stMsg.stHead, TAPP_CMDLINE_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
    stMsg.stBody.stCmdLineReq.wArgc = (uint16_t)gs_stTempVar.argc;
    return tappctrl_send_pkg(&stMsg);
}


static int tapp_controller_sendbin_proc()
{
    int iRet = 0;
    int iNeedFree = 0;
    int iXmlFormat = TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME;
    COMMANDINJECTPKG stMsg;
    TDRDATA stData;

    iRet = tapp_controller_init_meta();
    if (0 > iRet)
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "sendbin failed, for tappctrl_init_meta error\n");
    else
        iNeedFree = 1;

    if (0 == iRet)
    {
        stData.pszBuff = (char*)&stMsg.stBody.stBinDataReq.szData;
        stData.iBuff = sizeof(stMsg.stBody.stBinDataReq.szData);
        if (4 < gs_stTempVar.argc && !strcasecmp("ATTR", gs_stTempVar.argv[4]))
            iXmlFormat = TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME;
        iRet = tdr_input_file(gs_stTempVar.pstMeta, &stData, gs_stTempVar.argv[3], 0, iXmlFormat);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "sendbin failed, for tdr_input_file error: %s\n", tdr_error_string(iRet));
        }else
        {
            tappctrl_fill_pkghead(&stMsg.stHead, TAPP_BINARY_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
            strcpy((char*)&stMsg.stBody.stBinDataReq.szMetaName, gs_stTempVar.argv[2]);
            stMsg.stBody.stBinDataReq.wDataLen = (uint16_t)stData.iBuff;
            iRet = tappctrl_send_pkg(&stMsg);
        }
    }

    if (iNeedFree)
        tdr_free_lib(&gs_stTempVar.pstMetaLib);
    return iRet;
}


TSF4G_API int tapp_controller_proc(TAPPCTX *a_pstCtx, void* a_pvArg)
{

    int iRet = 0;
    int iBusy = 0;
    fd_set fds_read;
    struct timeval tvMon;
    int iOpenFds = 0;
    TOS_UNUSED_ARG(a_pvArg);
    // read cmd line from stdin
    FD_ZERO( &fds_read );
    tvMon.tv_sec = 0;
    tvMon.tv_usec = 1000;

    FD_SET(fileno(stdin), &fds_read);
    iOpenFds = select(FD_SETSIZE, &fds_read, NULL, NULL, &tvMon);

    if( iOpenFds && FD_ISSET(fileno(stdin), &fds_read) )
    {
        iBusy = 1;
        if (NULL != fgets(gs_stTempVar.pLineBuf, TAPPCTRL_INPUT_LINE_BUF_LEN, stdin))
        {
            tapp_controller_parse_input(a_pstCtx);
        }
    }

    while(1)
    {
        COMMANDINJECTPKG stMsg;
        iRet = tappctrl_recv_pkg(&stMsg);
        if ( 0 == iRet)
        {
            iBusy = 1;
            printf("\n++++++++\n");
            tapp_controller_dispatch_msg(a_pstCtx, &stMsg);
            printf("\n> ");
        }else
        {
            break;
        }
    }

    if (iBusy)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

TSF4G_API int tapp_controller_fini(TAPPCTX *a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    if (NULL != a_pstCtx->pfnControllerFini)
    {
        iRet = a_pstCtx->pfnControllerFini(a_pstCtx, a_pvArg);
        if (0 > iRet)
        {
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "a_pstCtx->pfnControllerFini return %d", iRet);
            return iRet;
        }
    }

	return iRet;
}

static int tapp_controller_authen_req(const char* a_pszCIAddr)
{
    int iRet = 0;
    COMMANDINJECTPKG stReqPkg;

    tappctrl_fill_pkghead(&stReqPkg.stHead, TAPP_AUTHEN_REQ, TDR_METALIB_TAPP_CTRL_VERSION);

    STRNCPY(stReqPkg.stBody.szAuthenReq, a_pszCIAddr, sizeof(stReqPkg.stBody.szAuthenReq));

    iRet = tappctrl_send_pkg(&stReqPkg);

    return iRet;
}

static int tapp_controller_show_bus_channel_proc()
{
    int iRet;
    int iHandleIndex;
    int iChannelIndex;
    COMMANDINJECTPKG stMsg;

    iHandleIndex = strtol(gs_stTempVar.argv[1], NULL, 0);
    if (0 >= iHandleIndex)
    {
        printf("Tbus handle index should be greater than 0, but %s was given.\n", gs_stTempVar.argv[1]);
        return -1;
    }

    iChannelIndex = strtol(gs_stTempVar.argv[2], NULL, 0);
    if (0 >= iChannelIndex)
    {
        printf("Tbus channel index should be greater than 0, but %s was given.\n", gs_stTempVar.argv[1]);
        return -1;
    }

    tappctrl_fill_pkghead(&stMsg.stHead, TAPP_BUS_SHOW_CHNNL_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
    stMsg.stBody.stBusChannelReq.wHandleIndex  = (uint16_t)(iHandleIndex);
    stMsg.stBody.stBusChannelReq.wChannelIndex = (uint16_t)(iChannelIndex - 1);

    iRet = tappctrl_send_pkg(&stMsg);

    return iRet;
}


static void tapp_controller_parse_input(TAPPCTX *a_pstCtx)
{
    int iRet = 0;
    short nCmdID;
    int iInterest = 0;

    tlog_trace(g_pstTappCtrlLogCat, 0, 0, "get standard input: %s", gs_stTempVar.pLineBuf);
    tapp_controller_parse_cmdline(gs_stTempVar.pLineBuf, &gs_stTempVar.argc, gs_stTempVar.argv);

    if (0 < gs_stTempVar.argc)
    {
        if (0 == strcasecmp(gs_stTempVar.argv[0], "quit"))
        {
            tapp_exit_mainloop();
            return;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "techo"))
        {
            nCmdID = TAPP_ECHO_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "help"))
        {
            nCmdID = TAPP_HELP_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tgetconf"))
        {
            nCmdID = TAPP_GET_CONF_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tgetcumulate"))
        {
            nCmdID = TAPP_GET_CUMULATE_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tgetstatus"))
        {
            nCmdID = TAPP_GET_STATUS_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "treload"))
        {
            nCmdID = TAPP_RELOAD_CONF_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tbusglobal"))
        {
            nCmdID = TAPP_BUS_GLOBAL_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tshowhandle"))
        {
            if (2 > gs_stTempVar.argc)
            {
                printf("Invalid command format, valid format:\n");
                printf("tshowhandle #handle_index\n");
                printf("> ");
                return;
            }
            nCmdID = TAPP_BUS_HANDLE_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tshowchannelbyindex"))
        {
            if (3 > gs_stTempVar.argc)
            {
                printf("Invalid command format, valid format:\n");
                printf("tshowchannelbyindex #handle_index #channel_index\n");
                printf("> ");
                return;
            }
            nCmdID = TAPP_BUS_SHOW_CHNNL_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tshowchannelbyaddrs"))
        {
            if (4 > gs_stTempVar.argc)
            {
                printf("Invalid command format, valid format:\n");
                printf("tshowchannelbyaddrs #handle_index #channel_addr1 #channel_addr2\n");
                printf("> ");
                return;
            }
            nCmdID = TAPP_BUS_SHOW_BY_ADDR_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tshowsimplechannel"))
        {
            if (2 > gs_stTempVar.argc)
            {
                printf("Invalid command format, valid format:\n");
                printf("tshowsimplechannel key [business_id]\n");
                printf("> ");
                return;
            }
            nCmdID = TAPP_BUS_SHOW_BY_KEY_REQ;
        }else if (0 == strcasecmp(gs_stTempVar.argv[0], "tsendbin"))
        {
            if (4 > gs_stTempVar.argc)
            {
                printf("Invalid command format, valid format:\n");
                printf("tsendbin tdr_file meta_name data_file [ATTR | LIST]\n");
                printf("> ");
                return;
            }
            nCmdID = TAPP_BINARY_REQ;
        }else if(0 == strcasecmp(gs_stTempVar.argv[0], "tshowlogconf"))
        {
        	nCmdID = TAPP_LOG_SHOW_CONF_REQ;
        }
        else
        {
            if (NULL != a_pstCtx->pfnPreprocCmdLine)
                iInterest = a_pstCtx->pfnPreprocCmdLine((uint16_t)gs_stTempVar.argc, gs_stTempVar.argv);
            printf("> ");
            if (0 == iInterest)
                nCmdID = TAPP_CMDLINE_REQ;
            else
                return;
        }

        iRet = tapp_controller_dispatch_input(nCmdID);
        if (0 > iRet)
            printf("Error happened when processing input.\n\n> ");
    }else
    {
        printf("> ");
    }
}

static void tapp_controller_parse_cmdline(char* pLine, int* piArgc, const char **argv)
{
    int iScanStart = 0;

    *piArgc = 0;
    iScanStart = 1;
    while ('\0' != *pLine)
    {
        if (!isspace(*pLine))
        {
            if (iScanStart)
            {
                argv[*piArgc] = pLine;
                ++*piArgc;
                iScanStart = 0;
            }
        }else
        {
            if (!iScanStart)
            {
                *pLine = '\0';
                iScanStart = 1;
            }
        }
        ++pLine;
    }
}


static int tapp_controller_show_bus_handle_proc()
{
    int iRet;
    int iIndex;
    COMMANDINJECTPKG stMsg;

    iIndex = strtol(gs_stTempVar.argv[1], NULL, 0);
    if (0 >= iIndex)
    {
        printf("Tbus handle index should be greater than 0, but %s was given.\n", gs_stTempVar.argv[1]);
        return -1;
    }

    tappctrl_fill_pkghead(&stMsg.stHead, TAPP_BUS_HANDLE_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
    stMsg.stBody.dwBusHandleReq = iIndex;

    iRet = tappctrl_send_pkg(&stMsg);

    return iRet;
}


static int tapp_controller_show_by_addrs_proc()
{
    int iRet;
    int iHandleIndex;
    COMMANDINJECTPKG stMsg;
    int iWriteLen;

    iHandleIndex = strtol(gs_stTempVar.argv[1], NULL, 0);
    if (0 >= iHandleIndex)
    {
        printf("Tbus handle index should be greater than 0, but %s was given.\n", gs_stTempVar.argv[1]);
        return -1;
    }

    iWriteLen = snprintf(stMsg.stBody.stShowByAddrReq.szChnnlAddr1,
            sizeof(stMsg.stBody.stShowByAddrReq.szChnnlAddr1),"%s", gs_stTempVar.argv[2]);
    if ((iWriteLen < 0)||(iWriteLen >= (int)sizeof(stMsg.stBody.stShowByAddrReq.szChnnlAddr1)))
    {
        printf("Error: channel addr_1 is too long<%"PRIdPTR">, max supported length is %"PRIdPTR".\n",
                strlen(gs_stTempVar.argv[2]), sizeof(stMsg.stBody.stShowByAddrReq.szChnnlAddr1) - 1);
        return -1;
    }

    iWriteLen = snprintf(stMsg.stBody.stShowByAddrReq.szChnnlAddr2,
            sizeof(stMsg.stBody.stShowByAddrReq.szChnnlAddr1),"%s", gs_stTempVar.argv[3]);
    if ((iWriteLen < 0)||(iWriteLen >= (int)sizeof(stMsg.stBody.stShowByAddrReq.szChnnlAddr1)))
    {
        printf("Error: channel addr_2 is too long<%"PRIdPTR">, max supported length is %"PRIdPTR".\n",
                strlen(gs_stTempVar.argv[3]), sizeof(stMsg.stBody.stShowByAddrReq.szChnnlAddr2) - 1);
        return -1;
    }

    tappctrl_fill_pkghead(&stMsg.stHead, TAPP_BUS_SHOW_BY_ADDR_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
    stMsg.stBody.stShowByAddrReq.dwHandleIndex  = iHandleIndex;

    iRet = tappctrl_send_pkg(&stMsg);

    return iRet;
}


static int tapp_controller_show_by_key_proc()
{
    int iRet;
    COMMANDINJECTPKG stMsg;
    int iKey;

    printf("\n");
    if (2 < gs_stTempVar.argc)
    {
        int iBussID;

        iBussID = strtol(gs_stTempVar.argv[2], NULL, 0);
        printf("id:  [%s ------> %d]\n", gs_stTempVar.argv[2], iBussID);
        iRet = tbus_gen_shmkey((key_t*)&iKey, gs_stTempVar.argv[1], iBussID);
        if (TBUS_ERR_IS_ERROR(iRet))
        {
            printf("Error: failed to generate key by (\'%s\', %d), for %s\n> ",
                    gs_stTempVar.argv[1], iBussID, tbus_error_string(iRet));
            return -1;
        }
        printf("key: [(%s, %d) ----------> %d]\n", gs_stTempVar.argv[1], iBussID, iKey);
    }
    else
    {
        iRet = tbus_gen_shmkey((key_t*)&iKey, gs_stTempVar.argv[1], 0);
        if (TBUS_ERR_IS_ERROR(iRet))
        {
            printf("Error: failed to generate key by \'%s\', for %s\n> ",
                    gs_stTempVar.argv[1], tbus_error_string(iRet));
            return -1;
        }
        printf("key: [%s ----------> %d]\n", gs_stTempVar.argv[1], iKey);
    }

    tappctrl_fill_pkghead(&stMsg.stHead, TAPP_BUS_SHOW_BY_KEY_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
    stMsg.stBody.iShowByKeyReq  = iKey;

    iRet = tappctrl_send_pkg(&stMsg);

    return iRet;
}



static int tapp_controller_show_log_conf_proc()
{
    int iRet;
    COMMANDINJECTPKG stMsg;
	memset(&stMsg,0,sizeof(stMsg));
    tappctrl_fill_pkghead(&stMsg.stHead, TAPP_LOG_SHOW_CONF_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
    iRet = tappctrl_send_pkg(&stMsg);
  	//dbgprintf("Send package done, %d\n",iRet);
	return iRet;
}

static int tapp_controller_dispatch_input(short a_nCmdID)
{
    int iRet = 0;

    switch (a_nCmdID)
    {
        case TAPP_ECHO_REQ:
            return tapp_controller_send_req_i(TAPP_ECHO_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
            break;
        case TAPP_HELP_REQ:
            tapp_controller_help();
            return tapp_controller_send_req_i(TAPP_HELP_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
            break;
        case TAPP_BINARY_REQ:
            return tapp_controller_sendbin_proc();
            break;
        case TAPP_CMDLINE_REQ:
            return tapp_controller_cmdline_proc();
            break;
        case TAPP_GET_CONF_REQ:
            return tapp_controller_send_req_i(TAPP_GET_CONF_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
            break;
        case TAPP_GET_CUMULATE_REQ:
            return tapp_controller_send_req_i(TAPP_GET_CUMULATE_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
            break;
        case TAPP_GET_STATUS_REQ:
            return tapp_controller_send_req_i(TAPP_GET_STATUS_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
            break;
        case TAPP_RELOAD_CONF_REQ:
            return tapp_controller_send_req_i(TAPP_RELOAD_CONF_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
            break;
        case TAPP_BUS_GLOBAL_REQ:
            return tapp_controller_send_req_i(TAPP_BUS_GLOBAL_REQ, TDR_METALIB_TAPP_CTRL_VERSION);
            break;
        case TAPP_BUS_HANDLE_REQ:
            return tapp_controller_show_bus_handle_proc();
        case TAPP_BUS_SHOW_CHNNL_REQ:
            return tapp_controller_show_bus_channel_proc();
            break;
        case TAPP_BUS_SHOW_BY_ADDR_REQ:
            return tapp_controller_show_by_addrs_proc();
            break;
        case TAPP_BUS_SHOW_BY_KEY_REQ:
            return tapp_controller_show_by_key_proc();
            break;
        case TAPP_LOG_SHOW_CONF_REQ:
            return tapp_controller_show_log_conf_proc();
            break;
        default:
            break;
    }

    return iRet;
}

static void tapp_controller_dispatch_msg(TAPPCTX *a_pstCtx, LPCOMMANDINJECTPKG a_pstCIMsg)
{
    LPCIHEAD pstHead;
    LPCIBODY pstBody;
    LPCIBINDATARES pstBinData;

    pstHead = &a_pstCIMsg->stHead;
    pstBody = &a_pstCIMsg->stBody;

    // process recved msg
    switch (pstHead->nCmdID)
    {
        case TAPP_STRING:
            printf("\n%s\n", (char*)pstBody);
            tlog_info(g_pstTappCtrlLogCat, 0, 0, "msg from server: %s\n", (char*)pstBody);
            break;
        case TAPP_HELP_RES:
            printf("HELP FROM SERVER:\n%s\n", (char*)pstBody);
            break;
        case TAPP_ECHO_RES:
            printf("%s\n", (char*)pstBody);
            break;
        case TAPP_CMDLINE_RES:
        case TAPP_BINARY_RES:
            pstBinData = (LPCIBINDATARES)pstBody;
            if (NULL != a_pstCtx->pfnProcCtrlRes)
            {
                a_pstCtx->pfnProcCtrlRes(pstBinData->szMetaName,
                        (char*)&pstBinData->szData, (int)pstBinData->wDataLen);
            }
            else
            {
                printf("Recv a tapp_binary msg, but TAPPCTX->pfnProcCtrlRes not set.\n");
                tlog_trace(g_pstTappCtrlLogCat, 0, 0, "recv a TAPP_BINARY pkg, no pfnBinMsgProcess, drop it");
            }
            break;
        case TAPP_GET_CONF_RES:
            printf("CONF DATA FROM SERVER:\n%s", (char*)pstBody);
            break;
        case TAPP_GET_CUMULATE_RES:
            printf("RUNTIME CUMULATE DATA FROM SERVER:\n%s", (char*)pstBody);
            break;
        case TAPP_GET_STATUS_RES:
            printf("RUNTIME STATUS DATA FROM SERVER:\n%s", (char*)pstBody);
            break;
        case TAPP_RELOAD_CONF_RES:
            printf("MSG FROM SERVER:\n%s\n", (char*)pstBody);
            break;
        case TAPP_BUS_GLOBAL_RES:
            tapp_controller_proc_bus_res((char*)pstBody, sizeof(CITBUSGLOBAL), TAPP_CTRL_TBUS_GLOBAL_META);
            break;
        case TAPP_BUS_HANDLE_RES:
            tapp_controller_proc_bus_res((char*)pstBody, sizeof(CITBUSHANDLE), TAPP_CTRL_TBUS_HANDLE_META);
            break;
        case TAPP_BUS_SHOW_CHNNL_RES:
            tapp_controller_proc_bus_res((char*)pstBody, sizeof(CISHOWCHNNLRES), TAPP_CTRL_TBUS_CHANNEL_META);
            break;
        case TAPP_BUS_SHOW_BY_ADDR_RES:
            tapp_controller_proc_bus_res((char*)pstBody, sizeof(CISHOWCHNNLRES), TAPP_CTRL_TBUS_CHANNEL_META);
            break;
        case TAPP_BUS_SHOW_BY_KEY_RES:
            tapp_controller_proc_bus_res((char*)pstBody, sizeof(CISHOWCHNNLRES), TAPP_CTRL_TBUS_CHNNL_HEAD_META);
            break;
        case TAPP_LOG_SHOW_CONF_RES:
            printf("TLOG CONF DATA FROM SERVER:\n%s", (char*)pstBody);
            break;

        default:
            printf("Recv one unknown msg <cmdID: %d> from server\n", pstHead->nCmdID);
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "recv one unknown msg <cmdID: %d> from server", pstHead->nCmdID);
            break;
    }
}

int tapp_controller_auth(const char* a_pszCIAddr)
{
    int iRet = 0;
    int i;
    COMMANDINJECTPKG stMsg;

    iRet = tapp_controller_authen_req(a_pszCIAddr);
    if (0 > iRet)
    {
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "send aunthen req failed");
        return iRet;
    }

    printf("\nEstablishing connetion with server...");

    for (i = 0; i < TAPPCTRL_CONN_TIME_LIMIT; i++)
    {
        iRet = tappctrl_recv_pkg(&stMsg);
        if (0 > iRet)
        {
            printf(".");
            tos_usleep(1000);
            continue;
        }
        switch (stMsg.stHead.nCmdID)
        {
            case TAPP_AUTHEN_RES:
                {
                    if (TAPP_AUTH_PASSED != stMsg.stBody.iAuthenRes)
                    {
                        tlog_error(g_pstTappCtrlLogCat, 0, 0, "authentication failed");
                        iRet = -1;
                    }
                }
                break;
            default:
                tlog_error(g_pstTappCtrlLogCat, 0, 0, "recv an unexpected msg from server, cmdID<%d>", stMsg.stHead.nCmdID);
                iRet = -1;
                break;
        }
        break;
    }

    if (TAPPCTRL_CONN_TIME_LIMIT == i)
    {
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "establishing connetion with server timeout");
    }

    return iRet;
}

TSF4G_API int tappctrl_init(IN const char *a_pszChannelInfo, IN void *a_pstLogCat, IN void* a_iMetaLib)
{
    return tappctrl_init_i(a_pszChannelInfo, 1, a_pstLogCat, a_iMetaLib);
}

static void tapp_controller_proc_bus_res(char * a_pBuf, unsigned a_uLen, const char* a_pszMetaName)
{
    int iRet;
    LPTDRMETA pstMeta;
    TDRDATA stHost;

    pstMeta = tdr_get_meta_by_name(g_pstTappProtoMetaLib, a_pszMetaName);
    if (!pstMeta)
    {
        printf("failed to get meta by name \'%s\'\n", a_pszMetaName);
		tlog_error(g_pstTappCtrlLogCat, 0, 0, "failed to get meta by name \'%s\'", a_pszMetaName);
        return;
    }
    stHost.pszBuff = a_pBuf;
    stHost.iBuff = a_uLen;
    printf("DATA FROM SERVER:\n");
    iRet = tdr_fprintf(pstMeta, stdout, &stHost, TDR_METALIB_TAPP_CTRL_VERSION);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("Error: tdr_fprintf failed, for \'%s\'\n", tdr_error_string(iRet));
		tlog_error(g_pstTappCtrlLogCat, 0, 0, "Error: tdr_fprintf failed, for \'%s\'\n", tdr_error_string(iRet));
    }
}
