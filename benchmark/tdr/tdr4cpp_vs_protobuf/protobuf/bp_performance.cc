#include <sys/time.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "mixture.pb.h"

#define TV_DIFF(t, t1, t2)        do\
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
/*

message Point {
    required fixed32 x = 1;
    required fixed32 y = 2;
}

message Attach {
    required    float   profit = 1;
    required    double  money  = 2;
    optional    string  desc   = 3;
    repeated    Point   points = 4;
}

message Mixture {
    required fixed32  length  = 1;
    required fixed32  version = 2;
    required sfixed32 command = 3;
    required fixed32  magic   = 4;
    required fixed64  src   = 5;
    required fixed64  dst   = 6;

    optional Attach attach = 7;
}
 */

void init(performance::Mixture& pkg)
{
    pkg.set_length(0x7f0000f1);
    pkg.set_version(1);
    pkg.set_command(100);
    pkg.set_magic(0xffee0001);
    pkg.set_src(0xff0000000001LL);
    pkg.set_dst(0xff0000000002LL);

    performance::Attach* attach = pkg.mutable_attach();

    attach->set_profit(0.0123);
    attach->set_money(1234567890123.123);

    attach->set_desc("this was a rough performance test");

    for (int i = 0; i < 100; i++)
    {
        performance::Point* point = attach->add_points();
        point->set_x(i);
        point->set_y(i);
    }

}

void print(performance::Mixture& pkg, ostream& out = cout)
{
    out << "----------------------------------" << endl;

    out << hex << endl;

    out << "length:   " << pkg.length() << endl;
    out << "version:  " << pkg.version() << endl;
    out << "command:  " << pkg.command() << endl;
    out << "magic:    " << pkg.magic() << endl;
    out << "src:      " << pkg.src() << endl;
    out << "dst:      " << pkg.dst() << endl;

    out << "   attach.profit " << pkg.attach().profit() << endl;
    out << "   attach.money  " << pkg.attach().money() << endl;
    out << "   attach.desc   " << pkg.attach().desc() << endl;

    for (int i = 0; i < pkg.attach().points_size(); i++)
    {
        out << "       attach.points[" << i << "]: "
            << "<" << pkg.attach().points(i).x()
            << ", " << pkg.attach().points(i).y()
            << ">" << endl;
    }

    out << dec << endl;

    out << "__________________________________" << endl;
}

int main(int argc, char* argv[])
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " LOOP_COUNT" << endl;
        return -1;
    }

    int iLoopCount = atoi(argv[1]);

    performance::Mixture pkg;
    performance::Mixture newPkg;

    init(pkg);
    static char buffer[4096];
    //    print(pkg);

    struct timeval stBegin;
    struct timeval stEnd;
    struct timeval stDelta;


    /* ------- pack ------- */
    gettimeofday(&stBegin, NULL);
    for (int i = 0; i < iLoopCount; i++)
    {
        if (!pkg.SerializeToArray(buffer, 4096))
        {
            cerr << "Failed to seriliaze pkg into stringstream. loop: " << i << endl;
            return -1;
        }
    }
    gettimeofday(&stEnd, NULL);
    TV_DIFF(stDelta, stEnd, stBegin);
    cout << iLoopCount << "   pack, time: " << std::setprecision(6)
        << stDelta.tv_sec + (double)stDelta.tv_usec/1000000 << endl;

    /* ------ unpack ------ */
    gettimeofday(&stBegin, NULL);
    for (int i = 0; i < iLoopCount; i++)
    {
        if (!newPkg.ParseFromArray(buffer, pkg.ByteSize()))
        {
            cerr << "Failed to restore newPkg from stringstream. loop: "  << i << endl;
            return -1;
        }
    }
    gettimeofday(&stEnd, NULL);
    TV_DIFF(stDelta, stEnd, stBegin);
    cout << iLoopCount << "   unpack, time: " << std::setprecision(6)
        << stDelta.tv_sec + (double)stDelta.tv_usec/1000000 << endl;

    //    print(newPkg);

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
