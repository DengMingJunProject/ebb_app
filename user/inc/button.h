#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "stm32f4xx_hal.h"
#include "stdint.h"
#include "stdbool.h"

#define KEY_K1		(1<<4)
#define KEY_K2		(1<<2)
#define KEY_K3		(1<<5)
#define KEY_K4		(1<<3)

#define BUTTON_VAIL_LEVEL                       GPIO_PIN_RESET  
#define BUTTON_LONG_CLICK_TIME                  50       /* Basic on BUTTON_SCAN_INTERVAL                 */
#define BUTTON_DOUBLE_CLICK_TIME                30       /* Basic on BUTTON_SCAN_INTERVAL                 */

typedef enum
{
  BUTTON_NO_EVENT     = 0,
  BUTTON_SINGLE_CLICK = 1,
  BUTTON_DOUBLE_EVENT = 2,
  BUTTON_LONG_CLICK   = 3,
}BUTTON_EVENT_T;

typedef enum
{
  BUTTON_STATE0 = 0,
  BUTTON_STATE1 = 1,
  BUTTON_STATE2 = 2,
  BUTTON_STATE3 = 3
}BUTTON_STATE_T;

typedef enum
{
	CON_BUTTON  = 0,
	CAN_BUTTON  = 1,
	DOWN_BUTTON   = 2,
	UP_BUTTON = 3,
}BUTTON_NAME_T;

typedef struct 
{
  BUTTON_NAME_T  		button_name;
  uint8_t				button_pin;
  uint16_t             	button_detect_cnt;
  uint16_t             	button_scan_cnt;
  BUTTON_STATE_T 		button_detect_step;
  BUTTON_STATE_T 		button_scan_step;
  BUTTON_EVENT_T 		button_event;
}BUTTON_T;

extern void button_scan(void);

#endif
