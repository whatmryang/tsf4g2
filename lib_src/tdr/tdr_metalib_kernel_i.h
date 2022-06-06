/**
*
* @file     tdr_metalib_kernel_i.h
* @brief    TDR元数据库核心结构
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-16
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
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

/** @defgroup TDR_METALIB_MANAGER TDRMetaLib管理
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
重定向net/host信息
*/
struct tagTDRSelector
{
	size_t iUnitSize;	/**<所占存储空间*/
    TDROFF iHOff;    /**<本地存储信息偏移*/
    TDRPTR ptrEntry;   /**<关联entry的指针*/
};

typedef struct tagTDRSelector TDRSelector;
typedef struct tagTDRSelector *LPTDRSelector;

/*跟网络处理相关的重定向器
*/
struct tagTDRRedirector
{
	TDROFF iNOff;   /**<网络传输信息偏移*/
	TDROFF iHOff;    /**<本地存储信息偏移*/
	size_t iUnitSize;
};

typedef struct tagTDRRedirector TDRREDIRECTOR;
typedef struct tagTDRRedirector *LPTDRREDIRECTOR;

/**
sortkey信息
*/
struct tagTDRSortKeyInfo
{
	TDRIDX      idxSortEntry;               /**< 用来排序的entry在meta中的索引*/
	TDROFF	iSortKeyOff;			/**< 排序元素的本地存储偏移. */
	TDRPTR	ptrSortKeyMeta;			/**< 用来排序的entry所在meta的索引*/
};
typedef struct tagTDRSortKeyInfo	TDRSORTKEYINFO;
typedef struct tagTDRSortKeyInfo	*LPTDRSORTKEYINFO;


/**
* 此结构定义一个数据成员。当iCount为0时表示此成员为一可变数组，
* struct/Union中仅容许出现一个可变数组成员
*/
struct tagTDRMetaEntry
{
    int iID;		/**< 此元数据的ID*/
    int iVersion;	        /**< 此元数据加到MetaLib库中时的版本*/
    int iType;		/**< the typeid of this entry. */
    TDRPTR ptrName;	/**< ptr of name, used for generating c/c++ header. */

	size_t iHRealSize;		/**<本地存储所需总共的空间*/
    size_t iNRealSize;          /**<网络传输时所需的的总空间*/
	size_t iHUnitSize;			/**<本地存储单个entry单元所需空间*/
	size_t iNUnitSize;			/**<网络存储单个entry单元所需空间*/
    size_t iCustomHUnitSize;    /**< 自定义存储单元大小*/

    int iCount;		/**< 1 means single, >1 means array, 0 means variable array */

    TDROFF iNOff;		/**< 网络传输时的偏移，1字节对齐*/
	TDROFF iHOff;		/**< 本地存储时的偏移，使用指定对齐方式*/


    TDRIDX idxID;  			/**< the id macro's index. */
    TDRIDX idxVersion;		/**< 如果版本取值为宏定义，则此成员保存对应宏定义在宏定义列表中的索引*/
    TDRIDX idxCount;		/**< the macro's referred by count. */
    TDRIDX idxType;			/**< the index of the typeinfo. */
    TDRIDX idxCustomHUnitSize;             /**< 自定义存储单元大小宏定义值的索引*/

    unsigned short    wFlag;		/**< 存取此元素对应标志信息，如指针，引用等*/
    char    chDBFlag;         /**< TDR-DB 对象关系映射 */
    char    chOrder;          /**< 1 if ascending, -1 if desending, else 0 */


    TDRSIZEINFO stSizeInfo;   /**<sizeinfo属性值*/
    TDRSelector stRefer;      /**<refer属性值*/
    TDRSelector stSelector;      /**<select 属性*/

    int iIO;		/**< the input/output control. */
    int idxIO;		/**< the idx of input/output control macro. */

    TDRPTR ptrMeta;			/**<当此成员为复合数据类型时，记录其复合数据类型元数据meta数据的指针. */

	/**<当selector的值在[iMinId,iMaxID]区间时，选择此元素*/
	int iMaxId;
	int iMinId;

	/**< index of macro of iMaxId,iMinId*/
	TDRIDX iMaxIdIdx;
	TDRIDX iMinIdIdx;


