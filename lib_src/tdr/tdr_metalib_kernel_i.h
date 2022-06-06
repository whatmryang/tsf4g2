/**
*
* @file     tdr_metalib_kernel_i.h
* @brief    TDRԪ���ݿ���Ľṹ
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-16
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_METALIB_KERNEL_H
#define TDR_METALIB_KERNEL_H

#include <stddef.h>
#include "tdr/tdr_types.h"
#include "tdr/tdr_define.h"
#include "tdr/tdr_ctypes_info.h"
#include "tdr/tdr_error.h"
#include "tdr_define_i.h"
#include "tdr_os.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup TDR_METALIB_MANAGER TDRMetaLib����
* @{
*/

	/**
	* This is the type delcaration for TDR MetaLib.
	*/
	typedef struct tagTDRMetaLib	TDRMETALIB;

	/**
	* This is the type delcaration for TDR Meta.
	*/
	typedef struct tagTDRMeta	TDRMETA;



	/**
	* This is the type delcaration for TDR MetaEntry.
	*/
	typedef struct tagTDRMetaEntry	TDRMETAENTRY;

	/**
	* This is the type delcaration for TDR Macro.
	*/
	typedef struct tagTDRMacro		TDRMACRO;

    /**
     * This is the type delcaration for TDR MacrosGroup.
     */
    typedef struct tagTDRMacrosGroup TDRMACROSGROUP;



/**
�ض���net/host��Ϣ
*/
struct tagTDRSelector
{
	size_t iUnitSize;	/**<��ռ�洢�ռ�*/
    TDROFF iHOff;    /**<���ش洢��Ϣƫ��*/
    TDRPTR ptrEntry;   /**<����entry��ָ��*/
};

typedef struct tagTDRSelector TDRSelector;
typedef struct tagTDRSelector *LPTDRSelector;

/*�����紦����ص��ض�����
*/
struct tagTDRRedirector
{
	TDROFF iNOff;   /**<���紫����Ϣƫ��*/
	TDROFF iHOff;    /**<���ش洢��Ϣƫ��*/
	size_t iUnitSize;
};

typedef struct tagTDRRedirector TDRREDIRECTOR;
typedef struct tagTDRRedirector *LPTDRREDIRECTOR;

/**
sortkey��Ϣ
*/
struct tagTDRSortKeyInfo
{
	TDRIDX      idxSortEntry;               /**< ���������entry��meta�е�����*/
	TDROFF	iSortKeyOff;			/**< ����Ԫ�صı��ش洢ƫ��. */
	TDRPTR	ptrSortKeyMeta;			/**< ���������entry����meta������*/
};
typedef struct tagTDRSortKeyInfo	TDRSORTKEYINFO;
typedef struct tagTDRSortKeyInfo	*LPTDRSORTKEYINFO;


/**
* �˽ṹ����һ�����ݳ�Ա����iCountΪ0ʱ��ʾ�˳�ԱΪһ�ɱ����飬
* struct/Union�н��������һ���ɱ������Ա
*/
struct tagTDRMetaEntry
{
    int iID;		/**< ��Ԫ���ݵ�ID*/
    int iVersion;	        /**< ��Ԫ���ݼӵ�MetaLib����ʱ�İ汾*/
    int iType;		/**< the typeid of this entry. */
    TDRPTR ptrName;	/**< ptr of name, used for generating c/c++ header. */

	size_t iHRealSize;		/**<���ش洢�����ܹ��Ŀռ�*/
    size_t iNRealSize;          /**<���紫��ʱ����ĵ��ܿռ�*/
	size_t iHUnitSize;			/**<���ش洢����entry��Ԫ����ռ�*/
	size_t iNUnitSize;			/**<����洢����entry��Ԫ����ռ�*/
    size_t iCustomHUnitSize;    /**< �Զ���洢��Ԫ��С*/

    int iCount;		/**< 1 means single, >1 means array, 0 means variable array */

    TDROFF iNOff;		/**< ���紫��ʱ��ƫ�ƣ�1�ֽڶ���*/
	TDROFF iHOff;		/**< ���ش洢ʱ��ƫ�ƣ�ʹ��ָ�����뷽ʽ*/


    TDRIDX idxID;  			/**< the id macro's index. */
    TDRIDX idxVersion;		/**< ����汾ȡֵΪ�궨�壬��˳�Ա�����Ӧ�궨���ں궨���б��е�����*/
    TDRIDX idxCount;		/**< the macro's referred by count. */
    TDRIDX idxType;			/**< the index of the typeinfo. */
    TDRIDX idxCustomHUnitSize;             /**< �Զ���洢��Ԫ��С�궨��ֵ������*/

    unsigned short    wFlag;		/**< ��ȡ��Ԫ�ض�Ӧ��־��Ϣ����ָ�룬���õ�*/
    char    chDBFlag;         /**< TDR-DB �����ϵӳ�� */
    char    chOrder;          /**< 1 if ascending, -1 if desending, else 0 */


    TDRSIZEINFO stSizeInfo;   /**<sizeinfo����ֵ*/
    TDRSelector stRefer;      /**<refer����ֵ*/
    TDRSelector stSelector;      /**<select ����*/

    int iIO;		/**< the input/output control. */
    int idxIO;		/**< the idx of input/output control macro. */

    TDRPTR ptrMeta;			/**<���˳�ԱΪ������������ʱ����¼�临����������Ԫ����meta���ݵ�ָ��. */

	/**<��selector��ֵ��[iMinId,iMaxID]����ʱ��ѡ���Ԫ��*/
	int iMaxId;
	int iMinId;

	/**< index of macro of iMaxId,iMinId*/
	TDRIDX iMaxIdIdx;
	TDRIDX iMinIdIdx;


    /*
    *��Ա��ʾ��, ������Ϣ����������ȱʡֵ�ַ���������Metalib������ַ����ڴ��
    *�У��˴��������ַ�������ʼ��ַ�ͳ���
    */
    size_t iDefaultValLen;			/**<ȱʡֵ�ĳ���*/
    TDRPTR ptrDesc;				/**< ptr of  the description info. */
    TDRPTR ptrChineseName;		/**< ptr of chinse name of entry*/
    TDRPTR ptrDefaultVal;		/**< ptr of default value of entry*/

	TDRPTR ptrMacrosGroup; /*�˳�ԱԪ�ذ󶨵ĺ궨����ָ�룬 Add at TDR build Version: 0x00000008*/
	TDRPTR ptrCustomAttr; /*ָ���Զ�������ֵ��ָ�룬 Add at TDR build Version: 0x00000008*/
    TDRPTR iOffToMeta; /*����ڴ˳�Ա�����ĸ������ݽṹ��������ƫ�ƣ� Add at TDR build Version: 0x00000009*/

