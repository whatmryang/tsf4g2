/********************************************************************
**       This head file is generated by program,                   **
**            Please do not change it directly.                    **
********************************************************************/

#ifndef TSEC_IPACL_DESC_H
#define TSEC_IPACL_DESC_H


#ifndef TDR_METALIB_TSEC_IP_VERSION
#define TDR_METALIB_TSEC_IP_VERSION 	1 /*version of metalib*/
#endif

#ifndef TDR_METALIB_TSEC_IP_HASH
#define TDR_METALIB_TSEC_IP_HASH 	"006605cc6e19acb4b851f3007384b56b" /*hash of metalib*/
#endif
/*
 User Define include[s].   */


/*
 User Define Macros.   */



#ifdef __cplusplus
extern "C" {
#endif

/*   Define c types.   */


#ifndef TDR_CUSTOM_C_TYPES
#define TDR_CUSTOM_C_TYPES
    #include <stddef.h>
    #include <time.h>
    #include <sys/types.h>

    #if !defined(_WIN32) && !defined(_WIN64)
        #include <stdint.h>
        #include <inttypes.h>

    #else /*if !defined(_WIN32) && !defined(_WIN64)*/

        //The stdint declaras
        typedef  signed char  int8_t;
        typedef  short int16_t;
        typedef  int   int32_t;
        typedef unsigned char  uint8_t;
        typedef unsigned short uint16_t;
        typedef unsigned int   uint32_t;
        #if _MSC_VER >= 1300
            typedef unsigned long long 	uint64_t;
            typedef long long 	int64_t;
        #else /* _MSC_VER */
            typedef unsigned __int64	uint64_t;
            typedef __int64	int64_t;
        #endif /* _MSC_VER */

    #endif /*if !defined(_WIN32) && !defined(_WIN64)*/


    typedef int64_t tdr_longlong;
    typedef uint64_t tdr_ulonglong;
    typedef uint16_t tdr_wchar_t;  /**<Wchar基本数据类型*/
    typedef uint32_t tdr_date_t;	/**<data基本数据类型*/
    typedef uint32_t tdr_time_t;	/**<time基本数据类型*/
    typedef uint64_t tdr_datetime_t; /**<datetime基本数据类型*/
    typedef uint32_t tdr_ip_t;  /**<IPv4数据类型*/
#endif /*TDR_CUSTOM_C_TYPES*/



/*   Structs/unions prototype.   */

struct tagIPLimit;
typedef struct tagIPLimit                                          	IPLIMIT;
typedef struct tagIPLimit                                          	*LPIPLIMIT;

struct tagIPACL;
typedef struct tagIPACL                                            	IPACL;
typedef struct tagIPACL                                            	*LPIPACL;



/*   Define structs/unions.   */

#pragma pack(1)

struct tagIPLimit
{
    char szLimit[8];
    char szSrcIP[128];
    char szDstIP[128];
};

struct tagIPACL
{
    char szDefaultRule[8];
    int32_t iLimitCnt;
    IPLIMIT astLimits[1024];
};


#pragma pack()

#ifdef __cplusplus
}
#endif


#endif /* TSEC_IPACL_DESC_H */
