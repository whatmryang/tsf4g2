#include <assert.h>
#include <scew/scew.h>
#include "tdr_XMLtags_i.h"
#include "tdr/tdr_error.h"
#include "../tdr_os.h"

#define TDR_ITEM_UNKOWN        -1
#define TDR_ITEM_METALIB       1
#define TDR_ITEM_MACRO         2
#define TDR_ITEM_MACROSGROUP   4
#define TDR_ITEM_STRUCT        8
#define TDR_ITEM_UNION        16
#define TDR_ITEM_ENTRY        32
#define TDR_ITEM_INCLUDE      64


#define TDR_ITEM_ALL (TDR_ITEM_METALIB | TDR_ITEM_MACRO | TDR_ITEM_MACROSGROUP\
    | TDR_ITEM_STRUCT | TDR_ITEM_UNKOWN | TDR_ITEM_ENTRY | TDR_ITEM_INCLUDE)

typedef struct stAttrInfo
{
    char* pszAttrName;
    int iFlag;
} stAttrInfo;

typedef stAttrInfo ATTRINFO;
typedef stAttrInfo* LPATTRINFO;

static const stAttrInfo gs_tdr_astCommAttrs[] = {
    {TDR_TAG_ID, TDR_ITEM_ALL & ~TDR_ITEM_MACRO & ~TDR_ITEM_MACROSGROUP},
    {TDR_TAG_NAME, TDR_ITEM_ALL},
    /*cname of macros will be used by resource-converter*/
    /*{TDR_TAG_CNNAME, TDR_ITEM_ALL & ~TDR_ITEM_MACRO & ~TDR_ITEM_MACROSGROUP & ~TDR_ITEM_METALIB},*/
    {TDR_TAG_CNNAME, TDR_ITEM_ALL & ~TDR_ITEM_MACROSGROUP & ~TDR_ITEM_METALIB},
    {TDR_TAG_DESCIPTION, TDR_ITEM_ALL & ~TDR_ITEM_METALIB},
    {NULL, 0}
};

#define TDR_PARENT_IS_STRUCT 0x1
#define TDR_PARENT_IS_UNION  0x2

#define TDR_SEARCH_ARRAY(a_iFound, a_pszCurrentAttr, a_pstAttrs)\
{\
    int i = 0;\
    while (!a_iFound && a_pstAttrs[i].pszAttrName)\
    {\
        if (!strcmp(a_pszCurrentAttr, a_pstAttrs[i++].pszAttrName))\
        {\
            a_iFound = 1;\
        }\
    }\
}\

#define TDR_SEARCH_ARRAY_WITH_FLAG(a_iFound, a_pszCurrentAttr, a_pstAttrs, a_iFlag)\
{\
    int i = 0;\
    while (!a_iFound && a_pstAttrs[i].pszAttrName)\
    {\
        if (!strcmp(a_pszCurrentAttr, a_pstAttrs[i].pszAttrName))\
        {\
            if (!a_iFlag || (a_iFlag & a_pstAttrs[i].iFlag))\
            {\
                a_iFound = 1;\
            }\
        }\
        i++;\
    }\
}\

#define TDR_SEARCH_ITEM_ATTRIBUTE(a_iFound, a_pszCurrentAttr, a_pstCommAttrs, a_pstSpecialAttrs, a_iType)\
{\
    a_iFound = 0;\
    TDR_SEARCH_ARRAY_WITH_FLAG(a_iFound, a_pszCurrentAttr, a_pstCommAttrs, a_iType);\
    TDR_SEARCH_ARRAY(a_iFound, a_pszCurrentAttr, a_pstSpecialAttrs);\
}\

#define TDR_SEARCH_ENTRY_ATTRIBUTE(a_iFound, a_pszCurrentAttr, a_pstCommAttrs, a_pstSpecialAttrs, a_iFlag)\
{\
    a_iFound = 0;\
    TDR_SEARCH_ARRAY(a_iFound, a_pszCurrentAttr, a_pstCommAttrs);\
    TDR_SEARCH_ARRAY_WITH_FLAG(a_iFound, a_pszCurrentAttr, a_pstSpecialAttrs, a_iFlag);\
}\