    TDRPTR iReserve1;   /**< reserve for extend . Add at TDR build Version: 0x00000009 */
    TDRPTR iReserve2;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */
    TDRPTR iReserve3;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */
};

#define TDR_ENTRY_CLEAR_ALL_FLAG(pstEntry)  (pstEntry)->wFlag = TDR_ENTRY_FLAG_NONE;
#define TDR_ENTRY_IS_VERSIONED(pstEntry)	((pstEntry)->wFlag & TDR_ENTRY_FLAG_RESOVLD )
#define TDR_ENTRY_SET_VERSIONED(pstEntry)	(pstEntry)->wFlag |= TDR_ENTRY_FLAG_RESOVLD
#define TDR_ENTRY_CLR_VERSIONED(pstEntry)	(pstEntry)->wFlag &= ~TDR_ENTRY_FLAG_RESOVLD

#define TDR_ENTRY_DO_HAVE_ID(pstEntry)     ((pstEntry)->wFlag & TDR_ENTRY_FLAG_HAVE_ID)
#define TDR_ENTRY_SET_HAVE_ID(pstEntry)       ((pstEntry)->wFlag |= TDR_ENTRY_FLAG_HAVE_ID)
#define TDR_ENTRY_CLR_HAVE_ID(pstEntry)		((pstEntry)->wFlag &= ~TDR_ENTRY_FLAG_HAVE_ID)

#define TDR_ENTRY_DO_HAVE_MAXMIN_ID(pstEntry)  ((pstEntry)->wFlag & TDR_ENTRY_FLAG_HAVE_MAXMIN_ID)
#define TDR_ENTRY_SET_HAVE_MAXMIN_ID(pstEntry) ((pstEntry)->wFlag |= TDR_ENTRY_FLAG_HAVE_MAXMIN_ID)
#define TDR_ENTRY_IS_VALID_SELECTID(pstEntry)  ((pstEntry)->iMinId <= (pstEntry)->iMaxId)

#define TDR_ENTRY_SET_POINT_TYPE(pstEntry)			((pstEntry)->wFlag |= TDR_ENTRY_FLAG_POINT_TYPE)
#define TDR_ENTRY_SET_REFER_TYPE(pstEntry)			((pstEntry)->wFlag |= TDR_ENTRY_FLAG_REFER_TYPE)
#define TDR_ENTRY_IS_POINTER_TYPE(pstEntry)           ((pstEntry)->wFlag & TDR_ENTRY_FLAG_POINT_TYPE)
#define TDR_ENTRY_IS_REFER_TYPE(pstEntry)           ((pstEntry)->wFlag & TDR_ENTRY_FLAG_REFER_TYPE)

#define TDR_ENTRY_IS_FIXSIZE(pstEntry)     ((pstEntry)->wFlag & TDR_ENTRY_FALG_FIXSIZE)
#define TDR_ENTRY_SET_FIXSIZE(pstEntry)   ((pstEntry)->wFlag |= TDR_ENTRY_FALG_FIXSIZE)

#define TDR_ENTRY_DO_EXTENDABLE(pstEntry)     ((pstEntry)->chDBFlag & TDR_ENTRY_DB_FLAG_EXTEND_TO_TABLE)
#define TDR_ENTRY_SET_EXTENDABLE(pstEntry)       ((pstEntry)->chDBFlag |= TDR_ENTRY_DB_FLAG_EXTEND_TO_TABLE)
#define TDR_ENTRY_CLR_EXTENDABLE(pstEntry)		((pstEntry)->chDBFlag &= ~TDR_ENTRY_DB_FLAG_EXTEND_TO_TABLE)

/*blob ѹ���洢*/
#define TDR_ENTRY_DO_COMPRESS(pstEntry)     ((pstEntry)->chDBFlag & TDR_ENTRY_DB_FLAG_COMPRESS)
#define TDR_ENTRY_SET_COMPRESS(pstEntry)       ((pstEntry)->chDBFlag |= TDR_ENTRY_DB_FLAG_COMPRESS)
#define TDR_ENTRY_CLR_COMPRESS(pstEntry)		((pstEntry)->chDBFlag &= ~TDR_ENTRY_DB_FLAG_COMPRESS)
#define TDR_ENTRY_IS_UNIQUE(pstEntry)     ((pstEntry)->chDBFlag & TDR_ENTRY_DB_FLAG_UNIQUE)
#define TDR_ENTRY_SET_UNIQUE(pstEntry)       ((pstEntry)->chDBFlag |= TDR_ENTRY_DB_FLAG_UNIQUE)
#define TDR_ENTRY_CLR_UNIQUE(pstEntry)		((pstEntry)->chDBFlag &= ~TDR_ENTRY_DB_FLAG_UNIQUE)

#define TDR_ENTRY_IS_NOT_NULL(pstEntry)     ((pstEntry)->chDBFlag & TDR_ENTRY_DB_FLAG_NOT_NULL)
#define TDR_ENTRY_SET_NOT_NULL(pstEntry)       ((pstEntry)->chDBFlag |= TDR_ENTRY_DB_FLAG_NOT_NULL)
#define TDR_ENTRY_CLR_NOT_NULL(pstEntry)		((pstEntry)->chDBFlag &= ~TDR_ENTRY_DB_FLAG_NOT_NULL)

#define TDR_ENTRY_IS_PRIMARYKEY(pstEntry)     ((pstEntry)->chDBFlag & TDR_ENTRY_DB_FLAG_PRIMARYKEY)
#define TDR_ENTRY_SET_PRIMARYKEY(pstEntry)       ((pstEntry)->chDBFlag |= TDR_ENTRY_DB_FLAG_PRIMARYKEY)
#define TDR_ENTRY_CLR_PRIMARYKEY(pstEntry)		((pstEntry)->chDBFlag &= ~TDR_ENTRY_DB_FLAG_PRIMARYKEY)

#define TDR_ENTRY_IS_COUNTER(pstEntry)     ((pstEntry)->wFlag & TDR_ENTRY_FLAG_REFER_COUNT)
#define TDR_ENTRY_SET_COUNTER(pstEntry)       ((pstEntry)->wFlag |= TDR_ENTRY_FLAG_REFER_COUNT)
#define TDR_ENTRY_CLR_COUNTER(pstEntry)		((pstEntry)->wFlag &= ~TDR_ENTRY_FLAG_REFER_COUNT)

#define TDR_ENTRY_IS_AUTOINCREMENT(pstEntry)     ((pstEntry)->chDBFlag & TDR_ENTRY_DB_FLAG_AUTOINCREMENT)
#define TDR_ENTRY_SET_AUTOINCREMENT(pstEntry)       ((pstEntry)->chDBFlag |= TDR_ENTRY_DB_FLAG_AUTOINCREMENT)
#define TDR_ENTRY_CLR_AUTOINCREMENT(pstEntry)		((pstEntry)->chDBFlag &= ~TDR_ENTRY_DB_FLAG_AUTOINCREMENT)


