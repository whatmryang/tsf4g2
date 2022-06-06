#ifndef _TBUS_CHECK_MULTI_DESC_H_
#define _TBUS_CHECK_MULTI_DESC_H_

#include <set>
#include <string>

#include "tbus_check_desc_base.h"
#include "tbus_comm_relation_desc.h"

using std::set;
using std::string;

class CheckMultiDesc : public CheckDescBase
{
    public:
        static int check(const MULTI_MACHINES& stDesc);

    private:
        static int checkReferrence(const MULTI_MACHINES& stDesc);

        static int checkIntegrity(const MULTI_MACHINES& stDesc);
        static int checkMachineIntegrity(const MACHINE_DESC& stMachine);
        static int checkMachineListIntegrity(const MACHINE_ARRAY& stMachineList);

        static int checkUnique(const MULTI_MACHINES& stDesc);
        static int checkMachineListUnique(const MACHINE_ARRAY& stMachineList);
        static int checkMachineUnique(const MACHINE_DESC& stMachine,
                                      set<string>& uniqueNameSet, set<string>& uniqueIdSet);
};

#endif
