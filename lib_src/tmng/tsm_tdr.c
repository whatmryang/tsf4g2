//============================================================================
// @Id:       $Id: tsm_tdr.c 16264 2011-01-11 02:04:40Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2011-01-11 10:04:40 +0800 #$
// @Revision: $Revision: 16264 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdlib.h>

#include "tsm_tdr.h"
#include "tsm_tdr_i.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
#define TT_R    1
#define TT_W    0

#define TTH     TsmTdrHandle *
#define TTP     TsmObjPtr *
#define TTR     TsmTdrResult *
#define TTL     TsmTdrRow *

#define TTMI(N, L, D, A) \
    int tsm_tdr_##N D \
    { \
        TTH p = P; \
        int i_r; \
        \
        tassert(p, NULL, 1, "¾ä±úÎª¿Õ"); \
        tassert1(0 == tsm_tdr_lock(p, (L)), \
                 NULL, 2, "ËøÊ§°Ü(%d)", (L)); \
        i_r = tsm_tdr_##N##_i A; \
        tsm_tdr_unlock(p); \
        return i_r; \
    } \

#define TTMP(N, T, L, D, A) \
    T *tsm_tdr_##N D \
    { \
        TTH p = P; \
        T *i_r = NULL; \
        \
        tassert_r(NULL, p, NULL, 1, "¾ä±úÎª¿Õ"); \
        tassert_r1(NULL, 0 == tsm_tdr_lock(p, (L)), \
                   NULL, 1, "ËøÊ§°Ü(%d)", (L)); \
        i_r = tsm_tdr_##N##_i A; \
        tsm_tdr_unlock(p); \
        return i_r; \
    }

#define TTMN(N, T, D, A) \
    T tsm_tdr_##N D \
    { \
        TTH p = P; \
        return tsm_tdr_##N##_i A; \
    }

#define TTM(N, T, D, A) \
    T tsm_tdr_##N D \
    { \
        return tsm_tdr_##N##_i A; \
    }

TTM (base_set,       int,          (int k, int m, int d),      (k, 0, m, 0, d))
TTM (open,           int,          (void **p),                 ((TTH *)p))
TTMN(init,           int,          (void *P),                  (p)) 
TTM (close,          int,          (void **p),                 ((TTH *)p))
TTMN(destroy,        int,          (void *P),                  (p)) 

TTMI(bind_data,      TT_R,         (void *P, const char *uri), (p, uri))
TTMI(bind_meta,      TT_R,         (void *P, const char *uri), (p, uri))
TTMN(bind_size,      int,          (void *P, size_t s),        (p, s))

TTMN(bind_get_data,  const char *, (void *P),                  (p)) 
TTMN(bind_get_meta,  const char *, (void *P),                  (p)) 
TTMN(bind_get_size,  size_t,       (void *P),                  (p)) 

TTMI(load_data,      TT_W,         (void *P, void *d,  int f), (p, d, f))
TTMI(load_meta,      TT_W,   (void *P, LPTDRMETALIB l, int b), (p, l, b))
TTMI(load_meta_file, TT_W,   (void *P, const char *f,  int b), (p, f, b))

TTMP(ptr,            void, TT_W,   (void *P, int f),           (p, f))
TTM (at,             void *,       (void *p, int f),           ((TTP)p, f))
TTM (dt,             int,          (void *p),                  (p))

TTMI(query,          TT_R,         (void *P, const char *uri), (p, uri))
TTMN(store_result,   void *,       (void *P),      (p)) 

TTM (num_rows,       int,          (void *p),                  ((TTR)p))
TTM (data_seek,      int,          (void *p, int i),           ((TTR)p, i))
TTM (free_result,    int,          (void *p),                  ((TTR)p))

TTM (fetch_row,      void *,       (void *p),                  ((TTR)p))

TTMI(row_unlink,     TT_W,         (void *r, void *P),         ((TTL)r, p))
TTM (row_type,       int,          (void *p),                  ((TTL)p))
TTM (row_is_data,    int,          (void *p),                  ((TTL)p))
TTM (row_is_meta,    int,          (void *p),                  ((TTL)p))
TTM (row_meta_uri,   const char *, (void *p),                  ((TTL)p))
TTM (row_uri,        const char *, (void *p),                  ((TTL)p))
TTM (row_meta_ptr,   void *,       (void *p),                  ((TTL)p))
TTM (row_data_ptr,   void *,       (void *p),                  ((TTL)p))
TTM (row_meta,       LPTDRMETA,    (void *p),                  ((TTL)p))
TTM (row_data,       void *,       (void *p, int f),           ((TTL)p, f))
TTM (row_size,       size_t,       (void *p),                  ((TTL)p))
TTMN(row_only,       void *,       (void *P),                  (p))
TTMN(row_first,      void *,       (void *P),                  (p))

#undef TTM
#undef TTMN
#undef TTMP
#undef TTMI

#undef TTL
#undef TTR
#undef TTP
#undef TTH

#undef TT_W
#undef TT_R


//----------------------------------------------------------------------------
// THE END
//============================================================================