    /*
    *成员表示名, 描述信息，中文名，缺省值字符串保存在Metalib管理的字符串内存池
    *中，此处仅保存字符串的起始地址和长度
    */
    size_t iDefaultValLen;			/**<缺省值的长度*/
    TDRPTR ptrDesc;				/**< ptr of  the description info. */
    TDRPTR ptrChineseName;		/**< ptr of chinse name of entry*/
    TDRPTR ptrDefaultVal;		/**< ptr of default value of entry*/

	TDRPTR ptrMacrosGroup; /*此成员元素绑定的宏定义组指针， Add at TDR build Version: 0x00000008*/
	TDRPTR ptrCustomAttr; /*指向自定义属性值的指针， Add at TDR build Version: 0x00000008*/
    TDRPTR iOffToMeta; /*相对于此成员所属的复合数据结构的描述的偏移， Add at TDR build Version: 0x00000009*/

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

/*blob 压缩存储*/
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
        TDROFF	iHOff;			/**< 元素的本地存储偏移. */
        TDRPTR	ptrEntry;		/**< entry指针*/
        int     iIsDesc;        /**< 1:DESC, 0:ASC */
        int     iCaredCharNum;  /**< 对于可变长度的字符串，指定索引关注的前N个字符*/
    };
    typedef struct tagTDRColumnInfo TDRCOLUMNINFO;
    typedef struct tagTDRColumnInfo *LPTDRCOLUMNINFO;

    /*
     * 此结构定义数据库建表操作时创建数据表索引的相关信息
     * added at tdr build 0x0000b
     */
    struct tagTDRMetaIndex
    {
        int iVersion;	        /**< 此元数据加到MetaLib库中时的版本*/
        TDRPTR ptrName;	        /**< ptr of name. */

        TDRPTR iOffToMeta;      /**< 相对于此成员所属的复合数据结构的描述的偏移 */

        int iColumnNum;         /**< 数据库索引表列成员的个数 */
        TDRPTR ptrColumnBase;   /**< 数据库索引表列成员的基址指针 */

        int iIndexType;         /**< 索引表的存储数据结构类型. RESERVED*/
                                /**< 1:BTREE, 2:HASH */
        int iIsUnique;          /**< 对应索引的UNIQUE属性. RESERVED*/

        TDRPTR ptrDesc;			/**< ptr of  the description info. RESERVED */
        TDRPTR ptrChineseName;	/**< ptr of chinse name of entry. RESERVED */

        TDRPTR iReserve1;       /**< RESERVED */
        TDRPTR iReserve2;       /**< RESERVED */
    };
    typedef struct tagTDRMetaIndex TDRMETAINDEX;
    typedef struct tagTDRMetaIndex *LPTDRMETAINDEX;

    /**DB主键信息
    */
    struct tagTDRDBKeyInfo
    {
        TDROFF	iHOff;			/**< 元素的本地存储偏移. */
        TDRPTR	ptrEntry;			/**< entry指针*/
    };
    typedef struct tagTDRDBKeyInfo	TDRDBKEYINFO;
    typedef struct tagTDRDBKeyInfo	*LPTDRDBKEYINFO;




/**
* 存储meta信息的结构
*/
struct tagTDRMeta
{
    uint32_t uFlags;		/**< flag info of mata*/

    int iID;			/**< id of meta*/
    int iBaseVersion;		/**< base version of meta*/
    int iCurVersion;	/**< current version of meta*/
    int iType;			/**< type of meta*/
	size_t iMemSize;			/**< 本meta结构内存结构大小*/


	size_t iNUnitSize;		/**<网络传输时的单元大小*/
	size_t iHUnitSize;		/**<本地存储单元大小*/

	size_t iCustomHUnitSize; /**<自定义本地存储单元大小*/
	int idxCustomHUnitSize; /**<自定义本地存储单元大小的宏定义值索引 */

    int iMaxSubID;			/**< max id of child entry*/
    int iEntriesNum;		/**< num of child entries*/

