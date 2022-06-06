/**
*
* @file     tdr_error.c
* @brief    TDR����������
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-04
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <assert.h>
#include <stdio.h>

#include "tdr/tdr_error.h"
#include "tdr_error_i.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

const char * tdr_error_string(IN int iErrorCode)
{
    int iTdrErrno = TDR_ERR_GET_ERROR_CODE(iErrorCode);
    const char *pchStr = NULL;

    static const char *message[] = {
        "û�д���",
        "���ð汾������ȡֵӦ����Ԫ���ݻ�׼�汾,����С��Ԫ���ݵĵ�ǰ�汾\n"
            "���Ԫ�����а汾ָʾ��������ð汾���벻С�ڰ汾ָʾ����������С�汾",
		"���������Ϣ������ʣ��ռ䲻��",
		"��Ԫ�ص�refer����ֵ����ȷ����ֵ����Ϊ�����ұ����count����ֵС",
		"Ԫ��������������������Ƕ�ײ�γ���32��",
		"��֧�ֵ�Ԫ��������",
		"string���͵�Ԫ�����ַ������ȳ�����Ԥ����󳤶�",
		"���ش洢������ʣ��ռ䲻��",
		"Ԫ�����汾ָʾ����ֵ����ȷ",
        "����洢�ռ�ʧ��",
        "����XML��Ϣ����",
        "XMLԪ������û�и�Ԫ��",
        "��Ч��Ԫ���ݸ�Ԫ��",
        "Ԫ�����������ֿռ��ͻ�������ܽ���Ԫ��name����ֵ��ͬ����Ϣ�ӵ�ͬһ������",
        "�궨��Ԫ�ر���ָ��name����",
        "includeԪ�ر��붨��file����",
        "û��ָ��version����",
        "��Ԫ��ID��ͻ�������ܽ���Ԫ��ID����ֵ��ͬ����Ϣ�ӵ�ͬһ������",
        "��֧�ֵ�Ԫ��������XML��ǩ���汾",
        "Ԫ���ݿ��������ȷ",
        "Ԫ���ݿ�У��Ͳ���ȷ",
        "��ӵ�Ԫ�����������еĺ궨������Ԥ�����Ҫ��",
        "�궨��Ԫ��û��ֵ����",
        "��֧�ֵ���������",
        "Ԫ�����������Ԫ�ر���ָ��name����",
        "û���㹻�Ŀռ�洢�Զ�����������",
        "�ַ����������ռ䲻��",
        "union/stuctԪ�ر������name����",
        "ͬ���͵�union��stuctԪ�ز�����ͬ��",
        "�ú���û�ж���",
        "ͬһ��Ԫ���²��ܳ���ID��ͬ����Ԫ��",
        "entryԪ�ر������type����",
        "entry��type����ֵ��Ч",
        "entry��io����ֵ��Ч",
        "entry��unique���Բ���ȷ����ȷȡֵΪfalse,true",
        "entry��notnull���Բ���ȷ����ȷȡֵΪfalse,true",
        "entry��size����ֵ����ȷ",
        "entry��sortkey����ֵ����ȷ",
        "entry��select����ֵ����ȷ",
        "entry��maxid���Բ���ȷ",
        "entry��minid���Բ���ȷ",
        "entry��minid��maxid����ֵ����ȷ",
        "entry��count����ֵ����ȷ",
		"entry��id����ֵ����ȷ",
		"entry��default����ֵ����ȷ",
		"entry��sortmethod����ֵ����ȷ",
		"entry��datetime����ֵ����ȷ",
		"entry��date����ֵ����ȷ",
		"entry��time����ֵ����ȷ",
		"entry��ip����ֵ����ȷ",
		"entry��extendtotable���Բ���ȷ",
        "structԪ�ص�size���Բ���ȷ",
        "structԪ�ص�align����ֵ����ȷ",
        "structԪ�ص�versionindicator���Բ���ȷ",
        "Ԫ�ص�sizetype/sizeinfo����ֵ����ȷ",
		"structԪ�ص�splittablefactor����ֵ����ȷ",
		"structԪ�ص�primarykey����ֵ����ȷ",
        "indexԪ�ص�column����ֵ����ȷ",
		"structԪ�ص�splittablekey����ֵ����ȷ",
		"structԪ�ص�splittablerule����ֵ����ȷ",
		"structԪ�ص�strictinput����ֵ����ȷ",
		"structԪ�ص�dependonstruct����ֵ����ȷ",
        "Ԫ�ص�path����ȷ��������ȷƥ��meta�е�Ԫ��",
        "Ԫ�ص�ƫ��ֵ����",
        "����Ϣд��������ʱ�ռ䲻��",
        "�Զ�����������û�а����κ��ӳ�Ա",
        "entryԪ�ص�refer����ֵ����ȷ",
        "entryԪ�ص�sizeinfo����ֵ����ȷ",
		"��֧�ֵ�IO��",
		"д�ļ�ʧ��",
		"���ļ�дʧ��",
		"������Ԫ���ݿ��ļ���Ч,��ȷ������Ԫ�����ļ���TDR�⺯�������汾�Ͷ�ȡ���ļ���TDR�⺯�������汾һ��",
		"���ļ���ʧ��",
        "���Ա��������ָ��refer����",
        "Ԫ������sizeinfo��Աǰ�ĳ�Ա�Ĵ洢�ռ�����ǹ̶���",
		"�����ַ���ת����unicode�ַ���ʧ��",
		"entryԪ�ص�ֵ�������Լ��",
		"��֧�ֵ����ݿ����ϵͳDBMS",
		"��֧��Ϊ�����������������Ա���ɽ������",
		"�������ݿ������ʧ��",
		"��֧�ֵ����ݲ���",
		"�ü��ð汾�޷�������Ч��������Ϣ",
		"ִ�����ݿ�SQL���ʧ��",
		"���ݿ����Ӵ���",
		"ȡSQL��ѯ���ʧ��",
		"SQL��ѯ�����Ϊ��",
		"�������û�и�������ݼ�¼������˴���",
		"��ǰ�������в�����ָ����������",
		"��֧��Ϊ�洢�ռ䲻�̶��Ľṹ���ɽ������",
		"����Ԫ���ݿ��ļ���tdr���ߵĺ������ݹ����汾��tdr��ĺ������ݹ������汾��һ��",
		"Ԫ���ݿ��ɢ��ֵ��������ɢ��ֵ��һ��",
		"�ṹ���Ա��ʵ����������Ԥ�ƵĲ�һ��",
		"��Ա��vesion����ֵ����ȷ",
		"�������ڵ����ݿ���Ƿֱ��洢��,�������ݵ�Ԫ��������û��ָ���ֱ��ؼ���",
		"����macrosgroup����ʧ��",
		"entry��bindmacrosgroup����ֵ��Ч",
		"��Աȡֵ�Ѿ����������͵�ֵ��Χ",
		"�ڿ���չ�Ľṹ�������Ա�ж����˲��������ֵ�����",
		"�����ָ��������������ݳ�Աʱ�����ûص�ʧ��",
		"�����ּ��������ͳ�Աʱ,���ûص�����ʧ��",
		"��Ա��autoincrement������Ч��Ŀǰֻ�з����������������͵ĳ�Ա���ܶ��������",
		"��Ա��custom����ֵ��Ч,��ȷ������ֵ�ĳ��Ȳ�������󳤶�����",
		"�ṹ���uniqueentryname����ֵ��Ч,�����Ե�����ֵֻ��Ϊtrue/false",
		"���ṹ���Աչ���洢ʱ���������",
		"������Ч������ÿ�������Ƿ�����ӿڵ�ǰ������Լ��",
		"��չ���洢ʱ��Ҫ����ǰ׺�Ľṹ�岻֧�ֽ������г�Ա����ƫ��������",
		"����db����ʧ��",
		"�ṹ���Ա�У�ֻ�������һ����Ա�ǿɱ��������͵ĳ�Ա",
		"�ṹ���Ա�У�û�ж���size���Ե�string���ͳ�Ա(��洢�ռ��ǿɱ��)ֻ����Ϊ���һ����Ա���Ҳ���Ϊ����",
        "���ִ�̫������������󳤶�����",
        "����Ԫ���ݿ��ļ���ϵͳƽ̨������tdr���ϵͳƽ̨��һ�£���ע��32λƽ̨��64λƽ̨������",
        "xml�ļ��д����޷�������include��ϵ",
        "�������ݰ��еİ汾ָʾ��ֵ����ȷ����ȡֵӦ����Ԫ���ݻ�׼�汾,����С��Ԫ���ݵĵ�ǰ�汾\n"
            "���Ԫ�����а汾ָʾ������汾ָʾ����ֵ���벻С�ڰ汾ָʾ����������С�汾",
        "entry��compress���Բ���ȷ",
        "entry��compress���Ժ�extendtotable���Գ�ͻ",
        "����ѹ��ʧ��",
        "���ݽ�ѹ��ʧ��",
        "����JSON������ʧ��",
        "������JSON����������",
        "JSON�������ĸ�ʽ����ȷ",
        "���뻺�����ĳ���̫С��δ����������metalib",
        "sizeinfo���õ��ֶζ�Ӧ���ڴ�ռ�̫С������sizeֵʱ���",
    };

    assert((sizeof(message) / sizeof(message[0])) == TDR_ERROR_COUNT);

    if (!TDR_ERR_IS_ERROR(iErrorCode))
    {
        pchStr = "û�д���";
    }else  if ((iTdrErrno < 0) || (iTdrErrno > TDR_ERROR_COUNT))
    {
        pchStr =  "δ֪����";
    }
    else
    {
        pchStr = message[iTdrErrno];
    }

    return pchStr;
}