#define TDR_CHECK_ITEM_ATTRIBUTE(a_pstElement, a_pszValueOfNameAttr, \
        astCommAttrs, astSpecialAttrs, a_pszTagName, a_iItemType, a_fp)\
{\
    scew_attribute* attribute = NULL;\
    const char* pszAttrName = NULL;\
    const char* pszAttrValue = NULL;\
    int iFound = 0;\
    while ((attribute = scew_attribute_next(a_pstElement, attribute)) != NULL) \
    { \
        pszAttrName = scew_attribute_name(attribute);\
        assert(pszAttrName);\
        pszAttrValue = scew_attribute_value(attribute);\
        assert(pszAttrValue);\
        TDR_SEARCH_ITEM_ATTRIBUTE(iFound, pszAttrName, astCommAttrs, astSpecialAttrs, a_iItemType);\
        if (!iFound)\
        {\
            if (a_pszValueOfNameAttr)\
            {\
                fprintf(a_fp, "warning:\n <%s name=\"%s\" ... %s=\"%s\" ...>\n"\
                        " 属性 %s 无效，请查阅tdr元数据描述规则关于 %s 的属性的部分\n", \
                        a_pszTagName, a_pszValueOfNameAttr, pszAttrName,\
                        pszAttrValue, pszAttrName, a_pszTagName);\
            }else\
            {\
                fprintf(a_fp, "warning:\n <%s ... %s=\"%s\" ...>\n"\
                        " 属性 %s 无效，请查阅tdr元数据描述规则关于 %s 的属性的部分\n", \
                        a_pszTagName, pszAttrName,\
                        pszAttrValue, pszAttrName, a_pszTagName);\
            }\
        }\
    }\
}\

void tdr_check_metalib_format_i(scew_tree* a_pstTree, FILE* a_fp);
static void tdr_check_item_attr_i(const scew_element* a_pstElement,
        const char* a_pszValueOfNameAttr, int a_iItemType, FILE* a_fp);
static void tdr_check_entry_attr_i(const scew_element* a_pstElement,
        const char* a_pszValueOfNameAttr, const char* a_pszParentName,
        FILE* a_fp, int a_iFlag);
static void tdr_check_index_attr_i(const scew_element* a_pstElement,
        const char* a_pszValueOfNameAttr, const char* a_pszParentName,
        FILE* a_fp, int a_iFlag);

#define TDR_CHECK_CHILDREN_ATTR(a_pstElement, a_pszValueOfNameAttr, a_fp, a_iFlag)\
{\
    scew_element* child = NULL;\
    while ((child = scew_element_next(a_pstElement, child)) != NULL) \
    { \
        const char *pszValueOfNameAttr = NULL;\
        scew_attribute *pstAttr = NULL;\
        const char *pszElemName = NULL;\
\
        pstAttr = scew_attribute_by_name(child, TDR_TAG_NAME);\
        if (!pstAttr)\
        {\
            /* no TDR_TAG_NAME will be regared as error in other place */\
            continue;\
        }\
        pszValueOfNameAttr = scew_attribute_value(pstAttr);\
\
        pszElemName = scew_element_name(child);\
        if (0 == tdr_stricmp(TDR_TAG_ENTRY, pszElemName))\
        {\
            tdr_check_entry_attr_i(child, pszValueOfNameAttr, \
                                   a_pszValueOfNameAttr, a_fp, a_iFlag);\
        } else if (0 == tdr_stricmp(TDR_TAG_INDEX, pszElemName))\
        {\
            if (TDR_PARENT_IS_UNION | a_iFlag)\
            {\
                /* other place has give warning about this */\
                continue;\
            }\
            tdr_check_index_attr_i(child, pszValueOfNameAttr, \
                                   a_pszValueOfNameAttr, a_fp, a_iFlag);\
        } else\
        {\
            /* other place has give warning about this */\
            continue;\
        }\
\
    } \
}\