        /* Add at TDR build version: 0x0000000b */
        int iIndexesNum;        /**< num of child indexes*/
        TDRPTR ptrIndexes;      /**< 指向index数组的起始地址，在(entries, primarykeys)之后*/
        TDRPTR ptrColumns;      /**< 指向column数组的起始地址，在(entries, primarykeys, indexies)之后*/
        /* Add at TDR build version: 0x0000000b */


    TDRPTR ptrMeta;		/**< offset of this meta from "data" member of head. */

    TDRIDX iIdx;		/**< index of this mata in metalib*/
    TDRIDX idxID;		/**< index of macro of id*/
    TDRIDX idxType;		/**< index of meta's type*/
    TDRIDX idxVersion;	/**< index of macro of meta's version*/

	int iCustomAlign;	/**< structs元素属性 指定结构各成员变量的对齐大小 Default is 1*/
	int iValidAlign;	/**< meta元素有效的对齐值，为各成员对齐值中最大的那个值*/

	int iVersionIndicatorMinVer;	/**< 版本指示器能指定的最少本版*/
	TDRSIZEINFO stSizeType;		/**< 记录打包信息*/
	TDRREDIRECTOR stVersionIndicator;

	TDRSORTKEYINFO stSortKey; /**<sortkey属性值*/

    TDRPTR ptrName;	/**< ptr of Name of meta*/

    /*描述信息，中文名字符串保存在Metalib管理的字符串内存池,此处仅保存字符串的起始地址和长度*/
    TDRPTR ptrDesc;				/**< ptr of  the description info. */
    TDRPTR ptrChineseName;		/**< ptr of chinse name of entry*/

	int iSplitTableFactor;	/**<数据库分表因子*/
	short nSplitTableRuleID;	/**<数据库分表规则id*/
	short nPrimayKeyMemberNum;	/**<组成主键的成员个数*/

	TDRIDX idxSplitTableFactor;	/**< index of macro of meta's wSplitTableFactor*/
    TDRDBKEYINFO stSplitTableKey;	/**<数据库分表主键成员的指针，实际存储相对metalib库的相对偏移值*/
	TDRPTR ptrPrimayKeyBase;	/**<数据库主键成员的基址指针，实际存储相对metalib库的相对偏移值*/
	TDRPTR	ptrDependonStruct;  /*此结构继承的元数据指针*/

    TDRPTR iReserve1;   /**< reserve for extend . Add at TDR build Version: 0x00000009 */
    TDRPTR iReserve2;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */
    TDRPTR iReserve3;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */

    TDRMETAENTRY stEntries[1];
};

#define TDR_STACK_SIZE		32    /**<元数据处理栈大小，也即容许的元数据最大嵌套层次*/
#define TDR_MIN_BSEARCH		16

/**元数据处理栈数据
*/
struct tagTDRStackItem
{
	LPTDRMETA pstMeta;	/**<当前正在处理的元数据 */
	LPTDRMETAENTRY pstEntry;	/**<当前结构在父结构中的成员句柄*/

	int iRealCount; /**<此元数据成员的实际数组长度*/
	int iCount;		/**<对于元数据数组，记录剩余要处理的元数据的个数 */
	int idxEntry;		/**< 此元数据当前正在处理的entry索引 */

	size_t iMetaSizeInfoUnit;  /**<此meta长度信息存储的空间大小*/
	TDROFF iMetaSizeInfoOff;	/**<sizeinfo信息的偏移*/
	TDROFF iEntrySizeInfoOff;  /**<entry sizeinfo属性*/

	int iCutOffVersion;		/**<meta剪裁得版本*/

	size_t iCode;			/*记录编解码的字节数*/
	char* pszHostBase; /**<本元数据存储的host基址*/
	char* pszHostEnd; /**<本元数据存储的缓冲区终止地址*/
	char* pszNetBase;  /**<本元数据存储的net基址*/
	char* pszMetaSizeInfoTarget;   /**<此meta打包长度信息存储的基址*/

	int iChange;		/*记录一个结构体的所有成员是否已经处理完，其值为0，则没有处理完，否则已经处理完*/
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


 //如果此标志位之上，则说明结构体展开后成员名不唯一，需要通过添加后缀才行
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
    TDRPTR ptrMeta;  /*meta的偏移**/
};

