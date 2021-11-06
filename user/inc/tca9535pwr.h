#ifndef __TCA9535PWR_H__
#define __TCA9535PWR_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

/** TCA9535PWR I2C��ַ */  
#define 	TCA9535PWR_ADDR			0x20

/** TCA9535PWR �Ĵ����� */  
#define		TCA9535PWR_INPUT0		0x00
#define 	TCA9535PWR_INPUT1		0x01
#define 	TCA9535PWR_OUTPUT0		0x02
#define 	TCA9535PWR_OUTPUT1		0x03
#define 	TCA9535PWR_INVERSION0	0x04
#define 	TCA9535PWR_INVERSION1	0x05
#define 	TCA9535PWR_CONFIG0		0x06
#define 	TCA9535PWR_CONFIG1		0x07

/** �����������,��Ӧλ1Ϊ���룬0Ϊ��� */  
#define		TCA9535PWR_IO_CONFIG	0x003c

/** TCA9535PWR �ж��������� */  
#ifndef TCA9535PWR_IRQ_PIN
#define TCA9535PWR_IRQ_PORT	GPIOE
#define TCA9535PWR_IRQ_PIN	GPIO_PIN_15
#endif

/** 
 * �������״̬����. 
 * ���ڲ�Ӱ�첻��ض˿�. 
 */  
typedef struct
{
	uint16_t		output;		///<���״̬
	uint16_t		input;		///<����״̬s
}TCA9535PWR_BUF_T;

extern void tca9535pwr_int(void);
extern void tca9535pwr_set_pin( uint16_t pin );
extern void tca9535pwr_clr_pin( uint16_t pin );
extern uint16_t tca9535pwr_read_pin(void);
extern void tca9535pwr_write_data( uint8_t data, uint8_t start, uint8_t mask );
extern void tca9535pwr_isr_handle(void);
extern FUN_STATUS_T tca55pwr_read_reg ( uint8_t reg_addr, uint16_t *p_data );
#endif