#define TDR_ENTRY_IS_SELECTOR(pstEntry)     ((pstEntry)->wFlag & TDR_ENTRY_FLAG_SELECTOR)
#define TDR_ENTRY_SET_SELECTOR(pstEntry)       ((pstEntry)->wFlag |= TDR_ENTRY_FLAG_SELECTOR)
#define TDR_ENTRY_CLR_SELECTOR(pstEntry)		((pstEntry)->wFlag &= ~TDR_ENTRY_FLAG_SELECTOR)


#define TDR_ENTRY_IS_STR_COMPRESS(pstEntry)		((pstEntry)->wFlag & TDR_ENTRY_FLAG_STR_COMPRESS )
#define TDR_ENTRY_SET_STR_COMPRESS(pstEntry)	((pstEntry)->wFlag |= TDR_ENTRY_FLAG_STR_COMPRESS)
#define TDR_ENTRY_CLR_STR_COMPRESS(pstEntry)    ((pstEntry)->wFlag &= ~TDR_ENTRY_FLAG_STR_COMPRESS)
	


    struct tagTDRColumnInfo
    {
        TDROFF	iHOff;			/**< Ԫ�صı��ش洢ƫ��. */
        TDRPTR	ptrEntry;		/**< entryָ��*/
        int     iIsDesc;        /**< 1:DESC, 0:ASC */
        int     iCaredCharNum;  /**< ���ڿɱ䳤�ȵ��ַ�����ָ��������ע��ǰN���ַ�*/
    };
    typedef struct tagTDRColumnInfo TDRCOLUMNINFO;
    typedef struct tagTDRColumnInfo *LPTDRCOLUMNINFO;

    /*
     * �˽ṹ�������ݿ⽨�����ʱ�������ݱ������������Ϣ
     * added at tdr build 0x0000b
     */
    struct tagTDRMetaIndex
    {
        int iVersion;	        /**< ��Ԫ���ݼӵ�MetaLib����ʱ�İ汾*/
        TDRPTR ptrName;	        /**< ptr of name. */

        TDRPTR iOffToMeta;      /**< ����ڴ˳�Ա�����ĸ������ݽṹ��������ƫ�� */

        int iColumnNum;         /**< ���ݿ��������г�Ա�ĸ��� */
        TDRPTR ptrColumnBase;   /**< ���ݿ��������г�Ա�Ļ�ַָ�� */

        int iIndexType;         /**< ������Ĵ洢���ݽṹ����. RESERVED*/
                                /**< 1:BTREE, 2:HASH */
        int iIsUnique;          /**< ��Ӧ������UNIQUE����. RESERVED*/

        TDRPTR ptrDesc;			/**< ptr of  the description info. RESERVED */
        TDRPTR ptrChineseName;	/**< ptr of chinse name of entry. RESERVED */

        TDRPTR iReserve1;       /**< RESERVED */
        TDRPTR iReserve2;       /**< RESERVED */
    };
    typedef struct tagTDRMetaIndex TDRMETAINDEX;
    typedef struct tagTDRMetaIndex *LPTDRMETAINDEX;

    /**DB������Ϣ
    */
    struct tagTDRDBKeyInfo
    {
        TDROFF	iHOff;			/**< Ԫ�صı��ش洢ƫ��. */
        TDRPTR	ptrEntry;			/**< entryָ��*/
    };
    typedef struct tagTDRDBKeyInfo	TDRDBKEYINFO;
    typedef struct tagTDRDBKeyInfo	*LPTDRDBKEYINFO;




/**
* �洢meta��Ϣ�Ľṹ
*/
struct tagTDRMeta
{
    uint32_t uFlags;		/**< flag info of mata*/

    int iID;			/**< id of meta*/
    int iBaseVersion;		/**< base version of meta*/
    int iCurVersion;	/**< current version of meta*/
    int iType;			/**< type of meta*/
	size_t iMemSize;			/**< ��meta�ṹ�ڴ�ṹ��С*/


	size_t iNUnitSize;		/**<���紫��ʱ�ĵ�Ԫ��С*/
	size_t iHUnitSize;		/**<���ش洢��Ԫ��С*/

	size_t iCustomHUnitSize; /**<�Զ��屾�ش洢��Ԫ��С*/
	int idxCustomHUnitSize; /**<�Զ��屾�ش洢��Ԫ��С�ĺ궨��ֵ���� */

    int iMaxSubID;			/**< max id of child entry*/
    int iEntriesNum;		/**< num of child entries*/

        /* Add at TDR build version: 0x0000000b */
        int iIndexesNum;        /**< num of child indexes*/
        TDRPTR ptrIndexes;      /**< ָ��index�������ʼ��ַ����(entries, primarykeys)֮��*/
        TDRPTR ptrColumns;      /**< ָ��column�������ʼ��ַ����(entries, primarykeys, indexies)֮��*/
        /* Add at TDR build version: 0x0000000b */


    TDRPTR ptrMeta;		/**< offset of this meta from "data" member of head. */

    TDRIDX iIdx;		/**< index of this mata in metalib*/
    TDRIDX idxID;		/**< index of macro of id*/
    TDRIDX idxType;		/**< index of meta's type*/
    TDRIDX idxVersion;	/**< index of macro of meta's version*/

	int iCustomAlign;	/**< structsԪ������ ָ���ṹ����Ա�����Ķ����С Default is 1*/
	int iValidAlign;	/**< metaԪ����Ч�Ķ���ֵ��Ϊ����Ա����ֵ�������Ǹ�ֵ*/

	int iVersionIndicatorMinVer;	/**< �汾ָʾ����ָ�������ٱ���*/
	TDRSIZEINFO stSizeType;		/**< ��¼�����Ϣ*/
	TDRREDIRECTOR stVersionIndicator;

	TDRSORTKEYINFO stSortKey; /**<sortkey����ֵ*/

    TDRPTR ptrName;	/**< ptr of Name of meta*/

    /*������Ϣ���������ַ���������Metalib������ַ����ڴ��,�˴��������ַ�������ʼ��ַ�ͳ���*/
    TDRPTR ptrDesc;				/**< ptr of  the description info. */
    TDRPTR ptrChineseName;		/**< ptr of chinse name of entry*/

	int iSplitTableFactor;	/**<���ݿ�ֱ�����*/
	short nSplitTableRuleID;	/**<���ݿ�ֱ����id*/
	short nPrimayKeyMemberNum;	/**<��������ĳ�Ա����*/

