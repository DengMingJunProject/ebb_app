/** 
* @file         smartcard.c 
* @brief        智能卡加密读写操作. 
* @details  	卡类型检测，读写卡等. 
* @author       Ken Deng 
* @date     	2019-07-05 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: Ken Deng, 2019-07-05, 建立\n 
*/  
#include "system.h"
#include "smartcard.h"
#include "ltc1955.h"
#include "dwt.h"
#include "at88sc_reg.h"
#include "Driver_USART.h"
#include "secure_storage.h"
#include "rng.h"
#include "election.h"
#include "display.h"
#include "crypt.h"
#include "ioc.h"
#include "menu_func.h"
#include "config.h"
#include "devfs.h"
#include "event_log.h"

//#undef SMARTCARD_DEBUG
#ifdef SMARTCARD_DEBUG
    #define	smartcard_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[SMARTCARD](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define smartcard_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[SMARTCARD] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define smartcard_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[SMARTCARD] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define smartcard_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define smartcard_log(...)
    #define smartcard_usr(...)
    #define smartcard_err(...)
    #define smartcard_dump(...)
#endif

CARD_MANAGE_T	smartcard_manage[SMARTCARD_NUM];
EventGroupHandle_t smartcard_event_handler = NULL;    /*定义事件标志组*/
static uint8_t GPA[20];
SC_CARD_IND_T	sc_card_ind;
SC_STA_T sc_sta;
	
#define IDENTITY_CARD			smartcard_manage[0]
#define ACTIVATE_CARD			smartcard_manage[1]
	
#define SMARTCARD_SEND_OK  		(1<<0)
#define SMARTCARD_RECV_OK  		(1<<1)
#define SMARTCARD_ERROR  		(1<<2)

uint8_t complete=1;
	
static FUN_STATUS_T sc_transmit_data(CARD_MANAGE_T *p_cardManageHandle, bool procedureFlag);
	
const SMARTCARD_CONFIG_T smartcard_config[CARD_CONFIG_MAX]={
	[VOTE_CARD_CONFIG].auth_key={0x37,0x78,0x21,0x41,0x25,0x44,0x2A,0x47},
	[VOTE_CARD_CONFIG].program_key={0x56,0x6B,0x59,0x70,0x33,0x73,0x36,0x76},
	[VOTE_CARD_CONFIG].zone_read_pwd[0]={0xFF,0x13,0x23},
	[VOTE_CARD_CONFIG].zone_write_pwd[0]={0xFF,0x13,0x23},
	[VOTE_CARD_CONFIG].zone_read_pwd[1]={0xB8,0xE7,0x8F},
	[VOTE_CARD_CONFIG].zone_write_pwd[1]={0xB8,0xE7,0x8F},
	[VOTE_CARD_CONFIG].zone_read_pwd[2]={0x4C,0xA8,0x64},
	[VOTE_CARD_CONFIG].zone_write_pwd[2]={0x4C,0xA8,0x64},
	[VOTE_CARD_CONFIG].zone_read_pwd[3]={0x6D,0x05,0x7F},
	[VOTE_CARD_CONFIG].zone_write_pwd[3]={0x6D,0x05,0x7F},
	[VOTE_CARD_CONFIG].write7_pwd={0x66,0x0E,0x00},
	[VOTE_CARD_CONFIG].id_number={0},
	[VOTE_CARD_CONFIG].issuer_code={0},
	[VOTE_CARD_CONFIG].answer_reset={0x70,0x7E,0xCA,0x9D,0x70,0x7E,0xCA,0x9D},
	[VOTE_CARD_CONFIG].block_num = 4,
	[VOTE_CARD_CONFIG].block_size = 128,
	[VOTE_CARD_CONFIG].password = AT88SC0404C_PWD,
	
	[ADMIN_CARD_CONFIG].auth_key={0x77,0x30,0xfe,0x78,0x3c,0x9a,0x6a,0x66},
	[ADMIN_CARD_CONFIG].program_key={0x9a,0x98,0x44,0x16,0x6c,0x6e,0xe0,0x81},
	[ADMIN_CARD_CONFIG].zone_read_pwd[0]={0xEE,0x81,0xB8},
	[ADMIN_CARD_CONFIG].zone_write_pwd[0]={0xA3,0x78,0x1C},
	[ADMIN_CARD_CONFIG].zone_read_pwd[1]={0xEE,0x81,0xB8},
	[ADMIN_CARD_CONFIG].zone_write_pwd[1]={0xA3,0x78,0x1C},
	[ADMIN_CARD_CONFIG].zone_read_pwd[2]={0xEE,0x81,0xB8},
	[ADMIN_CARD_CONFIG].zone_write_pwd[2]={0xA3,0x78,0x1C},
	[ADMIN_CARD_CONFIG].zone_read_pwd[3]={0xEE,0x81,0xB8},
	[ADMIN_CARD_CONFIG].zone_write_pwd[3]={0xA3,0x78,0x1C},
	[ADMIN_CARD_CONFIG].write7_pwd={0x82,0xE1,0x0E},
	[ADMIN_CARD_CONFIG].id_number={0},
	[ADMIN_CARD_CONFIG].issuer_code={0},
	[ADMIN_CARD_CONFIG].answer_reset={0xD4,0x62,0x08,0x2A,0x85,0x9D,0x27,0xEC},
	[ADMIN_CARD_CONFIG].block_num = 4,
	[ADMIN_CARD_CONFIG].block_size = 128,
	[ADMIN_CARD_CONFIG].password = AT88SC0404C_PWD,
	
	[MAINTE_CARD_CONFIG].auth_key={0x78,0xcc,0x67,0x92,0x6a,0xb3,0xbe,0x5b},
	[MAINTE_CARD_CONFIG].program_key={0xb8,0x0a,0x78,0xef,0x47,0xdb,0x9a,0xdd},
	[MAINTE_CARD_CONFIG].zone_read_pwd[0]={0xD1,0x3A,0x97},
	[MAINTE_CARD_CONFIG].zone_write_pwd[0]={0x68,0x74,0x35},
	[MAINTE_CARD_CONFIG].zone_read_pwd[1]={0xD1,0x3A,0x97},
	[MAINTE_CARD_CONFIG].zone_write_pwd[1]={0x68,0x74,0x35},
	[MAINTE_CARD_CONFIG].zone_read_pwd[2]={0xD1,0x3A,0x97},
	[MAINTE_CARD_CONFIG].zone_write_pwd[2]={0x68,0x74,0x35},
	[MAINTE_CARD_CONFIG].zone_read_pwd[3]={0xD1,0x3A,0x97},
	[MAINTE_CARD_CONFIG].zone_write_pwd[3]={0x68,0x74,0x35},
	[MAINTE_CARD_CONFIG].write7_pwd={0xD1,0x1C,0x10},
	[MAINTE_CARD_CONFIG].id_number={0},
	[MAINTE_CARD_CONFIG].issuer_code={0},
	[MAINTE_CARD_CONFIG].answer_reset={0x21,0xF0,0x54,0xC2,0xE3,0x89,0x3B,0x3A},
	[MAINTE_CARD_CONFIG].block_num = 4,
	[MAINTE_CARD_CONFIG].block_size = 128,
	[MAINTE_CARD_CONFIG].password = AT88SC0404C_PWD,
	
	[SYSTEM_CARD_CONFIG].auth_key={0x5a,0x55,0x99,0x78,0xde,0x65,0x07,0x3d},
	[SYSTEM_CARD_CONFIG].program_key={0x4c,0x03,0x58,0x1e,0x64,0x57,0x31,0xf4},
	[SYSTEM_CARD_CONFIG].zone_read_pwd[0]={0x71,0x8E,0x9D},
	[SYSTEM_CARD_CONFIG].zone_write_pwd[0]={0x50,0x0C,0xCB},
	[SYSTEM_CARD_CONFIG].zone_read_pwd[1]={0x71,0x8E,0x9D},
	[SYSTEM_CARD_CONFIG].zone_write_pwd[1]={0x50,0x0C,0xCB},
	[SYSTEM_CARD_CONFIG].zone_read_pwd[2]={0x71,0x8E,0x9D},
	[SYSTEM_CARD_CONFIG].zone_write_pwd[2]={0x50,0x0C,0xCB},
	[SYSTEM_CARD_CONFIG].zone_read_pwd[3]={0x71,0x8E,0x9D},
	[SYSTEM_CARD_CONFIG].zone_write_pwd[3]={0x50,0x0C,0xCB},
	[SYSTEM_CARD_CONFIG].write7_pwd={0x3F,0x18,0x24},
	[SYSTEM_CARD_CONFIG].id_number={0},
	[SYSTEM_CARD_CONFIG].issuer_code={0},
	[SYSTEM_CARD_CONFIG].answer_reset={0x17,0x26,0x8A,0x25,0xE3,0x7F,0x6C,0x11},
	[SYSTEM_CARD_CONFIG].block_num = 4,
	[SYSTEM_CARD_CONFIG].block_size = 128,
	[SYSTEM_CARD_CONFIG].password = AT88SC0404C_PWD,
	
	[BACKUP_CARD_CONFIG].auth_key={0x74,0x6B,0x15,0x0C,0xCF,0x56,0x9F,0xF5},
	[BACKUP_CARD_CONFIG].program_key={0x5D,0x1A,0x03,0xD5,0x85,0xEE,0x8C,0x49},
	[BACKUP_CARD_CONFIG].zone_read_pwd[0]={0x9B,0x1E,0xAC},
	[BACKUP_CARD_CONFIG].zone_write_pwd[0]={0x9B,0x1E,0xAC},
	[BACKUP_CARD_CONFIG].zone_read_pwd[1]={0},
	[BACKUP_CARD_CONFIG].zone_write_pwd[1]={0},
	[BACKUP_CARD_CONFIG].zone_read_pwd[2]={0},
	[BACKUP_CARD_CONFIG].zone_write_pwd[2]={0},
	[BACKUP_CARD_CONFIG].zone_read_pwd[3]={0},
	[BACKUP_CARD_CONFIG].zone_write_pwd[3]={0},
	[BACKUP_CARD_CONFIG].write7_pwd={0x00,0xB8,0x9A},
	[BACKUP_CARD_CONFIG].id_number={0},
	[BACKUP_CARD_CONFIG].issuer_code={0},
	[BACKUP_CARD_CONFIG].answer_reset={0x1B,0x03,0x62,0x3A,0xAC,0x6A,0x63,0xBE},
	[BACKUP_CARD_CONFIG].block_num = 4,
	[BACKUP_CARD_CONFIG].block_size = 128,
	[BACKUP_CARD_CONFIG].password = AT88SC0404C_PWD,
};

const SMARTCARD_AUTH_PWD_T smartcard_auth[CARD_CONFIG_MAX]={
	[VOTE_CARD_CONFIG].block[0]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[1]  = (SC_GC0_SEL<<4)|SC_PW1_SEL,
	[VOTE_CARD_CONFIG].block[2]  = (SC_GC0_SEL<<4)|SC_PW2_SEL,
	[VOTE_CARD_CONFIG].block[3]  = (SC_GC0_SEL<<4)|SC_PW3_SEL,
	[VOTE_CARD_CONFIG].block[4]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[5]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[6]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[7]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[8]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[9]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[10] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[11] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[12] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[13] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[14] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[VOTE_CARD_CONFIG].block[15] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	
	[ADMIN_CARD_CONFIG].block[0]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[1]  = (SC_GC0_SEL<<4)|SC_PW1_SEL,
	[ADMIN_CARD_CONFIG].block[2]  = (SC_GC0_SEL<<4)|SC_PW2_SEL,
	[ADMIN_CARD_CONFIG].block[3]  = (SC_GC0_SEL<<4)|SC_PW3_SEL,
	[ADMIN_CARD_CONFIG].block[4]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[5]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[6]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[7]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[8]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[9]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[10] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[11] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[12] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[13] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[14] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[ADMIN_CARD_CONFIG].block[15] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	
	[MAINTE_CARD_CONFIG].block[0]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[1]  = (SC_GC0_SEL<<4)|SC_PW1_SEL,
	[MAINTE_CARD_CONFIG].block[2]  = (SC_GC0_SEL<<4)|SC_PW2_SEL,
	[MAINTE_CARD_CONFIG].block[3]  = (SC_GC0_SEL<<4)|SC_PW3_SEL,
	[MAINTE_CARD_CONFIG].block[4]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[5]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[6]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[7]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[8]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[9]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[10] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[11] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[12] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[13] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[14] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[MAINTE_CARD_CONFIG].block[15] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	
	[SYSTEM_CARD_CONFIG].block[0]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[1]  = (SC_GC0_SEL<<4)|SC_PW1_SEL,
	[SYSTEM_CARD_CONFIG].block[2]  = (SC_GC0_SEL<<4)|SC_PW2_SEL,
	[SYSTEM_CARD_CONFIG].block[3]  = (SC_GC0_SEL<<4)|SC_PW3_SEL,
	[SYSTEM_CARD_CONFIG].block[4]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[5]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[6]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[7]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[8]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[9]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[10] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[11] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[12] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[13] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[14] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[SYSTEM_CARD_CONFIG].block[15] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	
	[BACKUP_CARD_CONFIG].block[0]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[1]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[2]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[3]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[4]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[5]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[6]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[7]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[8]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[9]  = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[10] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[11] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[12] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[13] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[14] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
	[BACKUP_CARD_CONFIG].block[15] = (SC_GC0_SEL<<4)|SC_PW0_SEL,
};

void HAL_SMARTCARD_MspInit(SMARTCARD_HandleTypeDef* hsmartcard)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hsmartcard->Instance==SC_USARTx)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();
  
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration    
    PB12     ------> USART3_CK
    PD8     ------> USART3_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	HAL_NVIC_SetPriority(SC_USARTx_IRQn,SC_USART_IRQ_PREPRI,SC_USART_IRQ_SUBPRI);

    /* USART3 interrupt Init */
//    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }

}

void HAL_SMARTCARD_MspDeInit(SMARTCARD_HandleTypeDef* hsmartcard)
{

//if(hsmartcard->Instance==SC_USARTx)
//{
///* USER CODE BEGIN USART3_MspDeInit 0 */

///* USER CODE END USART3_MspDeInit 0 */
///* Peripheral clock disable */
//__HAL_RCC_USART3_CLK_DISABLE();

///**USART3 GPIO Configuration    
//PB12     ------> USART3_CK
//PD8     ------> USART3_TX 
//*/
//HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);

//HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8);

///* USART3 interrupt DeInit */
//HAL_NVIC_DisableIRQ(SC_USARTx_IRQn);
///* USER CODE BEGIN USART3_MspDeInit 1 */

///* USER CODE END USART3_MspDeInit 1 */
//}

}

/** 
* 智能卡发送完成中断回调函数. 
* 设置发送完成标记位. 
* @param[in]   hsc:智能卡句柄. 
* @param[out]  none.  
* @retval  none 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-05创建 
*/
void HAL_SMARTCARD_TxCpltCallback(SMARTCARD_HandleTypeDef *hsc)
{
	if(hsc->Instance==SC_USARTx){
		
		HAL_SMARTCARD_AbortTransmit_IT(hsc);
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xEventGroupSetBitsFromISR(smartcard_event_handler,SMARTCARD_SEND_OK,&xHigherPriorityTaskWoken);
	}
}

/** 
* 智能卡接收完成中断回调函数. 
* 设置接收完成标记位. 
* @param[in]   hsc:智能卡句柄. 
* @param[out]  none.  
* @retval  none 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-05创建 
*/
void HAL_SMARTCARD_RxCpltCallback(SMARTCARD_HandleTypeDef *hsc)
{
	if(hsc->Instance==SC_USARTx){
		
		HAL_SMARTCARD_AbortReceive_IT(hsc);
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xEventGroupSetBitsFromISR(smartcard_event_handler,SMARTCARD_RECV_OK,&xHigherPriorityTaskWoken);
	}
}

/** 
* 智能卡错误中断回调函数. 
* 无. 
* @param[in]   hsc:智能卡句柄. 
* @param[out]  none.  
* @retval  none 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-05创建 
*/
void HAL_SMARTCARD_ErrorCallback(SMARTCARD_HandleTypeDef *hsc)
{
	if(hsc->Instance==SC_USARTx){
		
		HAL_SMARTCARD_AbortTransmit_IT(hsc);
		HAL_SMARTCARD_AbortReceive_IT(hsc);
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xEventGroupSetBitsFromISR(smartcard_event_handler,SMARTCARD_ERROR,&xHigherPriorityTaskWoken);
	}
}

