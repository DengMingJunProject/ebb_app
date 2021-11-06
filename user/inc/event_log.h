#ifndef __EVENT_LOG_H___
#define __EVENT_LOG_H___

#include "stdint.h"
#include "macro_def.h"

#define EVENT_LOG_DRIVER		"M0:\\"

typedef enum {
	CRITICAL_LEVEL=1,
	SYSTEM_LEVEL,
	STANDARD_LEVEL,
}EVENT_LEVEL_T;

typedef enum {
	MEMORY_TYPE=1,
	POWER_TYPE,
	COMMUNICATION_TYPE,
	SECURITY_TYPE,
	SYSTEM_ADMIN_TYPE,
	SYSTEM_TYPE,
	PRINTER_TYPE,
	ELECTION_TYPE,
	SETTINGS_TYPE,
	FIRMWARE_TYPE,
	KEY_CARD_TYPE,
	MANTEN_CARD_TYPE,
}EVENT_TYPE_T;

typedef struct {
	
	uint16_t	write_pos;			///<写入记录最新位置
	uint16_t	read_pos;			///<上传记录最后位置
	uint16_t	total;				///<未上传记录总数
	uint16_t	logs;				///<写入记录总数，用于翻查log
}EVENT_LOG_HEADER_T;

typedef struct{
	uint32_t 	addr;
}EVENT_LOG_MSG_T;

//#define EVENT_LOG_ADDR		1024*1024
#define EVNET_LOG_SIZE			200
#define EVENT_LOG_MAX			100
#define EVENT_FILE_LOG_MAX		10
#define	EVENT_LOG_QUEUE_MAX		10
#define EVENT_FILE_NAME_LEN		30

extern FUN_STATUS_T event_log_init(void);
extern FUN_STATUS_T event_log_write(EVENT_LEVEL_T level, EVENT_TYPE_T type, char *message);
extern FUN_STATUS_T event_log_read(uint16_t log_num, char *event_log);
extern FUN_STATUS_T event_log_header_read(EVENT_LOG_HEADER_T *event_log_header);
extern FUN_STATUS_T event_log_header_write(EVENT_LOG_HEADER_T *event_log_header);
extern FUN_STATUS_T event_log_read_upload( char *log_ptr);
#endif