	TDRIDX idxSplitTableFactor;	/**< index of macro of meta's wSplitTableFactor*/
    TDRDBKEYINFO stSplitTableKey;	/**<���ݿ�ֱ�������Ա��ָ�룬ʵ�ʴ洢���metalib������ƫ��ֵ*/
	TDRPTR ptrPrimayKeyBase;	/**<���ݿ�������Ա�Ļ�ַָ�룬ʵ�ʴ洢���metalib������ƫ��ֵ*/
	TDRPTR	ptrDependonStruct;  /*�˽ṹ�̳е�Ԫ����ָ��*/

    TDRPTR iReserve1;   /**< reserve for extend . Add at TDR build Version: 0x00000009 */
    TDRPTR iReserve2;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */
    TDRPTR iReserve3;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */

    TDRMETAENTRY stEntries[1];
};

#define TDR_STACK_SIZE		32    /**<Ԫ���ݴ���ջ��С��Ҳ�������Ԫ�������Ƕ�ײ��*/
#define TDR_MIN_BSEARCH		16

/**Ԫ���ݴ���ջ����
*/
struct tagTDRStackItem
{
	LPTDRMETA pstMeta;	/**<��ǰ���ڴ����Ԫ���� */
	LPTDRMETAENTRY pstEntry;	/**<��ǰ�ṹ�ڸ��ṹ�еĳ�Ա���*/

	int iRealCount; /**<��Ԫ���ݳ�Ա��ʵ�����鳤��*/
	int iCount;		/**<����Ԫ�������飬��¼ʣ��Ҫ�����Ԫ���ݵĸ��� */
	int idxEntry;		/**< ��Ԫ���ݵ�ǰ���ڴ����entry���� */

	size_t iMetaSizeInfoUnit;  /**<��meta������Ϣ�洢�Ŀռ��С*/
	TDROFF iMetaSizeInfoOff;	/**<sizeinfo��Ϣ��ƫ��*/
	TDROFF iEntrySizeInfoOff;  /**<entry sizeinfo����*/

	int iCutOffVersion;		/**<meta���õð汾*/

	size_t iCode;			/*��¼�������ֽ���*/
	char* pszHostBase; /**<��Ԫ���ݴ洢��host��ַ*/
	char* pszHostEnd; /**<��Ԫ���ݴ洢�Ļ�������ֹ��ַ*/
	char* pszNetBase;  /**<��Ԫ���ݴ洢��net��ַ*/
	char* pszMetaSizeInfoTarget;   /**<��meta���������Ϣ�洢�Ļ�ַ*/

	int iChange;		/*��¼һ���ṹ������г�Ա�Ƿ��Ѿ������꣬��ֵΪ0����û�д����꣬�����Ѿ�������*/
	char szMetaEntryName[TDR_NAME_LEN];
};

typedef struct tagTDRStackItem	TDRSTACKITEM;
typedef struct tagTDRStackItem	*LPTDRSTACKITEM;

typedef struct tagTDRStackItem TDRSTACK[TDR_STACK_SIZE];




#define TDR_META_IS_RESOLVED(pstMeta)		((pstMeta)->uFlags & TDR_META_FLAG_RESOVLED )
#define TDR_META_SET_RESOLVED(pstMeta)	(pstMeta)->uFlags |= TDR_META_FLAG_RESOVLED
#define TDR_META_CLR_RESOLVED(pstMeta)	(pstMeta)->uFlags &= ~TDR_META_FLAG_RESOVLED



#define TDR_META_DO_HAVE_ID(pstMeta)     ((pstMeta)->uFlags & TDR_META_FLAG_HAVE_ID)
#define TDR_META_SET_HAVE_ID(pstMeta)       ((pstMeta)->uFlags |= TDR_META_FLAG_HAVE_ID)
#define TDR_META_CLR_HAVE_ID(pstMeta)		((pstMeta)->uFlags &= ~TDR_META_FLAG_HAVE_ID)

#define TDR_META_IS_VARIABLE(pstMeta)		((pstMeta)->uFlags & TDR_META_FLAG_VARIABLE )
#define TDR_META_SET_VARIABLE(pstMeta)	    (pstMeta)->uFlags |= TDR_META_FLAG_VARIABLE
#define TDR_META_CLR_VARIABLE(pstMeta)	    (pstMeta)->uFlags &= ~TDR_META_FLAG_VARIABLE

#define TDR_META_IS_FIXSIZE(pstMeta)		((pstMeta)->uFlags & TDR_META_FLAG_FIXSIZE )
#define TDR_META_SET_FIXSIZE(pstMeta)		(pstMeta)->uFlags |= TDR_META_FLAG_FIXSIZE
#define TDR_META_CLR_FIXSIZE(pstMeta)		(pstMeta)->uFlags &= ~TDR_META_FLAG_FIXSIZE

#define TDR_META_IS_STRICT_INPUT(pstMeta)		((pstMeta)->uFlags & TDR_META_FALG_STRICT_INPUT )
#define TDR_META_SET_STRICT_INPUT(pstMeta)		(pstMeta)->uFlags |= TDR_META_FALG_STRICT_INPUT
#define TDR_META_CLR_STRICT_INPUT(pstMeta)		(pstMeta)->uFlags &= ~TDR_META_FALG_STRICT_INPUT

#define TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(pstMeta)     ((pstMeta)->uFlags & TDR_META_FALG_HAVE_AUTOINVREMENT_ENTRY)
#define TDR_META_SET_HAVE_AUTOINCREMENT_ENTRY(pstMeta)       ((pstMeta)->uFlags |= TDR_META_FALG_HAVE_AUTOINVREMENT_ENTRY)
#define TDR_META_CLR_HAVE_AUTOINCREMENT_ENTRY(pstMeta)		((pstMeta)->uFlags &= ~TDR_META_FALG_HAVE_AUTOINVREMENT_ENTRY)


 //����˱�־λ֮�ϣ���˵���ṹ��չ�����Ա����Ψһ����Ҫͨ����Ӻ�׺����
#define TDR_META_DO_NEED_PREFIX(pstMeta)     ((pstMeta)->uFlags & TDR_META_FLAG_NEED_PREFIX_FOR_UNIQUENAME)
#define TDR_META_SET_NEED_PREFIX(pstMeta)       ((pstMeta)->uFlags |= TDR_META_FLAG_NEED_PREFIX_FOR_UNIQUENAME)
#define TDR_META_CLR_NEED_PREFIX(pstMeta)		((pstMeta)->uFlags &= ~TDR_META_FLAG_NEED_PREFIX_FOR_UNIQUENAME)




/*Meta offset-index mapping info*/
struct tagTDRMapEntry
{
    TDRPTR iPtr;
    size_t iSize;
};