typedef struct tagTDRIDEntry	TDRIDENTRY;
typedef struct tagTDRIDEntry	*LPTDRIDENTRY;


struct tagTDRNameEntry
{
    TDRPTR ptrName;	/**< ptr of Name of meta*/
    TDRPTR ptrMeta;  /*meta的偏移**/
};

typedef struct tagTDRNameEntry	TDRNAMEENTRY;
typedef struct tagTDRNameEntry	*LPTDRNAMEENTRY;



/*保存宏定义信息的结构*/
struct tagTDRMacro
{
    TDRPTR ptrMacro;				/**< name of macro */
	int iValue;
	TDRPTR ptrDesc;				/**< ptr of  the description info. Add at TDR build Version: 0x00000004 */

    int iReserve;   /**< reserve for extend . Add at TDR build Version: 0x00000009 */
};

    struct tagTDRMacrosGroup
    {
        int iCurMacroCount;	/**<宏定义组中宏定义的个数*/
        int iMaxMacroCount;	/**<最多能存储的宏定义个数*/
        TDRPTR ptrDesc;				/**< ptr of  the description info. */
        TDRPTR ptrNameIdxMap;				/**< based address of macro name－index map. */
        TDRPTR ptrValueIdxMap;				/**< based address of macro value－index map. */
        char szName[TDR_NAME_LEN]; /**< 宏定义组的名字*/
        char data[1];			/**<映射数据区*/
    };

struct tagTDRValueFieldDefinition
{
	int iCurCount;	/**<值域定义个数*/
	int iMaxMacroCount;	/**<最多能存储的值域定义个数*/
	TDRPTR ptrDesc;				/**< ptr of  the description info. */
	char szName[TDR_NAME_LEN]; /**< 值域定义的名字*/
	char data[1];			/**<数据区*/
};
typedef struct tagTDRValueFieldDefinition TDRVALUEFIELDDEFINITION;
typedef struct tagTDRValueFieldDefinition *LPTDRVALUEFIELDDEFINITION;


/**
元数据库的结构信息
In order to refer the meta data fastly, We use two-level mapping.
First, A map entry for each meta data.
Second, A index value for each map entry.
There are two index array, one for id, one for name.
*/
struct tagTDRMetaLib
{
    unsigned short wMagic;
    short nBuild;
    uint32_t dwPlatformArch;  /*记录是32位平台还是64位平台描述库*/
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

	int iMaxMacrosGroupNum; /*最多能容纳的宏定义组数目, Add at TDR build Version: 0x00000008*/
	int iCurMacrosGroupNum; /*宏定义组当前数目， Add at TDR build Version: 0x00000008*/

	int iMaxValFieldDefNum; /*最多能容纳的值域定义组数目, Add at TDR build Version: 0x00000008*/
	int iCurValFieldDefNum; /*值域定义组当前数目， Add at TDR build Version: 0x00000008*/

    int iVersion;

    /** all the offset is start from the 'data' member.
    */
    TDRPTR ptrMacro;	/*ptr for macro info block*/
    TDRPTR ptrID;		/*ptr of begin address for id-metaidx mapping info block*/
    TDRPTR ptrName;		/*ptr of begin address for name-metaidx mapping info block*/
    TDRPTR ptrMap;		/*ptr of begin address for metaidx-metaOff mapping info block*/
    TDRPTR ptrMeta;		/*ptr of begin address for meta info block*/
    TDRPTR ptrLaseMeta;	/*ptr of last meta in mata*/

    /*字符串缓冲区定义*/
    size_t  iFreeStrBufSize;	/*字符串缓冲区空闲区域的大小*/
    TDRPTR	ptrStrBuf;		/*字符串缓冲区的偏移地址，从data成员开始计算*/
    TDRPTR	ptrFreeStrBuf;	/*可用的空闲缓冲区的首地址*/

	TDRPTR	ptrMacroGroupMap;	/*ptr of macrosgroup index-off mapping info block Add at TDR build Version: 0x00000008*/
	TDRPTR ptrMacrosGroup;	/*ptr of macrosgroup, Add at TDR build Version: 0x00000008*/

