/*
**  @file $RCSfile: ut_ttime.h,v $
**  general description of this module
**  $Id: ut_ttime.h,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008-05-28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef UT_TTIME_H
#define UT_TTIME_H

#ifdef __cplusplus
extern "C"
{
#endif

void ut_itimer(void);
void ut_sleep(void);
void ut_usleep(void);
void ut_nanosleep(void);
void ut_utimes(void);

#ifdef __cplusplus
}
#endif

#endif /* UT_TTIME_H */