typedef struct tagTDRMapEntry	TDRMAPENTRY;
typedef struct tagTDRMapEntry	*LPTDRMAPENTRY;


struct tagTDRIDEntry
{
    int iID;
    TDRPTR ptrMeta;  /*meta��ƫ��**/
};

typedef struct tagTDRIDEntry	TDRIDENTRY;
typedef struct tagTDRIDEntry	*LPTDRIDENTRY;


struct tagTDRNameEntry
{
    TDRPTR ptrName;	/**< ptr of Name of meta*/
    TDRPTR ptrMeta;  /*meta��ƫ��**/
};

typedef struct tagTDRNameEntry	TDRNAMEENTRY;
typedef struct tagTDRNameEntry	*LPTDRNAMEENTRY;



/*����궨����Ϣ�Ľṹ*/
struct tagTDRMacro
{
    TDRPTR ptrMacro;				/**< name of macro */
	int iValue;
	TDRPTR ptrDesc;				/**< ptr of  the description info. Add at TDR build Version: 0x00000004 */

    int iReserve;   /**< reserve for extend . Add at TDR build Version: 0x00000009 */
};

    struct tagTDRMacrosGroup
    {
        int iCurMacroCount;	/**<�궨�����к궨��ĸ���*/
        int iMaxMacroCount;	/**<����ܴ洢�ĺ궨�����*/
        TDRPTR ptrDesc;				/**< ptr of  the description info. */
        TDRPTR ptrNameIdxMap;				/**< based address of macro name��index map. */
        TDRPTR ptrValueIdxMap;				/**< based address of macro value��index map. */
        char szName[TDR_NAME_LEN]; /**< �궨���������*/
        char data[1];			/**<ӳ��������*/
    };

struct tagTDRValueFieldDefinition
{
	int iCurCount;	/**<ֵ�������*/
	int iMaxMacroCount;	/**<����ܴ洢��ֵ�������*/
	TDRPTR ptrDesc;				/**< ptr of  the description info. */
	char szName[TDR_NAME_LEN]; /**< ֵ���������*/
	char data[1];			/**<������*/
};
typedef struct tagTDRValueFieldDefinition TDRVALUEFIELDDEFINITION;
typedef struct tagTDRValueFieldDefinition *LPTDRVALUEFIELDDEFINITION;


/**
Ԫ���ݿ�Ľṹ��Ϣ
In order to refer the meta data fastly, We use two-level mapping.
First, A map entry for each meta data.
Second, A index value for each map entry.
There are two index array, one for id, one for name.
*/
struct tagTDRMetaLib
{
    unsigned short wMagic;
    short nBuild;
    uint32_t dwPlatformArch;  /*��¼��32λƽ̨����64λƽ̨������*/
    size_t iSize;

    unsigned int checksum;	/*  calculated. */
    int iReserve[3];


    int iID;
    int iXMLTagSetVer;	/**<XMLTag Set Verion of this metalib used*/

    int iMaxID;


    int iMaxMetaNum;
    int iCurMetaNum;
    int iMaxMacroNum;
    int iCurMacroNum;

	int iMaxMacrosGroupNum; /*��������ɵĺ궨������Ŀ, Add at TDR build Version: 0x00000008*/
	int iCurMacrosGroupNum; /*�궨���鵱ǰ��Ŀ�� Add at TDR build Version: 0x00000008*/

	int iMaxValFieldDefNum; /*��������ɵ�ֵ��������Ŀ, Add at TDR build Version: 0x00000008*/
	int iCurValFieldDefNum; /*ֵ�����鵱ǰ��Ŀ�� Add at TDR build Version: 0x00000008*/

    int iVersion;

    /** all the offset is start from the 'data' member.
    */
    TDRPTR ptrMacro;	/*ptr for macro info block*/
    TDRPTR ptrID;		/*ptr of begin address for id-metaidx mapping info block*/
    TDRPTR ptrName;		/*ptr of begin address for name-metaidx mapping info block*/
    TDRPTR ptrMap;		/*ptr of begin address for metaidx-metaOff mapping info block*/
    TDRPTR ptrMeta;		/*ptr of begin address for meta info block*/
    TDRPTR ptrLaseMeta;	/*ptr of last meta in mata*/

    /*�ַ�������������*/
    size_t  iFreeStrBufSize;	/*�ַ�����������������Ĵ�С*/
    TDRPTR	ptrStrBuf;		/*�ַ�����������ƫ�Ƶ�ַ����data��Ա��ʼ����*/
    TDRPTR	ptrFreeStrBuf;	/*���õĿ��л��������׵�ַ*/

	TDRPTR	ptrMacroGroupMap;	/*ptr of macrosgroup index-off mapping info block Add at TDR build Version: 0x00000008*/
	TDRPTR ptrMacrosGroup;	/*ptr of macrosgroup, Add at TDR build Version: 0x00000008*/

	size_t iMacrosGroupSize; /*�궨�������ʹ�õ����ռ䣬 Add at TDR build Version: 0x00000008*/

	TDRPTR	ptrValueFiledDefinitionsMap;	/*ֵ���������������� Add at TDR build Version: 0x00000008*/
	TDRPTR ptrValueFiledDefinitions;	/*ֵ�����ַ, Add at TDR build Version: 0x00000008*/
	size_t iValueFiledDefinitionsSize; /*ֵ������������С�� Add at TDR build Version: 0x00000008*/

    /* Ԥ���ֶ� */
    TDRPTR iReserve1;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */
    TDRPTR iReserve2;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */
    TDRPTR iReserve3;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */

    char szName[TDR_NAME_LEN];
    char data[1];		/* only used for reference data. */
};

#define  TDR_MIN_STR_BUF_SIZE   128  /**< �ַ�����������ȱʡ�ռ�*/

union tagTDRType
{
	char cValue;
	unsigned char byValue;
	short nValue;
	unsigned short wValue;
	int iValue;
	uint32_t dwValue;
	long lValue;
	unsigned long ulValue;
	int64_t llValue;
	uint64_t ullValue;
	float fValue;
	double dValue;
	tdr_ip_t iIP;
	tdr_date_t iDate;
	tdr_time_t iTime;
	tdr_wchar_t dwChar;
	char szValue[1];
};

typedef union tagTDRType		TDRTYPE;
typedef union tagTDRType		*LPTDRTYPE;


/*�����ַ����Ļ�ַ������ַ����׵�ַ*/
#define TDR_GET_STRING_BY_PTR(pstLib, ptr)			(char *)((pstLib)->data + (ptr))

#define TDR_GET_META_INDEX_PTR(pstMeta)  (LPTDRMETAINDEX)((char*)(pstMeta) + (pstMeta)->ptrIndexes)