	size_t iMacrosGroupSize; /*宏定义组可以使用的最大空间， Add at TDR build Version: 0x00000008*/

	TDRPTR	ptrValueFiledDefinitionsMap;	/*值域定义组索引数据区 Add at TDR build Version: 0x00000008*/
	TDRPTR ptrValueFiledDefinitions;	/*值域定义基址, Add at TDR build Version: 0x00000008*/
	size_t iValueFiledDefinitionsSize; /*值域定义数据区大小， Add at TDR build Version: 0x00000008*/

    /* 预留字段 */
    TDRPTR iReserve1;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */
    TDRPTR iReserve2;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */
    TDRPTR iReserve3;   /**< reserve for extend . Add at TDR build Version: 0x0000000a */

    char szName[TDR_NAME_LEN];
    char data[1];		/* only used for reference data. */
};

#define  TDR_MIN_STR_BUF_SIZE   128  /**< 字符串缓冲区的缺省空间*/

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


/*根据字符串的基址计算此字符的首地址*/
#define TDR_GET_STRING_BY_PTR(pstLib, ptr)			(char *)((pstLib)->data + (ptr))

#define TDR_GET_META_INDEX_PTR(pstMeta)  (LPTDRMETAINDEX)((char*)(pstMeta) + (pstMeta)->ptrIndexes)

#define TDR_GET_COLUMN_PTR(pstMeta, pstIndex) (LPTDRCOLUMNINFO)((char*)(pstMeta) + (pstIndex)->ptrColumnBase)

#define TDR_IDX_TO_META(pstLib, idx)	(LPTDRMETA) (pstLib->data + ((LPTDRMAPENTRY)(pstLib->data + pstLib->ptrMap))[idx].iPtr)

#define TDR_PTR_TO_META(pstLib, ptr)	(LPTDRMETA) (pstLib->data + (ptr))

#define TDR_META_TO_LIB(pstMeta)		(LPTDRMETALIB) ( ((intptr_t)(pstMeta)) - (pstMeta)->ptrMeta - offsetof(TDRMETALIB, data) )

#define TDR_GET_PRIMARYBASEPTR(pstMeta)  (LPTDRDBKEYINFO)(((intptr_t)(pstMeta)) + pstMeta->ptrPrimayKeyBase)

/*根据entry的偏移计算此entry的指针*/
#define TDR_PTR_TO_ENTRY(pstLib, ptr)   (LPTDRMETAENTRY)(pstLib->data + (ptr))

/*根据entry的指针计算此entry的偏移*/
#define TDR_ENTRY_TO_PTR(pstLib, pstEntry)	(TDRPTR)((char *)(pstEntry) - (pstLib)->data)

/*根据meta和entry的指针计算此entry的偏移*/
#define TDR_CALC_ENTRYOFF_OF_META(a_pstMeta, a_pstEntry)	(TDRPTR)((intptr_t)(a_pstEntry) - (intptr_t)(a_pstMeta))

/*根据entry的指针和其距离meta的偏移计算此meta的指针*/
#define TDR_ENTRY_TO_META(a_pstEntry)	(LPTDRMETA)((intptr_t)(a_pstEntry) - (a_pstEntry)->iOffToMeta)


/*取出metalib中宏定义表的地址*/
#define TDR_GET_MACRO_TABLE(pstLib)		(LPTDRMACRO)((pstLib)->data + (pstLib)->ptrMacro)

#define TDR_GET_MAP_TABLE(pstLib)           (LPTDRMAPENTRY)((pstLib)->data + (pstLib)->ptrMap)

#define TDR_GET_META_NAME_MAP_TABLE(pstLib)          (LPTDRNAMEENTRY)((pstLib)->data + (pstLib)->ptrName)

#define TDR_GET_META_ID_MAP_TABLE(pstLib)   (LPTDRIDENTRY)((pstLib)->data + (pstLib)->ptrID)

/*取宏定义组索引区表指针*/
#define TDR_GET_MACROSGROUP_MAP_TABLE(pstLib)           (LPTDRMAPENTRY)((pstLib)->data + (pstLib)->ptrMacroGroupMap)

