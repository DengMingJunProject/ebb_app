#ifndef __URG_H__
#define __URG_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

typedef union {
	uint8_t			byte;
	struct{
		uint8_t		device_opened:1;
		uint8_t		device_tilted:1;
		uint8_t		auth_failed_rf:1;
		uint8_t		auth_failed_remote:1;
		uint8_t		reserve:4;
	}bit;
}TAMPER_T;

enum{
	ALARM_TAMPER_DETECT=0,
	ALARM_BATTERY_LOW,
	ALARM_MISMATCH,
};

typedef union {
	
	uint8_t 		byte;
	struct{
		uint8_t		tamper_detect:1;
		uint8_t		low_power:1;
		uint8_t		reserve:6;
	}bit;
}URG_BIT_T;

typedef struct {
	URG_BIT_T		urg;
	URG_BIT_T		last_urg;
	TAMPER_T		tamper;
	uint8_t			tamper_alarm;
	uint8_t			fully_charged;
}URG_STATUS_T;

extern URG_STATUS_T	urg_status;

#define is_tamper_detect()				(urg_status.urg.bit.tamper_detect == 1)

extern FUN_STATUS_T urg_init(void);
extern uint8_t urg_cnt_err(void);
#endif
