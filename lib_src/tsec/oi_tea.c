
#include "tsec/md5.h"
#include "tsec/oi_tea.h"

#include <stdlib.h>
#include <string.h>

#if defined (_WIN32) || defined (_WIN64)
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

/*
	void TeaEncryptECB(char *pInBuf, char *pKey, char *pOutBuf);
	void TeaDecryptECB(char *pInBuf, char *pKey, char *pOutBuf);
*/

//typedef char BYTE;
//typedef char BOOL;
typedef unsigned long DWORD;

typedef unsigned int WORD32;

const WORD32 DELTA = 0x9e3779b9;

#define ROUNDS 16
#define LOG_ROUNDS 4


/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
void TeaEncryptECB(const char *pInBuf, const char *pKey, char *pOutBuf)
{
	WORD32 y, z;
	WORD32 sum;
	WORD32 k[4];
	int i;

	/*plain-text is TCP/IP-endian;*/

	/*GetBlockBigEndian(in, y, z);*/
	y = ntohl(*((WORD32*)pInBuf));
	z = ntohl(*((WORD32*)(pInBuf+4)));
	/*TCP/IP network char order (which is big-endian).*/

	for ( i = 0; i<4; i++)
	{
		/*now key is TCP/IP-endian;*/
		k[i] = ntohl(*((WORD32*)(pKey+i*4)));
	}

	sum = 0;
	for (i=0; i<ROUNDS; i++)
	{
		sum += DELTA;
		y += ((z << 4) + k[0]) ^ (z + sum) ^ ((z >> 5) + k[1]);
		z += ((y << 4) + k[2]) ^ (y + sum) ^ ((y >> 5) + k[3]);
	}



	*((WORD32*)pOutBuf) = htonl(y);
	*((WORD32*)(pOutBuf+4)) = htonl(z);


	/*now encrypted buf is TCP/IP-endian;*/
}

/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
void TeaDecryptECB(const char *pInBuf, const char *pKey, char *pOutBuf)
{
	WORD32 y, z, sum;
	WORD32 k[4];
	int i;

	/*now encrypted buf is TCP/IP-endian;*/
	/*TCP/IP network char order (which is big-endian).*/
	y = ntohl(*((WORD32*)pInBuf));
	z = ntohl(*((WORD32*)(pInBuf+4)));

	for ( i=0; i<4; i++)
	{
		/*key is TCP/IP-endian;*/
		k[i] = ntohl(*((WORD32*)(pKey+i*4)));
	}

	sum = DELTA << LOG_ROUNDS;
	for (i=0; i<ROUNDS; i++)
	{
		z -= ((y << 4) + k[2]) ^ (y + sum )^ ((y >> 5) + k[3]);
		y -= ((z << 4) + k[0]) ^ (z + sum )^ ((z >> 5) + k[1]);
		sum -= DELTA;
	}

	*((WORD32*)pOutBuf) = htonl(y);
	*((WORD32*)(pOutBuf+4)) = htonl(z);

	/*now plain-text is TCP/IP-endian;*/
}


/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
void TeaEncryptECB3(const char *pInBuf, const char *pKey, char *pOutBuf)
{
	WORD32 y, z;
	WORD32 sum;
	WORD32 k[4];
	int i;

	/*plain-text is TCP/IP-endian;*/

	/*GetBlockBigEndian(in, y, z);*/
	y = ntohl(*((WORD32*)pInBuf));
	z = ntohl(*((WORD32*)(pInBuf+4)));
	/*TCP/IP network char order (which is big-endian).*/

	for ( i = 0; i<4; i++)
	{
		/*now key is TCP/IP-endian;*/
		k[i] = ntohl(*((WORD32*)(pKey+i*4)));
	}

	sum = 0;
	for (i=0; i<13; i++)
	{
		sum += DELTA;
		y += ((z << 4) + k[0]) ^ (z + sum) ^ ((z >> 5) + k[1]);
		z += ((y << 4) + k[2]) ^ (y + sum) ^( (y >> 5) + k[3]);
	}



	*((WORD32*)pOutBuf) = htonl(y);
	*((WORD32*)(pOutBuf+4)) = htonl(z);


	/*now encrypted buf is TCP/IP-endian;*/
}