#define TDR_PTR_TO_MACROSGROUP(pstLib, ptr)		(LPTDRMACROSGROUP)((pstLib)->data + (ptr))

/*获取name－index索引表指针*/
#define TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(pstGroup)	(TDRIDX *)((intptr_t)pstGroup + pstGroup->ptrNameIdxMap)

/*获取value－index索引表指针*/
#define TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup)	(TDRIDX *)((intptr_t)pstGroup + pstGroup->ptrValueIdxMap)

/*计算单个宏定义组所需内存空间*/
#define TDR_CALC_MIN_MACROSGROUP_SIZE(iMacros) (offsetof(TDRMACROSGROUP, data) + \
	sizeof(TDRIDX)*(iMacros)*TDR_MACROSGROUP_MAP_NUM)

/*计算宏定义组索引区所需空间*/
#define TDR_CALC_MACROSGROUP_MAP_SIZE(iGroups)  (sizeof(TDRMAPENTRY)*(iGroups))



/*计算一个自定义类型描述存储所需的最少字节数*/
#define TDR_CALC_MIN_META_SIZE(iEntryNum)   ( sizeof(TDRMETAENTRY)*(iEntryNum) + offsetof(TDRMETA, stEntries) )

/*计算metalib库存储所需的最少字节数*/
#define TDR_CALC_MIN_SIZE(iMetas, iMacros)	( sizeof(TDRMACRO)*(iMacros) + \
(sizeof(TDRIDENTRY) + sizeof(TDRNAMEENTRY) + sizeof(TDRMAPENTRY))*(iMetas) + offsetof(TDRMETALIB, data) )

/*计算宏定义块的相对起始位置*/
#define TDR_CALC_MACRO_PTR(pstParam)            0

/*计算metalibID-index映射块存储的相对起始位置*/
#define TDR_CALC_ID_PTR(pstParam)   (TDR_CALC_MACRO_PTR(pstParam) + sizeof(TDRMACRO)*(pstParam->iMaxMacros))

/*计算metalibname-index映射块存储的相对起始位置*/
#define TDR_CALC_NAME_PTR(pstParam)  (TDR_CALC_ID_PTR(pstParam) + sizeof(TDRIDENTRY)*pstParam->iMaxMetas)

/*计算metalibIndex-ptr映射块存储的相对起始位置*/
#define TDR_CALC_MAP_PTR(pstParam)  (TDR_CALC_NAME_PTR(pstParam) + sizeof(TDRNAMEENTRY)*pstParam->iMaxMetas)

/*计算meta块存储的相对起始位置*/
#define TDR_CALC_META_PTR(pstParam) (TDR_CALC_MAP_PTR(pstParam) + sizeof(TDRMAPENTRY)*pstParam->iMaxMetas)

/*计算字符串缓冲数据块存储的相对起始位置*/
#define TDR_CALC_STRBUF_PTR(pstParam)        (TDR_CALC_META_PTR(pstParam) + pstParam->iMetaSize)

/*计算宏定义组索引区的基址*/
#define TDR_CALC_MACROSGROUP_MAP_PTR(pstParam) (TDR_CALC_STRBUF_PTR(pstParam) + (pstParam)->iStrBufSize)

/*计算宏定义组数据区的基址*/
#define TDR_CALC_MACROSGROUP_PTR(pstParam)	(TDR_CALC_MACROSGROUP_MAP_PTR(pstParam) + \
	TDR_CALC_MACROSGROUP_MAP_SIZE(pstParam->iMaxMacrosGroupNum))

#define TDR_GET_FREE_META_SPACE(pstLib)     ((pstLib)->ptrStrBuf - (pstLib)->ptrLaseMeta)

/*获取空闲缓冲区的首地址*/
#define TDR_GET_FREE_BUF(pstLib)            (char *)((pstLib)->data + (pstLib)->ptrFreeStrBuf)


/*判断lib的字符串缓冲区空间是否足够*/
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


//成员的refer属性所指成员必然在处理此成员时已经生成,所有不用检查剪裁版本
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
	}else /*有缺省值*/  																				\
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

/*获取指定元数据所对应的数据库表的元数据描述*/
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

/*检测元数据描述库构建版本和平台是否一致*/
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
