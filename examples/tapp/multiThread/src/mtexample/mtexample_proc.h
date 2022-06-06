#ifndef MTEXAMPLE_PROC_H
#define MTEXAMPLE_PROC_H

#include "mtexample.h"

//Process client msg
int mtexample_recv_client_req(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv);
int mtexample_proc_client_req(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv);
int mtexample_backward_to_client(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv);
int mtexample_forward_to_thread(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv);

//Process thread msg
int mtexample_proc_thread_res(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv);


//Timeout msg goes here.
int mtexample_timeout_proc(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv);
int mtexample_send_to_client(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv,int iDest);

//The thread use these apis
int mtexample_thread_recv_msg(TAPPTHREADCTX *pstCtx,MTEXAMPLETHREADENV *pstEnv);
int mtexample_thread_process_msg(TAPPTHREADCTX *pstCtx,MTEXAMPLETHREADENV *pstEnv);
int mtexample_thread_backward_msg(TAPPTHREADCTX *pstCtx,MTEXAMPLETHREADENV *pstEnv);


#endif