/** 
* 智能卡中断向量处理. 
* 无. 
* @param[in]   none. 
* @param[out]  none.  
* @retval  none 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-05创建 
*/
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_SMARTCARD_IRQHandler(&smartcard_manage[0].handle);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/** 
* 设置智能卡接口接收或输出模式. 
* 初始化接口卡接口的配置参数. 
* @param[in]   mdoe:模式. 
* @param[out]  none.  
* @retval  none 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-05创建 
*/
void sc_usart_mode(CARD_MANAGE_T *p_card_manage,uint32_t mode)
{
	p_card_manage->handle.Instance         = SC_USARTx;
	p_card_manage->handle.Init.Mode        = mode;
	p_card_manage->handle.Init.Parity      = SMARTCARD_PARITY_EVEN;
	p_card_manage->handle.Init.BaudRate    = SC_USART_BDRATE;
	p_card_manage->handle.Init.StopBits    = SMARTCARD_STOPBITS_1_5;
	p_card_manage->handle.Init.CLKPhase    = SMARTCARD_PHASE_1EDGE;
	p_card_manage->handle.Init.Prescaler   = SC_USART_PRESCALER;
	p_card_manage->handle.Init.GuardTime   = SC_USART_GUARDTIME;
	p_card_manage->handle.Init.NACKState   = SMARTCARD_NACK_ENABLE;
	p_card_manage->handle.Init.CLKLastBit  = SMARTCARD_LASTBIT_ENABLE;
	p_card_manage->handle.Init.WordLength  = SMARTCARD_WORDLENGTH_9B;
	p_card_manage->handle.Init.CLKPolarity = SMARTCARD_POLARITY_LOW;
	
	if (HAL_SMARTCARD_Init(&p_card_manage->handle)!= HAL_OK){
	}
}

/** 
* 智能卡发送函数. 
* 等待发送完成中断回调函数设置发送完成标志. 
* @param[in]   p_data:发送数据指针，len:发送数据长度. 
* @param[out]  none.  
* @retval  none 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-05创建 
*/
BaseType_t sc_send(CARD_MANAGE_T *p_card_manage,uint8_t *p_data, uint8_t len)
{
	EventBits_t event_bits=0;
//	sc_usart_mode(SMARTCARD_MODE_TX);
//	HAL_SMARTCARD_Transmit_IT(&hsc3, p_data, len);
//	event_bits = xEventGroupWaitBits(sc_event_handler,SMARTCARD_SEND_OK|SMARTCARD_ERROR,pdTRUE,pdFALSE,1000);
//	if( event_bits&SMARTCARD_SEND_OK )
//		return pdTRUE;
//	else
//		return pdFALSE;
	
	if( HAL_SMARTCARD_Transmit(&p_card_manage->handle, p_data, len, SC_TRANSMIT_TIMEOUT) == HAL_OK )
		return pdTRUE;
	else
		return pdFALSE;
}

/** 
* 智能卡接收函数. 
* 等待接收完成中断回调函数设置接收完成标志. 
* @param[in]   p_data:接收数据缓冲指针，len:润滑数据长度. 
* @param[out]  none.  
* @retval  none 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-05创建 
*/
FUN_STATUS_T sc_recv(CARD_MANAGE_T *p_card_manage,uint8_t *p_data, uint8_t len)
{
	if( HAL_SMARTCARD_Receive(&p_card_manage->handle, p_data, len,SC_RECEIVE_TIMEOUT) == HAL_OK)
		return FUN_OK;
	else
		return FUN_ERROR;
//	HAL_SMARTCARD_Receive_IT(&hsc3, p_data, len);
//	event_bits = xEventGroupWaitBits(sc_event_handler,SMARTCARD_RECV_OK|SMARTCARD_ERROR,pdTRUE,pdFALSE,1000);
//	if( event_bits&SMARTCARD_RECV_OK )
//		return pdTRUE;
//	else
//		return pdFALSE;
}

/** 
* 检测到卡片插入复位智能卡句柄参数. 
* 阻塞式读取，不成功再读一次. 
* @param[in]   p_cardManageHandle：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  none 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-05-09创建 
*/
static void samrtcard_reset_para(CARD_MANAGE_T *p_card_manage)
{
	uint8_t i = 0u;

	/* Reset Data from SC buffer                                                                                        */

	memset(p_card_manage->sc_atr_table,0,ATR_BUFFER_SIZE);
	p_card_manage->sc_atr_len   = 0u;

	/* Reset cardAtr Structure                                                                                          */
	p_card_manage->card_atr.ts = 0;
	p_card_manage->card_atr.t0 = 0;

	memset(p_card_manage->card_atr.t,0,ATR_BUFFER_SIZE);
	memset(p_card_manage->card_atr.h,0,ATR_BUFFER_SIZE);
	p_card_manage->sc_inverse_flag   = false;
	p_card_manage->card_atr.t_length = 0;
	p_card_manage->card_atr.h_length = 0;
}

void smartcard_deinit(CARD_MANAGE_T *p_card_manage)
{
  /* Disable CMDVCC                                                                                                   */
	ltc1955_power(p_card_manage->card_port, VCC_0V);
	ltc1955_reset(p_card_manage->card_port, 0);
	//election_status.card_type  = CARD_PRESENT_NONE;
	p_card_manage->card_state  = SC_POWER_OFF;
	p_card_manage->card_status = CARD_NOT_INSERTED;
	p_card_manage->card_present = CARD_PRESENT_NONE;
//	HAL_SMARTCARD_DeInit(&p_card_manage->handle);    /* Deinitializes the g_smartCardHandle                                 */
}

/** 
* 拔出卡后，关闭菜单显示切换信息显示. 
* 无. 
* @param[in]   p_card_manage:智能卡接口句柄. 
* @param[out]  无.  
* @retval  无。
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
void sc_popup_card_menu(CARD_MANAGE_T *p_card_manage)
{
	if( p_card_manage->card_port == CARD_A ){
		display_exit_to_info();
		ioc_keypad_led_off();
	}
}

/** 
* 明文读取智能卡数据. 
* 无. 
* @param[in]   p_card_manage:智能卡接口句柄,rd_cmd:命令，A1：参数1，A2;参数2，N：长度. 
* @param[out]  recv_ptr:返回数据指针.  
* @retval  无。
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T sc_read_paintext(CARD_MANAGE_T *card_manage,uint8_t rd_cmd,uint8_t A1,uint8_t A2,uint8_t N, uint8_t *recv_ptr)
{	
	if( N+4 < SC_READ_BUF_MAX_SIZE  ){
		
		FUN_STATUS_T scStatus;
			
		card_manage->card_apdu_cmd.header.cla = SC_CLA_CODE;
		card_manage->card_apdu_cmd.header.ins = rd_cmd;
		card_manage->card_apdu_cmd.header.p1  = A1;
		card_manage->card_apdu_cmd.header.p2  = A2;
		card_manage->card_apdu_cmd.body.lc    = N;
		card_manage->card_apdu_cmd.body.le    = READ_FROM_SC;

		scStatus = sc_transmit_data(card_manage,false);
	
		if( scStatus == FUN_OK ){
		
			memcpy(recv_ptr,card_manage->card_apdu_resp.data,N);
			return FUN_OK;
		}
		else{
			return FUN_ERROR;
		}
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 读取寄存器数据. 
* 无. 
* @param[in]   p_card_manage:智能卡接口句柄,reg:寄存器，ptr:数据. 
* @param[out]  无.  
* @retval  无。
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T sc_read_config_register(CARD_MANAGE_T *p_card_manage, uint16_t reg, uint8_t *ptr)
{
	
	uint8_t addr,len;
	addr = reg&0xff;
	len = reg>>8;
	return sc_read_paintext(p_card_manage, SC_SYSTEM_READ_CMD,0x00,addr,len,ptr); 
}

/** 
* 明文写入智能卡数据. 
* 无. 
* @param[in]   p_card_manage:智能卡接口句柄,rd_cmd:命令，A1：参数1，A2;参数2，N：长度，send_ptr:返回数据指针. 
* @param[out]  无.  
* @retval  无。
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T sc_write_paintext(CARD_MANAGE_T *p_card_manage,uint8_t wr_cmd,uint8_t A1,uint8_t A2,uint8_t N,uint8_t *send_ptr)
{
	if( N+4 < SC_BUF_MAX_SIZE  ){
		
		FUN_STATUS_T scStatus;
			
		p_card_manage->card_apdu_cmd.header.cla = SC_CLA_CODE;
		p_card_manage->card_apdu_cmd.header.ins = wr_cmd;
		p_card_manage->card_apdu_cmd.header.p1  = A1;
		p_card_manage->card_apdu_cmd.header.p2  = A2;
		p_card_manage->card_apdu_cmd.body.lc    = N;
		p_card_manage->card_apdu_cmd.body.le    = WRITE_TO_SC;
		memcpy(p_card_manage->card_apdu_cmd.body.data,send_ptr,N);
		scStatus = sc_transmit_data(p_card_manage,false);

		if( scStatus == HAL_OK ){
			return FUN_OK;
		}
		else{
			return FUN_ERROR;
		}
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 测试智能卡正常通讯. 
* 组织成at88sc协议再调用T0协议发送到智能卡. 
* @param[in]   card_manage：智能卡句柄，wd_ptr：读的数据. 
* @param[out]  none.  
* @retval  1：成功，0：失败 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-05-09创建 
*/
FUN_STATUS_T sc_write_config_register(CARD_MANAGE_T *p_card_manage, uint16_t reg, uint8_t *ptr){
	
	uint8_t addr,len;
	addr = reg&0xff;
	len  = reg>>8;
	return sc_write_paintext(p_card_manage, SC_SYSTEM_WRITE_CMD,0x00,addr,len,ptr); 
}

/** 
* 测试智能卡正常通讯. 
* 组织成at88sc协议再调用T0协议发送到智能卡. 
* @param[in]   card_manage：智能卡句柄，wd_ptr：读的数据. 
* @param[out]  none.  
* @retval  1：成功，0：失败 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-05-09创建 
*/
FUN_STATUS_T sc_mtz_test(CARD_MANAGE_T *p_card_manage){
	
	uint8_t send_buf[2];
	uint8_t recv_buf[4];
	send_buf[0]=0xa5;
	send_buf[1]=0x5a;
	sc_write_config_register(p_card_manage,MTZ_R,send_buf);
	sc_read_config_register(p_card_manage,MTZ_R,recv_buf);
	
	if (recv_buf[0]==0xa5 && recv_buf[1]==0x5a ){
		return FUN_OK;
	}
	else
		return FUN_ERROR;
}

/** 
* 智能卡读函数. 
* 组织成at88sc协议再调用T0协议发送到智能卡. 
* @param[in]   card_manage：智能卡句柄，wd_ptr：读的数据. 
* @param[out]  none.  
* @retval  1：成功，0：失败 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-05-09创建 
*/
static FUN_STATUS_T sc_read(CARD_MANAGE_T *p_card_manage,uint8_t  *rd_ptr){
	
	FUN_STATUS_T scStatus;
		
	p_card_manage->card_apdu_cmd.header.cla = SC_CLA_CODE;
	p_card_manage->card_apdu_cmd.header.ins = rd_ptr[0];
	p_card_manage->card_apdu_cmd.header.p1  = rd_ptr[1];
	p_card_manage->card_apdu_cmd.header.p2  = rd_ptr[2];
	p_card_manage->card_apdu_cmd.body.lc    = rd_ptr[3];
	p_card_manage->card_apdu_cmd.body.le    = READ_FROM_SC;

	scStatus = sc_transmit_data(p_card_manage,false);
	
	if( scStatus==HAL_OK ){
		memcpy(&rd_ptr[4],p_card_manage->card_apdu_resp.data,rd_ptr[3]);
	}
	
	if( scStatus==HAL_OK ){
		return FUN_OK;
	}
	else
	{
		return FUN_ERROR;
	}
}

/** 
* 智能卡写函数. 
* 组织成at88sc协议再调用T0协议发送到智能卡. 
* @param[in]   card_manage：智能卡句柄，wd_ptr：写的数据. 
* @param[out]  none.  
* @retval  1：成功，0：失败 
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-05-09创建 
*/
static FUN_STATUS_T sc_write(CARD_MANAGE_T *p_card_manage,uint8_t *wd_ptr){
	
	FUN_STATUS_T scStatus;

	/* Unlock configuration zone by writing secure code                                                                 */
	p_card_manage->card_apdu_cmd.header.cla = SC_CLA_CODE;
	p_card_manage->card_apdu_cmd.header.ins = wd_ptr[0];
	p_card_manage->card_apdu_cmd.header.p1  = wd_ptr[1];
	p_card_manage->card_apdu_cmd.header.p2  = wd_ptr[2];
	p_card_manage->card_apdu_cmd.body.lc    = wd_ptr[3];
	p_card_manage->card_apdu_cmd.body.le    = WRITE_TO_SC;
	
	if( wd_ptr[3]<=SC_APDU_LC_MAX )
		memcpy(p_card_manage->card_apdu_cmd.body.data,&wd_ptr[4],wd_ptr[3]);
	else
		return FUN_ERROR;

	scStatus = sc_transmit_data(p_card_manage,false);
	
	if( scStatus==HAL_OK )
		return FUN_OK;
	else{
		return FUN_ERROR;
	}
}

