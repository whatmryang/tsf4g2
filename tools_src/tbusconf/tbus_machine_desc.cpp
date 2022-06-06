#include <map>
#include <string>
#include <fstream>

#include "tbus_tools.h"
#include "tbus_macros.h"
#include "tbusd_desc.h"
#include "tbus_machine_desc.h"

#define TBUS_GCIM_META_NAME  "TbusGCIM"
#define TBUS_RELAY_META_NAME "RelayMnger"
#define TBUS_TBUSD_META_NAME "Tbusd"

using std::set;
using std::map;
using std::endl;
using std::string;
using std::ofstream;

int MachineDesc::outputGCIM()
{
    int iRet = 0;
    const char* pszOutPath = TbusTools::getOutPath(outputDir.c_str(), "tbusmgr.xml");
    if (0 == stGCIM.dwCount)
    {
        iRet = TbusTools::deleteFile(pszOutPath);
        if (0 != iRet)
        {
            fprintf(stderr, "Error: no need to have tbusmgr.xml, but failed to remove %s\n", pszOutPath);
            return -1;
        }

        return 0;
    }

    extern unsigned char g_szMetalib_TbusConf[];
    LPTDRMETA pstMeta = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_TbusConf, TBUS_GCIM_META_NAME);
    if (NULL == pstMeta)
    {
        fprintf(stderr, "Error: failed to get meta '%s'\n", TBUS_GCIM_META_NAME);
        return -1;
    }

    FILE* fpOut = fopen(pszOutPath, "wb");
    if (NULL == fpOut)
    {
        fprintf(stderr, "Error: failed to open file '%s' for write\n", pszOutPath);
        return -1;
    } else
    {
        iRet = 0;
        {
            const char* pszXmlHead = "<?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\" ?>\n\n";
            fwrite(pszXmlHead, strlen(pszXmlHead), 1, fpOut);
        }
        TDRDATA stHost;
        stHost.pszBuff = (char*)&stGCIM;
        stHost.iBuff   = sizeof(stGCIM);
        int iTdrRet = tdr_output_fp(pstMeta, fpOut, &stHost, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
        if (TDR_ERR_IS_ERROR(iTdrRet))
        {
            fprintf(stderr, "Error: tdr_output_fp with meta '%s' failed, for %s\n",
                    TBUS_GCIM_META_NAME, tdr_error_string(iTdrRet));
            iRet = -1;
        }

        fclose(fpOut);
        return iRet;
    }
}

int MachineDesc::outputAddrList(std::set<std::string>& processWithTbus)
{
    const char* pszOutPath = TbusTools::getOutPath(outputDir.c_str(), "process_id.txt");
    if (NULL == pszOutPath)
    {
        return -1;
    }

    ofstream outfile(pszOutPath);
    if (outfile.fail())
    {
        fprintf(stderr, "Error: failed to open file '%s' for write\n", pszOutPath);
        return -1;
    }

    map<string, string>::const_iterator iter;
    for (iter = processName2Id.begin(); iter != processName2Id.end(); iter++)
    {
        if (processWithTbus.end() != processWithTbus.find(iter->first))
        {
            outfile << iter->first << "\t--id=" << iter->second;
            outfile << "\t--use-bus\t--bus-key=" << this->stGCIM.szGCIMShmKey;
            outfile << endl;
        }
    }

    set<string>::const_iterator setIter;
    for (setIter = processWithTbus.begin(); setIter != processWithTbus.end(); setIter++)
    {
        processName2Id.erase(*setIter);
    }

    for (iter = processName2Id.begin(); iter != processName2Id.end(); iter++)
    {
        outfile << iter->first << "\t--id=" << iter->second << endl;
    }
    outfile.close();

    return 0;
}

int MachineDesc::output(std::set<std::string>& processWithTbus)
{
    int iRet = outputAddrList(processWithTbus);
    if (0 != iRet)
    {
        return iRet;
    }

    iRet = outputGCIM();
    if (0 != iRet)
    {
        return iRet;
    }

    if (0 < listenAddr.length())
    {
        iRet = outputTbusdConf();
        if (0 != iRet)
        {
            return iRet;
        }

        iRet = outputRelay();
        if (0 != iRet)
        {
            return iRet;
        }
    }

    return 0;
}

