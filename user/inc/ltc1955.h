#ifndef __LTC1955_H__
#define __LTC1955_H__

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"

#define		VCC_0V		0
#define 	VCC_1_8V	1
#define 	VCC_3V		2
#define 	VCC_5V		3

#define		SYNC_MODE		0
#define		ASYNC_STOP_LOW	2
#define		ASYNC_STOP_HIGH	3
#define		ASYNC_DIV_1		4
#define 	ASYNC_DIV_2		5
#define 	ASYNC_DIV_4		6
#define 	ASYNC_DIV_8		7

#define		NOTHING_SEL		0
#define 	C4A_DATA		1
#define 	C8A_DATA		2
#define 	IOA_DATA		3

enum{
	CARD_NONE = 0,
	CARD_A,
	CARD_B,
	
};

typedef struct{
	uint8_t		vcc_option:2;
	uint8_t		card_select:1;
	uint8_t		data_pull_up:1;
	uint8_t		reserve:1;
	uint8_t		clock_option:3;
}LTC1955_CONFIG_B_T;

typedef struct{
	uint8_t		vcc_option:2;
	uint8_t		card_select:1;
	uint8_t		communication_option:2;
	uint8_t		clock_option:3;
}LTC1955_CONFIG_A_T;

typedef struct{
	uint8_t		reserve:4;
	uint8_t		electrical_fault:1;
	uint8_t		atr_fault:1;
	uint8_t		vcc_ready:1;
	uint8_t		present:1;
}LTC1955_STATUS_T;

typedef struct{
	
	union{
		struct{
			LTC1955_CONFIG_B_T	card_b;
			LTC1955_CONFIG_A_T	card_a;
		}byte;
		uint16_t	word;
	}config;
	
	union{
		struct{
			LTC1955_STATUS_T	card_b;
			LTC1955_STATUS_T	card_a;
		}byte;
		uint16_t	word;
	}status;
	
}LTC1955_OPERATION_T;

extern LTC1955_OPERATION_T				ltc1955_operation;

#define LTC1955_CARD_A_SEL(v)			(ltc1955_operation.config.byte.card_a.card_select=v)
#define LTC1955_CARD_B_SEL(v)			(ltc1955_operation.config.byte.card_b.card_select=v)
#define LTC1955_CARD_A_VCC(v)			(ltc1955_operation.config.byte.card_a.vcc_option=v)
#define LTC1955_CARD_B_VCC(v)			(ltc1955_operation.config.byte.card_b.vcc_option=v)
#define LTC1955_CARD_A_CLOCK(v)			(ltc1955_operation.config.byte.card_a.clock_option=v)
#define LTC1955_CARD_B_CLOCK(v)			(ltc1955_operation.config.byte.card_b.clock_option=v)
#define LTC1955_CARD_COMM(v)			(ltc1955_operation.config.byte.card_a.communication_option=v)
#define LTC1955_PULL_UP(v)				(ltc1955_operation.config.byte.card_b.data_pull_up=v)

#define LTC1955_IS_CARD_A_PRESENT()		(ltc1955_operation.status.byte.card_a.present)
#define LTC1955_IS_CARD_B_PRESENT()		(ltc1955_operation.status.byte.card_b.present)
#define LTC1955_IS_CARD_A_VCC_READY()	(ltc1955_operation.status.byte.card_a.vcc_ready)
#define LTC1955_IS_CARD_B_VCC_READY()	(ltc1955_operation.status.byte.card_b.vcc_ready)
#define LTC1955_IS_CARD_A_ATR_FAULT()	(ltc1955_operation.status.byte.card_a.atr_fault)
#define LTC1955_IS_CARD_B_ATR_FAULT()	(ltc1955_operation.status.byte.card_b.atr_fault)
#define LTC1955_IS_CARD_A_ELECT_FAULT()	(ltc1955_operation.status.byte.card_a.electrical_fault)
#define LTC1955_IS_CARD_B_ELECT_FAULT()	(ltc1955_operation.status.byte.card_b.electrical_fault)

#ifndef LTC1955_DOUT_PIN
#define LTC1955_DOUT_PORT	GPIOE
#define LTC1955_DOUT_PIN	GPIO_PIN_0
#endif

#ifndef LTC1955_SCLK_PIN
#define LTC1955_SCLK_PORT	GPIOE
#define LTC1955_SCLK_PIN	GPIO_PIN_1
#endif

#ifndef LTC1955_DIN_PIN
#define LTC1955_DIN_PORT	GPIOE
#define LTC1955_DIN_PIN		GPIO_PIN_2
#endif

#ifndef LTC1955_LD_PIN
#define LTC1955_LD_PORT		GPIOE
#define LTC1955_LD_PIN		GPIO_PIN_3
#endif

#ifndef LTC1955_FAULT_PIN
#define LTC1955_FAULT_PORT	GPIOE
#define LTC1955_FAULT_PIN	GPIO_PIN_11
#endif

#ifndef LTC1955_RIN_PIN
#define LTC1955_RIN_PORT	GPIOA
#define LTC1955_RIN_PIN		GPIO_PIN_1
#endif

extern void ltc1955_init(void);
extern void ltc1955_poll_operation(void);
extern void ltc1955_select(uint8_t card);
extern void ltc1955_reset(uint8_t card_num, uint8_t sta);
extern void ltc1955_power(uint8_t card_num, uint8_t sta);
#endif

