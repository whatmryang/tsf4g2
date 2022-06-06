#ifndef MTEXAMPLE_MSG_PROC_H
#define MTEXAMPLE_MSG_PROC_H

#include "mtexample.h"

int mtexample_onlymain_req(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv);

int mtexample_msg_proc_timeout(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv);

int mtexample_onlythread_req();

int mtexample_thread_onlythread_req();



int mtexample_onlymain_res();

int mtexample_onlythread_res();


#endif
