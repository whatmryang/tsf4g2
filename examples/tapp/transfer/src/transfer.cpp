/*
 * Copyright (c) 2010, ��Ѷ�Ƽ����޹�˾���������з���
 * All rights reserved
 *
 * File name:       transfer.cpp
 * Description:     main logic
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-02 18:10
 * Last modified:   2010-06-09 10:09
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-02     flyma         1.0         creation
 *
 */

#include "transfer.h"
#include "version.h"
#include "transfer_ctrl.h"
#include "transfer_session_mgr.h"
#include "transfer_session_base.h"
#include "transfer_session_dirtycheck.h"

#include "tapp/tapp.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"

TRANSFERENV g_stEnv;
static TAPPCTX gs_stTappCtx;

int main(int argc, char* argv[])
{
    int iRet = 0;

    tapp_register_service_init((PFNTAPP_SERVICE_INIT)transfer_service_init);

    iRet = tapp_run_service(&gs_stTappCtx, &g_stEnv, argc, argv);

    return iRet;
}

int transfer_reload(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    TRANSFERCONF* pstPreConf;

    //TODO tapp����Ѿ�������ˢ�µ�a_pstTappCtx->stConfPrepareData.pszBuff��
    //�߼������ʵ�������������ˢ�µ�ǰ����
    pstPreConf = (TRANSFERCONF* )a_pstTappCtx->stConfPrepareData.pszBuff;


    tlog_info(a_pstEnv->pstLogCat, 0, 0, "transfer reload");

    return 0;
}

int transfer_fini(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);


    //TODO �����˳�������ʵ���ͷ���Դ���߼�
    tlog_info(a_pstEnv->pstLogCat, 0, 0, "transfer finish");
    return 0;
}

int transfer_tick(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    //��ʱɨ��session���У��������Ѿ���ʱ��session
    SessionMgr<DirtyCheckSession>::getInstance()->onTick(&a_pstTappCtx->stCurr, a_pstEnv);

    //TODO �߼�������
    return 0;
}

int transfer_stop(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    //TODO  ��������˳�ǰ���߼�����������һЩ�����
    //����(���罫������ʣ�����Ϣ���������˳�)��������ʵ��

    return 0;
}

int transfer_service_init(TAPPCTX *a_pstAppCtx, TRANSFERENV *a_pstEnv, int argc, char* argv[])
{
    int iRet = 0;

    assert(NULL != a_pstAppCtx);
    assert(NULL != a_pstEnv);

    a_pstAppCtx->argc = argc;
    a_pstAppCtx->argv = argv;

    a_pstAppCtx->pfnInit     = (PFNTAPPFUNC)transfer_init;
    a_pstAppCtx->pfnFini     = (PFNTAPPFUNC)transfer_fini;
    a_pstAppCtx->pfnProc     = (PFNTAPPFUNC)transfer_proc;
    a_pstAppCtx->pfnTick     = (PFNTAPPFUNC)transfer_tick;
    a_pstAppCtx->pfnReload   = (PFNTAPPFUNC)transfer_reload;
    a_pstAppCtx->pfnStop     =   (PFNTAPPFUNC)transfer_stop;

    a_pstAppCtx->iLib        = (intptr_t)g_szMetalib_transfer;
    a_pstAppCtx->uiVersion   = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);
    a_pstAppCtx->stConfData.pszMetaName  = "transferconf";

    transfer_set_controller(a_pstAppCtx);

    return iRet;
}