/** 
* authentication arithmetic function. 
* Functions Brief:i don`t know. 
* @param[in]   i don`t know.
* @param[out]  noen.
* @retval  none
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
void sc_gpa_clock(uint8_t Datain,uint8_t times){
	
	uint8_t t;
	uint8_t d;
	uint8_t din_gpa;
	uint8_t Ri;
	uint8_t R_Sum;
	uint8_t Si;
	uint8_t S_Sum;
	uint8_t Ti;
	uint8_t T_Sum;

	for(t=0x00;t<times;t++){
		
		din_gpa=Datain ^ GPA[0];
		Ri= din_gpa&0x1f;
		Si= ((din_gpa&0x0f)<<3)+((din_gpa&0xe0)>>5);
		Ti=(din_gpa&0xf8)>>3;
		
		//r parameter
		if(((GPA[4])+((GPA[1]&0x0f)<<1)+((GPA[1]&0x10)>>4))>31){
			R_Sum=((GPA[4])+((GPA[1]&0x0f)<<1)+((GPA[1]&0x10)>>4))-31;
		}
		else{
			R_Sum=((GPA[4])+((GPA[1]&0x0f)<<1)+((GPA[1]&0x10)>>4));
		}

		GPA[1]=GPA[2];
		GPA[2]=GPA[3];
		GPA[3]=GPA[4];
		GPA[4]=GPA[5]^Ri;
		GPA[5]=GPA[6];
		GPA[6]=GPA[7];
		GPA[7]=R_Sum;
		
		//s parameter
		if ((GPA[9]+((GPA[8]&0x3f)<<1)+((GPA[8]&0x40)>>6) )>127){
			S_Sum=( (GPA[9]) + ((GPA[8]&0x3f)<<1)+((GPA[8]&0x40)>>6) )-127;
		}
		else{
			S_Sum= (GPA[9]) + ((GPA[8]&0x3f)<<1)+((GPA[8]&0x40)>>6) ;
		}

		GPA[8]=GPA[9];
		GPA[9]=Si^GPA[10];
		GPA[10]=GPA[11];
		GPA[11]=GPA[12];
		GPA[12]=GPA[13];
		GPA[13]=GPA[14];
		GPA[14]=S_Sum;
		
		//t parameter
		if ((GPA[15]+GPA[17])> 31){
			T_Sum=GPA[15]+GPA[17]-31;
		}
		else{
			T_Sum=GPA[15]+GPA[17];
		}

		GPA[15]=GPA[16];
		GPA[16]=GPA[17];
		GPA[17]=GPA[18]^Ti;
		GPA[18]=GPA[19];
		GPA[19]=T_Sum;
		
		///Output
		if((GPA[14]&0x01)==0){
			d=((GPA[7]^GPA[3])&0x01);
		}
		else{
			d=((GPA[19]^GPA[16])&0x01);
		}

		if((GPA[14]&0x02)==0){
			d=d+((GPA[7]^GPA[3])&0x02);
		}
		else{
			d=d+((GPA[19]^GPA[16])&0x02);
		}

		if((GPA[14]&0x04)==0){
			d=d+((GPA[7]^GPA[3])&0x04);
		}
		else{
			d=d+((GPA[19]^GPA[16])&0x04);
		}

		if((GPA[14]&0x08)==0){
			d=d+((GPA[7]^GPA[3])&0x08);
		}
		else{
			d=d+((GPA[19]^GPA[16])&0x08);
		}

		GPA[0]= ( (((GPA[0])&0x0f)<<4) +d);
	}
}

/** 
* authentication the selected gc code. 
* authentication the selected gc code. 
* @param[in]   GC_select:gc code num.
* @param[out]  noen.
* @retval  return 0xff:success, other:failed
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
FUN_STATUS_T sc_authentication(CARD_MANAGE_T *p_card_manage,uint8_t GC_select,uint8_t *gc){

	#ifdef SC_NO_NEED_AUTHENTICATION
	return 0xff;
	#else
	
	uint8_t SK[8];
	uint8_t Q_CH[0x14];
	uint8_t j;

	for(j=0;j<=19;j++){
		GPA[j]=0x00;
	}

	Q_CH[0]=0xb6;
	Q_CH[1]=0x00;
	Q_CH[2]=GC_select<<4;
	Q_CH[2]=Q_CH[2]+0x50;
	Q_CH[3]=0x08;
	if( sc_read(p_card_manage,Q_CH) != FUN_OK)
		return FUN_ERROR;

	for(j=0;j<8;j++){
		Q_CH[j+12]=Q_CH[j+4];//CI
	}

	for(j=0;j<8;j++){
		Q_CH[j+4]=0xAA;      //$$can modify random number
	}

	for(j=0;j<4;j++){
		
	   sc_gpa_clock(Q_CH[12+2*j],0x03);
	   sc_gpa_clock(Q_CH[12+2*j+1],0x03);
	   sc_gpa_clock(Q_CH[4+j],0x01);
	}

	for(j=0;j<8;j++){
		Q_CH[12+j]=gc[j];
	}

	for(j=0;j<4;j++){
		
	   sc_gpa_clock(Q_CH[12+2*j],0x03);
	   sc_gpa_clock(Q_CH[12+2*j+1],0x03);
	   sc_gpa_clock(Q_CH[8+j],0x01);
	}
	sc_gpa_clock(0x00,0x06);
	Q_CH[12]=GPA[0];
	
	for(j=1;j<8;j++){
		
		sc_gpa_clock(0x00,0x07);
		Q_CH[12+j]=GPA[0];
	}

	Q_CH[0]=0xb8;//send ch
	Q_CH[2]=0x00;
	Q_CH[3]=0x10;
	Q_CH[1]=GC_select;
	if( sc_write(p_card_manage,Q_CH) != FUN_OK)
		return FUN_ERROR;

	//new ci
	Q_CH[12]=0xFF;
	for(j=1;j<8;j++){
		
		sc_gpa_clock(0x00,0x02);
		Q_CH[12+j]=GPA[0];
	}
	//new sk
	for(j=0;j<8;j++){
		
		sc_gpa_clock(0x00,0x02);
		SK[j]=GPA[0];
	}
	sc_gpa_clock(0x00,0x03);

	Q_CH[0]=0XB6;
	Q_CH[1]=0X00;
	Q_CH[2]=GC_select<<4;
	Q_CH[2]=Q_CH[2]+0x50;
	Q_CH[3]=0x08;
	if( sc_read(p_card_manage,Q_CH) != FUN_OK) 
		return FUN_ERROR;
	if (Q_CH[4]!=0xff){
		goto Aut_Report;
	}

	for(j=0;j<8;j++){
		
		if(Q_CH[4+j]!= Q_CH[12+j]){
			goto Aut_Report;
		}
	}

	sc_gpa_clock(0x00,0x05);
	sc_gpa_clock(Q_CH[2],0x01);
	sc_gpa_clock(0x00,0x05);
	sc_gpa_clock(0x08,0x01);
	for(j=0;j<8;j++){
		
		sc_gpa_clock(Q_CH[4+j],0x01);
		sc_gpa_clock(0x00,0x05);
	}

	//crypto_Authentication
	for(j=0;j<=19;j++){
		GPA[j]=0x00;
	}

	for(j=0;j<8;j++){
		Q_CH[j+4]=0xAA;      //$$can modify random number
	}
	
	for(j=0;j<4;j++){
		
		sc_gpa_clock(Q_CH[12+2*j],0x03);
		sc_gpa_clock(Q_CH[12+2*j+1],0x03);
		sc_gpa_clock(Q_CH[4+j],0x01);
	}

	for(j=0;j<4;j++){
		
		sc_gpa_clock(SK[2*j],0x03);
		sc_gpa_clock(SK[2*j+1],0x03);
		sc_gpa_clock(Q_CH[8+j],0x01);
	}

	sc_gpa_clock(0x00,0x06);
	Q_CH[12]=GPA[0];
	for(j=1;j<8;j++){
		
		sc_gpa_clock(0x00,0x07);
		Q_CH[12+j]=GPA[0];
	}

	Q_CH[0]=0xb8;
	Q_CH[1]=GC_select+0x10;
	Q_CH[2]=0x00;
	Q_CH[3]=0x10;
	if( sc_write(p_card_manage,Q_CH) != FUN_OK )
		return FUN_ERROR;

	//new ci
	Q_CH[12]=0xFF;
	for(j=1;j<8;j++){
		
		sc_gpa_clock(0x00,0x02);
		Q_CH[12+j]=GPA[0];
	}
	//new sk
	for(j=0;j<8;j++){
		
		sc_gpa_clock(0x00,0x02);
		SK[j]=GPA[0];
	}
	sc_gpa_clock(0x00,0x03);

	Q_CH[0]=0XB6;
	Q_CH[1]=0X00;
	Q_CH[2]=GC_select<<4;
	Q_CH[2]=Q_CH[2]+0x50;
	Q_CH[3]=0x08;
	if( sc_read(p_card_manage,Q_CH) != FUN_OK)
		return FUN_ERROR;
	if (Q_CH[4]!=0xff){
		goto Aut_Report;
	}

	for(j=0;j<8;j++){
		if(Q_CH[4+j]!= Q_CH[12+j]){
			goto Aut_Report;
		}
	}

	sc_gpa_clock(0x00,0x05);
	sc_gpa_clock(Q_CH[2],0x01);
	sc_gpa_clock(0x00,0x05);
	sc_gpa_clock(0x08,0x01);
	for(j=0;j<8;j++){
		
		sc_gpa_clock(Q_CH[4+j],0x01);
		sc_gpa_clock(0x00,0x05);
	}
	Aut_Report:
//	return Q_CH[4];
	return FUN_OK;
	#endif
}

/** 
* verify the user zone write password. 
* verify the user zone write password. 
* @param[in]   pw_select:write code num.
* @param[out]  noen.
* @retval  return 0xff:success, other:failed
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
FUN_STATUS_T sc_verify_write_password(CARD_MANAGE_T *p_card_manage,uint8_t pw_select,uint8_t *word){
	
	#ifdef SC_NO_NEED_PASSWORD
//	return 0xff;
	return FUN_OK;
	#else
	
	uint8_t j;
	uint8_t pw[7];

	for(j=0;j<3;j++)
		{pw[4+j]=word[j];}

	for(j=0;j<3;j++){
		sc_gpa_clock(pw[4+j],0x05);
		pw[4+j]=GPA[0];
	}
	pw[0]=0xba;
	pw[1]=pw_select&0x0f;
	pw[2]=0x00;
	pw[3]=0x03;
	if( sc_write(p_card_manage,pw) != FUN_OK )
		return FUN_ERROR;

	pw[0]=0XB6;
	pw[1]=0X00;
	pw[2]=pw_select&0x0f;
	pw[2]=pw[2]*0x08;
	pw[2]=pw[2]+0xb0;
	pw[3]=0x01;
	if( sc_read(p_card_manage,pw) != FUN_OK)
		return FUN_ERROR;

	sc_gpa_clock(0x00,0x05);
	sc_gpa_clock(pw[2],0x01);
	sc_gpa_clock(0x00,0x05);
	sc_gpa_clock(0x01,0x01);
	pw[4]=pw[4]^GPA[0];
	sc_gpa_clock(pw[4],0x01);
	sc_gpa_clock(0x00,0x05);

//	return pw[4];
	return FUN_OK;
	#endif
}

/** 
* verify the user zone read password. 
* verify the user zone read password. 
* @param[in]   pw_select:read code num.
* @param[out]  noen.
* @retval  return 0xff:success, other:failed
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
FUN_STATUS_T sc_verify_read_password(CARD_MANAGE_T *p_card_manage,uint8_t pw_select,uint8_t *word){
	
	uint8_t j;
	uint8_t pw[7];
//	if ((pw_select&0x0f)==0x00){
//		for(j=0;j<3;j++){
//			pw[4+j]=PW_READ0[j];
//		}
//	}//{*(p+42+j)=PW_WRITE0[j];}}
//	else if ((pw_select&0x0f)==0x01)
//		for(j=0;j<3;j++){
//			pw[4+j]=PW_READ1[j];
//		}
//	else if ((pw_select&0x0f)==0x02)
//		for(j=0;j<3;j++){
//			pw[4+j]=PW_READ2[j];
//		}
//	else if ((pw_select&0x0f)==0x03)
//		for(j=0;j<3;j++){
//			pw[4+j]=PW_READ3[j];
//		}
//	else if ((pw_select&0x0f)==0x04)
//		for(j=0;j<3;j++){
//			pw[4+j]=PW_READ4[j];
//		}
//	else if ((pw_select&0x0f)==0x05)
//		for(j=0;j<3;j++){
//			pw[4+j]=PW_READ5[j];
//		}
//	else if ((pw_select&0x0f)==0x06)
//		for(j=0;j<3;j++){
//			pw[4+j]=PW_READ6[j];
//		}
//	else
//		for(j=0;j<3;j++){
//			pw[4+j]=PW_READ7[j];
//		}
	for(j=0;j<3;j++)
		{pw[4+j]=word[j];}

	for(j=0;j<3;j++){
		sc_gpa_clock(pw[4+j],0x05);//GPA[0]=pw[4+j];at88sc_gpa_clock(0x05);
		pw[4+j]=GPA[0];//*(p+42+j)=*(p+11);
	}
	pw[0]=0xba;//*(p+38)=0xba; //
	pw[1]=pw_select&0x0f;//*(p+39)=*(p+1)&0x0f;
	pw[1]=pw[1]+0x10;
	pw[2]=0x00;//*(p+40)=0x00;
	pw[3]=0x03;//*(p+41)=0x03;
	if( sc_write(p_card_manage,pw) != FUN_OK)
		return FUN_ERROR;
	dwt_delay_ms(1); dwt_delay_ms(1); dwt_delay_ms(1);

	pw[0]=0XB6;
	pw[1]=0X00;
	pw[2]=pw_select&0x0f;
	pw[2]=pw[2]*0x08;
	pw[2]=pw[2]+0xb4;
	pw[3]=0x01;
	if( sc_read(p_card_manage,pw) != FUN_OK)
		return FUN_ERROR;

	sc_gpa_clock(0x00,0x05);//GPA[0]=0x00;at88sc_gpa_clock(5);
	sc_gpa_clock(pw[2],0x01);//GPA[0]=pw[2];at88sc_gpa_clock(1);
	sc_gpa_clock(0x00,0x05);//GPA[0]=0x00;at88sc_gpa_clock(5);
	sc_gpa_clock(0x01,0x01);//GPA[0]=0x01;at88sc_gpa_clock(1);

	pw[4]=pw[4]^GPA[0];
	sc_gpa_clock(pw[4],0x01);//GPA[0]=pw[4];sc_gpa_clock(1);
	sc_gpa_clock(0x00,0x05);//GPA[0]=0x00;at88sc_gpa_clock(5);

//	return pw[4];
	return FUN_OK;
}

/** 
* select user zone by the parameter. 
* select user zone by the parameter. 
* @param[in]   zone:zone num.
* @param[out]  noen.
* @retval  0:successful
* @retval -1:failed
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
FUN_STATUS_T sc_select_user_zone(CARD_MANAGE_T *p_card_manage,uint8_t zone){
	
	uint8_t zon[4];

	#if !defined(SC_NO_NEED_AUTHENTICATION) && !defined(SC_NO_NEED_PASSWORD)
	sc_gpa_clock(zone,0x01);
	#endif
	zon[0]=0xb4;
	zon[1]=0x03;
	zon[2]=zone;
	zon[3]=0x00;
	if( sc_write(p_card_manage,zon) != FUN_OK)
		return FUN_ERROR;
	return FUN_OK;
}

/** 
* read data from the user zone. 
* call this fuction before authentication & verify the password. 
* @param[in]   rd_high_addr:high 8 addr,rd_low_addr:low 8 addr,rd_number：read length, recv_ptr:reply data pointer.
* @param[out]  1:success,0:failed.
* @retval  none
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
FUN_STATUS_T sc_read_user_zone(CARD_MANAGE_T *p_card_manage,uint16_t addr,uint16_t rd_number,uint8_t *recv_ptr){
	
	if( rd_number+4<=SC_READ_BUF_MAX_SIZE ){
		
		uint8_t j;
		uint8_t recv_buf[SC_READ_BUF_MAX_SIZE];
		uint8_t rd_high_addr,rd_low_addr;
		
		rd_high_addr = addr>>8;
		rd_low_addr = addr&0xff;
		
		recv_buf[0]=0xb2;
		recv_buf[1]=rd_high_addr;
		recv_buf[2]=rd_low_addr;
		recv_buf[3]=rd_number;
		if( sc_read(p_card_manage,recv_buf) != FUN_OK )
			return FUN_ERROR;
		
		#if !defined(SC_NO_NEED_AUTHENTICATION) && !defined(SC_NO_NEED_PASSWORD)
		sc_gpa_clock(0x00,0x05);
		sc_gpa_clock(rd_low_addr,0x01);
		sc_gpa_clock(0x00,0x05);
		sc_gpa_clock(rd_number,0x01);

		for(j=0;j<rd_number;j++){
		
			recv_buf[4+j]=recv_buf[4+j]^GPA[0];
			sc_gpa_clock(recv_buf[4+j],0x01);
			sc_gpa_clock(0x00,0x05);
		}
		#endif
		memcpy_s(recv_ptr,&recv_buf[4],rd_number);
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* read data from the config zone. 
* read encryption data decode to original data. 
* @param[in]   rd_high_addr:high 8 addr,rd_low_addr:low 8 addr,rd_number：read length, recv_ptr:reply data pointer.
* @param[out]  1:success,0:failed.
* @retval  none
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
FUN_STATUS_T sc_read_config_zone(CARD_MANAGE_T *p_card_manage,unsigned char rd_high_addr,unsigned char rd_low_addr,unsigned char rd_number,uint8_t *recv_ptr){
	
	if( rd_number+4<=SC_READ_BUF_MAX_SIZE ){
		
		unsigned char j;
		uint8_t send_buf[SC_READ_BUF_MAX_SIZE];

		send_buf[0]=0xb6;
		send_buf[1]=rd_high_addr;
		send_buf[2]=rd_low_addr;
		send_buf[3]=rd_number;
		if( sc_read(p_card_manage,send_buf) != FUN_OK)
			return FUN_ERROR;

		sc_gpa_clock(0x00,0x05);
		sc_gpa_clock(rd_low_addr,0x01);
		sc_gpa_clock(0x00,0x05);
		sc_gpa_clock(rd_number,0x01);

		for(j=0;j<rd_number;j++){
			
			if(rd_low_addr>=0xb0){
				send_buf[4+j]=send_buf[4+j]^GPA[0];}
			sc_gpa_clock(send_buf[4+j],0x01);
			sc_gpa_clock(0x00,0x05);
		}
		memcpy(recv_ptr,&send_buf[4],rd_number);
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* encryption to send at88sc. 
* encryption original data . 
* @param[in]   encryptodatanumber:data length,ptr:data pointer.
* @param[out]  none.
* @retval  none
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
FUN_STATUS_T sc_encrypto_data(uint8_t encryptodatanumber,uint8_t *ptr){

	uint8_t j;
	uint8_t temp;
	for(j=0;j<encryptodatanumber;j++){
		
		sc_gpa_clock(0x00,0x05);
		temp=ptr[4+j];
		ptr[4+j]=ptr[4+j]^GPA[0];
		sc_gpa_clock(temp,0x01);
	}
	return FUN_OK;
}

/** 
* count protocol checksum. 
* count protocol checksum. 
* @param[in]   none.
* @param[out]  none.
* @retval  none
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken于2018-11-15 create
*/
void sc_send_checksum(void){
	
//	uint8_t dat6[6];

//	dat6[0]=0xb4;
//	dat6[1]=0x02;
//	dat6[2]=0x00;
//	dat6[3]=0x02;

//	sc_gpa_clock(0x00,0x0f);
//	dat6[4]=GPA[0];
//	sc_gpa_clock(0x00,0x05);
//	dat6[5]=GPA[0];

//	sc_write(dat6);
}

