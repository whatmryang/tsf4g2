/*
**  @file $RCSfile: tapp_controller.h ,v $
**  tapp controller
**  @author $Author: jackyai $
**  @date $Date: 2010-01-26 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TAPP_CONTROLLER_H
#define TAPP_CONTROLLER_H

#include "tapp/tapp.h"
#include "tappctrl_kernel_i.h"
#include "tapp_i.h"

#define TAPPCTRL_INPUT_LINE_BUF_LEN 1024

#ifdef __cplusplus
extern "C" 
{
#endif 

TSF4G_API int tapp_controller_proc(TAPPCTX  *a_pstCtx, void *a_pvArg);
TSF4G_API int tappctrl_init(IN const char *a_pszChannelInfo, IN void *a_pstLogcat, IN void* a_pvMetaLib);
TSF4G_API int tapp_controller_fini(TAPPCTX *a_pstCtx, void* a_pvArg);

#ifdef __cplusplus
}
#endif /*#ifdef __cplusplus*/
#endif /*#ifndef TAPP_CONTROLLER_H*/
