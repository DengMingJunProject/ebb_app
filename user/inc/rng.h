/*********************************************************************************************************************
 * FileName : bsp_rng.h
 * Brief    : Header file for bsp_rng.c
 * Author   : Bryant Dai
 * Date     : Sep-25-2018
 * Version  : V1.0 
 * Note     : 
***********************************************************************************************************************/
/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef __RNG_H
#define __RNG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"
/* Exported macro ----------------------------------------------------------------------------------------------------*/
#define RNG_GENERATE_NUM     2
extern uint32_t g_random32bitBuf[RNG_GENERATE_NUM];          /* Used for storing Random 32bit Numbers                        */ 
/* Exported types ----------------------------------------------------------------------------------------------------*/
/* Exported constants ------------------------------------------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------------------------------------------------*/
extern void bsp_rngInit(void);
extern HAL_StatusTypeDef rng_generate_num(uint8_t rngNum);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_RNG_H                                                                                                 */


/*****************************************************END OF FILE******************************************************/