/////////////////////////////////////////////////////////////////

void tdr_check_metalib_format_i(scew_tree* a_pstTree, FILE* a_fp)
{
	scew_element* pstRoot = NULL;
	scew_element* pstSubElement = NULL;
    const char *pszElemName = NULL;
    const char *pszValueOfNameAttr = NULL;
    scew_attribute *pstAttr = NULL;
    const char *pszMetalibName = NULL;

	assert(NULL != a_pstTree);
    assert(NULL != a_fp);

	pstRoot = scew_tree_root(a_pstTree);
    if (!pstRoot)
        return;

    pszElemName = scew_element_name(pstRoot);
    assert(pszElemName);
    pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_NAME);
    /* no TDR_TAG_NAME will be regared as error in other place */
    if (!pstAttr)
        return;

    pszMetalibName = pszValueOfNameAttr = scew_attribute_value(pstAttr);
    assert(pszMetalibName);
    tdr_check_item_attr_i(pstRoot, pszValueOfNameAttr, TDR_ITEM_METALIB, a_fp);

    pstSubElement = scew_element_next(pstRoot, NULL);
    while( NULL != pstSubElement )
    {
        int iItemType = TDR_ITEM_UNKOWN;

        pszElemName = scew_element_name(pstSubElement);
        assert(pszElemName);
        if (0 == tdr_stricmp( pszElemName, TDR_TAG_MACRO))
        {
            iItemType = TDR_ITEM_MACRO;
        }else if (0 == tdr_stricmp( pszElemName, TDR_TAG_MACROSGROUP))
        {
            iItemType = TDR_ITEM_MACROSGROUP;
        }else if (0 == strcmp( pszElemName, TDR_TAG_STRUCT))
        {
            iItemType = TDR_ITEM_STRUCT;
        }else if (0 == strcmp( pszElemName, TDR_TAG_UNION))
        {
            iItemType = TDR_ITEM_UNION;
        }else if (0 == strcmp( pszElemName, TDR_TAG_INCLUDE))
        {
            iItemType = TDR_ITEM_INCLUDE;
        }else
        {
            fprintf(a_fp, "warning:\n <%s name=\"%s\" ...>的子元素\n   <%s ...>无效\n"
                    " 请查阅tdr元数据描述规则关于 %s 的子元素的部分\n", \
                    TDR_TAG_METALIB, pszMetalibName, pszElemName, TDR_TAG_METALIB);
            pstSubElement = scew_element_next( pstRoot, pstSubElement );
            continue;
        }

        /* check attribute of invalid children of root 'metalib' */
        pszValueOfNameAttr  = NULL;
        pstAttr = scew_attribute_by_name(pstSubElement, TDR_TAG_NAME);
        if (pstAttr)
        {
            pszValueOfNameAttr = scew_attribute_value(pstAttr);
        }

        tdr_check_item_attr_i(pstSubElement, pszValueOfNameAttr, iItemType, a_fp);

        pstSubElement = scew_element_next( pstRoot, pstSubElement );
    }
}