#define TDR_GET_COLUMN_PTR(pstMeta, pstIndex) (LPTDRCOLUMNINFO)((char*)(pstMeta) + (pstIndex)->ptrColumnBase)

#define TDR_IDX_TO_META(pstLib, idx)	(LPTDRMETA) (pstLib->data + ((LPTDRMAPENTRY)(pstLib->data + pstLib->ptrMap))[idx].iPtr)

#define TDR_PTR_TO_META(pstLib, ptr)	(LPTDRMETA) (pstLib->data + (ptr))

#define TDR_META_TO_LIB(pstMeta)		(LPTDRMETALIB) ( ((intptr_t)(pstMeta)) - (pstMeta)->ptrMeta - offsetof(TDRMETALIB, data) )

#define TDR_GET_PRIMARYBASEPTR(pstMeta)  (LPTDRDBKEYINFO)(((intptr_t)(pstMeta)) + pstMeta->ptrPrimayKeyBase)

/*����entry��ƫ�Ƽ����entry��ָ��*/
#define TDR_PTR_TO_ENTRY(pstLib, ptr)   (LPTDRMETAENTRY)(pstLib->data + (ptr))

/*����entry��ָ������entry��ƫ��*/
#define TDR_ENTRY_TO_PTR(pstLib, pstEntry)	(TDRPTR)((char *)(pstEntry) - (pstLib)->data)

/*����meta��entry��ָ������entry��ƫ��*/
#define TDR_CALC_ENTRYOFF_OF_META(a_pstMeta, a_pstEntry)	(TDRPTR)((intptr_t)(a_pstEntry) - (intptr_t)(a_pstMeta))

/*����entry��ָ��������meta��ƫ�Ƽ����meta��ָ��*/
#define TDR_ENTRY_TO_META(a_pstEntry)	(LPTDRMETA)((intptr_t)(a_pstEntry) - (a_pstEntry)->iOffToMeta)


/*ȡ��metalib�к궨���ĵ�ַ*/
#define TDR_GET_MACRO_TABLE(pstLib)		(LPTDRMACRO)((pstLib)->data + (pstLib)->ptrMacro)

#define TDR_GET_MAP_TABLE(pstLib)           (LPTDRMAPENTRY)((pstLib)->data + (pstLib)->ptrMap)

#define TDR_GET_META_NAME_MAP_TABLE(pstLib)          (LPTDRNAMEENTRY)((pstLib)->data + (pstLib)->ptrName)

#define TDR_GET_META_ID_MAP_TABLE(pstLib)   (LPTDRIDENTRY)((pstLib)->data + (pstLib)->ptrID)

/*ȡ�궨������������ָ��*/
#define TDR_GET_MACROSGROUP_MAP_TABLE(pstLib)           (LPTDRMAPENTRY)((pstLib)->data + (pstLib)->ptrMacroGroupMap)

#define TDR_PTR_TO_MACROSGROUP(pstLib, ptr)		(LPTDRMACROSGROUP)((pstLib)->data + (ptr))

/*��ȡname��index������ָ��*/
#define TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(pstGroup)	(TDRIDX *)((intptr_t)pstGroup + pstGroup->ptrNameIdxMap)

/*��ȡvalue��index������ָ��*/
#define TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup)	(TDRIDX *)((intptr_t)pstGroup + pstGroup->ptrValueIdxMap)

/*���㵥���궨���������ڴ�ռ�*/
#define TDR_CALC_MIN_MACROSGROUP_SIZE(iMacros) (offsetof(TDRMACROSGROUP, data) + \
	sizeof(TDRIDX)*(iMacros)*TDR_MACROSGROUP_MAP_NUM)

/*����궨��������������ռ�*/
#define TDR_CALC_MACROSGROUP_MAP_SIZE(iGroups)  (sizeof(TDRMAPENTRY)*(iGroups))



/*����һ���Զ������������洢����������ֽ���*/
#define TDR_CALC_MIN_META_SIZE(iEntryNum)   ( sizeof(TDRMETAENTRY)*(iEntryNum) + offsetof(TDRMETA, stEntries) )

/*����metalib��洢����������ֽ���*/
#define TDR_CALC_MIN_SIZE(iMetas, iMacros)	( sizeof(TDRMACRO)*(iMacros) + \
(sizeof(TDRIDENTRY) + sizeof(TDRNAMEENTRY) + sizeof(TDRMAPENTRY))*(iMetas) + offsetof(TDRMETALIB, data) )

/*����궨���������ʼλ��*/
#define TDR_CALC_MACRO_PTR(pstParam)            0

/*����metalibID-indexӳ���洢�������ʼλ��*/
#define TDR_CALC_ID_PTR(pstParam)   (TDR_CALC_MACRO_PTR(pstParam) + sizeof(TDRMACRO)*(pstParam->iMaxMacros))

/*����metalibname-indexӳ���洢�������ʼλ��*/
#define TDR_CALC_NAME_PTR(pstParam)  (TDR_CALC_ID_PTR(pstParam) + sizeof(TDRIDENTRY)*pstParam->iMaxMetas)

/*����metalibIndex-ptrӳ���洢�������ʼλ��*/
#define TDR_CALC_MAP_PTR(pstParam)  (TDR_CALC_NAME_PTR(pstParam) + sizeof(TDRNAMEENTRY)*pstParam->iMaxMetas)

/*����meta��洢�������ʼλ��*/
#define TDR_CALC_META_PTR(pstParam) (TDR_CALC_MAP_PTR(pstParam) + sizeof(TDRMAPENTRY)*pstParam->iMaxMetas)

/*�����ַ����������ݿ�洢�������ʼλ��*/
#define TDR_CALC_STRBUF_PTR(pstParam)        (TDR_CALC_META_PTR(pstParam) + pstParam->iMetaSize)

/*����궨�����������Ļ�ַ*/
#define TDR_CALC_MACROSGROUP_MAP_PTR(pstParam) (TDR_CALC_STRBUF_PTR(pstParam) + (pstParam)->iStrBufSize)

/*����궨�����������Ļ�ַ*/
#define TDR_CALC_MACROSGROUP_PTR(pstParam)	(TDR_CALC_MACROSGROUP_MAP_PTR(pstParam) + \
	TDR_CALC_MACROSGROUP_MAP_SIZE(pstParam->iMaxMacrosGroupNum))

#define TDR_GET_FREE_META_SPACE(pstLib)     ((pstLib)->ptrStrBuf - (pstLib)->ptrLaseMeta)

/*��ȡ���л��������׵�ַ*/
#define TDR_GET_FREE_BUF(pstLib)            (char *)((pstLib)->data + (pstLib)->ptrFreeStrBuf)


