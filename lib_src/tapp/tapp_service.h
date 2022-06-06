#ifndef TAPP_SERVICE_H
#define TAPP_SERVICE_H

#include "tapp/tapp.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tagTappService
{
    char szServiceName[TAPP_MAX_SERVICE_NAME];
    char szServiceDisplayName[TAPP_MAX_SERVICE_NAME];
    char *pszServiceName;
    char *pszServiceDisplayName;

    char szServiceDesc[TAPP_MAX_SERVICE_DESC];
    char *pszServiceDesc;

    PFNTAPP_SERVICE_INIT pfnServiceInit;


    LPTAPPCTX pstCurCtx;    /*��¼�����̵�ctx*/
    void *pstTappEnv;       /*��¼�����̵Ļ�������ָ��*/
    int (*pfnServiceMainloop) (void);
    int (*pfnServiceFini) (void);
};
typedef struct tagTappService        TAPPSERVICE;
typedef struct tagTappService*        LPTAPPSERVICE;

void tapp_set_default_service_name(TAPPCTX* a_pstCtx);

extern TAPPSERVICE g_stTappService;

#ifdef __cplusplus
}
#endif
#endif /*TAPP_SERVICE_H*/
