/** 
* @file         hash.c 
* @brief        ����SHA-1ժҪ. 
* @details  	. 
* @author   	ken deng 
* @date     	2019-07-26 
* @version  	A001 
* @par Copyright (c):  
*       laxton company 
* @par History:          
*   version: ken deng, 2019-07-26, ����\n 
*/
#include "hash.h"
#include "stm32f4xx_hal.h"

static HASH_HandleTypeDef hhash;

static void MX_HASH_Init(void)
{

  /* USER CODE BEGIN HASH_Init 0 */

  /* USER CODE END HASH_Init 0 */

  /* USER CODE BEGIN HASH_Init 1 */

  /* USER CODE END HASH_Init 1 */
  hhash.Init.DataType = HASH_DATATYPE_8B;
  if (HAL_HASH_Init(&hhash) != HAL_OK)
  {
    
  }
  /* USER CODE BEGIN HASH_Init 2 */

  /* USER CODE END HASH_Init 2 */

}

void HAL_HASH_MspInit(HASH_HandleTypeDef* hhash)
{

  /* USER CODE BEGIN HASH_MspInit 0 */

  /* USER CODE END HASH_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_HASH_CLK_ENABLE();
  /* USER CODE BEGIN HASH_MspInit 1 */

  /* USER CODE END HASH_MspInit 1 */

}

/**
* @brief HASH MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hhash: HASH handle pointer
* @retval None
*/

void HAL_HASH_MspDeInit(HASH_HandleTypeDef* hhash)
{

  /* USER CODE BEGIN HASH_MspDeInit 0 */

  /* USER CODE END HASH_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_HASH_CLK_DISABLE();
  /* USER CODE BEGIN HASH_MspDeInit 1 */

  /* USER CODE END HASH_MspDeInit 1 */

}

/** 
* hashӲ����ʼ��. 
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
FUN_STATUS_T hash_int(void){
	MX_HASH_Init();
	
	return FUN_OK;
}

/** 
* ����SHA-1��. 
* ��. 
* @param[in]   in_buf:�������뻺��,size:�����С,out_buf:����������������. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-26���� 
*/
void hash_count(uint8_t *in_buf, uint16_t size, uint8_t *out_buf){
	hash_int();
	
	HAL_HASH_SHA1_Start(&hhash, in_buf, size, out_buf,100);
}
