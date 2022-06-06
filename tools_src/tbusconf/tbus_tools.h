#ifndef _TBUS_CONF_TOOLES_H_
#define _TBUS_CONF_TOOLES_H_

#if defined(_WIN32) || defined(_WIN64)
#define	OS_DIRSEP '\\'
#else
#define OS_DIRSEP '/'
#endif

#include <string>
#include "tdr/tdr.h"
#include "pal/pal.h"

class TbusTools
{
    public:
        static const char* getOutPath();
        static int checkDir(std::string& dir);
        static int deleteFile(const char* pszFile);
        static const char *tbus_basename(const char *a_pszObj);
        static const char* getOutPath(const char* pszOutDir, const char* pszFile);

        static int checkIP(const char* pszIp);

        static LPTDRMETA getSingleMeta();
        static LPTDRMETA getMultiMeta();

    private:
        static int checkDir(const char* pszDir);
        static std::string strOutPath;
        static LPTDRMETA getMetaByName(const char* pszMetaName);
};

#endif