/*�ж�lib���ַ����������ռ��Ƿ��㹻*/
#define TDR_STRBUF_HAVE_FREE_ROOM(a_pstLib, a_iNeedSize)    ((a_pstLib)->iFreeStrBufSize >= (a_iNeedSize))

#define TDR_COPY_STRING_TO_BUF(ptrStr, pszStr, shStrLen, pstLib) {\
    char *pszBuf = TDR_GET_FREE_BUF(pstLib);\
    TDR_STRNCPY(pszBuf, pszStr, shStrLen);\
    ptrStr = pstLib->ptrFreeStrBuf;\
    pstLib->ptrFreeStrBuf += shStrLen;\
    pstLib->iFreeStrBufSize -= shStrLen;\
}



#define TDR_GET_INT(i, iSize, p)		switch(iSize)			      \
{									      \
	case 2:								      \
		i = (int32_t)*(uint16_t*)(p);				      \
		break;							      \
	case 4:								      \
		i = (int32_t)*(uint32_t*)(p);				      \
		break;							      \
	case 8:									\
		i = (int64_t)*(uint64_t*)(p);		\
		break;										\
	default:							      \
		i = (int32_t)*(uint8_t*)(p);				      \
}

/*add by vinsonzuo 20100514 for mode bug*/
#define TDR_GET_UINT(i, iSize, p)		switch(iSize)			      \
{									      \
	case 2:								      \
		i = (unsigned int)*(unsigned short*)(p);				      \
		break;							      \
	case 4:								      \
		i = (unsigned int)*(unsigned int*)(p);				      \
		break;							      \
	case 8:									\
		i = (tdr_ulonglong)*(tdr_ulonglong*)(p);		\
		break;										\
	default:							      \
		i = (unsigned int)*(unsigned char*)(p);				      \
}


#define TDR_SET_INT(p, iSize, i)		switch(iSize)			      \
{									      \
	case 2:								      \
		*(uint16_t*)(p)	=	(uint16_t)(i);	      \
		break;							      \
	case 4:								      \
		*(uint32_t*)(p)	=	(uint32_t)(i);	      \
		break;							      \
	case 8:									\
		*(uint64_t*)(p) = (uint64_t)(i);		\
		break;											\
	default:							      \
		*(unsigned char*)(p)	=	(unsigned char)(i);	      \
}



#define TDR_SET_VERSION_INDICATOR(_iRet, a_pszHostBase, a_pszHostEnd, pstCurMeta, _iCutOffVersion) \
{																   \
	if (0 < pstCurMeta->stVersionIndicator.iUnitSize)				\
	{																\
		char *pszPtr = a_pszHostBase + pstCurMeta->stVersionIndicator.iHOff;	\
		if ((a_pszHostEnd - pszPtr) < (ptrdiff_t)pstCurMeta->stVersionIndicator.iUnitSize) \
		{																		\
            TDR_ERR_GET_LEN();\
            TDR_SET_ERROR(TDR_ERR_BUF, "host-buffer NOT enough:"\
                          " meta<%s>, a_pszHostBase<%p>, a_pszHostEnd<%p>,"\
                          " version-indicator offset<%u> and size<%u>",\
                          tdr_get_meta_name(pstCurMeta), a_pszHostBase, a_pszHostEnd,\
                          (unsigned)pstCurMeta->stVersionIndicator.iHOff,\
                          (unsigned)pstCurMeta->stVersionIndicator.iUnitSize);\
			_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);		\
		}else																	\
		{																		\
			TDR_SET_INT(pszPtr,pstCurMeta->stVersionIndicator.iUnitSize, _iCutOffVersion);	\
		}																		\
	}																			\
}



#define TDR_GET_VERSION_INDICATOR(_iRet, a_pszHostBase, a_pszHostEnd, pstCurMeta, _iCutOffVersion, _iBaseCutVersion) \
{																   \
	if (0 < pstCurMeta->stVersionIndicator.iUnitSize)				\
	{																\
		int64_t lVal;											\
		char *pszPtr = a_pszHostBase + pstCurMeta->stVersionIndicator.iHOff;	\
		if ((a_pszHostEnd - pszPtr) < pstCurMeta->stVersionIndicator.iUnitSize) \
		{																		\
			_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);		\
		}else																	\
		{																		\
			TDR_GET_INT(lVal, pstCurMeta->stVersionIndicator.iUnitSize, pszPtr);	\
			_iCutOffVersion = (int)lVal;					\
		}																				\
	}else																				\
	{																					\
		_iCutOffVersion = _iBaseCutVersion;													\
	}																					\
}

#define TDR_META_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange) \
{																						\
    pstStackTop->idxEntry++;													\
    if (pstStackTop->idxEntry >= pstCurMeta->iEntriesNum)									\
    {																			\
        pstStackTop->idxEntry = 0;												\
        iChange = pstStackTop->iChange;									\
    }/*if (pstStackTop->idxEntry >= pstCurMeta->iEntriesNum)	*/					\
}


#define TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange) \
{																						\
	if (TDR_TYPE_UNION == pstCurMeta->iType)			\
	{													\
		pstStackTop->iCount--;													\
		pstStackTop->pszHostBase += pstCurMeta->iHUnitSize;						\
		iChange = pstStackTop->iChange;									\
	}else																			\
	{																				\
		pstStackTop->idxEntry++;													\
		if (pstStackTop->idxEntry >= pstCurMeta->iEntriesNum)									\
		{																			\
			pstStackTop->idxEntry = 0;												\
			pstStackTop->iCount--;													\
			pstStackTop->pszHostBase += pstCurMeta->iHUnitSize;						\
			iChange = pstStackTop->iChange;									\
		}/*if (pstStackTop->idxEntry >= pstCurMeta->iEntriesNum)	*/					\
	}	/*if (TDR_TYPE_UNION == pstCurMeta->iType)*/									\
}


//��Ա��refer������ָ��Ա��Ȼ�ڴ���˳�Աʱ�Ѿ�����,���в��ü����ð汾
#define TDR_GET_ARRAY_REAL_COUNT(a_iArrayRealCount, a_pstEntry, a_pszHostBase, a_iCutVersion) \
{										\
	if (0 < a_pstEntry->stRefer.iUnitSize)	\
	{										\
		int64_t lVal;						\
		char *pszPtr =	a_pszHostBase + a_pstEntry->stRefer.iHOff;	\
		TDR_GET_INT(lVal, a_pstEntry->stRefer.iUnitSize, pszPtr);			\
		a_iArrayRealCount = (int)lVal;										\
	}else																	\
	{																\
		a_iArrayRealCount = a_pstEntry->iCount;							\
	}					\
}

