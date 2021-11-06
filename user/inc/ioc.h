#ifndef __IOC_H__
#define __IOC_H__

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "tca9535pwr.h"
#include "lcdxx.h"
#include "macro_def.h"
#include "tm_stm32_gpio.h"

#define		EXTIO_INPUT_NUM				8
#define 	EXTIO_INPUT_TIME			10
#define 	EXTIO_LED_NUM				4

#define		CON_LED_PIN					LCD_LED1
#define		CAN_LED_PIN					LCD_LED2
#define		UP_LED_PIN					LCD_LED3
#define		DOWN_LED_PIN				LCD_LED4

#ifndef 	LED_GREEN_PIN
#define 	LED_GREEN_PORT				GPIOD
#define 	LED_GREEN_PIN				GPIO_PIN_10
#endif

#ifndef 	LED_YELLOW_PIN
#define 	LED_YELLOW_PORT				GPIOD
#define 	LED_YELLOW_PIN				GPIO_PIN_9
#endif

#ifndef 	LED_RED_PIN
#define 	LED_RED_PORT				GPIOC
#define 	LED_RED_PIN					GPIO_PIN_4
#endif

#ifndef 	TILT_DECT_PORT
#define 	TILT_DECT_PORT				GPIOB
#define 	TILT_DECT_PIN				GPIO_PIN_4
#endif

#ifndef 	OPEN_DECT_PORT
#define 	OPEN_DECT_PORT				GPIOB
#define 	OPEN_DECT_PIN				GPIO_PIN_5
#endif


///<最低位在最上面,字节0在最上面
typedef union{
	uint8_t 		byte;
	struct{
		uint8_t		gpio1:1;
		uint8_t		gpio2:1;
		uint8_t		gpio3:1;
		uint8_t		gpio4:1;
		uint8_t		gpio5:1;
		uint8_t		reserve:3;
	}bit;
}IO_T;

typedef struct{
	IO_T		read_io;
	IO_T		save_io;
	uint8_t		timer[EXTIO_INPUT_NUM];
}EXTIO_INPUT_T;

typedef struct{
	
	uint8_t 	state[EXTIO_LED_NUM];						///<LED状态
	uint8_t		last_state[EXTIO_LED_NUM];					///<上一个LED状态
	uint8_t		timer[EXTIO_LED_NUM];						///<LED闪烁计时
	uint8_t		cnt[EXTIO_LED_NUM];							///<LED闪烁计时次数计数
	uint8_t		blink_time[EXTIO_LED_NUM];					///<LED闪烁时间
	uint8_t		blink_cnt[EXTIO_LED_NUM];					///<LED闪烁次数
	uint8_t		blink_flag[EXTIO_LED_NUM];					///<LED亮灭标志，用于闪烁处理
}EXTIO_LED_T;

typedef enum{
	
	LED_OFF=0,
	LED_ON,
	LED_BLINK,
}LED_STA_T;

///<根据实际使用改变命令，但序号一定以0起
enum{
	CON_LED=0,								
	CAN_LED,
	UP_LED,
	DOWN_LED,
};

enum{
	TILT_DECT=0,
	OPEN_DECT,
	REMOTE_DECT,
	SC2_OFF_DECT,
	SC1_OFF_DECT,
};

#define gpio1_read()	TM_GPIO_GetInputPinValue(TILT_DECT_PORT,TILT_DECT_PIN)
#define gpio2_read()	TM_GPIO_GetInputPinValue(OPEN_DECT_PORT,OPEN_DECT_PIN)

#define led1_set(sta)	(sta>0)?(tca9535pwr_set_pin(CON_LED_PIN)):(tca9535pwr_clr_pin(CON_LED_PIN))
#define led2_set(sta)	(sta>0)?(tca9535pwr_set_pin(CAN_LED_PIN)):(tca9535pwr_clr_pin(CAN_LED_PIN))
#define led3_set(sta)	(sta>0)?(tca9535pwr_set_pin(UP_LED_PIN)):(tca9535pwr_clr_pin(UP_LED_PIN))
#define led4_set(sta)	(sta>0)?(tca9535pwr_set_pin(DOWN_LED_PIN)):(tca9535pwr_clr_pin(DOWN_LED_PIN))

//<以下为向外，提供控制宏
extern EXTIO_INPUT_T g_extio_input;
//<读取IO状态，返回0或1
#define ioc_read_dect(num)				((g_extio_input.save_io.byte&(1<<num))!=0)

extern EXTIO_LED_T g_extio_led;
//<LED亮
#define ioc_led_on(led)					(g_extio_led.state[led]=LED_ON)
//<LED灭
#define ioc_led_off(led)				(g_extio_led.state[led]=LED_OFF)
//<LED长期闪烁，单位ms，最大值2500
#define ioc_led_blink(led,time)			{g_extio_led.blink_time[led]=(time>2500?2500:time)/10;g_extio_led.state[led]=LED_BLINK;g_extio_led.blink_cnt[led]=0xff;}
//<LED长期闪烁指定次数，单位ms，最大值2500
#define ioc_led_blink_cnt(led,time,ts)	{g_extio_led.blink_time[led]=(time>2500?2500:time)/10;g_extio_led.state[led]=LED_BLINK;g_extio_led.blink_cnt[led]=ts;g_extio_led.cnt[led]=0;}
	
#define ioc_keypad_led_on()				{ioc_led_on(CON_LED);ioc_led_on(CAN_LED);ioc_led_on(UP_LED);ioc_led_on(DOWN_LED);}
#define ioc_keypad_led_off()			{ioc_led_off(CON_LED);ioc_led_off(CAN_LED);ioc_led_off(UP_LED);ioc_led_off(DOWN_LED);}
#define ioc_keypad_led_blink()			{ioc_led_blink_cnt(CON_LED,500,3);ioc_led_blink_cnt(CAN_LED,500,3);ioc_led_blink_cnt(UP_LED,500,3);ioc_led_blink_cnt(DOWN_LED,500,3);}


extern FUN_STATUS_T ioc_init(void);
extern void ioc_led(uint8_t led, uint8_t sta);
extern void ioc_led_set( uint8_t led, uint8_t sta );
#endif

