#include <iostream>
#include "TdrBuf.h"
#include "TdrTypeUtil.h"
#include <stdlib.h>
#include "net_protocol.h"

using namespace net;
using namespace std;
using tsf4g_tdr::TdrTypeUtil;

void test_time_cmp();
void test_buf_print();
void test_buf_code();
void ex_init_host_pkg_data(Pkg* pstHostData);

int main()
{
    char test[16000];

    Pkg pkg;
    memset(&pkg, 0, sizeof(pkg));
    ex_init_host_pkg_data(&pkg);

    TdrWriteBuf packBuf(test, sizeof(test));

    /* 打包 */
    TdrError::ErrorType ret = pkg.pack(packBuf, 10);
    if (TdrError::TDR_NO_ERROR != ret)
    {
        cout << "pack: " << TdrError::getErrorString(ret) << endl;
        return -1;
    }

    /* 解包 */
    TdrReadBuf unpackBuf(test, sizeof(test));
    ret = pkg.unpack(unpackBuf, 10);
    if (TdrError::TDR_NO_ERROR != ret)
    {
        cout << "unpack: " << TdrError::getErrorString(ret) << endl;
        return -1;
    }

    /* 可视化 */
    packBuf.set(test, sizeof(test));
    ret = pkg.visualize(packBuf, 0, '\n');
    if (TdrError::TDR_NO_ERROR != ret)
    {
        cout << "visualize: " << TdrError::getErrorString(ret) << endl;
        return -1;
    }

    printf("%s\n", packBuf.getBeginPtr());

    test_time_cmp();

    return 0;
}

void test_time_cmp()
{
    tdr_time_t times[3];
    tdr_date_t  dates[3];
    tdr_datetime_t datetimes[3];

    TdrTypeUtil::str2TdrTime(times[0], "10:2:1");
    TdrTypeUtil::str2TdrTime(times[1], "11:2:2");
    TdrTypeUtil::str2TdrTime(times[2], "12:2:3");

    assert(1  == TdrTypeUtil::compareTdrTime(times[1], times[0]));
    assert(0  == TdrTypeUtil::compareTdrTime(times[1], times[1]));
    assert(-1 == TdrTypeUtil::compareTdrTime(times[1], times[2]));

    TdrTypeUtil::str2TdrDate(dates[0], "2010-10-11");
    TdrTypeUtil::str2TdrDate(dates[1], "2011-10-12");
    TdrTypeUtil::str2TdrDate(dates[2], "2012-10-13");

    assert(1  == TdrTypeUtil::compareTdrDate(dates[1], dates[0]));
    assert(0  == TdrTypeUtil::compareTdrDate(dates[1], dates[1]));
    assert(-1 == TdrTypeUtil::compareTdrDate(dates[1], dates[2]));

    TdrTypeUtil::str2TdrDateTime(datetimes[0], "2010-10-11 10:2:3");
    TdrTypeUtil::str2TdrDateTime(datetimes[1], "2010-10-12 10:2:2");
    TdrTypeUtil::str2TdrDateTime(datetimes[2], "2010-10-13 10:2:1");

    assert(1  == TdrTypeUtil::compareTdrDateTime(datetimes[1], datetimes[0]));
    assert(0  == TdrTypeUtil::compareTdrDateTime(datetimes[1], datetimes[1]));
    assert(-1 == TdrTypeUtil::compareTdrDateTime(datetimes[1], datetimes[2]));
}

void test_buf_print()
{
    char test[16000];

    TdrWriteBuf buf;
    buf.set(test, sizeof(test));

    buf.textize("[%s]:%d\n", "flyma", 24);

    printf("%s", buf.getBeginPtr());
}

void test_buf_code()
{
    char test[100];
    unsigned char ch = 255;

    TdrWriteBuf  writeBuf(test, sizeof(test));
    writeBuf.writeUChar(ch);

    TdrReadBuf  readBuf(test, sizeof(test));
    readBuf.readUChar(ch);

    cout << (int)ch << endl;
}

void ex_init_host_pkg_data(Pkg* pstHostData)
{
	pstHostData->stHead.dwMsgid = 100;
	pstHostData->stHead.nCmd = 0;
	pstHostData->stHead.nVersion = 9;
	pstHostData->stHead.nMagic = 0x3344;
	pstHostData->dwArray = 1;
    pstHostData->wCount = 1;

    time_t xx;
    TdrTypeUtil::str2TdrIP(pstHostData->ip[0], "172.25.40.106");
    TdrTypeUtil::str2TdrTime(pstHostData->time[0], "999:00:11");
    TdrTypeUtil::str2TdrDate(pstHostData->date[0], "2010-01-31");
    TdrTypeUtil::str2TdrDateTime(pstHostData->datetime[0], "2010-10-13 15:53:01");
    TdrTypeUtil::tdrDateTime2UTC(xx, pstHostData->datetime[0]);
    TdrTypeUtil::UTC2TdrDateTime(pstHostData->datetime[1], xx);
	strncpy(pstHostData->stBody.stLogin.szName, "haha", sizeof(pstHostData->stBody.stLogin.szName));
	strncpy(pstHostData->stBody.stLogin.szPass, "secret", sizeof(pstHostData->stBody.stLogin.szPass));
	strncpy(pstHostData->stBody.stLogin.szZone, "shenzhen", sizeof(pstHostData->stBody.stLogin.szZone));
	//strncpy(pstHostData->stBody.stLogin.szZone[1], "beijing", sizeof(pstHostData->stBody.stLogin.szZone));

}
