#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "tdr/tdr.h"
#include "net_protocol.h"

/////////////////////////////////////////////
LPTDRMETA test_get_meta(void);
int test_listen(void);
int test_accept(int listenfd);
int test_process_client(int clientfd, LPTDRMETA pstMeta);
int test_recv(int clientfd, char* buff, unsigned int len);
int test_recv_msg(int clientfd, LPTDRDATA pstNet);
int test_send(int clientfd, char* buff, unsigned int len);
int test_process_msg(LPTDRDATA pstHost, LPTDRDATA pstNet, LPTDRMETA pstMeta, unsigned int netBufSize);
/////////////////////////////////////////////

LPTDRMETA test_get_meta()
{
    int iRet = 0;
    LPTDRMETALIB pstLib;

    iRet = tdr_load_metalib(&pstLib, "net_protocol.tdr");
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("tdr_load_metalib error: %s\n", tdr_error_string(iRet));
        return NULL;
    }

    LPTDRMETA pstMeta = NULL;
    pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");
    if (!pstMeta)
    {
        printf("tdr_get_meta_by_name error\n");
        return NULL;
    }
    return pstMeta;
}

int test_listen()
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(2048);
    sa.sin_addr.s_addr = htons(INADDR_ANY);
    bzero(&(sa.sin_zero), 7);


    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        printf("failed to bind, for %s\n", strerror(errno));
        return -1;
    }

    if (listen(sockfd, 10) < 0)
    {
        printf("failed to listen, for %s\n", strerror(errno));
        return -1;
    }

    return sockfd;
}

int test_accept(int listenfd)
{
    int clientfd;
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    clientfd = accept(listenfd, (struct sockaddr*)&clientAddr,  &len);
    if (clientfd < 0)
    {
        printf("failed to accept, for %s\n", strerror(errno));
    }
    return clientfd;
}

int test_recv(int clientfd, char* buff, unsigned int len)
{
    unsigned int n = 0;
    while (n < len)
    {
        int temp = recv(clientfd, buff + n, len - n, 0);
        if (temp > 0)
        {
            n += temp;
        } else if (temp == 0)
        {
            printf("connetion was closed by peer\n");
            return -1;
        }
        else
        {
            printf("failed to recv, for %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}

int test_send(int clientfd, char* buff, unsigned int len)
{
    unsigned int n = 0;
    while (n < len)
    {
        int temp = send(clientfd, buff + n, len - n, 0);
        if (temp < 0)
        {
            printf("faild to write, for %s\n", strerror(errno));
        } else
        {
            n += temp;
        }
    }

    return 0;
}

int test_process_client(int clientfd, LPTDRMETA pstMeta)
{
    int iRet = 0;
    PKG msg;

    char netBuf[65536];
    TDRDATA stHost;
    TDRDATA stNet;

    while (1)
    {
        stNet.pszBuff = netBuf;
        stNet.iBuff = sizeof(netBuf);

        if (test_recv_msg(clientfd, &stNet) < 0)
        {
            printf("failed to recv one msg\n");
            close(clientfd);
            break;
        }

        stHost.pszBuff = (char*)&msg;
        stHost.iBuff = sizeof(msg);

        if (test_process_msg(&stHost, &stNet, pstMeta, sizeof(netBuf)) < 0)
        {
            printf("faild to process net msg\n");
            close(clientfd);
            break;
        }

        if (test_send(clientfd, stNet.pszBuff, stNet.iBuff) < 0)
        {
            printf("failed to send one msg\n");
            close(clientfd);
            break;
        }
    }

    return iRet;
}

int test_recv_msg(int clientfd, LPTDRDATA pstNet)
{
    int iRet = 0;

    if (test_recv(clientfd, pstNet->pszBuff, sizeof(int)) < 0)
    {
        printf("faild to recv msgLen\n");
        return -1;
    }

    unsigned int msgLen = ntohl(*(unsigned int*)pstNet->pszBuff);
    if (msgLen == 0)
    {
        printf("recv invalid msglen<%d>\n", msgLen);
        return -1;
    } else if (msgLen + 4 > pstNet->iBuff)
    {
        printf("msg sizeinfo is too bigger\n");
        return -1;
    } else
    {
        //        printf("msg sizeinfo: %d\n", msgLen);
    }

    if (test_recv(clientfd, pstNet->pszBuff + 4, msgLen) < 0)
    {
        printf("failed to recv msg\n");
        return -1;
    } else
    {
        //        printf("recv one msg\n");
        pstNet->iBuff = (int)msgLen + 4;
    }

    return iRet;
}

int test_process_msg(LPTDRDATA pstHost, LPTDRDATA pstNet, LPTDRMETA pstMeta, unsigned int netBufSize)
{
    int iRet = 0;

    memset(pstHost->pszBuff, 0, pstHost->iBuff);
    iRet = tdr_ntoh(pstMeta, pstHost, pstNet, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("tdr_ntoh error: %s\n", tdr_error_string(iRet));
        return -1;
    }

    iRet = tdr_fprintf(pstMeta, stdout, pstHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("tdr_fprintf error: %s\n", tdr_error_string(iRet));
        return -1;
    }

    pstNet->iBuff = netBufSize;
    iRet = tdr_hton(pstMeta, pstNet, pstHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("tdr_hton error: %s\n", tdr_error_string(iRet));
        return -1;
    }

    return iRet;
}

int main()
{
    int iRet = 0;

    LPTDRMETA pstMeta = NULL;
    pstMeta = test_get_meta();
    if (!pstMeta)
    {
        printf("failed to get meta\n");
        return -1;
    }

    int sockfd = test_listen();
    if (sockfd < 0)
    {
        printf("failed to listen\n");
        return -1;
    }

    int clientfd;
    int connNum = 0;
    while ((clientfd = test_accept(sockfd)) > 0)
    {
        printf("+++++++ got %dth connetion\n", ++connNum);
        test_process_client(clientfd, pstMeta);
        printf("------- close %dth connetion\n", connNum);
    }

    return iRet;
}