// mege tables
static void tdr_check_item_attr_i(const scew_element* a_pstElement,
        const char* a_pszValueOfNameAttr, int a_iItemType, FILE* a_fp)
{
    static const stAttrInfo astMetalibAttrs[] = {
        {TDR_TAG_TAGSET_VERSION, 0},
        {TDR_TAG_VERSION, 0},
        {NULL, 0}
    };
    static const stAttrInfo astIncludeAttrs[] = {
        {TDR_TAG_INCLUDE_FILE, 0},
        {NULL, 0}
    };
    static const stAttrInfo astMacroAttrs[] = {
        {TDR_TAG_MACRO_VALUE, 0},
        {NULL, 0}
    };
    static const stAttrInfo astMacrosgroupAttrs[] = {
        {NULL, 0}
    };
    static const stAttrInfo astStructAttrs[] = {
        {TDR_TAG_SIZE, 0},
        {TDR_TAG_VERSION, 0},
        {TDR_TAG_ALIGN, 0},
        {TDR_TAG_VERSIONINDICATOR, 0},
        {TDR_TAG_SIZEINFO, 0},
        {TDR_TAG_SORTKEY, 0},
        {TDR_TAG_PRIMARY_KEY, 0},
        {TDR_TAG_SPLITTABLEFACTOR, 0},
        {TDR_TAG_SPLITTABLEKEY, 0},
        {TDR_TAG_SPLITTABLERULE, 0},
        {TDR_TAG_DEPENDONSTRUCT, 0},
        {TDR_TAG_UNIQUEENTRYNAME, 0},
        {TDR_TAG_STRICTINPUT, 0},
        {NULL, 0}
    };
    static const stAttrInfo astUnionAttrs[] = {
        {TDR_TAG_ALIGN, 0},
        {TDR_TAG_VERSION, 0},
        {NULL, 0}
    };

    assert(a_pstElement);
    assert(a_iItemType);
    assert(a_fp);

    switch (a_iItemType)
    {
        case TDR_ITEM_METALIB:
            TDR_CHECK_ITEM_ATTRIBUTE(a_pstElement, a_pszValueOfNameAttr,
                    gs_tdr_astCommAttrs, astMetalibAttrs, TDR_TAG_METALIB, TDR_ITEM_METALIB, a_fp);
            break;
        case TDR_ITEM_INCLUDE:
            TDR_CHECK_ITEM_ATTRIBUTE(a_pstElement, a_pszValueOfNameAttr,
                    gs_tdr_astCommAttrs, astIncludeAttrs, TDR_TAG_INCLUDE, TDR_ITEM_INCLUDE, a_fp);
            break;
        case TDR_ITEM_MACRO:
            TDR_CHECK_ITEM_ATTRIBUTE(a_pstElement, a_pszValueOfNameAttr,
                    gs_tdr_astCommAttrs, astMacroAttrs, TDR_TAG_MACRO, TDR_ITEM_MACRO, a_fp);
            break;
        case TDR_ITEM_MACROSGROUP:
            TDR_CHECK_ITEM_ATTRIBUTE(a_pstElement, a_pszValueOfNameAttr,
                    gs_tdr_astCommAttrs, astMacrosgroupAttrs, TDR_TAG_MACROSGROUP, TDR_ITEM_MACROSGROUP, a_fp);
            break;
        case TDR_ITEM_STRUCT:
            TDR_CHECK_ITEM_ATTRIBUTE(a_pstElement, a_pszValueOfNameAttr,
                    gs_tdr_astCommAttrs, astStructAttrs, TDR_TAG_STRUCT, TDR_ITEM_STRUCT, a_fp);
            TDR_CHECK_CHILDREN_ATTR(a_pstElement, a_pszValueOfNameAttr, a_fp, TDR_PARENT_IS_STRUCT);
            break;
        case TDR_ITEM_UNION:
            TDR_CHECK_ITEM_ATTRIBUTE(a_pstElement, a_pszValueOfNameAttr,
                    gs_tdr_astCommAttrs, astUnionAttrs, TDR_TAG_UNION, TDR_ITEM_UNION, a_fp);
            TDR_CHECK_CHILDREN_ATTR(a_pstElement, a_pszValueOfNameAttr, a_fp, TDR_PARENT_IS_UNION);
            break;
        default:
            break;
    }
}

