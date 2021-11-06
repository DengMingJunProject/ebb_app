/**
  ******************************************************************************
  * File Name          : SPI.c
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "fs_spi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef hspi_fs;

//void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
//{

//  GPIO_InitTypeDef GPIO_InitStruct;
//  if(spiHandle->Instance==SPI2)
//  {
//  /* USER CODE BEGIN SPI1_MspInit 0 */

//  /* USER CODE END SPI1_MspInit 0 */
//    /* SPI1 clock enable */
//    __HAL_RCC_SPI2_CLK_ENABLE();
//  
//    /**SPI1 GPIO Configuration    
//    PA5     ------> SPI1_SCK
//    PA6     ------> SPI1_MISO
//    PA7     ------> SPI1_MOSI 
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /* USER CODE BEGIN SPI1_MspInit 1 */

//  /* USER CODE END SPI1_MspInit 1 */
//  }
//}


/* SPI1 init function */
void fs_spi_init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStruct;

  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();
  
    /**SPI1 GPIO Configuration    
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	hspi_fs.Instance = SPI2;
	hspi_fs.Init.Mode = SPI_MODE_MASTER;
	hspi_fs.Init.Direction = SPI_DIRECTION_2LINES;
	hspi_fs.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi_fs.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi_fs.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi_fs.Init.NSS = SPI_NSS_SOFT;
	hspi_fs.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi_fs.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi_fs.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi_fs.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi_fs.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi_fs) != HAL_OK)
	{
	//    _Error_Handler(__FILE__, __LINE__);
	}

}

//void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
//{

//  if(spiHandle->Instance==SPI2)
//  {
//  /* USER CODE BEGIN SPI1_MspDeInit 0 */

//  /* USER CODE END SPI1_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_SPI2_CLK_DISABLE();
//  
//    /**SPI1 GPIO Configuration    
//    PA5     ------> SPI1_SCK
//    PA6     ------> SPI1_MISO
//    PA7     ------> SPI1_MOSI 
//    */
//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

//  /* USER CODE BEGIN SPI1_MspDeInit 1 */

//  /* USER CODE END SPI1_MspDeInit 1 */
//  }
//} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
