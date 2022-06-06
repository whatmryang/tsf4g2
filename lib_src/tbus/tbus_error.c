
#include <assert.h>
#include <stdio.h>

#include "tbus/tbus_error.h"
#include "tbus/tbus.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif



const char * tbus_error_string(IN int iErrorCode)
{
	int iTdrErrno = TBUS_ERR_GET_ERROR_CODE(iErrorCode);
	const char *pchStr = NULL;

	static const char *message[] = {
		"û�д���",
		"�����ڴ�ʧ��",
		"��ȡtbusͷ����Ԫ��������ʧ��",
		"��ʼ����־ϵͳʧ��",
		"����ͨ�ŵ�ַʮ���Ƶ�ֱ�ʾģ��ʧ��",
		"���ʮ����ͨ�ŵ�ַ������ȷ",
		"�����ϵͳע��ҵ��IDʧ��",
		"ͨ��shmget���乲���ڴ�ʧ��",
		"ͨ��shmat���ع����ڴ�ʧ��",
		"���ɹ����ڴ��keyֵʧ��",
		"GCIM�����ڴ�У��ʧ��",
		"���ݸ��ӿڵĲ�������",
		"Tbusϵͳ��û�г�ʼ��",
		"�ѷ���tbus��������Ѿ��ﵽ�趨�������Ŀ�������ڷ���",
		"tbus��������Ч",
		"һ��tbus�������������ͨ�����Ѿ���������",
		"ͨ��ָ����ַ�󶨲����κ�ͨ��",
		"tbus���û�а��κ�ͨ��ͨ��",
		"ͨ�������ڴ��м�¼�ĵ�ַ��Ϣ��GCIM�м�¼�ĵ�ַ��Ϣ��һ��",
		"��tbus���������κ�ͨ���ĶԶ˵�ַ����ƥ��",
		"û����ָ��Դ��ַ��Ŀ�ĵ�ַƥ���ͨ��",
		"tbusͷ�����ʧ��",
		"tbusͷ�������Ϣ���ȳ��������Ԥ������",
		"tbus������Ϣͨ�������������ٷ�������",
		"tbus��Ϣ����ͨ���ѿգ�û���κ���Ϣ�ɽ���",
		"ͨ�������ݳ��ȴ���ֻ�ж��������ݲ��ָܻ�����",
		"�������ݵĻ�����̫С�������Խ����������ݰ�",
		"tbus��Ϣͷ�����ʧ��",
		"�����ת��·���������ƣ����ܼ���ת��",
		"·����Ϣ��Ч",
		"GCIM�����ò���ȷ",
		"shmctl���������ڴ�ʧ��",
		"�������תͨ�����Ѿ����������Ŀ����",
		"У�����ݰ�ͷ��ʧ��",
		"����ͨ������ʹ�ã�����ɾ����ͨ���Ĺ����ڴ�",
		"��֧�ֵ�option",
		"ѡ��ֵ����ȷ",
		"tbus channel����������ݵ�ͷ���ṹָ��ΪNULL",
		"TBus channel�����ڴ������м�¼��Magicƥ�䲻��",
		"Tbus ����ͨ���е��ֽڶ������ò���ȷ",
		"TBUS  ͨ�������С�빲���ڴ�ʵ�ʴ�С��һ��",
		"TBUS ����ͨ���Ĵ�С�� ָ�����õ�ֵ��һ��",
		"TBus ͨ�������еĲ�������ȷ",
        "��Ӧ��ʹ�������TBUS��ַ",
        "û��ƥ��ĶԶ˽���",
        "û�и����peer����",
	    "���յ�ַ�Ļ�����̫С�������Խ������е�peer��ַ",
	};

	assert((sizeof(message) / sizeof(message[0])) == TBUS_ERROR_COUNT);

	if (!TBUS_ERR_IS_ERROR(iErrorCode))
	{
		pchStr = "û�д���";
	}else  if ((iTdrErrno < 0) || (iTdrErrno > TBUS_ERROR_COUNT))
	{
		pchStr =  "δ֪����";
	}
	else
	{
		pchStr = message[iTdrErrno];
	}

	return pchStr;
}
