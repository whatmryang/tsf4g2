#ifndef _TBUS_CHECK_BASE_H_
#define _TBUS_CHECK_BASE_H_

#include <set>
#include <string>

#include "tbus_comm_relation_desc.h"

using std::set;
using std::string;

class CheckDescBase
{
    protected:
        static int checkCommRefer(const std::set<std::string>& processName,
                                  const COMMUNICATION_RELATION& stCommunication);

        static int checkProcessListIntegrity(const PROCESS_ARRAY& stProcessList);
        static int checkConnectIntegrity(const COMMUNICATION_RELATION& stConnectList);

        static int checkProcessListUnique(const PROCESS_ARRAY& stProcessList);
        static int checkCommunication(const COMMUNICATION_RELATION& stCommunication);

        static int checkProcessIDUnique(const PROCESS_ARRAY& stProcessList, set<string>& uniqueSet);
        static int checkProcessNameUnique(const PROCESS_ARRAY& stProcessList, set<string>& uniqueSet);
};

#endif
