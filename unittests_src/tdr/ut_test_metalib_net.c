/**
*
* @file     test_metalib_net.c  
* @brief    测试创建Metalib的函数
* 
* @author jackyai  
* @version 1.0
* @date 2007-05-09 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/


#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include <assert.h>


#include "pal/pal.h"
#include "tdr/tdr_metalib_manage.h"
#include "tdr/tdr_net.h"
#include "tdr/tdr_error.h"
#include "tdr_metalib_kernel_i.h"
#include "ut_tdr.h"
#include "simpleMetalibV1.h"


static void ut_simplemetalib_net();
static void ut_simplemetalib_net_point();
static void ut_failed_pack();
//static void ut_simplemsg_pack();

int ut_add_metalib_net_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite 增加一个Suite 
	//InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
	pSuite = CU_add_suite("TestMetalibNetSuites", NULL, NULL);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	//注册当前Suite下的测试用例　
	//pSuite:用例指针
	//"Test1": 测试单元名称 
	//Test1:测试函数 
	if( NULL == CU_add_test(pSuite, "ut_simplemetalib_net", (CU_TestFunc)ut_simplemetalib_net))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_simplemetalib_net_point", (CU_TestFunc)ut_simplemetalib_net_point))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_failed_pack", (CU_TestFunc)ut_failed_pack))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	

	return 0;
}

void ut_simplemetalib_net()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETALIB pstLib2 = NULL;
	TDRMETA *pstMeta = NULL;

	PKG stPkg;
    PKG stNetPkg;
	char szNet[TDR_MAX_NET_PACKAGE_LEN];
	TDRDATA stHostInfo;
	TDRDATA stNetInfo;
	PKG stPkg1;
	PKG stPkg2;
	FILE *fp;

	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 

		/*test save && load*/
	iRet = tdr_save_metalib(pstLib, "testMetalibParam.tdr");
	CU_ASSERT(0 <= iRet);

	iRet = tdr_load_verify_metalib(&pstLib2, "testMetalibParam.tdr", NULL);
	CU_ASSERT(0 <= iRet);
	tdr_free_lib(&pstLib2);

	iRet = tdr_load_verify_metalib(&pstLib2, "testMetalibParam.tdr", "bad hash");
	CU_ASSERT(0 > iRet);

	iRet = tdr_load_verify_metalib(&pstLib2, "testMetalibParam.tdr", TDR_METALIB_NET_HASH);
	CU_ASSERT(0 <= iRet);
	if (0 <= iRet)
	{
		CU_ASSERT(0 == memcmp(pstLib, pstLib2, pstLib2->iSize));
	}
	

	tdr_free_lib(&pstLib2);	

    tdr_dump_metalib_file(pstLib, "testMetalibParam_lib.txt");
	pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");
	CU_ASSERT( NULL != pstMeta);

	memset(&stPkg, 0, sizeof(stPkg));
	stPkg.stHead.dwMsgid = 100;
	stPkg.stHead.nCmd = 0;
	stPkg.stHead.nVersion = 65520;
	ut_get_rand_datetime(&stPkg.stHead.tTime);
	stPkg.stHead.nMagic = 0x3344;
	stPkg.nBodycount = 2;


	strncpy(stPkg.stBody.astLogin[0].szName, "haha", sizeof(stPkg.stBody.astLogin[0].szName));
	strncpy(stPkg.stBody.astLogin[0].szPass, "secret", sizeof(stPkg.stBody.astLogin[0].szPass));
	strncpy(stPkg.stBody.astLogin[0].aszZone[0], "shenzhen", sizeof(stPkg.stBody.astLogin[0].aszZone[0]));
	strncpy(stPkg.stBody.astLogin[0].aszZone[1], "beijing", sizeof(stPkg.stBody.astLogin[0].aszZone[0]));

	strncpy(stPkg.stBody.astLogin[1].szName, "hihi", sizeof(stPkg.stBody.astLogin[1].szName));
	strncpy(stPkg.stBody.astLogin[1].szPass, "secret1", sizeof(stPkg.stBody.astLogin[1].szPass));
	strncpy(stPkg.stBody.astLogin[1].aszZone[0], "hunan", sizeof(stPkg.stBody.astLogin[1].aszZone[0]));
	strncpy(stPkg.stBody.astLogin[1].aszZone[1], "hubei", sizeof(stPkg.stBody.astLogin[1].aszZone[0]));


	stPkg.astBody2[0].a = 0x56;
	stPkg.astBody2[0].nNamenum = 2;
	strncpy(stPkg.astBody2[0].aszName[0], "zhangsan", sizeof(stPkg.astBody2[0].aszName[0]));
	strncpy(stPkg.astBody2[0].aszName[1], "lisi", sizeof(stPkg.astBody2[0].aszName[1]));
	stPkg.astBody2[0].fMagic = (float)108.88;

	stPkg.astBody2[1].a = 0x1234;
	stPkg.astBody2[0].nNamenum = 1;
	strncpy(stPkg.astBody2[1].aszName[0], "wangwu", sizeof(stPkg.astBody2[1].aszName[0]));
	stPkg.astBody2[1].fMagic = (float)1586.787;

	stPkg.iChecksum = 0x12345678;

	


	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stNetInfo.iBuff = sizeof(szNet);
	stNetInfo.pszBuff = &szNet[0];

	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, 5);
	CU_ASSERT(iRet == (int)TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION));

    stHostInfo.iBuff = sizeof(stPkg);
    stHostInfo.pszBuff = (char *)&stPkg;
    stNetInfo.iBuff = sizeof(szNet);
    stNetInfo.pszBuff = &szNet[0];
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, pstMeta->iBaseVersion);
	CU_ASSERT( 0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		puts(tdr_error_string(iRet));
	}

    stHostInfo.iBuff = sizeof(stNetPkg);
    stHostInfo.pszBuff = (char *)&stNetPkg;
    //stNetInfo.iBuff = sizeof(szNet);
    //stNetInfo.pszBuff = &szNet[0];
    iRet = tdr_ntoh(pstMeta, &stHostInfo, &stNetInfo, pstMeta->iBaseVersion);
    CU_ASSERT( 0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		puts(tdr_error_string(iRet));
	}
	CU_ASSERT(stNetPkg.stHead.nVersion == pstMeta->iBaseVersion);




	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stNetInfo.iBuff = sizeof(szNet);
	stNetInfo.pszBuff = &szNet[0];
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, 0);
	CU_ASSERT( 0 <= iRet);
	CU_ASSERT(stHostInfo.iBuff == (offsetof(PKG,iChecksum) + sizeof(int32_t)))
	if (TDR_ERR_IS_ERROR(iRet))
	{
		puts(tdr_error_string(iRet));
	}
	CU_ASSERT(stHostInfo.iBuff == (offsetof(PKG,iChecksum) + sizeof(int32_t)));

	stHostInfo.iBuff = sizeof(stNetPkg);
	stHostInfo.pszBuff = (char *)&stNetPkg;
	//stNetInfo.iBuff = sizeof(szNet);
	//stNetInfo.pszBuff = &szNet[0];
	iRet = tdr_ntoh(pstMeta, &stHostInfo, &stNetInfo, 0);
	CU_ASSERT( 0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		puts(tdr_error_string(iRet));
	}
	CU_ASSERT(stHostInfo.iBuff == (offsetof(PKG,iChecksum) + sizeof(int32_t)));



	memcpy(&stPkg1, &stPkg, sizeof(stPkg1));
	memcpy(&stPkg2, &stPkg, sizeof(stPkg2));
	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stNetInfo.iBuff = sizeof(stPkg2);
	stNetInfo.pszBuff = (char *)&stPkg2;
	iRet = tdr_copy(pstMeta, &stNetInfo, &stHostInfo);
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(0 == memcmp(&stPkg1, &stPkg2, sizeof(stPkg1)));
	{
		stNetInfo.iBuff = sizeof(stPkg1);
		stNetInfo.pszBuff = (char *)&stPkg1;
		tdr_output_file(pstMeta, "stPkg1", &stNetInfo, 0, 2);
		stNetInfo.iBuff = sizeof(stPkg2);
		stNetInfo.pszBuff = (char *)&stPkg2;
		tdr_output_file(pstMeta, "stPkg2", &stNetInfo, 0, 2);
		
	}

	stHostInfo.iBuff = sizeof(stNetPkg);
	stHostInfo.pszBuff = (char *)&stNetPkg;
	fp = fopen("NetPkg.txt" ,"w");
	if (NULL != fp)
	{
		tdr_fprintf(pstMeta, fp, &stHostInfo, 0);
		fclose(fp);
		//CU_ASSERT(stHostInfo.iBuff == (int)sizeof(stNetPkg));
	}
	
	tdr_free_lib(&pstLib);
}