static void tdr_check_entry_attr_i(const scew_element* a_pstElement,
        const char* a_pszValueOfNameAttr, const char* a_pszParentName,
        FILE* a_fp, int a_iFlag)
{
    static const stAttrInfo astEntryAttrs[] = {
        {TDR_TAG_TYPE, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_BIND_MACROSGROUP, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_SIZE, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_COUNT, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_REFER, TDR_PARENT_IS_STRUCT},
        {TDR_TAG_DEFAULT_VALUE, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_SIZEINFO, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_SORTMETHOD, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_IO, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_SELECT, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_CUSTOMATTR, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_UNIQUE, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_NOTNULL, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_AUTOINCREMENT, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_EXTENDTOTABLE, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_COMPRESS, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_MAXID, TDR_PARENT_IS_UNION},
        {TDR_TAG_MINID, TDR_PARENT_IS_UNION},
        {TDR_TAG_VERSION, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {TDR_TAG_STRING_COMPRESS, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION},
        {NULL, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION}
    };
    int iFound = 0;
    scew_attribute* attribute = NULL;
    const char* pszAttrName = NULL;
    const char* pszAttrValue = NULL;

    assert(a_pstElement);
    assert(a_pszValueOfNameAttr);
    //assert(a_pszParentName);
    assert(a_fp);

    while ((attribute = scew_attribute_next(a_pstElement, attribute)) != NULL)
    {
        pszAttrName = scew_attribute_name(attribute);
        assert(pszAttrName);
        pszAttrValue = scew_attribute_value(attribute);
        assert(pszAttrValue);
        TDR_SEARCH_ENTRY_ATTRIBUTE(iFound, pszAttrName, gs_tdr_astCommAttrs, astEntryAttrs, a_iFlag);
        if (!iFound)
        {
            char* pszName = "";
            if (a_iFlag & TDR_PARENT_IS_STRUCT)
                pszName = "struct";
            else if (a_iFlag & TDR_PARENT_IS_UNION)
                pszName = "union";
            fprintf(a_fp, "warning:\n <%s ... name=\"%s\" ...>的子元素\n"
                    "   <%s name=\"%s\" ... %s=\"%s\" ...>\n 属性 %s 无效，"
                    " 请查阅tdr元数据描述规则关于 %s/%s 的属性的部分\n",
                    pszName, a_pszParentName, TDR_TAG_ENTRY, a_pszValueOfNameAttr,
                    pszAttrName, pszAttrValue, pszAttrName, pszName, TDR_TAG_ENTRY);
        }
    }
}

static void tdr_check_index_attr_i(const scew_element* a_pstElement,
        const char* a_pszValueOfNameAttr, const char* a_pszParentName,
        FILE* a_fp, int a_iFlag)
{
    static const stAttrInfo astIndexAttrs[] = {
        {TDR_TAG_INDEX_COLUMN, TDR_PARENT_IS_STRUCT},
        {TDR_TAG_VERSION, TDR_PARENT_IS_STRUCT},
        {NULL, TDR_PARENT_IS_STRUCT | TDR_PARENT_IS_UNION}
    };
    int iFound = 0;
    scew_attribute* attribute = NULL;
    const char* pszAttrName = NULL;
    const char* pszAttrValue = NULL;

    assert(a_pstElement);
    assert(a_pszValueOfNameAttr);
    //assert(a_pszParentName);
    assert(a_fp);

    while ((attribute = scew_attribute_next(a_pstElement, attribute)) != NULL)
    {
        pszAttrName = scew_attribute_name(attribute);
        assert(pszAttrName);
        pszAttrValue = scew_attribute_value(attribute);
        assert(pszAttrValue);
        TDR_SEARCH_ENTRY_ATTRIBUTE(iFound, pszAttrName, gs_tdr_astCommAttrs, astIndexAttrs, a_iFlag);
        if (!iFound)
        {
            char* pszName = "";
            if (a_iFlag & TDR_PARENT_IS_STRUCT)
                pszName = "struct";
            else if (a_iFlag & TDR_PARENT_IS_UNION)
                pszName = "union";
            fprintf(a_fp, "warning:\n <%s ... name=\"%s\" ...>的子元素\n"
                    "   <%s name=\"%s\" ... %s=\"%s\" ...>\n 属性 %s 无效，"
                    " 请查阅tdr元数据描述规则关于 %s/%s 的属性的部分\n",
                    pszName, a_pszParentName, TDR_TAG_INDEX, a_pszValueOfNameAttr,
                    pszAttrName, pszAttrValue, pszAttrName, pszName, TDR_TAG_INDEX);
        }
    }
}
