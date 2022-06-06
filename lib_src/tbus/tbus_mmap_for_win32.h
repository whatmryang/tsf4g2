#if defined(_WIN32) || defined(_WIN64)
#include "tbus_kernel.h"

#define tbus_open_mmap_by_key(ppShm, a_tKey, a_piSize, a_iFlag, a_piCreate, a_piMMapID)\
    tbus_open_mmap_i(ppShm, a_tKey, 0, 0, 1, a_piSize, a_iFlag, a_piCreate, a_piMMapID)

#define tbus_open_mmap_by_key_and_addr(ppShm, a_tKey, a_uAddr1, a_uAddr2, a_piSize, a_iFlag, a_piCreate, a_piMMapID)\
    tbus_open_mmap_i(ppShm, a_tKey, a_uAddr1, a_uAddr2, 0, a_piSize, a_iFlag, a_piCreate, a_piMMapID)

int tbus_open_mmap_i(INOUT void **ppShm, IN key_t a_tKey, IN unsigned a_uAddr1, IN unsigned a_uAddr2,
        IN int a_iUseOnlyKey, IN unsigned int *a_piSize, IN int a_iFlag, OUT int *a_piCreate, OUT HANDLE* a_piMMapID);



/* generate source file info */
#define tbus_key_to_path(a_tKey, a_pszFilePathBuf, a_iBufLen)\
    tbus_generate_path(a_tKey, 0, 0, 1, a_pszFilePathBuf, a_iBufLen)

#define tbus_key_and_addr_to_path(a_tKey, a_uAddr1, a_uAddr2, a_pszFilePathBuf, a_iBufLen)\
    tbus_generate_path(a_tKey, a_uAddr1, a_uAddr2, 0, a_pszFilePathBuf, a_iBufLen)

int tbus_generate_path(IN key_t a_tKey, IN unsigned a_uAddr1,
        IN unsigned a_uAddr2, IN int a_iOnlyUseKey, INOUT char* a_pszFilePathBuf, IN size_t a_iBufLen);


/* open mmap by source file */
int tbus_open_mmap_by_source_file_i(INOUT void **ppShm, IN const char* a_pszFile,
        IN unsigned int *a_piSize, IN int a_iFlag, OUT int *a_piCreate, OUT HANDLE *a_piMMapID);


void tbus_fini_security_attr(void);
int tbus_delete_channel_file_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead);
#endif
