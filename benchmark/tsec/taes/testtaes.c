#include "tsec/taes.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

static int test_taes1 (int data_size, int nloopCount)
{
    unsigned long long nUs;
    int iRet;
    struct timeval stBeginTime;
    struct timeval stEndTime;

    char* pOrg ;
    char* pEnc ;
    char* pDec ;

    int iEncLen;
    int iDecLen;
    int i;
    unsigned char pAESkey[16]={0};
    unsigned char pAESkey2[16]={0};
    int *pTempData;

    AES stAES;



    pOrg = (char *)calloc(1,data_size);
    pEnc = (char *)calloc(1,data_size+32);
    /*∑¿÷πoverlap*/
    pDec = (char *)calloc(1,data_size+32);

    srand(time(NULL));

    /*init data*/
    pTempData = (int *)pOrg;
    for( i=0; i< (int)(data_size/sizeof(int)+1); i++)
    {
        pTempData[i] = rand();
    }


    /*init key*/
    pTempData = (int *)pAESkey;
    for( i=0; i< (int)(sizeof(pAESkey)/sizeof(int)); i++)
    {
        pTempData[i] = rand();
    }
    pTempData = (int *)pAESkey2;
    for( i=0; i< (int)(sizeof(pAESkey)/sizeof(int)); i++)
    {
        pTempData[i] = rand();
    }


    iRet = taes_setkey(&stAES,pAESkey);
    if(iRet !=0)
    {
        printf("set key error:iRet=%d\n",iRet);
        return -1;
    }



    gettimeofday(&stBeginTime, NULL);
    for(i=0;i < nloopCount; i++ )
    {

        iEncLen = data_size+32;
        iRet = taes_encrypt(pOrg,data_size,&stAES,pEnc,&iEncLen);
        if(iRet != 0)
        {
            printf("Enc Error:iRet=%d\n",iRet);
            break;
        }

        iDecLen = data_size+32;
        iRet = taes_decrypt(pEnc,iEncLen,&stAES,pDec,&iDecLen);
        if( iRet != 0 )
        {
            printf("Dec Error,iEncLen=%d,iRet=%d\n",iEncLen,iRet);
            break;
        }



        if( i==1)
        {
            if( (memcmp(pOrg,pDec,data_size)!= 0) || (iDecLen != data_size))
            {
                printf("invalid\n");
                break;
            }


            iRet = taes_setkey(&stAES,pAESkey2);
            if(iRet !=0)
            {
                printf("set key2 error:iRet=%d\n",iRet);
                return -1;
            }
        }
    }
    gettimeofday(&stEndTime, NULL);

    nUs = (stEndTime.tv_sec - stBeginTime.tv_sec) * 1000000 + (stEndTime.tv_usec-stBeginTime.tv_usec);

    printf("Time info: datalen:%d , total time=%llu us & loopCount=%d\n",data_size,nUs,i);

    return 0;

}


static int test_taes2(int data_size, int nloopCount)
{
    int iRet;
    unsigned long long nUs;

    struct timeval stBeginTime;
    struct timeval stEndTime;

    char* pOrg ;
    char* pEnc ;
    char* pDec ;

    int iEncLen;
    int iDecLen;
    int i;
    unsigned char pAESkey[16]={0};
    unsigned char pAESkey2[16]={0};
    int *pTempData;

    //AES_KEY stEncKey;
    //AES_KEY stDecKey;
    AES stAES;


    pOrg = (char *)calloc(1,data_size);
    pEnc = (char *)calloc(1,data_size+32);
    /*∑¿÷πoverlap*/
    pDec = (char *)calloc(1,data_size+32);

    srand(time(NULL));

    /*init data*/
    pTempData = (int *)pOrg;
    for( i=0; i< (int)(data_size/sizeof(int)+1); i++)
    {
        pTempData[i] = rand();
    }


    /*init key*/
    pTempData = (int *)pAESkey;
    for( i=0; i< (int)(sizeof(pAESkey)/sizeof(int)); i++)
    {
        pTempData[i] = rand();
    }
    pTempData = (int *)pAESkey2;
    for( i=0; i< (int)(sizeof(pAESkey)/sizeof(int)); i++)
    {
        pTempData[i] = rand();
    }


    iRet = taes_setkey2(&stAES,pAESkey, 128);
    if(iRet !=0)
    {
        printf("set key error:iRet=%d\n",iRet);
        return -1;
    }



    gettimeofday(&stBeginTime, NULL);
    for(i=0;i < nloopCount; i++ )
    {


        // AES_cbc_encrypt(pOrg,pEnc,data_size,&stEncKey,pIV,AES_ENCRYPT);

        //   AES_cbc_encrypt(pEnc,pDec,data_size,&stDecKey,pIV,AES_DECRYPT);
        iEncLen = data_size+32;
        iRet = taes_encrypt2(pOrg,data_size,&stAES,pEnc,&iEncLen);
        if(iRet != 0)
        {
            printf("Enc Error:iRet=%d\n",iRet);
            break;
        }

        iDecLen = data_size+32;
        iRet = taes_decrypt2(pEnc,iEncLen,&stAES,pDec,&iDecLen);
        if( iRet != 0 )
        {
            printf("Dec Error,iEncLen=%d,iRet=%d\n",iEncLen,iRet);
            break;
        }



        if( i==1)
        {
            if( (memcmp(pOrg,pDec,data_size)!= 0) || (iDecLen != data_size))
            {
                printf("invalid\n");
                break;
            }


            iRet = taes_setkey2(&stAES,pAESkey2,128);
            if(iRet !=0)
            {
                printf("set key2 error:iRet=%d\n",iRet);
                return -1;
            }
        }






    }
    gettimeofday(&stEndTime, NULL);

    nUs = (stEndTime.tv_sec - stBeginTime.tv_sec) * 1000000 + (stEndTime.tv_usec-stBeginTime.tv_usec);

    printf("taes_encrypt2/taes_decrypt2 Time info: datalen:%d iDecLen:%d, total time=%llu us & loopCount=%d\n",
        data_size,iEncLen,nUs,i);

    return 0;

}


int main (int argc, char* argv[])
{

    int data_size;
    int iAes = 1;

    int nloopCount;




    if (argc < 3)
    {
        printf("usage: %s datalen loopcount [AES]\n", argv[0]);
        return -1;
    }
    if (argc >= 4)
    {
      iAes = atoi(argv[3]);
    }


    data_size= atoi(argv[1]);
    nloopCount=atoi(argv[2]);

    switch(iAes)
    {
    case 2:
        test_taes2(data_size,nloopCount);
        break;
    default:
        test_taes1(data_size,nloopCount);
    }


    return 0;

}
