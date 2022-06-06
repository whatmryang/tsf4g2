#!/usr/bin/env python
# -*- coding: utf-8 -*-
##============================================================================
## @Id:       $Id: tsm_handle.py 13779 2010-09-26 02:27:10Z tomaswang $
## @Author:   $Author: tomaswang $
## @Date:     $Date: 2010-09-26 10:27:10 +0800 (星期日, 2010-09-26) $
## @Revision: $Revision: 13779 $
## @HeadURL:  $HeadURL: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk/lib_src/tmng/tsm_handle.py $
##----------------------------------------------------------------------------

from tpy_c import tpy_c

if __name__ == "__main__":
    obj = tpy_c('tsm_handle')
    obj.add_head("tsm_handle.h")
    obj.add_head("<errno.h>")
    obj.add_head("tdr/tdr_metalib_init.h")
    obj.add_head("err_stack.h")
    obj.add_head("tsm_handle.h")
    obj.add_head("tsm_uri_i.h")
    obj.add_head("tsm_hash.h")
    obj.add_head("tsm_ptr_uri.h")
    obj.add_func("",
                 "int",
                 "tsm_open",
                 "TmngHandle *a_pHandle")
    obj.add_func("",
                 "int",
                 "tsm_close",
                 "TmngHandle *a_pHandle")
    obj.add_func("",
                 "int",
                 "tsm_status",
                 "TmngHandle *a_pHandle")
    obj.add_func("",
                 "int",
                 "tsm_lib_add",
                 "TmngHandle *a_pHandle",
                 "struct tagTDRMetaLib *a_pstLib",
                 "int a_iBid")
    obj.add_func("",
                 "TmngDataPtr *",
                 "tsm_lib_get",
                 "TmngHandle *a_pHandle",
                 "const char *a_pszMetaUri")
    obj.add_func("",
                 "int",
                 "tsm_data_add",
                 "TmngHandle *a_pHandle",
                 "void       *a_pData",
                 "size_t      a_iSize",
                 "const char *a_pszDataUri",
                 "const char *a_pszMetaUri")
    obj.add_func("",
                 "TmngDataPtr *",
                 "tsm_data_get",
                 "TmngHandle *a_pHandle",
                 "const char *a_pszDataUri")
    obj.set_option("libtsm.a ../tdr/libtdr.a ../pal/libpal.a -I ../../include")
    obj.compile()


##----------------------------------------------------------------------------
## THE END
##============================================================================
