/** 
* @file         rng.c 
* @brief        Ӳ�����������. 
* @details  	. 
* @author   	ken deng 
* @date     	2019-07-26 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-07-26, ����\n 
*/  
#include "rng.h"

RNG_HandleTypeDef g_rng_handle;                        /* RNG handler declaration                                      */
uint32_t g_random32bitBuf[RNG_GENERATE_NUM];          /* Used for storing Random 32bit Numbers                        */

static void rng_enable(void);
static void rng_disable(void);

/** 
* �����Ӳ����ʼ��. 
* ��.
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-26���� 
*/
FUN_STATUS_T rng_init(void)
{
	/* Configure the RNG peripheral                                                                                     */
	g_rng_handle.Instance = RNG;
  
	/* DeInitialize the RNG peripheral                                                                                  */
	if (HAL_RNG_DeInit(&g_rng_handle) != HAL_OK){
		//    Error_Handler(SYSTEM_RNG_INIT_ERROR);    /* DeInitialization Error                                                */
		return FUN_ERROR;
	}
  
	/* Initialize the RNG peripheral                                                                                    */
	if (HAL_RNG_Init(&g_rng_handle) != HAL_OK){
		//    Error_Handler(SYSTEM_RNG_INIT_ERROR);    /* Initialization Error                                                  */
		return FUN_ERROR;
	}
	return FUN_OK;
}

/** 
* ���������. 
* ��.
* @param[in]   rngNum:�������������. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-26���� 
*/
HAL_StatusTypeDef rng_generate_num(uint8_t rngNum)
{
	uint8_t i = 0u;
	HAL_StatusTypeDef status = HAL_OK;

	rng_init();
	
	rng_enable();
	if (rngNum > RNG_GENERATE_NUM){
		
		rngNum = RNG_GENERATE_NUM;
	}

	for (i = 0;i < rngNum;i++){
		
		if ((status = HAL_RNG_GenerateRandomNumber(&g_rng_handle,&g_random32bitBuf[i])) != HAL_OK){
			rng_disable();
			return status;
		}
	}
	return status;
}

/** 
* �ر������������. 
* ��.
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-26���� 
*/
static void rng_disable(void)
{
	CLEAR_BIT(g_rng_handle.Instance->CR, RNG_CR_IE   | RNG_CR_RNGEN);  /* Disable the RNG Peripheral                     */
	CLEAR_BIT(g_rng_handle.Instance->SR, RNG_SR_CEIS | RNG_SR_SEIS );  /* Clear RNG interrupt status flags               */
	g_rng_handle.State = HAL_RNG_STATE_READY;                          /* Initialize the RNG state                       */
}


/** 
* �������������. 
* ��.
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-26���� 
*/
static void rng_enable(void)
{
	__HAL_RNG_ENABLE(&g_rng_handle);             /* Enable the RNG Peripheral                                            */

	g_rng_handle.State = HAL_RNG_STATE_READY;    /* Initialize the RNG state                                             */
}


void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{  
	__HAL_RCC_RNG_CLK_ENABLE();                     /* RNG Peripheral clock enable                                      */

	/* NVIC Configuration for RNG interrupts                                                                            */
	HAL_NVIC_SetPriority(HASH_RNG_IRQn,0x09,0x09);  /* Set priority for HASH_RNG_IRQn                                   */
	HAL_NVIC_EnableIRQ(HASH_RNG_IRQn);              /* Enable HASH_RNG_IRQn                                             */       
}


void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
	__HAL_RCC_RNG_FORCE_RESET();      /* Enable RNG reset state                                                         */
	__HAL_RCC_RNG_RELEASE_RESET();    /* Release RNG from reset state                                                   */
} 

