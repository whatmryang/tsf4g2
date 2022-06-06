#ifndef _TMNG_ERRNO_H
#define _TMNG_ERRNO_H


#include "pal/pal.h"

#if defined(_WIN32) || defined(_WIN64)
#define __inline__
#endif

typedef enum
{
    TMNG_ERR_OK = 0,    // û�д�����
//-->error_table tmng code begin
    TMNG_ERR_ARGS,  // �������û��ͨ��
    TMNG_ERR_COMP,  // ��ƥ�������
    TMNG_ERR_FULL,  // ��������
    TMNG_ERR_LOCKOPEN,  // ����ʧ��
    TMNG_ERR_MEMORY,  // �ڴ����ʧ��
    TMNG_ERR_NULLMETA,  // tdr ��û���ҵ�ƥ���Ԫ����
    TMNG_ERR_SHMAT,  // �����ڴ��ȡʧ��
    TMNG_ERR_SHMDEL,  // �����ڴ�ɾ��ʧ��
    TMNG_ERR_SHMOPEN,  // �����ڴ��ʧ��
    TMNG_ERR_SIZE,  // ����Ԥ���ߴ�
    TMNG_ERR_TDR_LOAD_METALIB,  // tdr_load_metalib ��������ʧ��
    TMNG_ERR_VERSION,  // �汾��ƥ��
    TMNG_ERR_TMB_FIND_UNLOCKED,  // �����ڴ���û���ҵ�ƥ���Ԫ����
    TMNG_ERR_ALIGN,  // ���ݶ������
    TMNG_ERR_TDR_FPRINTF,  // tdr_printf ��������ʧ��
    TMNG_ERR_TMIB_GET_DATA,  // û���ҵ�ƥ��� tmib ����
    TMNG_ERR_CANCEL,  // ����ȡ��
    TMNG_ERR_HELP,  // ����ִ�� -h ��ȡ������Ϣ
    TMNG_ERR_SET_ENTRY_VALUE,  // ���ñ�������
    TMNG_ERR_TDR_CREATE_LIB_FILE,  // tdr_create_lib_file �������ó���
    TMNG_ERR_TDR_DATA_TYPE,  // tdr �������ʹ���
    TMNG_ERR_TDR_ENTRY_PATH_TO_OFF,  // tdr_entry_path_to_off �������ó���
    TMNG_ERR_TDR_INPUT_FILE,  // tdr_input_file �������ó���
//-->error_table tmng code end
} TMNG_ERRNO;

static const char *g_ppszErrStr[] = {
    "û�д�����",
//-->error_table tmng string begin
    "�������û��ͨ��",
    "��ƥ�������",
    "��������",
    "����ʧ��",
    "�ڴ����ʧ��",
    "tdr ��û���ҵ�ƥ���Ԫ����",
    "�����ڴ��ȡʧ��",
    "�����ڴ�ɾ��ʧ��",
    "�����ڴ��ʧ��",
    "����Ԥ���ߴ�",
    "tdr_load_metalib ��������ʧ��",
    "�汾��ƥ��",
    "�����ڴ���û���ҵ�ƥ���Ԫ����",
    "���ݶ������",
    "tdr_printf ��������ʧ��",
    "û���ҵ�ƥ��� tmib ����",
    "����ȡ��",
    "����ִ�� -h ��ȡ������Ϣ",
    "���ñ�������",
    "tdr_create_lib_file �������ó���",
    "tdr �������ʹ���",
    "tdr_entry_path_to_off �������ó���",
    "tdr_input_file �������ó���",
//-->error_table tmng string end
};

static __inline__ const char *tmng_errstr(int a_iErrNo)
{
    if (0 > a_iErrNo)
    {
        a_iErrNo = -a_iErrNo;
    }

    if (sizeof(g_ppszErrStr) / sizeof(*g_ppszErrStr) <= (size_t)a_iErrNo)
    {
        return "����ų�����Χ";
    }

    return g_ppszErrStr[a_iErrNo];
}

#endif