int MachineDesc::outputTbusdConf()
{
    extern unsigned char g_szMetalib_tbusd[];
    LPTDRMETA pstMeta = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_tbusd, TBUS_TBUSD_META_NAME);
    if (NULL == pstMeta)
    {
        fprintf(stderr, "Error: failed to get meta '%s'\n", TBUS_TBUSD_META_NAME);
        return -1;
    }

    TBUSD stTbusd;
    TDRDATA stHost;
    stHost.pszBuff = (char*)&stTbusd;
    stHost.iBuff   = sizeof(stTbusd);

    int iRet = tdr_init(pstMeta, &stHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        fprintf(stderr, "Error: tdr_init with meta '%s' failed, for %s\n",
                TBUS_TBUSD_META_NAME, tdr_error_string(iRet));
        return -1;
    }

    strncpy(stTbusd.szGCIMShmKey, stGCIM.szGCIMShmKey, sizeof(stTbusd.szGCIMShmKey));
    strncpy(stTbusd.szRelayShmKey, stRelay.szRelayShmKey, sizeof(stTbusd.szRelayShmKey));
    strncpy(stTbusd.szListen, listenAddr.c_str(), sizeof(stTbusd.szListen));

    const char* pszOutPath = TbusTools::getOutPath(outputDir.c_str(), "tbusd.xml");
    FILE* fpOut = fopen(pszOutPath, "wb");
    if (NULL == fpOut)
    {
        fprintf(stderr, "Error: failed to open file '%s' for write\n", pszOutPath);
        return -1;
    } else
    {
        iRet = 0;
        {
            const char* pszXmlHead = "<?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\" ?>\n\n";
            fwrite(pszXmlHead, strlen(pszXmlHead), 1, fpOut);
        }
        stHost.pszBuff = (char*)&stTbusd;
        stHost.iBuff   = sizeof(stTbusd);
        int iTdrRet = tdr_output_fp(pstMeta, fpOut, &stHost, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
        if (TDR_ERR_IS_ERROR(iTdrRet))
        {
            fprintf(stderr, "Error: tdr_output_fp with meta '%s' failed, for %s\n",
                    TBUS_TBUSD_META_NAME, tdr_error_string(iTdrRet));
            iRet = -1;
        }

        fclose(fpOut);
        return iRet;
    }
}

int MachineDesc::outputRelay()
{
    int iRet = 0;
    const char* pszOutPath = TbusTools::getOutPath(outputDir.c_str(), "trelaymgr.xml");
    if (0 == stRelay.dwUsedCnt)
    {
        iRet = TbusTools::deleteFile(pszOutPath);
        if (0 != iRet)
        {
            fprintf(stderr, "Error: no need to have trelaymgr.xml, but failed to remove %s\n", pszOutPath);
            return -1;
        }

        return 0;
    }

    extern unsigned char g_szMetalib_TbusConf[];
    LPTDRMETA pstMeta = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_TbusConf, TBUS_RELAY_META_NAME);
    if (NULL == pstMeta)
    {
        fprintf(stderr, "Error: failed to get meta '%s'\n", TBUS_RELAY_META_NAME);
        return -1;
    }

    FILE* fpOut = fopen(pszOutPath, "wb");
    if (NULL == fpOut)
    {
        fprintf(stderr, "Error: failed to open file '%s' for write\n", pszOutPath);
        return -1;
    } else
    {
        iRet = 0;
        {
            const char* pszXmlHead = "<?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\" ?>\n\n";
            fwrite(pszXmlHead, strlen(pszXmlHead), 1, fpOut);
        }
        TDRDATA stHost;
        stHost.pszBuff = (char*)&stRelay;
        stHost.iBuff   = sizeof(stRelay);
        int iTdrRet = tdr_output_fp(pstMeta, fpOut, &stHost, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
        if (TDR_ERR_IS_ERROR(iTdrRet))
        {
            fprintf(stderr, "Error: tdr_output_fp with meta '%s' failed, for %s\n",
                    TBUS_RELAY_META_NAME, tdr_error_string(iTdrRet));
            iRet = -1;
        }
        fclose(fpOut);

        return iRet;
    }
}

int MachineDesc::initGCIM()
{
    extern unsigned char g_szMetalib_TbusConf[];
    LPTDRMETA pstMeta = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_TbusConf, TBUS_GCIM_META_NAME);
    if (NULL == pstMeta)
    {
        fprintf(stderr, "Error: failed to get meta '%s'\n", TBUS_GCIM_META_NAME);
        return -1;
    }

    TDRDATA stHost;
    stHost.pszBuff = (char*)&stGCIM;
    stHost.iBuff   = sizeof(stGCIM);

    int iRet = tdr_init(pstMeta, &stHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        fprintf(stderr, "Error: tdr_init with meta '%s' failed, for %s\n",
                TBUS_GCIM_META_NAME, tdr_error_string(iRet));
        return -1;
    }

    return 0;
}

int MachineDesc::initRelay()
{
    extern unsigned char g_szMetalib_TbusConf[];
    LPTDRMETA pstMeta = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_TbusConf, TBUS_RELAY_META_NAME);
    if (NULL == pstMeta)
    {
        fprintf(stderr, "Error: failed to get meta '%s'\n", TBUS_RELAY_META_NAME);
        return -1;
    }

    TDRDATA stHost;
    stHost.pszBuff = (char*)&stRelay;
    stHost.iBuff   = sizeof(stRelay);

    int iRet = tdr_init(pstMeta, &stHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        fprintf(stderr, "Error: tdr_init with meta '%s' failed, for %s\n",
                TBUS_RELAY_META_NAME, tdr_error_string(iRet));
        return -1;
    }

    return 0;
}