/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
void TeaDecryptECB3(const char *pInBuf, const char *pKey, char *pOutBuf)
{
	WORD32 y, z, sum;
	WORD32 k[4];
	int i;

	/*now encrypted buf is TCP/IP-endian;*/
	/*TCP/IP network char order (which is big-endian).*/
	y = ntohl(*((WORD32*)pInBuf));
	z = ntohl(*((WORD32*)(pInBuf+4)));

	for ( i=0; i<4; i++)
	{
		/*key is TCP/IP-endian;*/
		k[i] = ntohl(*((WORD32*)(pKey+i*4)));
	}

	/*13=8+5*/
	sum = DELTA << 3;
	for (i=1; i<=5; i++)
	{
		sum += DELTA;
	}


	for (i=0; i<13; i++)
	{
		z -= ((y << 4) + k[2]) ^ (y + sum )^ ((y >> 5) + k[3]);
		y -= ((z << 4) + k[0]) ^ (z + sum) ^ ((z >> 5) + k[1]);
		sum -= DELTA;
	}

	*((WORD32*)pOutBuf) = htonl(y);
	*((WORD32*)(pOutBuf+4)) = htonl(z);

	/*now plain-text is TCP/IP-endian;*/
}




#define SALT_LEN 2
#define ZERO_LEN 7

/*pKey为16char*/
/*
	输入:pInBuf为需加密的明文部分(Body),nInBufLen为pInBuf长度;
	输出:pOutBuf为密文格式,pOutBufLen为pOutBuf的长度是8char的倍数;
*/
/*TEA加密算法,CBC模式*/
/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
void oi_symmetry_encrypt(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen)
{

	int nPadSaltBodyZeroLen/*PadLen(1char)+Salt+Body+Zero的长度*/;
	int nPadlen;
	char src_buf[8], zero_iv[8], *iv_buf;
	int src_i, i, j;

	/*根据Body长度计算PadLen,最小必需长度必需为8char的整数倍*/
	nPadSaltBodyZeroLen = nInBufLen/*Body长度*/+1+SALT_LEN+ZERO_LEN/*PadLen(1char)+Salt(2char)+Zero(7char)*/;
    nPadlen=nPadSaltBodyZeroLen%8;
	if(0 != nPadlen) /*len=nSaltBodyZeroLen%8*/
	{
		/*模8余0需补0,余1补7,余2补6,...,余7补1*/
		nPadlen=8-nPadlen;
	}

	/*srand( (unsigned)time( NULL ) ); 初始化随机数*/
	/*加密第一块数据(8char),取前面10char*/
	src_buf[0] = ((((char)rand()) & 0x0f8/*最低三位存PadLen,清零*/) | (char)nPadlen);
	src_i = 1; /*src_i指向src_buf下一个位置*/

	while(nPadlen--)
		src_buf[src_i++]=(char)rand(); /*Padding*/

	/*come here, i must <= 8*/

	memset(zero_iv, 0, 8);
	iv_buf = zero_iv; /*make iv*/

	*pOutBufLen = 0; /*init OutBufLen*/

	for (i=1;i<=SALT_LEN;) /*Salt(2char)*/
	{
		if (src_i<8)
		{
			src_buf[src_i++]=(char)rand();
			i++; /*i inc in here*/
		}

		if (src_i==8)
		{
			/*src_i==8*/

			for (j=0;j<8;j++) /*CBC XOR*/
				src_buf[j]^=iv_buf[j];
			/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
			TeaEncryptECB(src_buf, pKey, pOutBuf);
			src_i=0;
			iv_buf=pOutBuf;
			*pOutBufLen+=8;
			pOutBuf+=8;
		}
	}

	/*src_i指向src_buf下一个位置*/

	while(nInBufLen)
	{
		if (src_i<8)
		{
			src_buf[src_i++]=*(pInBuf++);
			nInBufLen--;
		}

		if (src_i==8)
		{
			/*src_i==8*/

			for (i=0;i<8;i++) /*CBC XOR*/
				src_buf[i]^=iv_buf[i];
			/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
			TeaEncryptECB(src_buf, pKey, pOutBuf);
			src_i=0;
			iv_buf=pOutBuf;
			*pOutBufLen+=8;
			pOutBuf+=8;
		}
	}

	/*src_i指向src_buf下一个位置*/

	for (i=1;i<=ZERO_LEN;)
	{
		if (src_i<8)
		{
			src_buf[src_i++]=0;
			i++; /*i inc in here*/
		}

		if (src_i==8)
		{
			/*src_i==8*/

			for (j=0;j<8;j++) /*CBC XOR*/
				src_buf[j]^=iv_buf[j];
			/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
			TeaEncryptECB(src_buf, pKey, pOutBuf);
			src_i=0;
			iv_buf=pOutBuf;
			*pOutBufLen+=8;
			pOutBuf+=8;
		}
	}

}

