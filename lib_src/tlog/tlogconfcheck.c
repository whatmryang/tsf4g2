/*
 * tlogconfcheck.c
 *
 *  Created on: 2010-7-26
 *      Author: mickeyxu
 */
#include "tlogconfcheck.h"

int tlog_check_conf(TLOGCONF *pstConf)
{

    if(NULL == pstConf)
    {
        return -1;
    }

    if(TLOG_CONF_MAGIC != pstConf->iMagic)
    {
        return -2;
    }

    return 0;
}
