#ifndef TLOG_EVENT_I_H
#define TLOG_EVENT_I_H


#include "pal/pal.h"
#include "tlog_buffer.h"
#include "tlog/tlog_loc_info.h"
#include "tlog/tlog_event.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TLOG_AUX_BUFFER_SIZE    2048 /*记录消息前后缀信息的缓冲区大小*/

typedef struct 
{
	const char* evt_category;
	int	evt_priority;
	char* evt_msg;
	const char* evt_rendered_msg;

	char *evt_hostName;
	char *evt_moduleName;
	
	tlog_buffer_t evt_buffer;
	struct timeval evt_timestamp;
	const tlog_loc_info_t* evt_loc;
	
	int evt_is_msg_binary;
	int evt_msg_len;
	size_t evt_rendered_msg_len;
	int evt_id;
	int evt_cls;
	int evt_type;
	int evt_version;
	
    char evt_msg_prefix_buffer[TLOG_AUX_BUFFER_SIZE]; /*消息前缀缓冲区*/
    int evt_msg_prefix_len;  /*消息前缀信息串长度*/

    char evt_msg_sufix_buffer[TLOG_AUX_BUFFER_SIZE]; /*消息后缀缓冲区*/
    int evt_msg_sufix_len;  /*消息后缀信息串长度*/
} tlog_event_base_t;

typedef tlog_event_base_t	TLOGEVENTBASE;
typedef tlog_event_base_t	*LPTLOGEVENTBASE;

#ifdef __cplusplus
}
#endif

#endif /* TLOG_EVENT_H */

