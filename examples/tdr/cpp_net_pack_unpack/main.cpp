#include <ctime>
#include <iostream>

#include "net_protocol.h"
#include "TdrTypeUtil.h"

using std::cout;
using std::endl;
using tsf4g_tdr::TdrTypeUtil;

using namespace net;

void initPkg(Pkg* pstPkg);

void sendPackedPkg(const char* buffer, size_t size);
void logBeforeSend(const Pkg& packedPkg);
void processPackError(const Pkg& pkg4pack, TdrError::ErrorType errorCode);

void logAfterUnpack(const Pkg& unpackedPkg);
void processUnpackError(const char* buffer, size_t size, TdrError::ErrorType errorCode);

int main()
{
    char buffer[4096];

    /* pack into buf */
    Pkg pkg4pack;

    /* if need to init all members, call construct() */
    // pkg4pack.construct();

    initPkg(&pkg4pack);

    /* be careful, cutVer = 12 means:
     * entries with version bigger than 12 will not be packed.
     */
    int cutVer = 12;
    size_t usedPackBufSize;
    TdrError::ErrorType ret = pkg4pack.pack(buffer, sizeof(buffer), &usedPackBufSize, cutVer);
    if (TdrError::TDR_NO_ERROR == ret)
    {
        logBeforeSend(pkg4pack);
        sendPackedPkg(buffer, usedPackBufSize);
    } else
    {
        processPackError(pkg4pack, ret);
        return -1;
    }


    /* unpack from buf */
    Pkg pkg4unpack;
    size_t usedUnpackBufSize;
    ret = pkg4unpack.unpack(buffer, usedPackBufSize, &usedUnpackBufSize);
    if (TdrError::TDR_NO_ERROR == ret)
    {
        logAfterUnpack(pkg4unpack);
        /* other process */
    } else
    {
        processUnpackError(buffer, usedPackBufSize, ret);
        return -1;
    }


    return 0;
}

void initPkg(Pkg* pstPkg)
{
    pstPkg->stHead.dwMsgid = 100;
    pstPkg->stHead.nCmd = CMD_LOGIN;
    pstPkg->stHead.nMagic = 0x3344;
    TdrTypeUtil::str2TdrIP(pstPkg->stHead.ulSrcIp, "172.25.40.107");
    TdrTypeUtil::UTC2TdrDateTime(pstPkg->stHead.tDatetime, time(NULL));

    strncpy(pstPkg->stBody.stLogin.szName, "sterma", sizeof(pstPkg->stBody.stLogin.szName));
    strncpy(pstPkg->stBody.stLogin.szPass, "secret", sizeof(pstPkg->stBody.stLogin.szPass));
    strncpy(pstPkg->stBody.stLogin.szZone, "shenzhen", sizeof(pstPkg->stBody.stLogin.szZone));
}

void sendPackedPkg(const char* buffer, size_t size)
{
    /* printing buf4send to represent sending it through socket or tbus */

    char hexStrBuf[4096];
    size_t hexBufSize = sizeof(hexStrBuf);
    TdrError::ErrorType ret = TdrReadBuf(buffer, size).toHexStr(hexStrBuf, hexBufSize);
    if (TdrError::TDR_NO_ERROR != ret)
    {
        cout << "Error: failed to transform binary data into hex-string, for: "
            << TdrError::getErrorString(ret) << endl;
    }

    cout << "------> sending data: " << endl << hexStrBuf << endl << endl;

    /* after sending,
     * common practice is record hexBufSize into log,
     * but with a low log priority.
     */
}

void logBeforeSend(const Pkg& packedPkg)
{
    /* common practice is to record a log,
     * but here only print packedPkg into stdout,
     */
    char buffer[4096];
    TdrError::ErrorType ret = packedPkg.visualize(buffer, sizeof(buffer));
    if (TdrError::TDR_NO_ERROR != ret)
    {
        cout << "Error: visualize failed, for: " << TdrError::getErrorString(ret) << endl;
    }

    cout << "------> visualized pkg:" << endl << buffer << endl;
}

void processPackError(const Pkg& pkg4pack, TdrError::ErrorType errorCode)
{
    /* common practice is to record errorCode and pkg4pack into log,
     * but here only print errorCode and pkg4pack to stdout,
     */
    cout << "Error: pack failed, errorCode<" << errorCode
        << ">: " << TdrError::getErrorString(errorCode) << endl;

    char hexStrBuf[4096];
    size_t hexBufSize = sizeof(hexStrBuf);
    TdrError::ErrorType ret = TdrReadBuf((char*)&pkg4pack, sizeof(pkg4pack)).toHexStr(hexStrBuf, hexBufSize);
    if (TdrError::TDR_NO_ERROR != ret)
    {
        cout << "Error: failed to transform binary data into hex-string, for: "
            << TdrError::getErrorString(ret) << endl;
    }

    cout << "------> loging pkg: " << endl << hexStrBuf << endl << endl;

    /* when tring to locate pack error,
     * follow the following steps,
     * 1. create a small program,
     * 2. restore pkg4pack from log,
     * 3. debug,
     */

    /* demo: restore pkg4pack from log */
    /*
        ret = tsf4g_tdr::restoreFromHexStr(pkg4pack, hexStrBuf, strlen(hexStrBuf));
        if (TdrError::TDR_NO_ERROR != ret)
        {
            cout << "Error: failed to restore pkg4pack from hex-string, for: "
                << TdrError::getErrorString(ret) << endl;
        }
    */
}

void logAfterUnpack(const Pkg& unpackedPkg)
{
    /* common practice is to record a log,
     * but here only print pkg into stdout,
     */
    char buffer[4096];
    TdrError::ErrorType ret = unpackedPkg.visualize(buffer, sizeof(buffer));
    if (TdrError::TDR_NO_ERROR != ret)
    {
        cout << "Error: visualize failed, for: " << TdrError::getErrorString(ret) << endl;
    }

    cout << "------> unpacked pkg:" << endl << buffer << endl;
}

void processUnpackError(const char* buffer, size_t size, TdrError::ErrorType errorCode)
{
    /* common practice is to record content in buffer into log,
     * but here only print it to stdout,
     */
    cout << "Error: unpack failed, errorCode<" << errorCode
        << ">: " << TdrError::getErrorString(errorCode) << endl;

    char hexStrBuf[4096];
    size_t hexBufSize = sizeof(hexStrBuf);
    TdrError::ErrorType ret = TdrReadBuf(buffer, size).toHexStr(hexStrBuf, hexBufSize);
    if (TdrError::TDR_NO_ERROR != ret)
    {
        cout << "Error: failed to transform binary data into hex-string, for: "
            << TdrError::getErrorString(ret) << endl;
    }

    cout << "------> loging data-from-net: " << endl << hexStrBuf << endl << endl;

    /* when tring to locate unpack error,
     * follow the following steps,
     * 1. create a small program,
     * 2. restore an TdrReadBuf object from log,
     * 3. debug,
     */

    /* demo: restore pkg4pack from log */
    /*
        char netData[4096];
        TdrReadBuf netBuf(netData, sizeof(netData));
        ret = netBuf.fromHexStr(hexStrBuf, strlen(hexStrBuf));
        if (TdrError::TDR_NO_ERROR != ret)
        {
            cout << "Error: failed to restore pkg4pack from hex-string, for: "
                << TdrError::getErrorString(ret) << endl;
        }
    */
}
