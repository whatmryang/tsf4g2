/*
qqcrypt.h


QQ����
hyj@tencent.com
1999/12/24
2002/03/27

  ʵ�������㷨:
  Hash�㷨: MD5,��ʵ��
  �Գ��㷨: DES,δʵ��
  �ǶԳ��㷨: RSA,δʵ��


2002/06/14
	�޸�v3���ܺ�����ȥ��v3���ܺ���

*/

typedef char BYTE;
typedef char BOOL;
typedef unsigned long DWORD;
#define TRUE 1
#define FALSE 0


#ifndef _INCLUDED_OICQCRYPT_H_
#define _INCLUDED_OICQCRYPT_H_



#define MD5_DIGEST_LENGTH	16
#define ENCRYPT_PADLEN		18
#define	CRYPT_KEY_SIZE		16

#ifdef _DEBUG
	BOOL Md5Test(); /*����MD5�����Ƿ���rfc1321����*/
#endif



/************************************************************************************************
	MD5���ݽṹ
************************************************************************************************/

#define MD5_LBLOCK	16
typedef struct MD5state_st
	{
	unsigned long A,B,C,D;
	unsigned long Nl,Nh;
	unsigned long data[MD5_LBLOCK];
	int num;
	} MD5_CTX;

void MD5_Init(MD5_CTX *c);
void MD5_Update(MD5_CTX *c, const register unsigned char *data, unsigned long len);
void MD5_Final(unsigned char *md, MD5_CTX *c);


/************************************************************************************************
	Hash����
************************************************************************************************/
/*
	����const BYTE *inBuffer��int length
	���BYTE *outBuffer
	����length��Ϊ0,outBuffer�ĳ���ΪMD5_DIGEST_LENGTH(16byte)
*/
void Md5HashBuffer( BYTE *outBuffer, const BYTE *inBuffer, int length);



/************************************************************************************************
	�ԳƼ��ܵײ㺯��
************************************************************************************************/
//pOutBuffer��pInBuffer��Ϊ8byte, pKeyΪ16byte
void TeaEncryptECB(const BYTE *pInBuf, const BYTE *pKey, BYTE *pOutBuf);
void TeaDecryptECB(const BYTE *pInBuf, const BYTE *pKey, BYTE *pOutBuf);
void TeaEncryptECB3(const BYTE *pInBuf, const BYTE *pKey, BYTE *pOutBuf);
void TeaDecryptECB3(const BYTE *pInBuf, const BYTE *pKey, BYTE *pOutBuf);



/************************************************************************************************
	QQ�ԳƼ��ܵ�һ������
************************************************************************************************/

/*pKeyΪ16byte*/
/*
	����:pInBufΪ����ܵ����Ĳ���(Body),nInBufLenΪpInBuf����;
	���:pOutBufΪ���ĸ�ʽ,pOutBufLenΪpOutBuf�ĳ�����8byte�ı���,����ӦԤ��nInBufLen+17;
*/
/*TEA�����㷨,CBCģʽ*/
/*���ĸ�ʽ:PadLen(1byte)+Padding(var,0-7byte)+Salt(2byte)+Body(var byte)+Zero(7byte)*/
void oi_symmetry_encrypt(const BYTE* pInBuf, int nInBufLen, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen);

/*pKeyΪ16byte*/
/*
	����:pInBufΪ���ĸ�ʽ,nInBufLenΪpInBuf�ĳ�����8byte�ı���; *pOutBufLenΪ���ջ������ĳ���
		�ر�ע��*pOutBufLenӦԤ�ý��ջ������ĳ���!
	���:pOutBufΪ����(Body),pOutBufLenΪpOutBuf�ĳ���,����ӦԤ��nInBufLen-10;
	����ֵ:�����ʽ��ȷ����TRUE;
*/
/*TEA�����㷨,CBCģʽ*/
/*���ĸ�ʽ:PadLen(1byte)+Padding(var,0-7byte)+Salt(2byte)+Body(var byte)+Zero(7byte)*/
BOOL oi_symmetry_decrypt(const BYTE* pInBuf, int nInBufLen, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen);







