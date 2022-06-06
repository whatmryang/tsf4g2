/**
*
* @file     test_date_io.c
* @brief    测试md5hash算法
* 
* @author jackyai  
* @version 1.0
* @date 2007-10-30 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include "tdr_md5.h"

void Md5Test( void );

int ut_add_md5hash_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite 增加一个Suite 
	//InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
	pSuite = CU_add_suite("ut_md5hash_test_suites", NULL, NULL);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	//注册当前Suite下的测试用例　
	if( NULL == CU_add_test(pSuite, "Md5Test", (CU_TestFunc)Md5Test))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	



	return 0;
}

/*
**********************************************************************************************
MD5自测试
**********************************************************************************************
*/



static struct {
	char *data;						/* Data to hash */
	int length;						/* Length of data */
	unsigned char digest[ TDR_MD5_DIGEST_LENGTH ];	/* Digest of data */
} digestValues[] = {
	{ "", 0,
	{ 0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,
	0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E } },
	{ "a", 1,
	{ 0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,
	0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61 } },
	{ "abc", 3,
	{ 0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,
	0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72 } },
	{ "message digest", 14,
	{ 0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,
	0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0 } },
	{ "abcdefghijklmnopqrstuvwxyz", 26,
	{ 0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,
	0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B } },
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", 62,
	{ 0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,
	0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F } },
	{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890", 80,
	{ 0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,
	0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A } },
	{ NULL, 0, { 0 } }
};


void Md5Test( void )
{
	unsigned char digest[ TDR_MD5_DIGEST_LENGTH ];
	int i;
	char szDigest[TDR_MD5_DIGEST_LENGTH*2 + 1];
	
	/* Test MD5 against the test vectors given in RFC 1320 */
	for( i = 0; digestValues[ i ].data != NULL; i++ )
	{
		/*void Md5HashBuffer( BYTE *outBuffer, BYTE *inBuffer, int length)*/
		/*md5HashBuffer( NULL, digest, ( BYTE * ) digestValues[ i ].data,
		digestValues[ i ].length, HASH_ALL );*/
		tdr_md5hash_buffer( digest, ( unsigned char * ) digestValues[ i ].data,
			digestValues[ i ].length);
		
		CU_ASSERT(0 == memcmp( digest, digestValues[ i ].digest, TDR_MD5_DIGEST_LENGTH ) );
		tdr_md5hash2str(digest, szDigest, sizeof(szDigest));
		tdr_str2md5hash(digest, szDigest);
	}
	

}