void ut_simplemetalib_net_point()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;

	TDRMETA *pstMeta = NULL;

	NETHEAD stPkg;
	NETHEAD stNetPkg ;
	char szNet[TDR_MAX_NET_PACKAGE_LEN];
	TDRDATA stHostInfo;
	TDRDATA stNetInfo;
	PKGHEAD stHostHead;
	PKGHEAD stNetHead;


	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 

	pstMeta = tdr_get_meta_by_name(pstLib, "NetHead");
	CU_ASSERT( NULL != pstMeta);

	memset(&stPkg, 0, sizeof(stPkg));
	stPkg.nVersion = 0x1234;
	stPkg.pstHead = &stHostHead;
	stHostHead.dwMsgid = 100;
	stHostHead.nCmd = 0;
	stHostHead.nVersion = 10;
	ut_get_rand_datetime(&stHostHead.tTime);
	stHostHead.nMagic = 0x3344;


	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stNetInfo.iBuff = sizeof(szNet);
	stNetInfo.pszBuff = &szNet[0];
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, pstMeta->iBaseVersion);
	CU_ASSERT( 0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("ut_simplemetalib_net_point: tdr_hton failed:%s", tdr_error_string(iRet));
		tdr_fprintf(pstMeta, stdout, &stHostInfo, 0);
	}
	CU_ASSERT( stHostInfo.iBuff == sizeof(stPkg));

	stHostInfo.iBuff = sizeof(stNetPkg);
	stHostInfo.pszBuff = (char *)&stNetPkg;
	stNetPkg.pstHead = &stNetHead;
	iRet = tdr_ntoh(pstMeta, &stHostInfo, &stNetInfo, pstMeta->iBaseVersion);
	CU_ASSERT( 0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("ut_simplemetalib_net_point: tdr_ntoh failed:%s", tdr_error_string(iRet));
		tdr_fprintf(pstMeta, stdout, &stHostInfo, 0);
	}
	CU_ASSERT( stHostInfo.iBuff == sizeof(stNetPkg));
	CU_ASSERT( stPkg.nVersion == stNetPkg.nVersion);

	tdr_free_lib(&pstLib);
}