/** 
* write data to user zone. 
* call this fuction before authentication & verify the password. 
* @param[in]   rd_high_addr:high 8 addr,rd_low_addr:low 8 addr,rd_number：read length, send_ptr:send data pointer.
* @param[out]  none.
* @retval  0:failed
* @retval  1:successful
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken 2018-11-15 create
*/
FUN_STATUS_T sc_write_user_zone(CARD_MANAGE_T *p_card_manage,uint16_t addr,uint8_t wr_number,uint8_t *send_ptr){
	
	if( wr_number<=SC_BUF_MAX_SIZE ){
		
		uint8_t send_buf[SC_BUF_MAX_SIZE+4];
		uint8_t wr_high_addr,wr_low_addr;
		
		wr_high_addr = addr>>8;
		wr_low_addr = addr&0xff;
		
		#if !defined(SC_NO_NEED_AUTHENTICATION) && !defined(SC_NO_NEED_PASSWORD)
		sc_gpa_clock(0x00,0x05);
		sc_gpa_clock(wr_low_addr,0x01);
		sc_gpa_clock(0x00,0x05);
		sc_gpa_clock(wr_number,0x01);
		#endif

		send_buf[0]=0xb0;
		send_buf[1]=wr_high_addr;
		send_buf[2]=wr_low_addr;
		send_buf[3]=wr_number;
		memcpy(&send_buf[4],send_ptr,wr_number);
		#if !defined(SC_NO_NEED_AUTHENTICATION) && !defined(SC_NO_NEED_PASSWORD)
		sc_encrypto_data(wr_number,send_buf);
		#endif
		
		if( sc_write(p_card_manage,send_buf) != FUN_OK ){
			return FUN_ERROR;
		}

		dwt_delay_ms(1);dwt_delay_ms(1);dwt_delay_ms(1);dwt_delay_ms(1);dwt_delay_ms(1);
		sc_send_checksum();
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* write data to config zone. 
* write data to config zone. 
* @param[in]   rd_high_addr:high 8 addr,rd_low_addr:low 8 addr,rd_number：read length, send_ptr:send data pointer.
* @param[out]  none.
* @retval  none
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken 2018-11-15 create
*/
FUN_STATUS_T sc_write_config_zone(CARD_MANAGE_T *p_card_manage,uint8_t wr_high_addr,uint8_t wr_low_addr,uint8_t wr_number,uint8_t *send_ptr){
	
	if( wr_number+4 < SC_BUF_MAX_SIZE  ){
		
		uint8_t send_buf[SC_BUF_MAX_SIZE];
		uint8_t j;
		uint8_t temp;
		sc_gpa_clock(0x00,0x05);
		sc_gpa_clock(wr_low_addr,0x01);
		sc_gpa_clock(0x00,0x05);
		sc_gpa_clock(wr_number,0x01);

		send_buf[0]=0xb4;
		send_buf[1]=wr_high_addr;
		send_buf[2]=wr_low_addr;
		send_buf[3]=wr_number;
		//sc_encrypto_data(wr_number);
		memcpy(&send_buf[4],send_ptr,wr_number);
		for(j=0;j<wr_number;j++){
			
			sc_gpa_clock(0x00,0x05);
			temp=send_buf[4+j];
			if(wr_low_addr>=0xb0){
				send_buf[4+j]=send_buf[4+j]^GPA[0];
			}
			sc_gpa_clock(temp,0x01);
		}

		if( sc_write(p_card_manage,send_buf) != FUN_OK)
			return FUN_ERROR;
		dwt_delay_ms(1);dwt_delay_ms(1);dwt_delay_ms(1);dwt_delay_ms(1);dwt_delay_ms(1);
		sc_send_checksum();
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* plaintext verify config zone sceure code. 
* for verify config passoword must after the chip reset,nor the case the verfiy well failed. 
* @param[in]   sc_first_byte:sc first byte,sc_second_byte:sc second byte,sc_third_byte: sc third byte.
* @param[out]  none.
* @retval  return 0xff:success, 0:failed
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken 2018-11-15 create
*/
FUN_STATUS_T sc_verify_sc_plaintext(CARD_MANAGE_T *p_card_manage,uint8_t sc_first_byte,uint8_t sc_second_byte,uint8_t sc_third_byte){
	
	uint8_t scdata[7];
	scdata[0]=0xBA;
	scdata[1]=0x07;
	scdata[2]=0x00;
	scdata[3]=0x03;
	scdata[4]=sc_first_byte;
	scdata[5]=sc_second_byte;
	scdata[6]=sc_third_byte;
	if( sc_write(p_card_manage,scdata) != FUN_OK )
		return FUN_ERROR;

	scdata[0]=0xb6;
	scdata[1]=0x00;
	scdata[2]=0xE8;
	scdata[3]=0x01;
	if( sc_read(p_card_manage,scdata) != FUN_OK)
		return FUN_ERROR;
//	return scdata[4];
	return FUN_OK;
}

/** 
* plaintext set user zone num. 
* plaintext set user zone num. 
* @param[in]   zonep:zone num.
* @param[out]  none.
* @retval  0:successful
* @retval -1:failed
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken 2018-11-15 create
*/
FUN_STATUS_T sc_set_user_zone_plaintext(CARD_MANAGE_T *p_card_manage,uint8_t zonep){
	
	uint8_t zon[4];
	zon[0]=0xb4;
	zon[1]=0x03;
	zon[2]=zonep;
	zon[3]=0x00;
	if( sc_write(p_card_manage,zon) != FUN_OK )
		return FUN_ERROR;
	return FUN_OK;
}

/** 
* at88sc write block. 
* send data support more than block size. 
* @param[in]   zone:block num,addr:user zone addr,len:read length,ptr:send data pointer.
* @param[out]  none.
* @retval  >0:send data length,0: failed.
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken 2018-11-15 create
*/
//uint16_t sc_write_user_block(uint8_t zone, uint16_t addr, uint16_t len, uint8_t *ptr){
//	
//	uint16_t 	send=0;
//	uint16_t 	seg;
//	uint8_t gc,pw;
//	
//	///<last use the block and this times use is the same, with no need for to do
//	gc = sc_block_admini[zone]>>4;
//	pw = sc_block_admini[zone]&0x0f;
//	
////	if( sc_sta.error_code != 0 ){
////		if( sc_authentication(gc,smartcard_config[sc_sta.atr_type].auth_key)!=0xff ){
////			smartcard_log("auth error\r\n");
////			return 0;
////		}
////		sc_sta.error_code = 0;
////	}
//	sc_sta.error_code = 0;
//	
//	if( sc_sta.curr_use_block != zone ){
//	//{
//	
//		
//		
////		if( sc_authentication(gc)!=0xff ){
////			return 0;
////		}
////		else {
////			if( sc_verify_write_password(pw)!=0xff  )
////				return 0;
////		}
//		
//		if( sc_authentication(gc,smartcard_config[sc_sta.atr_type].auth_key)!=0xff ){
//			return 0;
//		}
//		else {
//			if( sc_verify_write_password(pw,smartcard_config[sc_sta.atr_type].zone_write_pwd[pw])!=0xff  ){
//				return 0;
//			}
//		}		
//		
//		sc_sta.curr_use_block = zone;
//	}
//	
//	if( addr+len <= SC_BLOCK_SIZE ){
//		
//		sc_select_user_zone(zone);
//		
//		while( send<len ){
//			
//			if( len-send < SC_BUF_MAX_SIZE ){
//				seg = (len-send);
//			}
//			else{
//				seg = SC_BUF_MAX_SIZE;
//			}
//			
//			if( addr/SC_BUF_MAX_SIZE != (addr+seg)/SC_BUF_MAX_SIZE )
//				seg = SC_BUF_MAX_SIZE-addr%SC_BUF_MAX_SIZE;
//			
//			
//			if( sc_write_user_zone(addr,seg,ptr) == 0){
//				sc_sta.curr_use_block = 0xff;
//				return 0;
//			}
//			
//			if( sc_sta.error_code!=0 ){
//				sc_sta.curr_use_block = 0xff;
//				return 0;
//			}
//			
//			//while(sc_write_user_zone(addr,seg,ptr)==0);
//			addr += seg;
//			send += seg;
//			ptr += seg;
//			
//			//for use in rtos, release system
//			delay_ms(1);
//		}
//		return len;
//	}
//	else{
//		return 0;
//	}
//}

/** 
* 写入用户数据区. 
* 无. 
* @param[in]   p_card_manage:智能卡接口句柄，zone:区号，addr:开始地址，len:长度，ptr：数据指针. 
* @param[out]  无.  
* @retval  FUN_STATUS_T:函数状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T sc_write_user_block(CARD_MANAGE_T *p_card_manage, uint8_t zone, uint16_t addr, uint16_t len, uint8_t *ptr){
	
	uint16_t 	send=0;
	uint16_t 	seg;
	uint8_t gc,pw;
	uint8_t		try_cnt=0;
	
	uint16_t orig_addr;
	uint8_t *orig_ptr;
	
	orig_addr = addr;
	orig_ptr = ptr;
	
	///<last use the block and this times use is the same, with no need for to do
	gc = (p_card_manage->auth_pwd->block[zone])>>4;
	pw = (p_card_manage->auth_pwd->block[zone])&0x0f;
	
//	if( sc_sta.error_code != 0 ){
//		if( sc_authentication(gc,smartcard_config[sc_sta.atr_type].auth_key)!=0xff ){
//			smartcard_log("auth error\r\n");
//			return 0;
//		}
//		sc_sta.error_code = 0;
//	}
	do{
		
	
		addr = orig_addr;
		ptr = orig_ptr;
		sc_sta.error_code = 0;
		send = 0;
		
		if( sc_sta.curr_use_block != zone ){
		//{
		
			
			
	//		if( sc_authentication(gc)!=0xff ){
	//			return 0;
	//		}
	//		else {
	//			if( sc_verify_write_password(pw)!=0xff  )
	//				return 0;
	//		}
			if( sc_authentication(p_card_manage,gc,(uint8_t*)smartcard_config[p_card_manage->atr_type].auth_key)!=FUN_OK ){
//				smartcard_log("write auth error\r\n");
				goto ERR_CONTINUE;
			}
			else {
//				vTaskDelay(5);
				if( sc_verify_write_password(p_card_manage,pw,(uint8_t*)smartcard_config[p_card_manage->atr_type].zone_write_pwd[pw])!=FUN_OK  ){
//					smartcard_log("write password error\r\n");
					goto ERR_CONTINUE;
				}
			}		
//			vTaskDelay(5);
			sc_sta.curr_use_block = zone;
		}
		
		if( addr+len <= p_card_manage->block_size ){
			
			sc_select_user_zone(p_card_manage,zone);
			
			while( send<len ){
				
				if( len-send < SC_BUF_MAX_SIZE ){
					seg = (len-send);
				}
				else{
					seg = SC_BUF_MAX_SIZE;
				}
				
				if( addr/SC_BUF_MAX_SIZE != (addr+seg)/SC_BUF_MAX_SIZE )
					seg = SC_BUF_MAX_SIZE-addr%SC_BUF_MAX_SIZE;
				
				
				if( sc_write_user_zone(p_card_manage,addr,seg,ptr) != FUN_OK){
					sc_sta.curr_use_block = 0xff;
					goto ERR_CONTINUE;
				}
				
				if( sc_sta.error_code!=0 ){
					sc_sta.curr_use_block = 0xff;
					goto ERR_CONTINUE;
				}
				addr += seg;
				send += seg;
				ptr += seg;
				
				//for use in rtos, release system
//				vTaskDelay(5);
			}
//			return len;
			return FUN_OK;
		}
	ERR_CONTINUE:	
		smartcard_log("retry write\r\n");
		vTaskDelay( 100 );
	}while(try_cnt++<SC_RETYR_NUM);
	smartcard_err("write fault\r\n");
	return FUN_ERROR;
}

/** 
* 读取用户数据区. 
* 无. 
* @param[in]   p_card_manage:智能卡接口句柄，zone:区号，addr:开始地址，len:长度. 
* @param[out]  ptr：返回数据指针.  
* @retval  FUN_STATUS_T:函数状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T sc_read_user_block(CARD_MANAGE_T *p_card_manage,uint8_t zone, uint16_t addr, uint16_t len, uint8_t *ptr){
	
	uint8_t gc,pw;
	uint16_t 	seg;
	uint16_t 	recv=0;
	uint8_t		try_cnt=0;
	
	uint16_t orig_addr;
	uint8_t *orig_ptr;
	
	orig_addr = addr;
	orig_ptr = ptr;
	
	///<last use the block and this times use is the same, with no need for to do
	gc = (p_card_manage->auth_pwd->block[zone])>>4;
	pw = (p_card_manage->auth_pwd->block[zone])&0x0f;
	
//	if( sc_authentication(gc,smartcard_config[sc_sta.atr_type].auth_key)!=0xff ){
//		smartcard_log("auth error\r\n");
//		return 0;
//	}
//	
	do{
	
		addr = orig_addr;
		ptr = orig_ptr;
		recv= 0;
		sc_sta.error_code = 0;
		
		if( sc_sta.curr_use_block != zone ){
		
			
			
	//		if( sc_authentication(gc)!=0xff ){
	//			return 0;
	//		}
	//		else {
	//			if( sc_verify_write_password(pw)!=0xff  )
	//				return 0;
	//		}
//			taskENTER_CRITICAL();
			if( sc_authentication(p_card_manage,gc,(uint8_t*)smartcard_config[p_card_manage->atr_type].auth_key)!=FUN_OK ){
//				smartcard_log("read auth error\r\n");
//				taskEXIT_CRITICAL();
				goto ERR_CONTINUE;
			}
			else
			{
//				vTaskDelay(5);
				if( sc_verify_write_password(p_card_manage,pw,(uint8_t*)smartcard_config[p_card_manage->atr_type].zone_write_pwd[pw]) != FUN_OK ){
//					smartcard_log("read password error\r\n");
//					taskEXIT_CRITICAL();
					goto ERR_CONTINUE;
				}
			}
//			taskEXIT_CRITICAL();
//			vTaskDelay(5);
			sc_sta.curr_use_block = zone;
		}
		
		if( addr+len <= p_card_manage->block_size ){
			
			sc_select_user_zone(p_card_manage,zone);
	//		sc_read_user_zone(addr,len,ptr);
			
			while( recv<len ){
				
				if( len-recv < SC_BUF_MAX_SIZE ){
					seg = (len-recv);
				}
				else{
					seg = SC_BUF_MAX_SIZE;
				}
				
				if( addr/SC_BUF_MAX_SIZE != (addr+seg)/SC_BUF_MAX_SIZE )
					seg = SC_BUF_MAX_SIZE-addr%SC_BUF_MAX_SIZE;
				
				if( sc_read_user_zone(p_card_manage,addr,seg,ptr) != FUN_OK){
					sc_sta.curr_use_block = 0xff;
					goto ERR_CONTINUE;
				}
				
				if( sc_sta.error_code!=0 ){
					sc_sta.curr_use_block = 0xff;
					goto ERR_CONTINUE;
				}
				addr += seg;
				recv += seg;
				ptr += seg;
				
				//for use in rtos, release system
//				vTaskDelay(5);
			}
//			return len;
			return FUN_OK;
		}
	ERR_CONTINUE:	
		smartcard_log("retry read\r\n");
		vTaskDelay( 100 );
	}while(try_cnt++<SC_RETYR_NUM);
	smartcard_err("read fault\r\n");
	return FUN_ERROR;
}


/** 
* write at88sc storage. 
* write at88sc all the block in continuous address,such as 0x0000-0x8000 for at88sc25616. 
* @param[in]   addr:write address,len:write length,ptr:write data pointer.
* @param[out]  none.
* @retval  >0:send data length,0: failed.
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken 2018-11-20 create
*/
FUN_STATUS_T sc_write_storage(CARD_MANAGE_T *p_card_manage, uint16_t addr, uint16_t len, uint8_t *ptr){
	
	uint16_t block_addr,write_len;
	uint16_t start_addr,count=0;
	uint8_t block_num;
	
	start_addr =  addr;
	
	if( addr+len < p_card_manage->storage_size ){
	
		while( count < len ){
			
			block_addr = start_addr%p_card_manage->block_size;
			block_num = start_addr/p_card_manage->block_size;
			
			write_len = p_card_manage->block_size-block_addr;
			
			if( write_len > (len-count) )
				write_len = len-count;
			
			if(sc_write_user_block(p_card_manage,block_num, block_addr, write_len, ptr)!=FUN_OK){
				return FUN_ERROR;
			}
			
			count += write_len;
			start_addr += write_len;
			ptr += write_len;
			
		}
		
//		return count;
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* at88sc read storage. 
* read at88sc all the block in continuous address,such as 0x0000-0x8000 for at88sc25616. 
* @param[in]   addr:write address,len:write length,ptr:write data pointer.
* @param[out]  none.
* @retval  >0:receive data length,0: failed.
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken 2018-11-20 create
*/
FUN_STATUS_T sc_read_storage(CARD_MANAGE_T *p_card_manage, uint16_t addr, uint16_t len, uint8_t *ptr){
	
	uint16_t block_addr,write_len;
	uint16_t start_addr,count=0;
	uint8_t block_num;
	
	start_addr =  addr;
	
	if( addr+len < p_card_manage->storage_size ){
	
		while( count < len ){
			
			block_addr = start_addr%p_card_manage->block_size;
			block_num = start_addr/p_card_manage->block_size;
			
			write_len = p_card_manage->block_size-block_addr;
			
			if( write_len > (len-count) )
				write_len = len-count;
			
			if( sc_read_user_block(p_card_manage,block_num, block_addr, write_len, ptr)!=FUN_OK){
				smartcard_log("read exit\r\n");
				return FUN_ERROR;
			}
			
			count += write_len;
			start_addr += write_len;
			ptr += write_len;
			
		}
		
//		return count;
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 清空指定存储区. 
* 无. 
* @param[in]   p_card_manage:智能卡接口句柄，addr:开始地址，len:清空长度. 
* @param[out]  无.  
* @retval  FUN_STATUS_T:函数状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T sc_clear_storage(CARD_MANAGE_T *p_card_manage, uint16_t addr, uint16_t len){
	
	uint16_t block_addr,write_len;
	uint16_t start_addr,count=0;
	uint8_t block_num;
	
	start_addr =  addr;
	
	if( addr+len <= p_card_manage->storage_size ){
		
		uint8_t		*buf = malloc(p_card_manage->block_size);
		memset(buf,0,p_card_manage->block_size);
	
		if( buf==NULL )
			return FUN_ERROR;
		
		while( count < len ){
			
			block_addr = start_addr%p_card_manage->block_size;
			block_num = start_addr/p_card_manage->block_size;
			
			write_len = p_card_manage->block_size-block_addr;
			
			if( write_len > (len-count) )
				write_len = len-count;
			
			if(sc_write_user_block(p_card_manage, block_num, block_addr, write_len, buf)!=FUN_OK){
				free(buf);
				return FUN_ERROR;
			}
			
			count += write_len;
			start_addr += write_len;
			
		}
		free(buf);
		
//		return count;
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* read at88sc fuse status. 
* read at88sc fuse status. 
* @param[in]   none.
* @param[out]  stata:return fuse state.
* @retval  0:successful
* @retval  -1:failed
* @par TAG 
*      reserved 
* @par other 
*      none 
* @par modify
*      ken 2018-11-20 create
*/
FUN_STATUS_T sc_read_fuse_state(CARD_MANAGE_T *p_card_manage,uint8_t *state){
	
	uint8_t send_buf[SC_READ_BUF_MAX_SIZE];
		
	send_buf[0]=SC_SYSTEM_READ_CMD;//0xb2 or 0xb6
	send_buf[1]=0x01;    //A1
	send_buf[2]=0x00;    //A2
	send_buf[3]=0x01;     //N
	if( sc_read(p_card_manage,send_buf) != FUN_OK)
		return FUN_ERROR;
	
	*state = send_buf[4];
	return FUN_OK;
}

static uint32_t sc_InverseData(uint32_t revData)
{
	return (~(__RBIT(revData)))>>24;
}

static void sc_InverseBuf(uint8_t *revBuf,uint8_t bufLen)
{
	uint8_t i = 0u;

	for(i = 0u;i < bufLen;i++){
		
		revBuf[i] = (uint8_t)sc_InverseData((uint32_t)revBuf[i]);
	}
}

/** 
* 识别智能类型. 
* ATR保存着各个类型信息，用于判断卡类型. 
* @param[in]   p_card_manage：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  返回智能卡类型序号
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
uint8_t sc_idf_card_type(CARD_MANAGE_T *p_card_manage)
{
	
	p_card_manage->atr_type = 0xff;
	for( uint8_t i=0; i<CARD_CONFIG_MAX; i++ ){
		if( memcmp(p_card_manage->sc_atr_table,smartcard_config[i].answer_reset,sizeof(((SMARTCARD_CONFIG_T*)0)->answer_reset))==0 ){
			p_card_manage->atr_type = i;
			p_card_manage->block_num = smartcard_config[i].block_num;
			p_card_manage->block_size = smartcard_config[i].block_size;
			p_card_manage->auth_pwd = (SMARTCARD_AUTH_PWD_T *)&smartcard_auth[i];
			p_card_manage->storage_size = p_card_manage->block_num*p_card_manage->block_size;
			return i;
		}
	}
	return 0xff;
}

/** 
* 读取卡的厂家编号. 
* 具体没用处，用于识别与智能卡通讯是否正常. 
* @param[in]   p_cardHandle：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  HAL_StatusTypeDef:状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-05-09创建 
*/
static HAL_StatusTypeDef sc_read_card_manu_code(CARD_MANAGE_T *p_card_manage)
{
	FUN_STATUS_T scStatus;
    
	{
		p_card_manage->card_apdu_cmd.header.cla = SC_CLA_CODE;
		p_card_manage->card_apdu_cmd.header.ins = SC_SYSTEM_READ_CMD;
		p_card_manage->card_apdu_cmd.header.p1  = 0x00;
		p_card_manage->card_apdu_cmd.header.p2  = 0x0C;
		p_card_manage->card_apdu_cmd.body.lc    = 0x04;
		p_card_manage->card_apdu_cmd.body.le    = READ_FROM_SC;
		scStatus = sc_transmit_data(p_card_manage,false);
	}

	return scStatus;
}

/** 
* 识别出插入智能卡类型. 
* 无. 
* @param[in]   p_card_manage:智能卡接口句柄，cardType:返回卡类型，card_id:返回的卡ID. 
* @param[out]  无.  
* @retval  FUN_STATUS_T:函数状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
static FUN_STATUS_T sc_indentify_card_type(CARD_MANAGE_T *p_card_manage,CARD_PRESENT_T *cardType, uint8_t *card_id)
{
	uint8_t unique_id[7];
	static uint8_t last_admin_unique_id[7];
	uint8_t select_block=0;
	
	sc_read_config_register(p_card_manage,IDN_R,unique_id);
	
	if( p_card_manage->atr_type==VOTE_CARD_CONFIG ){
		select_block = 0;//unique_id[SC_CARD_TYPE_IDF]-1;
	}
	else if( p_card_manage->atr_type!=0xff ){
		select_block = 0;
	}
	else{
		
		*cardType    = CARD_PRESENT_NONE;
		return FUN_OK;
	}
	
	#if defined(SC_USE_CRYPT)
	//if( crypt_read(CRYPT_SC,(select_block)*card_manage->block_size,sizeof(SC_CARD_INFO_T),&sc_card_ind.card_info) ){
	if( crypt_read(p_card_manage,(select_block)*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&sc_card_ind.card_info) ){
	#else
	if( sc_read_storage(p_card_manage,(select_block)*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&sc_card_ind.card_info) == FUN_OK ){
	#endif
		uint8_t *ptr=sc_card_ind.card_info.unique_id;
		smartcard_log("unique_id:%02x %02x %02x %02x %02x %02x %02x\r\n",*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5),*(ptr+6));
		if( memcmp(unique_id,sc_card_ind.card_info.unique_id,sizeof(((SC_CARD_INFO_T*)0)->unique_id))==0 ){
		
			if( memcmp(sc_card_ind.card_info.key,default_password.voter_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->voter_key))==0 ){
					
				*cardType = VOTE_CARD_PRESENT;
//				event_log_write(STANDARD_LEVEL, KEY_CARD_TYPE, "Voter Card presented");
				memcpy(card_id,unique_id,7);
			}
			else if( memcmp(sc_card_ind.card_info.key,default_password.administrator_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->administrator_key))==0 ){
				*cardType = ADMIN_CARD_PRESENT;
				
				char buf[100];
				memset(buf,0,100);
				snprintf(buf,100,"Maintenance/Administrator Card presented(%02X%02X%02X%02X%02X%02X%02X)",unique_id[0],unique_id[1],unique_id[2],unique_id[3],
					unique_id[4],unique_id[5],unique_id[6]);
//				event_log_write(STANDARD_LEVEL, KEY_CARD_TYPE, buf);
				memcpy(card_id,unique_id,7);
				
				if( is_req_admin_card2() ){
					if( memcmp(unique_id,last_admin_unique_id,7)==0 ){
						clr_admin_card2_present();
					}
					else{
						set_admin_card2_present();
					}
				}
				memcpy(last_admin_unique_id,unique_id,7);
			}
			else if( memcmp(sc_card_ind.card_info.key,default_password.system_administrator_key1,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator_key1))==0 ){
				if( crypt_read(p_card_manage,(1)*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&sc_card_ind.card_info) ){
					if( memcmp(sc_card_ind.card_info.key,default_password.system_administrator_key2,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator_key2))==0 )
					{
						*cardType = SYSTEM_CARD_PRESENT;
						
						char buf[100];
						memset(buf,0,100);
						snprintf(buf,100,"System Administrator Card Presented(%02X%02X%02X%02X%02X%02X%02X)",unique_id[0],unique_id[1],unique_id[2],unique_id[3],
							unique_id[4],unique_id[5],unique_id[6]);
//						event_log_write(CRITICAL_LEVEL, SYSTEM_ADMIN_TYPE, buf);
						
						memcpy(card_id,unique_id,7);
					}
				}
			}
			else if( memcmp(sc_card_ind.card_info.key,default_password.maintenance_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->maintenance_key))==0 ){
				*cardType    = MAINTE_CARD_PRESENT;
//				event_log_write(STANDARD_LEVEL, MANTEN_CARD_TYPE, "Operation performed by Maintenance/Administrator Card");
				memcpy(card_id,unique_id,7);
			}
			else{
				*cardType = CARD_PRESENT_NONE;
//				tamper_set_incorrect_key();
			}
		}
		else if( memcmp(sc_card_ind.card_info.unique_id,"\0\0\0\0\0\0\0",sizeof(((SC_CARD_INFO_T*)0)->unique_id))==0 ){
			if( p_card_manage->atr_type==VOTE_CARD_CONFIG ){
				*cardType = VOTE_CARD_PRESENT;
			}
			else{
				*cardType = VOTE_USED_PRESENT;
			}
		}
		else{
			if( p_card_manage->atr_type==VOTE_CARD_CONFIG ){
				*cardType = VOTE_CARD_PRESENT;
			}
			else{	
				*cardType = CARD_PRESENT_NONE;
			}
		}
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 投票卡计数. 
* 未达到最大投票计数，计数值加1，达到最大投票数禁止使用本卡. 
* @param[in]   p_cardHandle：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  CARD_ERROR_TypeDef:状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-05-09创建 
*/
static CARD_ERROR_T sc_ChkVoteCardActiveStatus(CARD_MANAGE_T *p_card_manage)
{
	CARD_ERROR_T voteCardStatus = ERR_CARD_ERROR;
	uint8_t select_block;
	
	if( !election_status.flag.bit.configured ){
		
		voteCardStatus = ERR_SYSTEM_NOT_CFG;
	}
	else if( election_status.flag.bit.start ){
		
		if( sc_card_ind.card_info.vote_count<SC_CARD_COUNTER_MAX ){
			
			sc_card_ind.card_info.vote_count++;
			
			voteCardStatus = ERR_CARD_NONE;
			
			#if defined(SC_USE_CRYPT)
			if( crypt_write(p_card_manage,offsetof(SC_CARD_INFO_T,vote_count)+(select_block)*p_card_manage->block_size,sizeof(((SC_CARD_INFO_T*)0)->vote_count),&sc_card_ind.card_info.vote_count)!=FUN_OK){
			#else
			if( sc_write_storage(p_card_manage,offsetof(SC_CARD_INFO_T,vote_count)+(select_block)*p_card_manage->block_size,sizeof(((SC_CARD_INFO_T*)0)->vote_count),&sc_card_ind.card_info.vote_count)!=1){
			#endif
				
				voteCardStatus = ERR_CARD_ERROR;
			}
		}
		else{
			voteCardStatus = ERR_CARD_VOTE_NOT_DONE; 
		}
	}
	else
		voteCardStatus = ERR_CARD_NOT_IN_TIME;
	
	return voteCardStatus;
}

/** 
* 根据插入不同的卡类型切换不同的菜单. 
* 无. 
* @param[in]   cardType:卡类型，card_id:卡ID. 
* @param[out]  无.  
* @retval  FUN_STATUS_T:函数状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
void sc_card_type_menu(CARD_PRESENT_T cardType,uint8_t *card_id)
{
	switch (cardType){    
  
		case ADMIN_CARD_PRESENT:
		{
			lcdxx_clear_screen();
			display_info_in("ADMINISTRATOR\nCARD PRESENTED\n\n",0);
			display_switch_menu(admin_card_menu);
			memcpy(election_status.card_id,card_id,7);
			char log[EVNET_LOG_SIZE];
			snprintf(log,EVNET_LOG_SIZE,"Operation performed by Administrator Card(%02X%02X%02X%02X%02X%02X%02X)",\
			*card_id,*(card_id+1),*(card_id+2),*(card_id+3),*(card_id+4),*(card_id+5),*(card_id+6));
			event_log_write(CRITICAL_LEVEL, SYSTEM_ADMIN_TYPE, log);
			ioc_keypad_led_on();
		}
		break;

//		case MAINTE_CARD_PRESENT:
//		{
//			display_info_in("MAINTENANCE\nCARD PRESENTED",0);
//			display_switch_menu(maintenance_card_menu);
//			memcpy(election_status.card_id,card_id,7);
//		}
//		break;
	
		case SYSTEM_CARD_PRESENT:
		{
			lcdxx_clear_screen();
			display_info_in("SYSTEM ADMIN\nCARD PRESENTED\n\n",0);
			display_switch_menu(system_card_menu);
			memcpy(election_status.card_id,card_id,7);
			char log[EVNET_LOG_SIZE];
			snprintf(log,EVNET_LOG_SIZE,"System Administrator Card Present(%02X%02X%02X%02X%02X%02X%02X)",\
			*card_id,*(card_id+1),*(card_id+2),*(card_id+3),*(card_id+4),*(card_id+5),*(card_id+6));
			event_log_write(CRITICAL_LEVEL, SYSTEM_ADMIN_TYPE, log);
			ioc_keypad_led_on();
		}
		break;
			
    
		case CARD_PRESENT_NONE:
		{
			lcdxx_clear_screen();
			display_info_in("CARD TYPE ERR",0);
			memset(election_status.card_id,0,7);
		}
		break;
	
		default: 
		break;
  }
}

/** 
* 识别卡片类型. 
* 根据智能卡接口硬件序号，读取卡序列号，显示卡类型. 
* @param[in]   p_card_manage：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  CARD_ERROR_TypeDef:卡状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-05-09创建 
*/
static CARD_ERROR_T sc_card_identify(CARD_MANAGE_T *p_card_manage)
{
	CARD_PRESENT_T card_type;
	CARD_ERROR_T sc_status = ERR_CARD_NONE;
	sc_sta.curr_use_block = 0xff;
	uint8_t		card_id[7];

	if( p_card_manage->card_port==CARD_A ){
	
		/* Check what card present in the system                                                                            */
		if (sc_read_card_manu_code(p_card_manage) == HAL_OK){
			
			if (sc_indentify_card_type(p_card_manage,&card_type,card_id) == HAL_OK){
				
				p_card_manage->card_present = card_type;
				
				if (card_type != VOTE_CARD_PRESENT){
					
					sc_card_type_menu(card_type,card_id);
				}
			}
			else{
				
				sc_status = ERR_CARD_NOT_AUTH;
			}
		}
		else{
			smartcard_log("can not read manucode\r\n");
			sc_status = ERR_CARD_NOT_AUTH;
		}
	}
	else if( p_card_manage->card_port==CARD_B ){
		
		if (sc_read_card_manu_code(p_card_manage) == HAL_OK){
			
			if (sc_indentify_card_type(p_card_manage,&card_type,card_id) == HAL_OK){
				
				p_card_manage->card_present = card_type;
				
				if (card_type == VOTE_CARD_PRESENT){
					
//					sc_status = sc_ChkVoteCardActiveStatus(p_card_manage);
//					beep_set_alarm(BZ_EVENT_AUTH_CARD);
				}

				if (sc_status == ERR_CARD_NONE){
					
//					app_GetCardMenuInfor(card_type,card_id);
				}
			}
			else{
				
				sc_status = ERR_CARD_NOT_AUTH;
			}
		}
		else{
			smartcard_log("can not read manucode\r\n");
			sc_status = ERR_CARD_NOT_AUTH;
		}
			
//			if( memcmp(p_card_manage->sc_atr_table,smartcard_config[BACKUP_CARD_CONFIG].answer_reset,sizeof(((SMARTCARD_CONFIG_T*)0)->answer_reset))==0 ){
//				election_status.second_card_type        = BACKUP_CARD_PRESENT;
//				p_card_manage->atr_type = BACKUP_CARD_CONFIG;
//				p_card_manage->block_num = 4;
//				p_card_manage->block_size = 128;
//				p_card_manage->auth_pwd = (SMARTCARD_AUTH_PWD_T *)&smartcard_auth[BACKUP_CARD_CONFIG];
//				p_card_manage->storage_size = p_card_manage->block_num*p_card_manage->block_size;
//			}
//			else{
//				election_status.second_card_type        = CARD_PRESENT_NONE;
//			}
//			
//			sc_status = ERR_CARD_NONE;
//		}
//		else{
//			sc_status = ERR_CARD_NOT_AUTH;
//		}
	}

	return sc_status;
}

/** 
* 向智能卡发送数据并等待返回. 
* 与智能卡通讯重要函数，组识T0数据协议. 
* @param[in]   p_cardHandle：智能卡硬件句柄,procedureFlag:未使用. 
* @param[out]  none.  
* @retval  HAL_StatusTypeDef:状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-25创建 
*/
static FUN_STATUS_T sc_transmit_data(CARD_MANAGE_T *p_card_manage, bool procedureFlag)
{
	uint8_t i = 0;
	uint8_t command[5] = {0x00};
	uint8_t answer[40] = {0x00};
	FUN_STATUS_T 	scStatus = FUN_OK;
	HAL_StatusTypeDef	status;
  
	ltc1955_select(p_card_manage->card_port);

	/* Reset response buffer                                                                                            */
//	for(i = 0; i < SC_APDU_LC_MAX ; i++){
//		p_card_manage->card_apdu_resp.data[i] = 0;
//	}
	memset(p_card_manage->card_apdu_resp.data,0,SC_APDU_LC_MAX);

	p_card_manage->card_apdu_resp.sw1 = 0;
	p_card_manage->card_apdu_resp.sw2 = 0;


	/* Send header                                                                                                      */
	command[0] = p_card_manage->card_apdu_cmd.header.cla;
	command[1] = p_card_manage->card_apdu_cmd.header.ins;
	command[2] = p_card_manage->card_apdu_cmd.header.p1;
	command[3] = p_card_manage->card_apdu_cmd.header.p2;
	command[4] = p_card_manage->card_apdu_cmd.body.lc;     /* Send body length to / from SC                          */
	
	HAL_SMARTCARD_Transmit(&p_card_manage->handle, (uint8_t *)command, 5, SC_TRANSMIT_TIMEOUT);
	__HAL_SMARTCARD_FLUSH_DRREGISTER(&p_card_manage->handle);     /* Flush the SC_USART DR                                  */

	if (procedureFlag == true){
		
		status = HAL_SMARTCARD_Receive(&p_card_manage->handle, (uint8_t *)&answer[0], 1, SC_RECEIVE_TIMEOUT);

		/* Answer from the card before the over time counter reach to zero                                                */
		if (status == HAL_OK){
			
			/* Wait Procedure byte from card: 1 - ACK  2 - NULL  3 - SW1; SW2-----------------------------------------------*/
			if(((answer[0] & (uint8_t)0xF0) == 0x60) || ((answer[0] & (uint8_t)0xF0) == 0x90)){
				
				p_card_manage->card_apdu_resp.sw1 = answer[0];    /* SW1 received                                          */
				if(HAL_SMARTCARD_Receive(&p_card_manage->handle, (uint8_t *)&answer[1], 1, SC_RECEIVE_TIMEOUT) == HAL_OK){
					p_card_manage->card_apdu_resp.sw2 = answer[1];  /* SW2 received                                          */
				}

				/* Check if there is error occur                                                                              */
				if ((answer[0] << 8 | answer[1]) != SC_OP_TERMINATED){
					scStatus = FUN_ERROR;
				}
			}
			/* If the value is the value of INS, apart from the values '6X' and '9X', it is an ACK byte.                    */
			else if(((answer[0] & 0xFE) == ((~(p_card_manage->card_apdu_cmd.header.ins)) & 0xFE)) \
				|| ((answer[0] & 0xFE) ==    (p_card_manage->card_apdu_cmd.header.ins   & 0xFE))){
					
				p_card_manage->card_apdu_resp.data[0] = answer[0];       /* ACK received                                   */
			}
		} /* End of if (g_smartCardManage.scOverTimer != 0)                                                               */
		else{
			
			scStatus = FUN_TIMEOUT;
		}
	}
  
	/* The device replies with a "procedure byte” normally equal to the INS code received. If a problem occurred, then the
     device will respond with a status word pair SW1-SW2, indicating the end of the command                           */
	if ((p_card_manage->card_apdu_resp.sw1 == 0x00) && (scStatus == HAL_OK)){
		/* Send body data to SC                                                                                           */
		if (p_card_manage->card_apdu_cmd.body.le == WRITE_TO_SC){
			/* Send body data                                                                                               */
			HAL_SMARTCARD_Transmit(&p_card_manage->handle, (uint8_t *)&p_card_manage->card_apdu_cmd.body.data[0],p_card_manage->card_apdu_cmd.body.lc, SC_TRANSMIT_TIMEOUT);
      
			/* Flush the SC_USART DR                                                                                        */
			__HAL_SMARTCARD_FLUSH_DRREGISTER(&p_card_manage->handle);
//      __HAL_SMARTCARD_DMA_REQUEST_DISABLE(&g_smartCardHandle, SMARTCARD_DMAREQ_RX); /* Disable the DMA Receive        */  
      
			status = HAL_SMARTCARD_Receive(&p_card_manage->handle, (uint8_t *)&answer[0], 2, SC_RECEIVE_TIMEOUT);

			if (status == HAL_OK){
				/* Check if there is error occur                                                                              */
				if ((answer[0] << 8 | answer[1]) != SC_OP_TERMINATED){
					scStatus = FUN_ERROR;
				}
			}
			else{
				scStatus = FUN_TIMEOUT;
			}
		} /* End of if(p_cardManageHandle->cardApduCmd.Body.LC)                                                           */
		else if (p_card_manage->card_apdu_cmd.body.le == READ_FROM_SC){ /* Or receive body data from SC                 */
			
			status = HAL_SMARTCARD_Receive(&p_card_manage->handle, (uint8_t *)&answer[0], (p_card_manage->card_apdu_cmd.body.lc + 2),4000);
      
	  
			if (status == HAL_OK){
				
				/* Decode the body data                                                                                       */
				for(i = 0; i < p_card_manage->card_apdu_cmd.body.lc; i++){
					p_card_manage->card_apdu_resp.data[i] = answer[i];
				}

				/* Check if there is error occur                                                                              */
				if ((answer[p_card_manage->card_apdu_cmd.body.lc] << 8 | answer[p_card_manage->card_apdu_cmd.body.lc +1]) != SC_OP_TERMINATED){
					scStatus = FUN_ERROR;
				}
			}
			else{
				scStatus = FUN_TIMEOUT;
			}
      
		} /* End of else if (p_cardManageHandle->cardApduCmd.Body.LE == READ_FROM_SC)                                     */
	} /*End of if(p_cardManageHandle->cardApduResp.SW1 == 0x00)                                                         */ 
	
	return scStatus;
}

/** 
* 解释智能卡ATR数据. 
* ATR保存着各个类型信息，用于判断卡类型. 
* @param[in]   p_card_manage：智能卡硬件句柄,p_atrBuf:atr数据,atrLen:atr长度. 
* @param[out]  none.  
* @retval  是否为智能卡的T0协议
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-17创建 
*/
static uint8_t sc_decode_atr(CARD_MANAGE_T *p_card_manage,uint8_t *p_atrBuf,uint8_t atrLen)
{
	uint8_t i        = 0u;
	uint8_t buf      = 0u;
	uint8_t flag     = 0u;
	uint8_t protocol = 0u;
  
	/* ATR has at least two bytes and the first byte must is TS byte                                                    */
	if ((atrLen > 1) && ((p_atrBuf[0] == ATR_DIRECT_CONVERTION) || (p_atrBuf[0] == ATR_INVERSE_CONVERTION))){
		if (p_atrBuf[0] == ATR_INVERSE_CONVERTION){
			sc_InverseBuf(&p_atrBuf[0],atrLen);
			p_card_manage->sc_inverse_flag = true;
		}

		p_card_manage->card_atr.ts = p_atrBuf[0];             /* Initial character                                      */
		p_card_manage->card_atr.t0 = p_atrBuf[1];             /* Format character                                       */
		p_card_manage->card_atr.h_length = p_atrBuf[1] & 0x0F; /* Bits 4 to 1 store number of historical bytes           */

		/* Check if there are TA1, TB1, TC1, TD1 present                                                                  */
		for (i = 0; i < 4; i++){
			
			/* Bits 8 to 5 state whether TAi for bit 5, TBi for bit 6, TCi for bit 7, TDi for bit 8                         */
			p_card_manage->card_atr.t_length = p_card_manage->card_atr.t_length + (((p_card_manage->card_atr.t0 & (uint8_t)0xF0) >> (4 + i)) & (uint8_t)0x1);
		}

		/* Store the value of TA1,TB1,TC1,TD1 if there are present                                                        */
		for (i = 0; i < p_card_manage->card_atr.t_length; i++){
			p_card_manage->card_atr.t[i] = p_atrBuf[i + 2];
		}

		/* Check the protocol that the card support(If TD1 is absent, then the only offer is T=0.)                        */
		if ((p_card_manage->card_atr.t0 & (uint8_t)0x80) == 0x00){
			
			protocol = SC_T0_PROTOCOL;
		}
		else{
			flag = 1;
			protocol = p_card_manage->card_atr.t[p_card_manage->card_atr.t_length - 1] & (uint8_t)0x0F;
		}

		while (flag){
			
			/* Check if there are another group of TAi, TBi, TCi, TDi present or not                                        */
			if ((p_card_manage->card_atr.t[p_card_manage->card_atr.t_length - 1] & (uint8_t)0x80) == 0x80){	
				flag = 1;                           /* Yes, there are another group of  TAi, TBi, TCi, TDi present            */
			}
			else{
				flag = 0;                           /* No,  this is the last group of  TAi, TBi, TCi, TDi present             */
			}
			buf = p_card_manage->card_atr.t_length;
			p_card_manage->card_atr.t_length = 0;

			/* Check if another group of TAi, TBi, TCi, TDi present                                                         */
			for (i = 0; i < 4; i++){
				/* Bits 8 to 5 state whether TAi for bit 5, TBi for bit 6, TCi for bit 7, TDi for bit 8                       */
				p_card_manage->card_atr.t_length = p_card_manage->card_atr.t_length + (((p_card_manage->card_atr.t[buf - 1] & (uint8_t)0xF0) >> (4 + i)) & (uint8_t)0x1);
			}
      
			/* Store the value of TAi,TBi,TCi,TDi if there are present                                                      */
			for (i = 0;i < p_card_manage->card_atr.t_length; i++){
				
				p_card_manage->card_atr.t[buf + i] = p_atrBuf[i + 2 + buf];
			}
			p_card_manage->card_atr.t_length += (uint8_t)buf;
		}

		/* Copy all the number of historical bytes                                                                        */
		for (i = 0; i < p_card_manage->card_atr.h_length; i++){
			p_card_manage->card_atr.h[i] = p_atrBuf[i + 2 + p_card_manage->card_atr.t_length];
		}
	} /* End of if ((atrLen > 1) && ((p_atrBuf[0] == ATR_DIRECT_CONVERTION) || (p_atrBuf[0] == ATR_INVERSE_CONVERTION)))*/

	return (uint8_t)protocol;
}

/** 
* 获取智能卡ATR数据. 
* 第一次获取失败会重复获取第二次. 
* @param[in]   p_card_manage:智能卡接口句柄，p_atrBuf:返回的ATR数据指针,length：获取长度. 
* @param[out]  无.  
* @retval  FUN_STATUS_T:函数状态
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
static FUN_STATUS_T sc_atr_req(CARD_MANAGE_T *p_card_manage, uint8_t *p_atrBuf, uint8_t length)
{
	FUN_STATUS_T atrStatus=FUN_OK;
  
	if( sc_recv(p_card_manage,p_atrBuf,length) == FUN_OK ){
		if (p_atrBuf[0] != 0x00){
			
			atrStatus = FUN_OK;
			ltc1955_reset(p_card_manage->card_port, 1);
		}
		else{
			vTaskDelay(10);
			ltc1955_reset(p_card_manage->card_port, 0);
			vTaskDelay(10);
			ltc1955_reset(p_card_manage->card_port, 1);
			if( sc_recv(p_card_manage,p_atrBuf,length) == FUN_OK ){
				if (p_atrBuf[0] != 0x00){
					
					atrStatus = FUN_OK;
				}
				else{
					atrStatus = FUN_ERROR;
					ltc1955_reset(p_card_manage->card_port, 0);
				}
			}
			else{
				atrStatus = FUN_ERROR;
				ltc1955_reset(p_card_manage->card_port, 0);
			}
		}
	}
	return atrStatus;
}

FUN_STATUS_T sc_compare_vote_type( uint8_t type_bit, uint8_t type_no )
{
	for( uint8_t i=0; i<type_no; i++ ){
		if( type_bit & (1<<i) )
			continue;
		else{
			return FUN_ERROR;
		}
	}
	return FUN_OK;
}

/** 
* 激活投票卡. 
* 擦除所有扇区，根据选择的类型写入相应区域. 
* @param[in]   vote_type:投票类型(0-3). 
* @param[out]  无.  
* @retval  0  成功 
* @retval  ERROR   错误  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-26创建 
*/
FUN_STATUS_T sc_activate_vote_card(uint8_t vote_type)
{
	uint8_t unique_id[7];
	uint8_t ebb_type=0;
				
	if( sc_read_config_register(&ACTIVATE_CARD,IDN_R,unique_id) != FUN_OK )
		return FUN_ERROR;
	display_process_bar(2);
	
	SC_CARD_INFO_T card_info;
	memset(&card_info,0,sizeof(SC_CARD_INFO_T));
	
	if( vote_type==0 ){
		
		if( FUN_OK != devfs_read_type_numbers( &ebb_type ) ){
			return FUN_ERROR;
		}
		display_process_bar(4);
		
		///<比较多个区域是否允许操作
		if( FUN_OK != sc_compare_vote_type( unique_id[SC_CARD_TYPE_IDF], ebb_type ) ){
			return FUN_ERROR;
		}
		display_process_bar(6);
		
		for( uint8_t i=0; i<ebb_type; i++ ){
			
			if( sc_write_storage(&ACTIVATE_CARD,i*ACTIVATE_CARD.block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) != FUN_OK){
				return FUN_ERROR;
			}
		}
		display_process_bar(8);
		
		memcpy(card_info.unique_id,unique_id,7);
		card_info.vote_count = 0;
		memcpy(card_info.key,default_password_config.voter_key,32);
		display_process_bar(10);
		
		for( uint8_t i=0; i<ebb_type; i++ ){
			
			if( crypt_write(&ACTIVATE_CARD,i*ACTIVATE_CARD.block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) == 0 ){
				return FUN_ERROR;
			}
		}
		display_process_bar(16);
		return FUN_OK;
	}
	else if( vote_type<VOTE_TYPE_MAX ){
		
		///<比较当前区域是否允许操作
		if( unique_id[SC_CARD_TYPE_IDF] & (1<<vote_type) ){
			
			if( sc_write_storage(&ACTIVATE_CARD,(vote_type-1)*ACTIVATE_CARD.block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) != FUN_OK){
				return FUN_ERROR;
			}
			display_process_bar(4);
			
			memcpy(card_info.unique_id,unique_id,7);
			card_info.vote_count = 0;
			memcpy(card_info.key,default_password_config.voter_key,32);
			display_process_bar(6);
			
			if( crypt_write(&ACTIVATE_CARD,(vote_type-1)*ACTIVATE_CARD.block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) == 0 ){
				return FUN_ERROR;
			}
			
			display_process_bar(16);
			return FUN_OK;
		}
	}
	return FUN_ERROR;
}

/** 
* 显示相应的插卡状态的LED灯. 
* 无. 
* @param[in]   p_card_manage：智能卡硬件句柄,sta:led开关. 
* @param[out]  none.  
* @retval  FUN_STATUS_T:程序状态
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T sc_card_insert_led( CARD_MANAGE_T *p_card_manage, uint8_t sta )
{
	if( p_card_manage->card_port==CARD_A ){
		ioc_led(1, sta);
	}
	else if( p_card_manage->card_port==CARD_B ){
		ioc_led(2, sta);
	}
	return FUN_OK;
}

/** 
* 智能卡功能处理句柄. 
* 检测卡片的ATR及卡片类型，读写卡. 
* @param[in]   p_card_manage：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-17创建 
*/
static void sc_function_handle(CARD_MANAGE_T *p_card_manage)
{
	CARD_ERROR_T cardStatus = ERR_CARD_NONE;
	static uint8_t write_card=0;
	static uint16_t err_cnt=0;
	static uint16_t ok_cnt=0;
	static uint8_t complete =1;
	uint32_t cmd = HAL_RCC_GetPCLK2Freq();
	switch (p_card_manage->card_state){
		
		case SC_POWER_ON :
			ltc1955_power(p_card_manage->card_port,VCC_3V);					/* Enable CMDVCC                                                 */
			sc_usart_mode(p_card_manage,SMARTCARD_MODE_TX_RX);
			samrtcard_reset_para(p_card_manage);
			ltc1955_reset(p_card_manage->card_port,0);
			vTaskDelay(20);
			ltc1955_reset(p_card_manage->card_port,1);
			vTaskDelay(1);
		
			switch_smartcard_step(SC_GET_ATR);

		case SC_GET_ATR :
			if (sc_atr_req(p_card_manage,(uint8_t *)&(p_card_manage->sc_atr_table[0]),ATR_BUFFER_SIZE) == FUN_OK){
				switch_smartcard_step(SC_DECODE_ATR);
			}
			else{
				if( FUN_OK == sc_read_config_register(p_card_manage,ATR_R,p_card_manage->sc_atr_table) )
					switch_smartcard_step(SC_DECODE_ATR);
				else
					switch_smartcard_step(SC_WAIT_OUT);
//				display_info_in("smartcard no \nreply atr",0);
			}
			break;

		case SC_DECODE_ATR :    /* Get information from ATR when the card still inserted                                  */
			if ((sc_decode_atr(p_card_manage,&(p_card_manage->sc_atr_table[0]),p_card_manage->sc_atr_len) == SC_T0_PROTOCOL)){
				
				if( sc_idf_card_type(p_card_manage)!=0xff )
					switch_smartcard_step(SC_IDEN_AND_AUTH);
				else{
					switch_smartcard_step(SC_WRITE_ATR);
//					display_info_in("smartcard is not \nauthorization",0);
				}
			}
			else{
				switch_smartcard_step(SC_WAIT_OUT);
//				display_info_in("smartcard is not \nT0 protcol",0);
			}
			break;
			
		case SC_WRITE_ATR:
		{
			static uint8_t atr_type=0xff;
			smartcard_log("Please select write ATR card type\r\n");
			switch( atr_type )
			{
				case VOTE_CARD_CONFIG:
					sc_verify_sc_plaintext(p_card_manage,(smartcard_config[VOTE_CARD_CONFIG].password>>16)&0xff,\
						(smartcard_config[VOTE_CARD_CONFIG].password>>8)&0xff,(smartcard_config[VOTE_CARD_CONFIG].password)&0xff);
					if( FUN_OK == sc_write_config_register(p_card_manage,ATR_R,(uint8_t*)smartcard_config[VOTE_CARD_CONFIG].answer_reset)){
						smartcard_log("Write VOTE ATR card type successful\r\n");
					}
					else{
						smartcard_log("Write VOTE ATR card type failed\r\n");
					}
					break;
				case ADMIN_CARD_CONFIG:
					sc_verify_sc_plaintext(p_card_manage,(smartcard_config[ADMIN_CARD_CONFIG].password>>16)&0xff,\
						(smartcard_config[ADMIN_CARD_CONFIG].password>>8)&0xff,(smartcard_config[ADMIN_CARD_CONFIG].password)&0xff);
					if( FUN_OK == sc_write_config_register(p_card_manage,ATR_R,(uint8_t*)smartcard_config[ADMIN_CARD_CONFIG].answer_reset)){
						smartcard_log("Write ADMIN ATR card type successful\r\n");
					}
					else{
						smartcard_log("Write ADMIN ATR card type failed\r\n");
					}
					break;
				case MAINTE_CARD_CONFIG:
					sc_verify_sc_plaintext(p_card_manage,(smartcard_config[MAINTE_CARD_CONFIG].password>>16)&0xff,\
						(smartcard_config[MAINTE_CARD_CONFIG].password>>8)&0xff,(smartcard_config[MAINTE_CARD_CONFIG].password)&0xff);
					if( FUN_OK == sc_write_config_register(p_card_manage,ATR_R,(uint8_t*)smartcard_config[MAINTE_CARD_CONFIG].answer_reset)){
						smartcard_log("Write MAINTE card type successful\r\n");
					}
					else{
						smartcard_log("Write MAINE ATR card type failed\r\n");
					}
					break;
				case SYSTEM_CARD_CONFIG:
					sc_verify_sc_plaintext(p_card_manage,(smartcard_config[SYSTEM_CARD_CONFIG].password>>16)&0xff,\
						(smartcard_config[SYSTEM_CARD_CONFIG].password>>8)&0xff,(smartcard_config[SYSTEM_CARD_CONFIG].password)&0xff);
					if( FUN_OK == sc_write_config_register(p_card_manage,ATR_R,(uint8_t*)smartcard_config[SYSTEM_CARD_CONFIG].answer_reset)){
						smartcard_log("Write SYSTEM ATR card type successful\r\n");
					}
					else{
						smartcard_log("Write SYSTEM ATR card type failed\r\n");
					}
					break;
				case BACKUP_CARD_CONFIG:
					sc_verify_sc_plaintext(p_card_manage,(smartcard_config[BACKUP_CARD_CONFIG].password>>16)&0xff,\
						(smartcard_config[BACKUP_CARD_CONFIG].password>>8)&0xff,(smartcard_config[BACKUP_CARD_CONFIG].password)&0xff);
					if( FUN_OK == sc_write_config_register(p_card_manage,ATR_R,(uint8_t*)smartcard_config[BACKUP_CARD_CONFIG].answer_reset)){
						smartcard_log("Write BACKUP ATR card type successful\r\n");
					}
					else{
						smartcard_log("Write BACKUP ATR card type failed\r\n");
					}
					break;
				default:
					break;
			}
			switch_smartcard_step(SC_WAIT_OUT);
			break;
		}
		case SC_IDEN_AND_AUTH :
			if( write_card ){
				write_card = 0;
//				sc_sta.atr_type = 0;
				
				p_card_manage->block_num = 4;
				p_card_manage->block_size = 128;
				p_card_manage->auth_pwd = (SMARTCARD_AUTH_PWD_T *)&smartcard_auth[p_card_manage->atr_type];
				p_card_manage->storage_size = p_card_manage->block_num*p_card_manage->block_size;
			
				uint8_t state=0;
				static uint8_t vc_flag=0,card_type=0,wr_flag=0,card_reg=0;
			
				if( card_type!=0 ){
					uint8_t unique_id[7];
				
					uint8_t dcr = 0;
					sc_write_config_register(p_card_manage,DCR_R,&dcr);
				
					sc_read_config_register(p_card_manage,IDN_R,unique_id);
	
					SC_CARD_INFO_T card_info;
					memset(&card_info,0,sizeof(SC_CARD_INFO_T));
					sc_write_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
					sc_write_storage(p_card_manage,1*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
					sc_write_storage(p_card_manage,2*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
					sc_write_storage(p_card_manage,3*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
				
					memcpy(card_info.unique_id,unique_id,7);
					card_info.vote_count = 0;
					if( card_type<=4 ){
					
						static uint8_t write_vote_test_card=0;
						
						if( write_vote_test_card==0 ){
						
							memcpy(card_info.key,default_password_config.voter_key,32);
							#if defined(SC_USE_CRYPT)
							if( crypt_write(p_card_manage,(card_type-1)*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) ){
							#else
							if( sc_write_storage(p_card_manage,(card_type-1)*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info)==FUN_OK ){
							#endif
								smartcard_log("write unipue id:");
								smartcard_dump(card_info.unique_id,7);
								smartcard_log("write passwrod:");
								smartcard_dump((uint8_t *)default_password_config.voter_key,32);
								memset(&card_info,0,sizeof(SC_CARD_INFO_T));
								#if defined(SC_USE_CRYPT)
								crypt_read(p_card_manage,(card_type-1)*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
								#else
								sc_read_storage(p_card_manage,(card_type-1)*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info);
								#endif
							
								smartcard_log("read unipue id:");
								smartcard_dump(card_info.unique_id,7);
								smartcard_log("read passwrod:");
								smartcard_dump((uint8_t *)default_password_config.voter_key,32);
								
								char buf[16];
								snprintf(buf,16,"vote %d card",card_type);
								display_info_in(buf,0);
							}
						}
						else{
							write_vote_test_card = 0;
							
							memcpy(card_info.key,"TEST VOTE CARD \\ TEST VOTE CARD",32);
							
							uint8_t i;
							for( i=0; i<4; i++ ){
								if( crypt_write(p_card_manage,i*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) ){
									continue;
								}
								else{
									break;
								}
							}
							if( i==4 ){
								display_info_in("TEST VOTE CARD WRITE SUCCESSFULLY",0);
								smartcard_log("TEST VOTE CARD WRITE SUCCESSFULLY");
							}
							else{
								display_info_in("TEST VOTE CARD WRITE FAILED",0);
								smartcard_log("TEST VOTE CARD WRITE FAILED");
							}
						}
					}
					else if( card_type==5 ){
						memcpy(card_info.key,default_password_config.administrator_key,32);
						#if defined(SC_USE_CRYPT)
						if( crypt_write(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) ){
						#else
						if( sc_write_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info)== FUN_OK ){
						#endif
							memset(&card_info,0,sizeof(SC_CARD_INFO_T));
							#if defined(SC_USE_CRYPT)
							crypt_read(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
							#else
							sc_read_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info);
							#endif
							sc_read_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
							display_info_in("administrator card",0);
						}
					}
					else if( card_type==6 ){
						memcpy(card_info.key,default_password_config.maintenance_key,32);
						#if defined(SC_USE_CRYPT)
						if( crypt_write(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) ){
						#else
						if( sc_write_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info) == FUN_OK ){
						#endif
							memset(&card_info,0,sizeof(SC_CARD_INFO_T));
							#if defined(SC_USE_CRYPT)
							crypt_read(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
							#else
							sc_read_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info);
							#endif
							display_info_in("maintenance card",0);
						}
					}
					else if( card_type==7 ){
						memcpy(card_info.key,default_password_config.system_administrator_key1,32);
						#if defined(SC_USE_CRYPT)
						if( crypt_write(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) ){
						#else
						if( sc_write_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info) == FUN_OK ){
						#endif
							memset(&card_info,0,sizeof(SC_CARD_INFO_T));
							#if defined(SC_USE_CRYPT)
							crypt_read(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
							#else
							sc_read_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info);
							#endif
						}
						
						memcpy(card_info.key,default_password_config.system_administrator_key2,32);
						#if defined(SC_USE_CRYPT)
						if( crypt_write(p_card_manage,1*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info) ){
						#else
						if( sc_write_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info) == FUN_OK ){
						#endif
							memset(&card_info,0,sizeof(SC_CARD_INFO_T));
							#if defined(SC_USE_CRYPT)
							crypt_read(p_card_manage,1*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),(uint8_t *)&card_info);
							#else
							sc_read_storage(p_card_manage,0*p_card_manage->block_size,sizeof(SC_CARD_INFO_T),&card_info);
							#endif
						}
						display_info_in("system card",0);
					}
					else if( card_type==8 ){
						
					}
					card_type = 0;
				}
			
				if( vc_flag ){
					vc_flag = 0;
					uint8_t pwd_idx;
					if( card_reg<5 )
						pwd_idx = 0;
					else{
						pwd_idx = card_reg-4;
					}
					
					state = sc_verify_sc_plaintext(p_card_manage,(smartcard_config[pwd_idx].password>>16)&0xff,\
						(smartcard_config[pwd_idx].password>>8)&0xff,(smartcard_config[pwd_idx].password)&0xff);				///<at88sc0404
				}
				else{
					state = FUN_ERROR;
				}
				if( state==FUN_OK ){
					uint8_t buf[128];
					
					buf[0]= 0;
					sc_write_config_register(p_card_manage,DCR_R,buf);
					
					//buf[0]= 0x47;
					buf[0]= 0xff;
					sc_write_config_register(p_card_manage,AR0_R,buf);
					sc_write_config_register(p_card_manage,AR1_R,buf);
					sc_write_config_register(p_card_manage,AR2_R,buf);
					sc_write_config_register(p_card_manage,AR3_R,buf);
					buf[0]= 0x08;
					sc_write_config_register(p_card_manage,PR0_R,buf);
					buf[0]= 0x09;
					sc_write_config_register(p_card_manage,PR1_R,buf);
					buf[0]= 0x0a;
					sc_write_config_register(p_card_manage,PR2_R,buf);
					buf[0]= 0x0b;
					sc_write_config_register(p_card_manage,PR3_R,buf);
					
					if( card_reg<=4 ){
						rng_generate_num(2);
						uint8_t id_num[7];
						memcpy(id_num,g_random32bitBuf,7);
						///<保存投票卡的类型在UID
						///<一个标志位代表一个投票类型
						id_num[SC_CARD_TYPE_IDF] = 0x00;
						for( uint8_t i=0; i<card_reg; i++ ){
							id_num[SC_CARD_TYPE_IDF] |= (1<<i);
						}
						
						sc_write_config_register(p_card_manage,SSG0_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].auth_key);
						sc_write_config_register(p_card_manage,SSG1_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].program_key);
						sc_write_config_register(p_card_manage,W0_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R0_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W1_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].zone_write_pwd[1]);
						sc_write_config_register(p_card_manage,R1_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].zone_read_pwd[1]);
						sc_write_config_register(p_card_manage,W2_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].zone_write_pwd[2]);
						sc_write_config_register(p_card_manage,R2_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].zone_read_pwd[2]);
						sc_write_config_register(p_card_manage,W3_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].zone_write_pwd[3]);
						sc_write_config_register(p_card_manage,R3_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].zone_read_pwd[3]);
						//sc_write_config_register(W7_R,&smartcard_config[VOTE_CARD_CONFIG].write7_pwd);
						sc_write_config_register(p_card_manage,IDN_R,id_num);
						sc_write_config_register(p_card_manage,IC_R,(uint8_t *)"laxton company evt vote card    ");
						sc_write_config_register(p_card_manage,ATR_R,(uint8_t *)&smartcard_config[VOTE_CARD_CONFIG].answer_reset);
						
					}
					else if( card_reg==5 ){
						rng_generate_num(2);
						uint8_t id_num[7];
						memcpy(id_num,g_random32bitBuf,7);
						id_num[SC_CARD_TYPE_IDF] = 0x01;
						
						sc_write_config_register(p_card_manage,SSG0_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].auth_key);
						sc_write_config_register(p_card_manage,SSG1_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].program_key);
						sc_write_config_register(p_card_manage,W0_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R0_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W1_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].zone_write_pwd[1]);
						sc_write_config_register(p_card_manage,R1_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].zone_read_pwd[1]);
						sc_write_config_register(p_card_manage,W2_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].zone_write_pwd[2]);
						sc_write_config_register(p_card_manage,R2_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].zone_read_pwd[2]);
						sc_write_config_register(p_card_manage,W3_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].zone_write_pwd[3]);
						sc_write_config_register(p_card_manage,R3_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].zone_read_pwd[3]);
						//sc_write_config_register(W7_R,&smartcard_config[ADMIN_CARD_CONFIG].write7_pwd);
						sc_write_config_register(p_card_manage,IDN_R,id_num);
						sc_write_config_register(p_card_manage,IC_R,(uint8_t *)"laxton company evt admin card   ");
						sc_write_config_register(p_card_manage,ATR_R,(uint8_t *)&smartcard_config[ADMIN_CARD_CONFIG].answer_reset);
					}
					else if( card_reg==6 ){
						rng_generate_num(2);
						uint8_t id_num[7];
						memcpy(id_num,g_random32bitBuf,7);
						id_num[SC_CARD_TYPE_IDF] = 0x01;
						
						sc_write_config_register(p_card_manage,SSG0_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].auth_key);
						sc_write_config_register(p_card_manage,SSG1_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].program_key);
						sc_write_config_register(p_card_manage,W0_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R0_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W1_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].zone_write_pwd[1]);
						sc_write_config_register(p_card_manage,R1_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].zone_read_pwd[1]);
						sc_write_config_register(p_card_manage,W2_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].zone_write_pwd[2]);
						sc_write_config_register(p_card_manage,R2_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].zone_read_pwd[2]);
						sc_write_config_register(p_card_manage,W3_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].zone_write_pwd[3]);
						sc_write_config_register(p_card_manage,R3_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].zone_read_pwd[3]);
						//sc_write_config_register(W7_R,&smartcard_config[MAINTE_CARD_CONFIG].write7_pwd);
						sc_write_config_register(p_card_manage,IDN_R,id_num);
						sc_write_config_register(p_card_manage,IC_R,(uint8_t *)"laxton company evt mainte card  ");
						sc_write_config_register(p_card_manage,ATR_R,(uint8_t *)&smartcard_config[MAINTE_CARD_CONFIG].answer_reset);
					}
					else if( card_reg==7 || card_reg==8 ){
						rng_generate_num(2);
						uint8_t id_num[7];
						memcpy(id_num,g_random32bitBuf,7);
						id_num[SC_CARD_TYPE_IDF] = 0x01;
						
						sc_write_config_register(p_card_manage,SSG0_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].auth_key);
						sc_write_config_register(p_card_manage,SSG1_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].program_key);
						sc_write_config_register(p_card_manage,W0_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R0_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W1_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].zone_write_pwd[1]);
						sc_write_config_register(p_card_manage,R1_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].zone_read_pwd[1]);
						sc_write_config_register(p_card_manage,W2_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].zone_write_pwd[2]);
						sc_write_config_register(p_card_manage,R2_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].zone_read_pwd[2]);
						sc_write_config_register(p_card_manage,W3_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].zone_write_pwd[3]);
						sc_write_config_register(p_card_manage,R3_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].zone_read_pwd[3]);
						//sc_write_config_register(W7_R,&smartcard_config[SYSTEM_CARD_CONFIG].write7_pwd);
						sc_write_config_register(p_card_manage,IDN_R,id_num);
						sc_write_config_register(p_card_manage,IC_R,(uint8_t *)"laxton company evt system card  ");
						sc_write_config_register(p_card_manage,ATR_R,(uint8_t *)&smartcard_config[SYSTEM_CARD_CONFIG].answer_reset);
					}
					else if( card_reg==9 ){
						rng_generate_num(2);
						uint8_t id_num[7];
						memcpy(id_num,g_random32bitBuf,7);
						id_num[SC_CARD_TYPE_IDF] = 0x01;
						
						sc_write_config_register(p_card_manage,SSG0_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].auth_key);
						sc_write_config_register(p_card_manage,SSG1_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].program_key);
						sc_write_config_register(p_card_manage,W0_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R0_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W1_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R1_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W2_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R2_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W3_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R3_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W4_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R4_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W5_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R5_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_read_pwd[0]);
						sc_write_config_register(p_card_manage,W6_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R6_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_read_pwd[0]);
