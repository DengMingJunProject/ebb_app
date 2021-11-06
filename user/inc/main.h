#ifndef __MAIN_H__
#define __MAIN_H__

#include "stdint.h"
#include "stdio.h"
#include "tm_stm32_rtc.h"
#include "macro_def.h"

typedef struct{
	
    uint32_t id;
    char uuid[12];
    char uuid_str[32];
} SYS_ID_T;

typedef struct{

	uint32_t		heap_size;								///<内存池大小
	uint32_t		send_cnt_total;							///<总的发送次数
	uint32_t		send_timeout;							///<等待超时次数
	uint32_t		send_disconnect_cnt;					///<连接断开发送次数
	uint8_t			queue_num;								///<空闲队列数
	float			cpu_usage;								///<CPU使用率
	uint32_t		flash_usage;
	uint32_t		flash_total;
}SYS_RUN_PARA_T;

typedef union{
	struct{
		uint32_t		rf_module:1;
		uint32_t		nand_flash:1;
		uint8_t			sd_card:1;
	}bit;
	uint32_t 			word;
}PERIPHERAL_INIT;

typedef struct {
	SYS_RUN_PARA_T	sys_run_para;
	SYS_ID_T		sys_id;
	PERIPHERAL_INIT	peripheral_init;
//	uint32_t		local_addr;
//	uint32_t		target_addr;
	uint8_t			start_complete;
	uint8_t			update_req;
	
	uint8_t			main_power;
	
	uint8_t			have_sc_eeprom;
	uint8_t			have_spi_flash;
	uint8_t			have_rf_module;
	
	uint16_t		en_send_log;
	
	uint32_t		rd_number;

	uint8_t			self_test_progress;
	
	TM_RTC_t 		rtc;
	
}SYS_STATUS_T;

extern SYS_STATUS_T	sys_status;
extern void error_handle(char *file, int line);
extern void _Error_Handler(char *, int);

#endif
