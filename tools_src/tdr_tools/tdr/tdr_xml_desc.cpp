#include <cassert>
#include <iostream>

#include "../tdr_tools.h"

#include "../lib_src/tdr/tdr_os.h"
#include "tdr/tdr_XMLMetaLib.h"
#include "tdr_xml_desc.h"

#define TDR_TAIL_CMP(a_iResult, a_pszLong, a_pszShort)\
{\
    if (strlen(a_pszShort) > strlen(a_pszLong))\
        a_iResult = 1;\
    a_iResult = strcmp(a_pszLong + strlen(a_pszLong) - strlen(a_pszShort), a_pszShort);\
}

using std::cout;
using std::endl;

extern "C" {
    int tdr_create_XMLParser_tree_byFileName(scew_tree **a_ppstTree, const char *a_pszFileName, FILE *a_fpError);
}

void
XMLDesc::initXMLDesc(list<XMLDesc>& xmlDescList, char** paszFiles, int iFileNum)
{
    if (NULL == paszFiles)
    {
        throw string("paszFiles to XMLDesc::init is null");
    }


    for (int i = 0; i < iFileNum; i++)
    {
        if (NULL == paszFiles[i])
        {
            throw string("paszFiles[" + int2str(i) + "] to XMLDesc::init is null");
        }

        xmlDescList.push_back(XMLDesc());
        XMLDesc& desc = xmlDescList.back();

        desc.headFileName = desc.cppFileName = desc.csFileName = tdr_basename(paszFiles[i]);
        tdr_add_suffix(desc.headFileName, "h");
        tdr_add_suffix(desc.cppFileName, "cpp");
        tdr_add_suffix(desc.csFileName, "cs");

        desc.init(paszFiles[i]);
    }
}

void
XMLDesc::init(const char* pszFile)
{
    assert(NULL != pszFile);

    int iRet = TDR_SUCCESS;
    scew_tree* pstTree = NULL;

    iRet =	tdr_create_XMLParser_tree_byFileName(&pstTree, pszFile, stderr);
    if( TDR_ERR_IS_ERROR(iRet))
    {
        throw string("tdr_create_XMLParser_tree_byFileName in XMLDesc::initXMLDesc failed");
    }

    scew_element *pstRoot = scew_tree_root(pstTree);
    if (NULL == pstRoot)
    {
        throw string("scew_tree_root return a null pstRoot");
    }

    initNameSpace(pszFile, pstRoot);
    initIncludes(pszFile, pstRoot);
    initMacros(pszFile, pstRoot);
    initGroups(pszFile, pstRoot);
    initMetas(pszFile, pstRoot);

    scew_tree_free(pstTree);
}


void
XMLDesc::initNameSpace(const char* pszFile, scew_element* pstRoot)
{
    assert(NULL != pszFile);
    assert(NULL != pstRoot);

    scew_attribute* pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_NAME);
    if (!pstAttr)
    {
        throw string(pszFile) + "'s root element SHOULD have attribute 'name=...'";
    }

    this->nameSpace = scew_attribute_value(pstAttr);
}

void
XMLDesc::initIncludes(const char* pszFile, scew_element* pstRoot)
{
    assert(NULL != pszFile);
    assert(NULL != pstRoot);

	scew_element *pstSubItem = NULL;

    while (NULL != (pstSubItem = scew_element_next(pstRoot, pstSubItem)))
    {
        int iResult;

        const char* pszElemName = scew_element_name(pstSubItem);
        if (!pszElemName || strcmp(pszElemName, TDR_TAG_INCLUDE))
        {
            continue;
        }

        scew_attribute* pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_INCLUDE_FILE);
        if (!pstAttr)
        {
            continue;
        }

        const char* pszIncludeFile = scew_attribute_value(pstAttr);
        if (!pszIncludeFile)
        {
            continue;
        }
        TDR_TAIL_CMP(iResult, pszFile, pszIncludeFile);
        if (!iResult)
        {
            continue;
        }

        string fileName(tdr_basename((pszIncludeFile)));
        tdr_add_suffix(fileName, "h");
        includes.push_back(fileName);
    }
}

void
XMLDesc::initMacros(const char* pszFile, scew_element* pstRoot)
{
    assert(NULL != pszFile);
    assert(NULL != pstRoot);

	scew_element *pstSubItem = NULL;
    while (NULL != (pstSubItem = scew_element_next(pstRoot, pstSubItem)))
    {
		if (0 != tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_MACRO))
		{
            continue;
        }

        scew_attribute* pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
        if (!pstAttr)
        {
            continue;
        }

        const char* pszMacroName = scew_attribute_value(pstAttr);
        if (!pszMacroName)
        {
            continue;
        }

        macros.push_back(pszMacroName);
    }
}

void
XMLDesc::initGroups(const char* pszFile, scew_element* pstRoot)
{
    assert(NULL != pszFile);
    assert(NULL != pstRoot);

	scew_element *pstSubItem = NULL;
    while (NULL != (pstSubItem = scew_element_next(pstRoot, pstSubItem)))
    {
		if (0 != tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_MACROSGROUP))
		{
            continue;
        }

        scew_attribute* pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
        if (!pstAttr)
        {
            continue;
        }

        const char* pszGroupName = scew_attribute_value(pstAttr);
        if (!pszGroupName)
        {
            continue;
        }

        groups.push_back(pszGroupName);
    }

}

void
XMLDesc::initMetas(const char* pszFile, scew_element* pstRoot)
{
    assert(NULL != pszFile);
    assert(NULL != pstRoot);

	scew_element *pstSubItem = NULL;
    while (NULL != (pstSubItem = scew_element_next(pstRoot, pstSubItem)))
    {
        if (0 != tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_STRUCT)
            && 0 != tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_UNION))
		{
            continue;
        }

        scew_attribute* pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
        if (!pstAttr)
        {
            continue;
        }

        const char* pszGroupName = scew_attribute_value(pstAttr);
        if (!pszGroupName)
        {
            continue;
        }

        metas.push_back(pszGroupName);
    }

}
