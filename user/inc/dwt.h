/**********************************************************************************************************
 *	FileName : bsp_dwt.c
 *	Version  : V1.0
 *	Brief    : There are three kinds of tracking sources in CM3 and CM4: ETM, ITM and DWT.The clock cycle (CYCCNT) 
 *             counting function in DWT, this function is very important, it is convenient to calculate the number 
 *             of clock cycles executed by the program.
 *	Modify  :
 *		Version   Date        Author     Brief
 *		V1.0    2015-08-18   Eric2013    Release
 *
 *	Copyright (C), 2015-2020, www.armfly.com
 *
**********************************************************************************************************/

#ifndef __BSP_DWT_H
#define __BSP_DWT_H

#include "stm32f4xx_hal.h"
#include "stdint.h"

void dwt_init(void);
void dwt_delay_us(uint32_t _ulDelayTime);
void dwt_delay_ms(uint32_t _ulDelayTime);
void dwt_delay_cycle(uint32_t _ulDelayTime);
extern uint32_t dwt_read(void);
#endif

/***************************** www.armfly.com (END OF FILE) *********************************/