#define TDR_GET_ENTRY(idx, entries, size, iId)	\
{ 		      \
	int i;								      \
	int imin;							      \
	int imax;							      \
	if( !TDR_ENTRY_IS_VALID_SELECTID(&entries[0]))	      \
	{                                                   \
		idx = 0;										\
	}else												\
	{													\
		idx = TDR_INVALID_INDEX;						\
	}													\
	i =	iId - entries[0].iMinId;				      \
	if ( (i >= 0) && (i < size) && (entries[i].iMinId == iId ) && (TDR_ENTRY_IS_VALID_SELECTID(&entries[i])) )		      \
	{																	  \
		idx = i;										\
	}else if (size < TDR_MIN_BSEARCH)					\
	{													\
		for (i = 0; i < size; i++)						\
		{												\
			if ((entries[i].iMinId <= iId) && (iId <= entries[i].iMaxId))	\
			{															\
				idx = i;												\
				break;													\
			}															\
		}																\
	}else																\
	{																	\
		imin = 0;														\
		imax = size - 1;													\
		while(imin <= imax)												\
		{																\
			i =	(imin + imax)>>1;											\
			if (!TDR_ENTRY_IS_VALID_SELECTID(&entries[i]))					\
			{																\
				imin = i + 1;													\
				continue;														\
			}																	\
			if (iId < entries[i].iMinId)								\
			{															\
				imax = i -1;												\
			}else if (iId <= entries[i].iMaxId)							\
			{															\
				idx = i;												\
				break;													\
			}else														\
			{															\
				imin = i + 1;											\
			}															\
		}																\
	}																	\
}



#define TDR_GET_UNION_ENTRY_TYPE_META_INFO(a_pszHostBase, pstLib, pstEntry, a_iVersion, pstTypeMeta, idxSubEntry) \
{																			\
	int64_t iID;										\
	char *pszPtr = a_pszHostBase + pstEntry->stSelector.iHOff;	\
	TDR_GET_INT(iID, pstEntry->stSelector.iUnitSize, pszPtr);							\
	pstTypeMeta	= TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);				\
	TDR_GET_ENTRY(idxSubEntry, pstTypeMeta->stEntries, pstTypeMeta->iEntriesNum, (int)iID);	\
	if( (TDR_INVALID_INDEX == idxSubEntry) || (pstTypeMeta->stEntries[idxSubEntry].iVersion > a_iVersion )) \
	{																									\
		pstTypeMeta	= NULL;																				\
	}											\
}

#define TDR_SET_DEFAULT_VALUE(a_iRet, a_pszHostStart, a_pszHostEnd, a_pstLib, a_pstEntry, a_iArrayRealCount) \
{																										\
	ptrdiff_t iCopyLen = 0;																					\
	if ((TDR_INVALID_PTR == (a_pstEntry)->ptrDefaultVal))												\
	{																									\
		if (0 < (a_pstEntry)->iCustomHUnitSize)																\
		{																								\
			iCopyLen = (ptrdiff_t)((a_pstEntry)->iCustomHUnitSize * (a_iArrayRealCount));												\
		}else																							\
		{																								\
			iCopyLen = (ptrdiff_t)((a_pstEntry)->iHUnitSize * (a_iArrayRealCount));													\
		}																								\
		if ((a_pszHostEnd - a_pszHostStart) < iCopyLen)													\
		{																								\
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);								\
		}else																							\
		{																								\
			memset(a_pszHostStart, 0, iCopyLen);										                \
		}																								\
		a_pszHostStart += iCopyLen;																		\
	}else /*��ȱʡֵ*/  																				\
	{																									\
		if ((TDR_TYPE_STRING == a_pstEntry->iType)|| (TDR_TYPE_WSTRING == a_pstEntry->iType))														\
		{																								\
			if (0 < a_pstEntry->iCustomHUnitSize)														\
			{																							\
				iCopyLen = a_pstEntry->iCustomHUnitSize;													\
			}else																						\
			{																							\
				iCopyLen = a_pszHostEnd - a_pszHostStart;													\
			}																							\
		}else																							\
		{																								\
			iCopyLen = a_pstEntry->iHUnitSize;\
		}/*if (TDR_TYPE_STRING == a_pstEntry->iType)*/													\
		iCopyLen = TDR_MIN((a_pszHostEnd - a_pszHostStart), iCopyLen);\
		if (iCopyLen < (ptrdiff_t)a_pstEntry->iDefaultValLen)														\
		{																								\
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);								\
		}else																							\
		{																								\
            int i;                                                                                      \
			char *pszSrc = TDR_GET_STRING_BY_PTR(a_pstLib, a_pstEntry->ptrDefaultVal);					\
            char *pszSavedSrc = pszSrc;                                                                 \
            for (i = 0; i < a_iArrayRealCount; i++)                                                     \
            {                                                                                           \
                TDR_MEMCPY(a_pszHostStart, pszSrc, a_pstEntry->iDefaultValLen, TDR_MIN_COPY);			\
                pszSrc = pszSavedSrc;                                                                   \
                a_pszHostStart += (iCopyLen - a_pstEntry->iDefaultValLen);								\
            }                                                                                           \
		}																								\
	}/*if ((TDR_INVALID_PTR == a_pstEntry->ptrDefaultVal))	*/   										\
}

#define TDR_CHECK_BUFF(pszBuf, iBufLen, iWriteSize, iError) \
	if ((0 > iWriteSize) || (iWriteSize >= (int)iBufLen)) \
	{   \
	iError = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_SPACE_TO_WRITE);   \
	} else \
	{ \
	pszBuf += (iWriteSize); \
	iBufLen -= (iWriteSize); \
	}

/*��ȡָ��Ԫ��������Ӧ�����ݿ���Ԫ��������*/
#define TDR_GET_DBTABLE_META(a_pstTableMeta, a_pstLib, a_pstMeta) \
{																\
	if (TDR_INVALID_PTR != a_pstMeta->ptrDependonStruct)		\
	{															\
	a_pstTableMeta = TDR_PTR_TO_META(a_pstLib, a_pstMeta->ptrDependonStruct);	\
	}else																\
	{																	\
	a_pstTableMeta = a_pstMeta;										\
	}																	\
}

/*���Ԫ���������⹹���汾��ƽ̨�Ƿ�һ��*/
#define TDR_CHECK_METALIB(a_pstLib) \
{                                   \
    if( (TDR_MAGIC != (a_pstLib)->wMagic) || (TDR_BUILD != (a_pstLib)->nBuild)) \
    {                                                                           \
        return  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT);      \
    }                                                                           \
    if (TDR_FLATFORM_MAGIC != (a_pstLib)->dwPlatformArch)                       \
    {                                                                           \
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_PLATFORM_CONFLICT);            \
    }                                                                           \
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TDR_METALIB_KERNEL_H */