/************************************************************************************************
	QQ�ԳƼ��ܵڶ�������
************************************************************************************************/

/*pKeyΪ16byte*/
/*
	����:nInBufLenΪ����ܵ����Ĳ���(Body)����;
	���:����Ϊ���ܺ�ĳ���(��8byte�ı���);
*/
/*TEA�����㷨,CBCģʽ*/
/*���ĸ�ʽ:PadLen(1byte)+Padding(var,0-7byte)+Salt(2byte)+Body(var byte)+Zero(7byte)*/
int oi_symmetry_encrypt2_len(int nInBufLen);


/*pKeyΪ16byte*/
/*
	����:pInBufΪ����ܵ����Ĳ���(Body),nInBufLenΪpInBuf����;
	���:pOutBufΪ���ĸ�ʽ,pOutBufLenΪpOutBuf�ĳ�����8byte�ı���,����ӦԤ��nInBufLen+17;
*/
/*TEA�����㷨,CBCģʽ*/
/*���ĸ�ʽ:PadLen(1byte)+Padding(var,0-7byte)+Salt(2byte)+Body(var byte)+Zero(7byte)*/
void oi_symmetry_encrypt2(const BYTE* pInBuf, int nInBufLen, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen);



/*pKeyΪ16byte*/
/*
	����:pInBufΪ���ĸ�ʽ,nInBufLenΪpInBuf�ĳ�����8byte�ı���; *pOutBufLenΪ���ջ������ĳ���
		�ر�ע��*pOutBufLenӦԤ�ý��ջ������ĳ���!
	���:pOutBufΪ����(Body),pOutBufLenΪpOutBuf�ĳ���,����ӦԤ��nInBufLen-10;
	����ֵ:�����ʽ��ȷ����TRUE;
*/
/*TEA�����㷨,CBCģʽ*/
/*���ĸ�ʽ:PadLen(1byte)+Padding(var,0-7byte)+Salt(2byte)+Body(var byte)+Zero(7byte)*/
BOOL oi_symmetry_decrypt2(const BYTE* pInBuf, int nInBufLen, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen);




/************************************************************************************************
	QQ�Գƽ��ܵ���������
************************************************************************************************/

/*pKeyΪ16byte*/
/*
	����:nInBufLenΪ����ܵ����Ĳ���(Body)����;
	���:����Ϊ���ܺ�ĳ���(��8byte�ı���);
*/
/*TEA�����㷨,CBCģʽ*/
/*���ĸ�ʽ:PadLen(1byte)+Padding(var,0-7byte)+Salt(2byte)+Body(var byte)+Zero(7byte)*/
int oi_symmetry_encrypt3_len(int nInBufLen);


/*pKeyΪ16byte*/
/*
	����:pInBufΪ���ĸ�ʽ,nInBufLenΪpInBuf�ĳ�����8byte�ı���; *pOutBufLenΪ���ջ������ĳ���
		�ر�ע��*pOutBufLenӦԤ�ý��ջ������ĳ���!
	���:pOutBufΪ����(Body),pOutBufLenΪpOutBuf�ĳ���,����ӦԤ��nInBufLen-10;
	����ֵ:�����ʽ��ȷ����TRUE;
*/
/*TEA�����㷨,CBCģʽ*/
/*���ĸ�ʽ:PadLen(1byte)+Padding(var,0-7byte)+Salt(2byte)+Body(var byte)+Zero(7byte)*/
BOOL qq_symmetry_decrypt3(const BYTE* pInBuf, int nInBufLen, BYTE chMainVer, BYTE chSubVer, DWORD dwUin, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen);

#endif // #ifndef _INCLUDED_OICQCRYPT_H_