/*pKey为16char*/
/*
	输入:pInBuf为密文格式,nInBufLen为pInBuf的长度是8char的倍数;
	输出:pOutBuf为明文(Body),pOutBufLen为pOutBuf的长度;
	返回值:如果格式正确返回TRUE;
*/
/*TEA解密算法,CBC模式*/
/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
char oi_symmetry_decrypt(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen)
{

	int nPadLen, nPlainLen;
	char dest_buf[8];
	const char *iv_buf;
	int dest_i, i, j;


	if ((nInBufLen%8) || (nInBufLen<16)) return FALSE;


	TeaDecryptECB(pInBuf, pKey, dest_buf);

	nPadLen = dest_buf[0] & 0x7/*只要最低三位*/;

	/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
	i = nInBufLen-1/*PadLen(1char)*/-nPadLen-SALT_LEN-ZERO_LEN; /*明文长度*/
	if (*pOutBufLen<i) return FALSE;
	*pOutBufLen = i;
	if (*pOutBufLen < 0) return FALSE;


	iv_buf = pInBuf; /*init iv*/
	nInBufLen -= 8;
	pInBuf += 8;

	dest_i=1; /*dest_i指向dest_buf下一个位置*/


	/*把Padding滤掉*/
	dest_i+=nPadLen;

	/*dest_i must <=8*/

	/*把Salt滤掉*/
	for (i=1; i<=SALT_LEN;)
	{
		if (dest_i<8)
		{
			dest_i++;
			i++;
		}

		if (dest_i==8)
		{
			/*dest_i==8*/
			TeaDecryptECB(pInBuf, pKey, dest_buf);
			for (j=0; j<8; j++)
				dest_buf[j]^=iv_buf[j];

			iv_buf = pInBuf;
			nInBufLen -= 8;
			pInBuf += 8;

			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}
	}

	/*还原明文*/

	nPlainLen=*pOutBufLen;
	while (nPlainLen)
	{
		if (dest_i<8)
		{
			*(pOutBuf++)=dest_buf[dest_i++];
			nPlainLen--;
		}
		else if (dest_i==8)
		{
			/*dest_i==8*/
			TeaDecryptECB(pInBuf, pKey, dest_buf);
			for (i=0; i<8; i++)
				dest_buf[i]^=iv_buf[i];

			iv_buf = pInBuf;
			nInBufLen -= 8;
			pInBuf += 8;

			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}
	}

	/*校验Zero*/
	for (i=1;i<=ZERO_LEN;)
	{
		if (dest_i<8)
		{
			if(dest_buf[dest_i++]) return FALSE;
			i++;
		}
		else if (dest_i==8)
		{
			/*dest_i==8*/
			TeaDecryptECB(pInBuf, pKey, dest_buf);
			for (j=0; j<8; j++)
				dest_buf[j]^=iv_buf[j];

			iv_buf = pInBuf;
			nInBufLen -= 8;
			pInBuf += 8;

			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}

	}

	return TRUE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////// */

/*pKey为16char*/
/*
	输入:nInBufLen为需加密的明文部分(Body)长度;
	输出:返回为加密后的长度(是8char的倍数);
*/
/*TEA加密算法,CBC模式*/
/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
int oi_symmetry_encrypt2_len(int nInBufLen)
{

	int nPadSaltBodyZeroLen/*PadLen(1char)+Salt+Body+Zero的长度*/;
	int nPadlen;

	/*根据Body长度计算PadLen,最小必需长度必需为8char的整数倍*/
	nPadSaltBodyZeroLen = nInBufLen/*Body长度*/+1+SALT_LEN+ZERO_LEN/*PadLen(1char)+Salt(2char)+Zero(7char)*/;
    nPadlen=nPadSaltBodyZeroLen%8;
	if(0 != nPadlen) /*len=nSaltBodyZeroLen%8*/
	{
		/*模8余0需补0,余1补7,余2补6,...,余7补1*/
		nPadlen=8-nPadlen;
	}

	return nPadSaltBodyZeroLen+nPadlen;
}


/*pKey为16char*/
/*
	输入:pInBuf为需加密的明文部分(Body),nInBufLen为pInBuf长度;
	输出:pOutBuf为密文格式,pOutBufLen为pOutBuf的长度是8char的倍数;
*/
/*TEA加密算法,CBC模式*/
/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
void oi_symmetry_encrypt2(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen)
{

	int nPadSaltBodyZeroLen/*PadLen(1char)+Salt+Body+Zero的长度*/;
	int nPadlen;
	char src_buf[8], iv_plain[8], *iv_crypt;
	int src_i, i, j;

	/*根据Body长度计算PadLen,最小必需长度必需为8char的整数倍*/
	nPadSaltBodyZeroLen = nInBufLen/*Body长度*/+1+SALT_LEN+ZERO_LEN/*PadLen(1char)+Salt(2char)+Zero(7char)*/;
    nPadlen=nPadSaltBodyZeroLen%8;
	if(0 != nPadlen) /*len=nSaltBodyZeroLen%8*/
	{
		/*模8余0需补0,余1补7,余2补6,...,余7补1*/
		nPadlen=8-nPadlen;
	}

	/*srand( (unsigned)time( NULL ) ); 初始化随机数*/
	/*加密第一块数据(8char),取前面10char*/
	src_buf[0] = ((((char)rand()) & 0x0f8/*最低三位存PadLen,清零*/) | (char)nPadlen);
	src_i = 1; /*src_i指向src_buf下一个位置*/

	while(nPadlen--)
		src_buf[src_i++]=(char)rand(); /*Padding*/

	/*come here, src_i must <= 8*/

	for ( i=0; i<8; i++)
		iv_plain[i] = 0;
	iv_crypt = iv_plain; /*make zero iv*/

	*pOutBufLen = 0; /*init OutBufLen*/

	for (i=1;i<=SALT_LEN;) /*Salt(2char)*/
	{
		if (src_i<8)
		{
			src_buf[src_i++]=(char)rand();
			i++; /*i inc in here*/
		}

		if (src_i==8)
		{
			/*src_i==8*/

			for (j=0;j<8;j++) /*加密前异或前8个char的密文(iv_crypt指向的)*/
				src_buf[j]^=iv_crypt[j];

			/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
			/*加密*/
			TeaEncryptECB(src_buf, pKey, pOutBuf);

			for (j=0;j<8;j++) /*加密后异或前8个char的明文(iv_plain指向的)*/
				pOutBuf[j]^=iv_plain[j];

			/*保存当前的iv_plain*/
			for (j=0;j<8;j++)
				iv_plain[j]=src_buf[j];

			/*更新iv_crypt*/
			src_i=0;
			iv_crypt=pOutBuf;
			*pOutBufLen+=8;
			pOutBuf+=8;
		}
	}

	/*src_i指向src_buf下一个位置*/

	while(nInBufLen)
	{
		if (src_i<8)
		{
			src_buf[src_i++]=*(pInBuf++);
			nInBufLen--;
		}

		if (src_i==8)
		{
			/*src_i==8*/

			for (j=0;j<8;j++) /*加密前异或前8个char的密文(iv_crypt指向的)*/
				src_buf[j]^=iv_crypt[j];
			/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
			TeaEncryptECB(src_buf, pKey, pOutBuf);

			for (j=0;j<8;j++) /*加密后异或前8个char的明文(iv_plain指向的)*/
				pOutBuf[j]^=iv_plain[j];

			/*保存当前的iv_plain*/
			for (j=0;j<8;j++)
				iv_plain[j]=src_buf[j];

			src_i=0;
			iv_crypt=pOutBuf;
			*pOutBufLen+=8;
			pOutBuf+=8;
		}
	}

	/*src_i指向src_buf下一个位置*/

	for (i=1;i<=ZERO_LEN;)
	{
		if (src_i<8)
		{
			src_buf[src_i++]=0;
			i++; /*i inc in here*/
		}

		if (src_i==8)
		{
			/*src_i==8*/

			for (j=0;j<8;j++) /*加密前异或前8个char的密文(iv_crypt指向的)*/
				src_buf[j]^=iv_crypt[j];
			/*pOutBuffer、pInBuffer均为8char, pKey为16char*/
			TeaEncryptECB(src_buf, pKey, pOutBuf);

			for (j=0;j<8;j++) /*加密后异或前8个char的明文(iv_plain指向的)*/
				pOutBuf[j]^=iv_plain[j];

			/*保存当前的iv_plain*/
			for (j=0;j<8;j++)
				iv_plain[j]=src_buf[j];

			src_i=0;
			iv_crypt=pOutBuf;
			*pOutBufLen+=8;
			pOutBuf+=8;
		}
	}

}


/*pKey为16char*/
/*
	输入:pInBuf为密文格式,nInBufLen为pInBuf的长度是8char的倍数; *pOutBufLen为接收缓冲区的长度
		特别注意*pOutBufLen应预置接收缓冲区的长度!
	输出:pOutBuf为明文(Body),pOutBufLen为pOutBuf的长度,至少应预留nInBufLen-10;
	返回值:如果格式正确返回TRUE;
*/
/*TEA解密算法,CBC模式*/
/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
char oi_symmetry_decrypt2(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen)
{

	int nPadLen, nPlainLen;
	char dest_buf[8], zero_buf[8];
	const char *iv_pre_crypt, *iv_cur_crypt;
	int dest_i, i, j;
	const char *pInBufBoundary;
	int nBufPos;
	nBufPos = 0;



	if ((nInBufLen%8) || (nInBufLen<16)) return FALSE;


	TeaDecryptECB(pInBuf, pKey, dest_buf);

	nPadLen = dest_buf[0] & 0x7/*只要最低三位*/;

	/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
	i = nInBufLen-1/*PadLen(1char)*/-nPadLen-SALT_LEN-ZERO_LEN; /*明文长度*/
	if ((*pOutBufLen<i) || (i<0)) return FALSE;
	*pOutBufLen = i;

	pInBufBoundary = pInBuf + nInBufLen; /*输入缓冲区的边界，下面不能pInBuf>=pInBufBoundary*/


	for ( i=0; i<8; i++)
		zero_buf[i] = 0;

	iv_pre_crypt = zero_buf;
	iv_cur_crypt = pInBuf; /*init iv*/

	pInBuf += 8;
	nBufPos += 8;

	dest_i=1; /*dest_i指向dest_buf下一个位置*/


	/*把Padding滤掉*/
	dest_i+=nPadLen;

	/*dest_i must <=8*/

	/*把Salt滤掉*/
	for (i=1; i<=SALT_LEN;)
	{
		if (dest_i<8)
		{
			dest_i++;
			i++;
		}
		else if (dest_i==8)
		{
			/*解开一个新的加密块*/

			/*改变前一个加密块的指针*/
			iv_pre_crypt = iv_cur_crypt;
			iv_cur_crypt = pInBuf;

			/*异或前一块明文(在dest_buf[]中)*/
			for (j=0; j<8; j++)
			{
				if( (nBufPos + j) >= nInBufLen)
					return FALSE;
				dest_buf[j]^=pInBuf[j];
			}

			/*dest_i==8*/
			TeaDecryptECB(dest_buf, pKey, dest_buf);

			/*在取出的时候才异或前一块密文(iv_pre_crypt)*/


			pInBuf += 8;
			nBufPos += 8;

			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}
	}

	/*还原明文*/

	nPlainLen=*pOutBufLen;
	while (nPlainLen)
	{
		if (dest_i<8)
		{
			*(pOutBuf++)=dest_buf[dest_i]^iv_pre_crypt[dest_i];
			dest_i++;
			nPlainLen--;
		}
		else if (dest_i==8)
		{
			/*dest_i==8*/

			/*改变前一个加密块的指针*/
			iv_pre_crypt = iv_cur_crypt;
			iv_cur_crypt = pInBuf;

			/*解开一个新的加密块*/

			/*异或前一块明文(在dest_buf[]中)*/
			for (j=0; j<8; j++)
			{
				if( (nBufPos + j) >= nInBufLen)
					return FALSE;
				dest_buf[j]^=pInBuf[j];
			}

			TeaDecryptECB(dest_buf, pKey, dest_buf);

			/*在取出的时候才异或前一块密文(iv_pre_crypt)*/


			pInBuf += 8;
			nBufPos += 8;

			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}
	}

	/*校验Zero*/
	for (i=1;i<=ZERO_LEN;)
	{
		if (dest_i<8)
		{
			if(dest_buf[dest_i]^iv_pre_crypt[dest_i]) return FALSE;
			dest_i++;
			i++;
		}
		else if (dest_i==8)
		{
			/*改变前一个加密块的指针*/
			iv_pre_crypt = iv_cur_crypt;
			iv_cur_crypt = pInBuf;

			/*解开一个新的加密块*/

			/*异或前一块明文(在dest_buf[]中)*/
			for (j=0; j<8; j++)
			{
				if( (nBufPos + j) >= nInBufLen)
					return FALSE;
				dest_buf[j]^=pInBuf[j];
			}

			TeaDecryptECB(dest_buf, pKey, dest_buf);

			/*在取出的时候才异或前一块密文(iv_pre_crypt)*/


			pInBuf += 8;
			nBufPos += 8;
			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}

	}

	return TRUE;
}



/* ///////////////////////////////////////////////////////////////////////////////////////////// */

/*pKey为16char*/
/*
	输入:nInBufLen为需加密的明文部分(Body)长度;
	输出:返回为加密后的长度(是8char的倍数);
*/
/*TEA加密算法,CBC模式*/
/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
int qq_symmetry_encrypt3_len(int nInBufLen)
{

	int nPadSaltBodyZeroLen/*PadLen(1char)+Salt+Body+Zero的长度*/;
	int nPadlen;

	/*根据Body长度计算PadLen,最小必需长度必需为8char的整数倍*/
	nPadSaltBodyZeroLen = nInBufLen/*Body长度*/+1+SALT_LEN+ZERO_LEN/*PadLen(1char)+Salt(2char)+Zero(7char)*/;
    nPadlen=nPadSaltBodyZeroLen%8;
	if(0 != nPadlen) /*len=nSaltBodyZeroLen%8*/
	{
		/*模8余0需补0,余1补7,余2补6,...,余7补1*/
		nPadlen=8-nPadlen;
	}

	return nPadSaltBodyZeroLen+nPadlen;
}




/*pKey为16char*/
/*
	输入:pInBuf为密文格式,nInBufLen为pInBuf的长度是8char的倍数; *pOutBufLen为接收缓冲区的长度
		特别注意*pOutBufLen应预置接收缓冲区的长度!
	输出:pOutBuf为明文(Body),pOutBufLen为pOutBuf的长度,至少应预留nInBufLen-10;
	返回值:如果格式正确返回TRUE;
*/
/*TEA解密算法,CBC模式*/
/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
/*
	Hash(Hash(chMainVerc1, uin1), Hash(c2,uin2)) ^ pKey
*/
char qq_symmetry_decrypt3(const char* pInBuf, int nInBufLen, char chMainVer, char chSubVer, DWORD dwUin, const char* pKey, char* pOutBuf, int *pOutBufLen)
{

	int nPadLen, nPlainLen;
	char dest_buf[8], zero_buf[8], key[16], temp_buf[32];
	const char *iv_pre_crypt, *iv_cur_crypt;
	int dest_i, i, j;
	const char *pInBufBoundary;
	int nBufPos;
	nBufPos = 0;


	if ((nInBufLen%8) || (nInBufLen<16)) return FALSE;


	dwUin = htonl(dwUin);
	/**************************************************/
	/*
		Hash(Hash(chMainVerc1, uin1), Hash(c2,uin2)) ^ pKey
	*/

	temp_buf[0] = chMainVer;
	*(DWORD*)&temp_buf[1] = dwUin;
	Md5HashBuffer( temp_buf, &temp_buf[0], sizeof(char)+sizeof(DWORD));

	temp_buf[16] = chSubVer;
	*(DWORD*)&temp_buf[17] = dwUin;
	Md5HashBuffer( &temp_buf[16], &temp_buf[16], sizeof(char)+sizeof(DWORD));

	Md5HashBuffer( key, &temp_buf[0], sizeof(temp_buf));

	for (i=0; i<(int)sizeof(key); i++)
	{
		key[i] ^= *(pKey+i);
	}

	/**************************************************/

	TeaDecryptECB3(pInBuf, key, dest_buf);

	nPadLen = dest_buf[0] & 0x7/*只要最低三位*/;

	/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/
	i = nInBufLen-1/*PadLen(1char)*/-nPadLen-SALT_LEN-ZERO_LEN; /*明文长度*/
	if ((*pOutBufLen<i) || (i<0)) return FALSE;
	*pOutBufLen = i;

	pInBufBoundary = pInBuf + nInBufLen; /*输入缓冲区的边界，下面不能pInBuf>=pInBufBoundary*/


	for ( i=0; i<8; i++)
		zero_buf[i] = 0;

	iv_pre_crypt = zero_buf;
	iv_cur_crypt = pInBuf; /*init iv*/

	pInBuf += 8;
	nBufPos += 8;

	dest_i=1; /*dest_i指向dest_buf下一个位置*/


	/*把Padding滤掉*/
	dest_i+=nPadLen;

	/*dest_i must <=8*/

	/*把Salt滤掉*/
	for (i=1; i<=SALT_LEN;)
	{
		if (dest_i<8)
		{
			dest_i++;
			i++;
		}
		else if (dest_i==8)
		{
			/*解开一个新的加密块*/

			/*改变前一个加密块的指针*/
			iv_pre_crypt = iv_cur_crypt;
			iv_cur_crypt = pInBuf;

			/*异或前一块明文(在dest_buf[]中)*/
			for (j=0; j<8; j++)
			{
				if( (nBufPos + j) >= nInBufLen)
					return FALSE;
				dest_buf[j]^=pInBuf[j];
			}

			/*dest_i==8*/
			TeaDecryptECB3(dest_buf, key, dest_buf);

			/*在取出的时候才异或前一块密文(iv_pre_crypt)*/


			pInBuf += 8;
			nBufPos += 8;

			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}
	}

	/*还原明文*/

	nPlainLen=*pOutBufLen;
	while (nPlainLen)
	{
		if (dest_i<8)
		{
			*(pOutBuf++)=dest_buf[dest_i]^iv_pre_crypt[dest_i];
			dest_i++;
			nPlainLen--;
		}
		else if (dest_i==8)
		{
			/*dest_i==8*/

			/*改变前一个加密块的指针*/
			iv_pre_crypt = iv_cur_crypt;
			iv_cur_crypt = pInBuf;

			/*解开一个新的加密块*/

			/*异或前一块明文(在dest_buf[]中)*/
			for (j=0; j<8; j++)
			{
				if( (nBufPos + j) >= nInBufLen)
					return FALSE;
				dest_buf[j]^=pInBuf[j];
			}

			TeaDecryptECB3(dest_buf, key, dest_buf);

			/*在取出的时候才异或前一块密文(iv_pre_crypt)*/


			pInBuf += 8;
			nBufPos += 8;

			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}
	}

	/*校验Zero*/
	for (i=1;i<=ZERO_LEN;)
	{
		if (dest_i<8)
		{
			if(dest_buf[dest_i]^iv_pre_crypt[dest_i]) return FALSE;
			dest_i++;
			i++;
		}
		else if (dest_i==8)
		{
			/*改变前一个加密块的指针*/
			iv_pre_crypt = iv_cur_crypt;
			iv_cur_crypt = pInBuf;

			/*解开一个新的加密块*/

			/*异或前一块明文(在dest_buf[]中)*/
			for (j=0; j<8; j++)
			{
				if( (nBufPos + j) >= nInBufLen)
					return FALSE;
				dest_buf[j]^=pInBuf[j];
			}

			TeaDecryptECB3(dest_buf, key, dest_buf);

			/*在取出的时候才异或前一块密文(iv_pre_crypt)*/


			pInBuf += 8;
			nBufPos += 8;
			dest_i=0; /*dest_i指向dest_buf下一个位置*/
		}

	}

	return TRUE;
}
