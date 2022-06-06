#include <string>
#include <cstring>
#include <cassert>

#include "pal/pal.h"
#include "tbus_tools.h"

#define TBUS_DESC_SINGLE_META "single_machine"
#define TBUS_DESC_MULTI_MDETA "multi_machines"

using std::string;

const char *
TbusTools::tbus_basename(const char *a_pszObj)
{
	const char *pch;

	pch = strrchr(a_pszObj, OS_DIRSEP);
	if (NULL == pch)
	{
		return (char *)a_pszObj;
	}
	pch ++;

	return pch;
}

string TbusTools::strOutPath;

const char*
TbusTools::getOutPath()
{
    if (0 < strOutPath.length())
    {
        return strOutPath.c_str();
    } else
    {
        return NULL;
    }
}

int TbusTools::checkDir(std::string& dir)
{
    if (0 == dir.length())
    {
        return -1;
    }

    int iRet = checkDir(dir.c_str());
    if (0 != iRet)
    {
        return -1;
    }

    if (OS_DIRSEP != dir[dir.length()-1])
    {
        dir += OS_DIRSEP;
    }

    return 0;
}

const char*
TbusTools::getOutPath(const char* pszOutDir, const char* pszFile)
{
    assert(NULL != pszFile);

    if (NULL != pszOutDir && 0 < strlen(pszOutDir))
    {
        if (0 < strlen(pszOutDir))
        {
            int iRet = checkDir(pszOutDir);
            if (0 != iRet)
            {
                strOutPath.clear();
                return getOutPath();
            }
        }

        strOutPath = pszOutDir;

        if (OS_DIRSEP != pszOutDir[strlen(pszOutDir)-1])
        {
            strOutPath += OS_DIRSEP;
        }
        strOutPath += pszFile;
    } else
    {
        strOutPath = pszFile;
    }

    return getOutPath();
}

int TbusTools::checkDir(const char* pszDir)
{
    assert(NULL != pszDir);

    if (0 < strlen(pszDir))
    {
        if (0 != tfexist(pszDir))
        {
            int iRet = tfmkdir(pszDir);
            if (0 != iRet)
            {
                fprintf(stderr, "Error: failed to create direcotry '%s'\n", pszDir);
                return -1;
            }
        }
    }

    return 0;
}

int TbusTools::deleteFile(const char* pszFile)
{
    if (0 == tfexist(pszFile))
    {
        return tfdelete(pszFile);
    }
    return 0;
}

LPTDRMETA TbusTools::getSingleMeta()
{
    return getMetaByName(TBUS_DESC_SINGLE_META);
}

LPTDRMETA TbusTools::getMultiMeta()
{
    return getMetaByName(TBUS_DESC_MULTI_MDETA);
}

LPTDRMETA TbusTools::getMetaByName(const char* pszMetaName)
{
    assert(NULL != pszMetaName);

    extern unsigned char g_szMetalib_tbus_conf[];
    LPTDRMETA pstMeta = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_tbus_conf, pszMetaName);
    if (NULL == pstMeta)
    {
        fprintf(stderr, "Error: failed to get meta '%s'\n", pszMetaName);
    }

    return pstMeta;
}

int TbusTools::checkIP(const char* pszIP)
{
    struct sockaddr_in stIn;

	if (!tnet_is_number(pszIP) || 0 == inet_aton(pszIP, &stIn.sin_addr))
	{
        return -1;
    }

    return 0;
}
