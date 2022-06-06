/*
 **  @file $RCSfile: tlog_layout.c,v $
 **  general description of this module
 **  $Id: tlog_layout.c,v 1.9 2009-03-27 06:17:02 kent Exp $
 **  @author $Author: kent $
 **  @date $Date: 2009-03-27 06:17:02 $
 **  @version $Revision: 1.9 $
 **  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
 **  @note Platform: Linux
 */

#include "pal/pal.h"
#include "tlog_priority.h"
#include "tlog_layout.h"
#include "tlog_event_i.h"

void tlog_layout_format(TLOGEVENTBASE* a_pstEvt, const char* a_pszFmt)
{
    const char* pszFmt;
    char *pszBuffer;
    size_t uiBufferSize;
    size_t uiBuffOff = 0;
    size_t iSubLen = 0;

    a_pstEvt->evt_msg_prefix_len = 0;
    a_pstEvt->evt_msg_sufix_len = 0;
    if (!a_pszFmt || '\0' == a_pszFmt[0])
    {
        return;
    }

    pszFmt = a_pszFmt;

    uiBufferSize = sizeof(a_pstEvt->evt_msg_prefix_buffer);
    pszBuffer = a_pstEvt->evt_msg_prefix_buffer;
    while (*pszFmt && uiBuffOff < uiBufferSize)
    {
        if ('%' != *pszFmt)
        {
            pszBuffer[uiBuffOff++] = *pszFmt++;
            continue;
        }

        switch (*(++pszFmt))
        {
        case 'c':
        {
            iSubLen = snprintf(pszBuffer + uiBuffOff, uiBufferSize - uiBuffOff,
                    "%s", a_pstEvt->evt_category);

            if (iSubLen > 0 && iSubLen <= (uiBufferSize - uiBuffOff))
            {
                uiBuffOff += iSubLen;
            }
            break;
        }
        case 'd':
        {
            struct tm tm;

#if defined (_WIN32) || defined (_WIN64)
#pragma warning (disable: 4133)
#endif


            time_t tv_sec = (time_t)a_pstEvt->evt_timestamp.tv_sec;
            localtime_r(&tv_sec, &tm);

            iSubLen = snprintf(pszBuffer + uiBuffOff, uiBufferSize - uiBuffOff,
                    "%04d%02d%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon
                            + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            if (iSubLen > 0 && iSubLen <= (uiBufferSize - uiBuffOff))
            {
                uiBuffOff += iSubLen;
            }
            break;
        }

        case 'u':
        {
            iSubLen = snprintf(pszBuffer + uiBuffOff, uiBufferSize - uiBuffOff,
                    "%06d", (int) a_pstEvt->evt_timestamp.tv_usec);
            if (iSubLen > 0 && iSubLen <= (uiBufferSize - uiBuffOff))
            {
                uiBuffOff += iSubLen;
            }
            break;
        }

        case 'm':
        {
            //不拷贝消息，而仅是 将消息输出切换到后缀部分
            a_pstEvt->evt_msg_prefix_len = uiBuffOff;
            if (uiBuffOff >= sizeof(a_pstEvt->evt_msg_prefix_buffer))
            {
                a_pstEvt->evt_msg_prefix_buffer[sizeof(a_pstEvt->evt_msg_prefix_buffer)
                        - 1] = -'\0';
            }

            pszBuffer = a_pstEvt->evt_msg_sufix_buffer;
            uiBufferSize = sizeof(a_pstEvt->evt_msg_sufix_buffer);
            uiBuffOff = 0;

            break;
        }
        case 'P':
        {
            iSubLen = snprintf(pszBuffer + uiBuffOff, uiBufferSize - uiBuffOff,
                    "%d", getpid());
            if (iSubLen > 0 && iSubLen <= (uiBufferSize - uiBuffOff))
            {
                uiBuffOff += iSubLen;
            }
            break;
        }
        case 'p':
        {
            iSubLen = snprintf(pszBuffer + uiBuffOff, uiBufferSize - uiBuffOff,
                    "%-8s", tlog_priority_to_string(a_pstEvt->evt_priority));
            if (iSubLen > 0 && iSubLen <= (uiBufferSize - uiBuffOff))
            {
                uiBuffOff += iSubLen;
            }

            break;
        }
#if !defined (_WIN32) && !defined (_WIN64)
            /*TODO:在windows环境pthread_self的实现不支持将返回值转换成整数*/
            case 't' :
            {
                iSubLen = snprintf(pszBuffer + uiBuffOff,
                        uiBufferSize -uiBuffOff, "%lu", (unsigned long)pthread_self());
                if(iSubLen > 0 && iSubLen <= (uiBufferSize -uiBuffOff))
                {
                    uiBuffOff += iSubLen;
                }

                break;
            }
#endif

#if  !((defined(_WIN32) || defined(_WIN64))  && _MSVCVER < 1500)
            case 'f' :
            {
                iSubLen = snprintf(pszBuffer + uiBuffOff,
                        uiBufferSize -uiBuffOff, "%s",
                        (a_pstEvt->evt_loc ? a_pstEvt->evt_loc->loc_file : "(nil)"));
                if(iSubLen > 0 && iSubLen <= (uiBufferSize -uiBuffOff))
                {
                    uiBuffOff += iSubLen;
                }
                break;
            }
            case 'F' :
            {
                iSubLen = snprintf(pszBuffer + uiBuffOff,
                        uiBufferSize -uiBuffOff, "%s",
                        (a_pstEvt->evt_loc ? a_pstEvt->evt_loc->loc_function: "(nil)"));
                if(iSubLen > 0 && iSubLen <= (uiBufferSize -uiBuffOff))
                {
                    uiBuffOff += iSubLen;
                }
                break;
            }
            case 'l' :
            {
                iSubLen = snprintf(pszBuffer + uiBuffOff,
                        uiBufferSize -uiBuffOff, "%d",
                        (a_pstEvt->evt_loc ? a_pstEvt->evt_loc->loc_line: 0));
                if(iSubLen > 0 && iSubLen <= (uiBufferSize -uiBuffOff))
                {
                    uiBuffOff += iSubLen;
                }
                break;
            }
#endif
        case 'n':
        {
            iSubLen = snprintf(pszBuffer + uiBuffOff, uiBufferSize - uiBuffOff,
                    "\n");
            if (iSubLen > 0 && iSubLen <= (uiBufferSize - uiBuffOff))
            {
                uiBuffOff += iSubLen;
            }
            break;
        }

            // Mickey add %h and %M for host name and module name.
        case 'h':
        {
            iSubLen = snprintf(pszBuffer + uiBuffOff, uiBufferSize - uiBuffOff,
                    "%s", a_pstEvt->evt_hostName);
            if (iSubLen > 0 && iSubLen <= (uiBufferSize - uiBuffOff))
            {
                uiBuffOff += iSubLen;
            }
            break;
        }
        case 'M':
        {
            iSubLen = snprintf(pszBuffer + uiBuffOff, uiBufferSize - uiBuffOff,
                    "%s", a_pstEvt->evt_moduleName);
            if (iSubLen > 0 && iSubLen <= (uiBufferSize - uiBuffOff))
            {
                uiBuffOff += iSubLen;
            }
            break;
        }
        default:
        {
            /* unknown */
            break;
        }
        }
        if (*pszFmt)
        {
            ++pszFmt;
        }
    }

    if (pszBuffer == a_pstEvt->evt_msg_prefix_buffer)
    {
        //仅有前缀
         if (uiBuffOff >= sizeof(a_pstEvt->evt_msg_prefix_buffer))
        {
            uiBuffOff = sizeof(a_pstEvt->evt_msg_prefix_buffer);
            a_pstEvt->evt_msg_prefix_buffer[uiBuffOff - 1] = '\0';
        }
        a_pstEvt->evt_msg_prefix_len = uiBuffOff;
        a_pstEvt->evt_msg_sufix_len = 0;
    }
    else
    {

        if (uiBuffOff >= sizeof(a_pstEvt->evt_msg_sufix_buffer))
        {
            uiBuffOff = sizeof(a_pstEvt->evt_msg_sufix_buffer);
            a_pstEvt->evt_msg_sufix_buffer[uiBuffOff  - 1] = '\0';
        }
        a_pstEvt->evt_msg_sufix_len = uiBuffOff;
    }/*if (pszBuffer == a_pstEvt->evt_msg_prefix_buffer)*/

}

