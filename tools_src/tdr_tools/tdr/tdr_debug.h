#include <iostream>

using std::cout;
using std::endl;
using std::flush;

#ifdef MYDEBUG
#define TDEBUG_PRINT(info)\
    cout << #info << ":" << (info) << flush;
#else
#define TDEBUG_PRINT(info)
#endif

#ifdef MYDEBUG
#define TDEBUG_PRINTLN(info)\
    cout << #info << ":" << (info) << endl;
#else
#define TDEBUG_PRINTLN(info)
#endif

#ifdef MYDEBUG
#define TDEBUG_PRINT_STR(info)\
    cout << (info) << flush;
#else
#define TDEBUG_PRINT_STR(info)
#endif

#ifdef MYDEBUG
#define TDEBUG_PRINT_STRLN(info)\
    cout << (info) << endl;
#else
#define TDEBUG_PRINT_STRLN(info)
#endif
