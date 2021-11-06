#ifndef __BEEP_H__
#define __BEEP_H__

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx.h"
#include "macro_def.h"

typedef enum
{
	BP_START_UP      =  0,
	BP_SETTING_DONE,
	BP_TIME_REACH,
	BP_ASSIST_REQ,
	BP_VOTE_CARD_ACT,
	BP_TAMPER_ACT,
	BP_TAMPER_DETECT,
	BP_BATTERY_LOW,
	BP_STOP,
	BP_OCCUR_MAX,
}BP_TypeDef;

typedef struct{
	FunctionalState buzzerStatus;             /* The current status of the buzzer                                       */
	uint32_t   cycle;        /* The cycle number of buzzer enable                                      */
	uint32_t   counter;            /* Time counter of the buzzer enable status                               */
	uint32_t   time;         /* Time period of buzzer enable                                           */
	uint32_t   period;             /* The period of the buzzer    */
}BEEP_STATUS_T;

#ifndef BEEP_PIN		
#define BEEP_PORT    	GPIOB
#define BEEP_PIN     	GPIO_PIN_14
#endif

extern FUN_STATUS_T beep_init(void);
extern void beep_set_alarm(BP_TypeDef bzAlarm);
#endif
