#ifndef _TBUS_CHECK_SINGLE_H_
#define _TBUS_CHECK_SINGLE_H_

#include "tbus_check_desc_base.h"
#include "tbus_comm_relation_desc.h"

class CheckSingleDesc : public CheckDescBase
{
    public:
        static int check(const SINGLE_MACHINE& stDesc);

    private:
        static int checkIntegrity(const SINGLE_MACHINE& stDesc);
        static int checkUnique(const SINGLE_MACHINE& stDesc);
        static int checkReferrence(const SINGLE_MACHINE& stDesc);
};

#endif
