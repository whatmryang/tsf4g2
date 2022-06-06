/*
 * TmemlogOption.h
 *
 *  Created on: 2010-7-13
 *      Author: mickeyxu
 */

#ifndef TMEMLOGOPTION_H_
#define TMEMLOGOPTION_H_
#include <stdio.h>

#include "version.h"
#include "pal/pal.h"

#include "../lib_src/tlog/tlogmem.h"
#include "../lib_src/tlog/tlog_i.h"
#include "../lib_src/tloghelp/tlogload_i.h"
#include "../lib_src/tmng/err_stack.h"

class Option
{
public:
    Option();
    int Parse(int argc, char **argv);
    int Help(int argc, char **argv);
public:
    char *pszLogConf;
    char *pszUri;
    char *pszOutPutFile;
    int iFollow;
    int iNeedExit;
private:
};


#endif /* TMEMLOGOPTION_H_ */
