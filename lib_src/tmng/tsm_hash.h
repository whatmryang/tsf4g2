#ifndef _TSM_HASH_H_
#define _TSM_HASH_H_
//============================================================================
// @Id:       $Id: tsm_hash.h 13779 2010-09-26 02:27:10Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-09-26 10:27:10 +0800 #$
// @Revision: $Revision: 13779 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include "tsm_index.h"
#include "tsm_uri_i.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
#define tsm_hash_find(I, D, S) \
    toffset_hash_find((I), MOFFSET(stHash, TsmIndex), (D), (S))

#define tsm_hash_next(I, N) \
    toffset_hash_next((I), MOFFSET(stHash, TsmIndex), (N))


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_hash_set(TsmIndex *a_pstIndex,
                 TsmUri   *a_pstData,
                 TsmUri   *a_pstMeta,
                 int       a_iRow);

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_hash_get(TsmIndex *a_pstIndex,
                 TsmUri   *a_pstUri,
                 int       a_pArray[],
                 int       a_iSize);

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_hash_del(TsmIndex *a_pstIndex,
                 int       a_iType,
                 int       a_iRow);
 
//----------------------------------------------------------------------------
// THE END
//============================================================================
#endif