//						sc_write_config_register(W7_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_write_pwd[0]);
						sc_write_config_register(p_card_manage,R7_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].zone_read_pwd[0]);
						//sc_write_config_register(W7_R,&smartcard_config[BACKUP_CARD_CONFIG].write7_pwd);
						sc_write_config_register(p_card_manage,IDN_R,id_num);
						sc_write_config_register(p_card_manage,IC_R,(uint8_t *)"laxton company evt backup card  ");
						sc_write_config_register(p_card_manage,ATR_R,(uint8_t *)&smartcard_config[BACKUP_CARD_CONFIG].answer_reset);
					}
					card_reg = 0;
					
					sc_read_config_register(p_card_manage,DCR_R,buf);
					smartcard_log("DCR %02X\r\n",buf[0]);
					
					sc_read_config_register(p_card_manage,AR0_R,buf);
					smartcard_log("AR0 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PR0_R,buf);
					smartcard_log("PR0 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,AR1_R,buf);
					smartcard_log("AR1 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PR1_R,buf);
					smartcard_log("PR1 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,AR2_R,buf);
					smartcard_log("AR2 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PR2_R,buf);
					smartcard_log("PR2 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,AR3_R,buf);
					smartcard_log("AR3 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PR3_R,buf);
					smartcard_log("PR3 %02X\r\n",buf[0]);
					
					sc_read_config_register(p_card_manage,SSG0_R,buf);
					smartcard_log("SSG0 ");
					smartcard_dump(buf,8);
					sc_read_config_register(p_card_manage,SSG1_R,buf);
					smartcard_log("SSG1 ");
					smartcard_dump(buf,8);
					sc_read_config_register(p_card_manage,SSG2_R,buf);
					smartcard_log("SSG2 ");
					smartcard_dump(buf,8);
					sc_read_config_register(p_card_manage,SSG3_R,buf);
					smartcard_log("SSG3 ");
					smartcard_dump(buf,8);
					
					sc_read_config_register(p_card_manage,W0_R,buf);
					smartcard_log("W0 ");
					smartcard_dump(buf,3);
					sc_read_config_register(p_card_manage,W1_R,buf);
					smartcard_log("W1 ");
					smartcard_dump(buf,3);
					sc_read_config_register(p_card_manage,W2_R,buf);
					smartcard_log("W2 ");
					smartcard_dump(buf,3);
					sc_read_config_register(p_card_manage,W3_R,buf);
					smartcard_log("W3 ");
					smartcard_dump(buf,3);
					sc_read_config_register(p_card_manage,W4_R,buf);
					smartcard_log("W4 ");
					smartcard_dump(buf,3);
					sc_read_config_register(p_card_manage,W5_R,buf);
					smartcard_log("W5 ");
					smartcard_dump(buf,3);
					sc_read_config_register(p_card_manage,W6_R,buf);
					smartcard_log("W6 ");
					smartcard_dump(buf,3);
					sc_read_config_register(p_card_manage,W7_R,buf);
					smartcard_log("W7 ");
					smartcard_dump(buf,3);
					
					sc_read_config_register(p_card_manage,PACW0_R,buf);
					smartcard_log("PACW0 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PACW1_R,buf);
					smartcard_log("PACW1 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PACW2_R,buf);
					smartcard_log("PACW2 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PACW3_R,buf);
					smartcard_log("PACW3 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PACW4_R,buf);
					smartcard_log("PACW4 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PACW5_R,buf);
					smartcard_log("PACW5 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PACW6_R,buf);
					smartcard_log("PACW6 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,PACW7_R,buf);
					smartcard_log("PACW7 %02X\r\n",buf[0]);
					
					sc_read_config_register(p_card_manage,AAC0_R,buf);
					smartcard_log("AAC0 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,AAC1_R,buf);
					smartcard_log("AAC1 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,AAC2_R,buf);
					smartcard_log("AAC2 %02X\r\n",buf[0]);
					sc_read_config_register(p_card_manage,AAC3_R,buf);
					smartcard_log("AAC3 %02X\r\n",buf[0]);
				}
				if( wr_flag ){
					wr_flag = 0;
					uint8_t buf[128];
					for( uint8_t i=0; i<128; i++ )
						buf[i] = i;
					sc_write_storage(p_card_manage,0,128,buf);
	//				sc_write_user_block(0, 0, 128, buf);
					memset(buf,0,128);
					vTaskDelay( 100 );
	//				sc_read_user_block(0, 0, 128, buf);
					sc_read_storage(p_card_manage,0,128,buf);
					uint8_t i;
					for( i=0; i<128; i++ ){
						if( buf[i] != i ){
							err_cnt++;
							break;
						}
					}
					if( i==128 ){
						ok_cnt++;
					}
					smartcard_log("ok %d-err %d\r\n",ok_cnt, err_cnt);
					vTaskDelay( 100 );
					write_card = 1;
					wr_flag = 1;
	//				smartcard_dump(&buf[112],16);
				}
			}
			else if ((cardStatus = sc_card_identify(p_card_manage)) == ERR_CARD_NONE){
			
				p_card_manage->card_state = SC_ACTIVE;
			 
			}
			else
			{
//				app_SmartCardShowErrorMsg(cardStatus);
//				bsp_SetBuzzerAlarm(BZ_EVENT_CARD_ERROR); 
//				sc_power_control(p_card_manage, DISABLE);
				p_card_manage->card_state = SC_WAIT_OUT;
			}
			break;

		case SC_ACTIVE       :
		{
			sc_card_insert_led(p_card_manage,1);
		}
		break;
    
		case SC_WAIT_OUT     :
//			ltc1955_power(p_card_manage->card_port,0);
//			ltc1955_reset(p_card_manage->card_port, 0);
			break;

		case SC_REMOVED      :
			sc_card_insert_led(p_card_manage,0);
			smartcard_deinit(p_card_manage);
			sc_popup_card_menu(p_card_manage);
			break;
        
		default              :
			p_card_manage->card_state = SC_POWER_OFF;
		break;
	}
}

/** 
* 检测智能卡插入状态. 
* 无. 
* @param[in]   p_card_manage：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  card_status:返回卡插入状态
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-26创建 
*/
static CARD_STATUS_T sc_check_insert_status(CARD_MANAGE_T *p_card_manage)
{
	CARD_STATUS_T card_status;
	if( p_card_manage->card_port==CARD_A ){
		
		if (LTC1955_IS_CARD_A_PRESENT()) {
			
			card_status = CARD_INSERTED;
		}
		else{
			card_status = CARD_NOT_INSERTED;
		}

		return card_status;
	}
	else if( p_card_manage->card_port==CARD_B ){
		
		if (LTC1955_IS_CARD_B_PRESENT()) {
			card_status = CARD_INSERTED;
		}
		else{
			card_status = CARD_NOT_INSERTED;
		}

		return card_status;
	}
	
	return CARD_NOT_INSERTED;
}

/** 
* 在指定的端口是否插入了指定的智能卡类型. 
* 无. 
* @param[in]   card_port:智能卡端口，card_type:卡类型. 
* @param[out]  none.  
* @retval  FUN_STATUS_T
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-11-19创建 
*/
FUN_STATUS_T sc_card_type_insert(uint8_t card_port, CARD_PRESENT_T card_type)
{
	for( uint8_t i=0; i<SMARTCARD_NUM; i++ ){
		if( smartcard_manage[i].card_port == card_port ){
			if( smartcard_manage[i].card_present == card_type ){
				return FUN_OK;
			}
		}
	}
	return FUN_ERROR;
}

/** 
* 根据智能卡的插入状态设置卡数据的状态. 
* 设置电源状态，卡插入状态. 
* @param[in]   p_card_manage：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-26创建 
*/
void sc_insert_detect(CARD_MANAGE_T *p_card_manage)
{
	static uint8_t	card_state[SMARTCARD_NUM]={SC_REMOVED,SC_REMOVED};
	
	if( p_card_manage->card_port > CARD_NONE ){
		if( card_state[p_card_manage->card_port-1] != SC_POWER_ON && sc_check_insert_status(p_card_manage) == CARD_INSERTED ){
			
			p_card_manage->card_state  = SC_POWER_ON; 
			p_card_manage->card_status = CARD_INSERTED; 
			card_state[p_card_manage->card_port-1] = SC_POWER_ON;
		}
		else if( card_state[p_card_manage->card_port-1] != SC_REMOVED && sc_check_insert_status(p_card_manage) == CARD_NOT_INSERTED  ){
			p_card_manage->card_state  = SC_REMOVED; 
			p_card_manage->card_status = CARD_NOT_INSERTED; 
			card_state[p_card_manage->card_port-1] = SC_REMOVED;
		}
	}
}

/** 
* 智能卡任务处理线程. 
* 无. 
* @param[in]   p_card_manage：智能卡硬件句柄. 
* @param[out]  none.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-07-26创建 
*/
void sc_task(void *pvParameters)
{
	///<等待所有模块初始化完成才执行线程
	wait_for_startup();
	
	while(1){
		
		ltc1955_poll_operation();
		
		for( uint8_t i=0; i<SMARTCARD_NUM; i++ ){
			
			sc_insert_detect(&smartcard_manage[i]);
			
//			if ((smartcard_manage[i].card_status == CARD_INSERTED) && (smartcard_manage[i].card_state != SC_POWER_OFF)){
				sc_function_handle(&smartcard_manage[i]);
//			}
		}
		
		vTaskDelay(10);
	}
}

/** 
* 智能卡硬件接口初始化. 
* 无. 
* @param[in]   无. 
* @param[out]  none.  
* @retval  FUN_STATUS_T：函数返回状态
* @par 标识符 
*      保留 
* @par 其它 
*      无
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T sc_init(void)
{
	ltc1955_init();
	smartcard_manage[0].card_port = CARD_A;
	smartcard_manage[1].card_port = CARD_B;
	
	ltc1955_reset(1,0);
	ltc1955_reset(2,0);
		
	smartcard_event_handler=xEventGroupCreate();      /*创建事件标志组*/
	if( smartcard_event_handler==NULL )
		return FUN_ERROR;
	if( xTaskCreate( sc_task, "SMARTCARD", SMARTCARD_STACK_SIZE, NULL, SMARTCARD_TASK_PRIORITY, NULL ) == pdPASS)
		return FUN_OK;
	else
		return FUN_ERROR;
}
