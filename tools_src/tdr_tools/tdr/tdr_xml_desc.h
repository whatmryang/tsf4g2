#ifndef _TDR_XML_DESC_H_
#define _TDR_XML_DESC_H_


#include <list>
#include <string>

#include "tdr/tdr.h"
#include "scew/scew.h"

using std::list;
using std::string;


class XMLDesc
{
    public:
        string headFileName;
        string cppFileName;
        string csFileName;
        string nameSpace;      /* 记录 metalib 的 name 属性 */
        list<string> includes; /* 记录 include 的 file 属性值对应的 .h 文件 */
        list<string> macros;   /* 记录 macro 的 name 属性 */
        list<string> groups;   /* 记录 macrosgroup 的 name 属性 */
        list<string> metas;    /* 记录 meta 的 name 属性 */

    public:
        static void initXMLDesc(list<XMLDesc>& xmlDescList, char** paszFiles, int iFileNum);

        void init(const char* pszFile);
        void initNameSpace(const char* pszFile, scew_element* pstRoot);
        void initIncludes(const char* pszFile, scew_element* pstRoot);
        void initMacros(const char* pszFile, scew_element* pstRoot);
        void initGroups(const char* pszFile, scew_element* pstRoot);
        void initMetas(const char* pszFile, scew_element* pstRoot);
};

#endif
