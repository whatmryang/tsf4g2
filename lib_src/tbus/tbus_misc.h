/** @file $RCSfile: tbus_misc.h,v $
  Misc functions header file for tsf4g-tbus relay module
  $Id: tbus_misc.h,v 1.5 2009-08-04 07:27:56 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-08-04 07:27:56 $
@version $Revision: 1.5 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/



#ifndef _TBUS_MISC_H
#define _TBUS_MISC_H

#include "tbus/tbus.h"
#include "tbus_head.h"
#include "tbus_kernel.h"

#define tbus_init_head_for_send(a_pstHead, a_iFlag)\
{\
    assert(NULL != (a_pstHead));\
\
    memset((a_pstHead), 0, sizeof(TBUSHEAD));\
\
    (a_pstHead)->bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;\
\
    if ((TBUS_FLAG_START_DYE_MSG & (a_iFlag)) || (TBUS_FLAG_KEEP_DYE_MSG & (a_iFlag)))\
    {\
        tbus_dye_pkg((a_pstHead), NULL, (a_iFlag));\
    }\
\
    if ((TBUS_FLAG_SET_TEST_MSG & (a_iFlag)))\
    {\
        (a_pstHead)->bFlag |= TBUS_HEAD_FLAG_TEST_MSG;\
    }\
}


#define tbus_init_head_for_forward(iRet, a_pstHead, a_iFlag, a_pstPreHead)\
{\
    (iRet) = TBUS_SUCCESS;\
\
    memset((a_pstHead), 0, sizeof(TBUSHEAD));\
\
    (a_pstHead)->bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;\
    if ((TBUS_FLAG_START_DYE_MSG & (a_iFlag)) || (TBUS_FLAG_KEEP_DYE_MSG & (a_iFlag)))\
        tbus_dye_pkg((a_pstHead), (a_pstPreHead), (a_iFlag));\
\
    if ((TBUS_FORWARD_MAX_ROUTE <= (a_pstPreHead)->stExtHead.stDataHead.bRoute))\
    {\
        tbus_log(TLOG_PRIORITY_ERROR,"invalid route num(%d), its scope must be [0-%d), so cannot forward",\
                 (a_pstPreHead)->stExtHead.stDataHead.bRoute, TBUS_FORWARD_MAX_ROUTE);\
        (iRet) = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_FORWARD_ROUTE_LIMITED);\
    }\
\
    if (TBUS_SUCCESS == (iRet))\
    {\
        (a_pstHead)->stExtHead.stDataHead.bRoute = (a_pstPreHead)->stExtHead.stDataHead.bRoute;\
        memcpy(&(a_pstHead)->stExtHead.stDataHead.routeAddr[0], &(a_pstPreHead)->stExtHead.stDataHead.routeAddr[0],\
               sizeof(int)*(a_pstHead)->stExtHead.stDataHead.bRoute);\
        (a_pstHead)->bFlag |= TBUS_HEAD_FLAG_WITH_ROUTE;\
\
        /* TBUS_SET_LAST_ROUTE will set last route, so no need here */\
        (a_pstHead)->stExtHead.stDataHead.bRoute++;\
    }\
}

#define tbus_init_head_for_backward(iRet, a_pstHead, a_iFlag, a_pstPreHead, a_piDst)\
{\
    int i = 0;\
\
    (iRet) = TBUS_SUCCESS;\
    memset((a_pstHead), 0, sizeof(TBUSHEAD));\
\
    (a_pstHead)->bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;\
    if ((TBUS_FLAG_START_DYE_MSG & a_iFlag) || (TBUS_FLAG_KEEP_DYE_MSG & a_iFlag))\
        tbus_dye_pkg((a_pstHead), (a_pstPreHead), a_iFlag);\
\
    if ((0 >= (a_pstPreHead)->stExtHead.stDataHead.bRoute ) ||\
        (TBUS_FORWARD_MAX_ROUTE < (a_pstPreHead)->stExtHead.stDataHead.bRoute))\
    {\
        tbus_log(TLOG_PRIORITY_ERROR,"invalid  route num(%d), its scope is [1-%d], so cannot backward",\
                 (a_pstPreHead)->stExtHead.stDataHead.bRoute, TBUS_FORWARD_MAX_ROUTE);\
        (iRet) = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ROUTE);\
    }\
    if (TBUS_SUCCESS == (iRet))\
    {\
        if ( TBUS_ADDR_DEFAULT == *(a_piDst) )\
        {\
            if (1 > (a_pstPreHead)->stExtHead.stDataHead.bRoute)\
            {\
                /* weird, should *NOT* has this value at this function */\
                tbus_log(TLOG_PRIORITY_ERROR,"no dst address %d in the route info of the pre_recved msg", *(a_piDst)  ) ;\
                (iRet) = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED) ; ;\
            }\
            *(a_piDst) = (a_pstPreHead)->stExtHead.stDataHead.routeAddr[(a_pstPreHead)->stExtHead.stDataHead.bRoute-1] ;\
        }\
    }\
\
    if (TBUS_SUCCESS == (iRet))\
    {\
        for (i = 0; i < (a_pstPreHead)->stExtHead.stDataHead.bRoute-1; i++)\
        {\
            (a_pstHead)->stExtHead.stDataHead.routeAddr[i] = (a_pstPreHead)->stExtHead.stDataHead.routeAddr[i];\
        }\
        (a_pstHead)->stExtHead.stDataHead.bRoute = i;\
    }\
}

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */





/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
char *tbus_conv_hexstr ( IN const unsigned char *a_pszStr, IN const int a_iStrLen ) ;



/**
  @brief
  @retval 0 --
  @retval !0 --

  @note
*/
unsigned short tbus_head_checksum ( IN const TBUSHEAD *a_ptHead ) ;


/**
  @brief Show Hex val of a string, if a_iFlag = 1, show printable character as char
  @retval 0 --
  @retval !0 --

  @note
*/
void tbus_print_hexstr ( IN const unsigned char *a_pszStr, IN const int a_iLen, IN const int a_iFlag ) ;



/**
@brief try to attach share memory via specified key and size. If no old share memory exists, this function
would create it automatically
*@param[in,out] a_ppvShm  pointer to saving shm address
*@param[out] a_piCreated  指明此共享内存是否是刚被创建的，其值为非零值表示此共享内存是刚被创建的，否则是之前
*@	已经存在的
@retval 0 -- get shm successfully ok
@retval <0 -- failed
@note
*/
int tbus_auto_get_shm (INOUT void **a_ppvShm, OUT int *a_piCreated, IN const int a_iShmKey, IN  int *a_piSize, IN  int a_iFlag,
                       HANDLE *a_pShmID) ;

int tbus_init_headmeta(void);

void tbus_trim_str( char *a_strInput );

int IsChannelInGCIM(CHANNELHEAD * a_pstHead, TBUSSHMGCIM *a_pstGCIM);

LPTBUSPROCESSSTATE tbus_get_process_state_i(IN LPTBUSHANDLE a_pstHandle, IN TBUSADDR a_iPeerAddr);

void tbus_set_head_timestamp(IN LPTBUSHEAD a_pstHead, IN LPTBUSHEAD a_pstPreTime, IN int a_iNeedGetCurTime);


#ifdef __cplusplus
}
#endif


#endif /**< _TBUS_MISC_H */