void ut_failed_pack()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;

	TDRMETA *pstMeta = NULL;

	NETHEAD stPkg;
	NETHEAD stNetPkg ;
	char szNet[TDR_MAX_NET_PACKAGE_LEN];
	TDRDATA stHostInfo;
	TDRDATA stNetInfo;
	PKGHEAD stHostHead;
	PKGHEAD stNetHead;


	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 

	pstMeta = tdr_get_meta_by_name(pstLib, "NetHead");
	CU_ASSERT( NULL != pstMeta);

	memset(&stPkg, 0, sizeof(stPkg));
	stPkg.nVersion = 100;
	stPkg.pstHead = &stHostHead;
	stHostHead.dwMsgid = 100;
	stHostHead.nCmd = 0;
	stHostHead.nVersion = 9;
	ut_get_rand_datetime(&stHostHead.tTime);
	stHostHead.nMagic = 0x3344;


	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stNetInfo.iBuff = sizeof(stPkg)/2;
	stNetInfo.pszBuff = &szNet[0];
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, pstMeta->iBaseVersion);
	CU_ASSERT( 0 > iRet);
	CU_ASSERT((stHostInfo.iBuff >0) && (stHostInfo.iBuff) <= sizeof(stPkg)/2);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		
		printf("failed tdr_hton at <%"PRIdPTR":%s> for %s\n", stHostInfo.iBuff, tdr_entry_off_to_path(pstMeta, stHostInfo.iBuff),
			tdr_error_string(iRet));
	}


	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stNetInfo.iBuff = sizeof(stPkg)/2;
	stNetInfo.pszBuff = &szNet[0];
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, pstMeta->iBaseVersion);
	CU_ASSERT( 0 > iRet);
	CU_ASSERT((stHostInfo.iBuff >0) && (stHostInfo.iBuff) < sizeof(stPkg)/2);
	if (TDR_ERR_IS_ERROR(iRet))
	{

		printf("failed tdr_hton at <%"PRIdPTR":%s> for %s\n", stHostInfo.iBuff, tdr_entry_off_to_path(pstMeta, stHostInfo.iBuff),
			tdr_error_string(iRet));
	}
	

	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stNetInfo.iBuff = sizeof(szNet);
	stNetInfo.pszBuff = &szNet[0];
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, pstMeta->iBaseVersion);
	CU_ASSERT( 0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed tdr_hton at %s for %s\n", tdr_entry_off_to_path(pstMeta, stHostInfo.iBuff),
			tdr_error_string(iRet));
		tdr_fprintf(pstMeta, stdout, &stHostInfo,0);
	}
	//CU_ASSERT(stHostInfo.iBuff == sizeof(stPkg));

	stHostInfo.iBuff = sizeof(stNetPkg)/2;
	stHostInfo.pszBuff = (char *)&stNetPkg;
	stNetPkg.pstHead = &stNetHead;
	iRet = tdr_ntoh(pstMeta, &stHostInfo, &stNetInfo, pstMeta->iBaseVersion);
	CU_ASSERT( 0 > iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed tdr_hton at <%"PRIdPTR":%s> for %s\n", stHostInfo.iBuff, tdr_entry_off_to_path(pstMeta, stHostInfo.iBuff),
			tdr_error_string(iRet));
	}


	pstMeta = tdr_get_meta_by_name(pstLib, "SimpleMsg");
	CU_ASSERT( NULL != pstMeta);
	if (NULL != pstMeta)
	{
		SIMPLEMSG stMsg;
		int i;

		stMsg.iCount = 15;
		stMsg.dstID[0] = 10000;
		stMsg.dstID[1] = 10001;
		strncpy(stMsg.szMsg, "hello", sizeof(stMsg.szMsg));
		stHostInfo.iBuff = sizeof(stMsg);
		stHostInfo.pszBuff = (char *)&stMsg;
		stNetInfo.iBuff = sizeof(szNet);
		stNetInfo.pszBuff = &szNet[0];		
		//tdr_fprintf(pstMeta, stdout, &stHostInfo, 0);
		iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo,0);
		if (0 != iRet)
		{
			printf("tdr_hton failed: %s\n", tdr_error_string(iRet));
			tdr_fprintf(pstMeta, stdout, &stHostInfo, 0);
		}

		stMsg.iCount = 1;
		stMsg.dstID[0] = 10000;
		for (i = 0;i < sizeof(stMsg.szMsg);i++)
		{
			stMsg.szMsg[i] = '\n';
		}
		stHostInfo.iBuff = sizeof(stMsg);
		stHostInfo.pszBuff = (char *)&stMsg;
		stNetInfo.iBuff = sizeof(szNet);
		stNetInfo.pszBuff = &szNet[0];		
		//tdr_fprintf(pstMeta, stdout, &stHostInfo, 0);
		iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo,0);
		if (0 != iRet)
		{
			printf("tdr_hton failed: %s\n", tdr_error_string(iRet));
			tdr_fprintf(pstMeta, stdout, &stHostInfo, 0);
		}
	}/*if (NULL != pstMeta)*/

	tdr_free_lib(&pstLib);
}




