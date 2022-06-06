#include <time.h>
#include <Winsock2.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "mixture.h"
#include "pal/pal.h"

#define TV_DIFF2(t, t1, t2)        do\
{\
    if ((t1).tv_usec >= (t2).tv_usec)\
    {\
        (t).tv_sec =    (t1).tv_sec - (t2).tv_sec;\
        (t).tv_usec =    (t1).tv_usec - (t2).tv_usec;\
    }else\
    {\
        (t).tv_sec =    (t1).tv_sec - (t2).tv_sec -1;\
        (t).tv_usec =    1000000 + (t1).tv_usec - (t2).tv_usec;\
    }\
} while(0)

using namespace std;

void init(performance::Mixture& pkg)
{
    pkg.iCommand  = 100;
    pkg.dwMagic   = 0xffee0001;
    pkg.ullSrc    = 0xff0000000001LL;
    pkg.ullDst    = 0xff0000000002LL;

    pkg.iHasAttach = 1;
    performance::Attach* attach = &pkg.stAttach;

    attach->fProfit = 0.0123;
    attach->dMoney  = 1234567890123.123;

    attach->dwHasDesc = 1;
    memcpy(attach->szDesc, "this was a rough performance test", sizeof(attach->szDesc));

    attach->dwCount = 100;
    for (unsigned int i = 0; i < attach->dwCount; i++)
    {
        attach->astPoints[i].x = i;
        attach->astPoints[i].y = i;
    }

}

void print(performance::Mixture& pkg, ostream& out = cout)
{
    out << "----------------------------------" << endl;

    static char visualBuffer[4096];

    int iRet = pkg.visualize(visualBuffer, sizeof(visualBuffer));
    if (TdrError::TDR_NO_ERROR == iRet)
    {
        out << visualBuffer << endl;
    }


    out << "__________________________________" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " LOOP_COUNT" << endl;
        return -1;
    }

    int iLoopCount = atoi(argv[1]);

    performance::Mixture pkg;
    performance::Mixture newPkg;

    init(pkg);
    //    print(pkg);

    struct timeval stBegin;
    struct  timeval stEnd;
    struct timeval stDelta;

    static char packBuffer[1024];
    size_t usedSize = 0;

    /* ------- pack ------- */
    gettimeofday(&stBegin, NULL);
    for (int i = 0; i < iLoopCount; i++)
    {
        int iRet = pkg.pack(packBuffer, sizeof(packBuffer), &usedSize);
        if (TdrError::TDR_NO_ERROR != iRet)
        {
            cerr << "Failed to pack pkg into buffer. loop: " << i << endl;
            return -1;
        }
    }
   gettimeofday(&stEnd, NULL);
   TV_DIFF2(stDelta, stEnd, stBegin);
    cout << iLoopCount << "   pack, time: " << std::setprecision(6)
        << stDelta.tv_sec + (double)stDelta.tv_usec/1000000 << endl;

    /* ------ unpack ------ */
   gettimeofday(&stBegin, NULL);
    for (int i = 0; i < iLoopCount; i++)
    {
        int iRet = newPkg.unpack(packBuffer, usedSize);
        if (TdrError::TDR_NO_ERROR != iRet)
        {
            cerr << "Failed to unpack newPkg from buffer. loop: " << i << endl;
            return -1;
        }
    }
    gettimeofday(&stEnd, NULL);
    TV_DIFF2(stDelta, stEnd, stBegin);
    cout << iLoopCount << "   unpack, time: " << std::setprecision(6)
        << stDelta.tv_sec + (double)stDelta.tv_usec/1000000 << endl;

    //    print(newPkg);

    return 0;
}
