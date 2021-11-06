/** 
* @file         crypt.c 
* @brief        AES256 加密解密硬件计算. 
* @details  	无. 
* @author       ken deng 
* @date     	2019-07-26
* @version  	A001 
* @par Copyright (c):  
*       laxton company 
* @par History:          
*   version: ken deng, 2019-07-26, desc\n 
*/  
#include "system.h"
#include "crypt.h"
//#include "at88sc.h"
#include "smartcard.h"
//#include "aes.h"
#include "secure_storage.h"
#include "string.h"
//#include "aes.h"
#include "stm32f4xx_hal.h"

#undef CRYPT_DEBUG
#ifdef CRYPT_DEBUG
    #define	crypt_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[CRYPT](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define crypt_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[CRYPT] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define crypt_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[CRYPT] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define crypt_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define crypt_log(...)
    #define crypt_usr(...)
    #define crypt_err(...)
    #define crypt_dump(...)
#endif

CRYP_HandleTypeDef hcryp;
DMA_HandleTypeDef hdma_cryp_in;
DMA_HandleTypeDef hdma_cryp_out;

xSemaphoreHandle	crypt_semaphore;
uint8_t crypt_complete=0;
//extern void Error_Handler(SYSTEM_ERROR_CODE_TypeDef errorCode);

/**
* @brief CRYP MSP Initialization
* This function configures the hardware resources used in this example
* @param hcryp: CRYP handle pointer
* @retval None
*/
void HAL_CRYP_MspInit(CRYP_HandleTypeDef* hcryp)
{

  if(hcryp->Instance==CRYP)
  {
  /* USER CODE BEGIN CRYP_MspInit 0 */

  /* USER CODE END CRYP_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CRYP_CLK_ENABLE();
    /* CRYP interrupt Init */
    HAL_NVIC_SetPriority(CRYP_IRQn, 14, 0);
    HAL_NVIC_EnableIRQ(CRYP_IRQn);
  /* USER CODE BEGIN CRYP_MspInit 1 */

  /* USER CODE END CRYP_MspInit 1 */
  }

}

/**
* @brief CRYP MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hcryp: CRYP handle pointer
* @retval None
*/

void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef* hcryp)
{

  if(hcryp->Instance==CRYP)
  {
  /* USER CODE BEGIN CRYP_MspDeInit 0 */

  /* USER CODE END CRYP_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRYP_CLK_DISABLE();

    /* CRYP interrupt DeInit */
    HAL_NVIC_DisableIRQ(CRYP_IRQn);
  /* USER CODE BEGIN CRYP_MspDeInit 1 */

  /* USER CODE END CRYP_MspDeInit 1 */
  }

}

static void MX_CRYP_Init(uint8_t *key)
{

  /* USER CODE BEGIN CRYP_Init 0 */

  /* USER CODE END CRYP_Init 0 */

  /* USER CODE BEGIN CRYP_Init 1 */
	
  /* USER CODE END CRYP_Init 1 */
  hcryp.Instance = CRYP;
  hcryp.Init.DataType = CRYP_DATATYPE_8B;
  hcryp.Init.KeySize = CRYP_KEYSIZE_256B;
  hcryp.Init.pKey = (uint8_t *)key;
  if (HAL_CRYP_Init(&hcryp) != HAL_OK)
  {
//    Error_Handler();
  }
  /* USER CODE BEGIN CRYP_Init 2 */

  /* USER CODE END CRYP_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 14, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 14, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief This function handles CRYP global interrupt.
  */
void CRYP_IRQHandler(void)
{
  /* USER CODE BEGIN CRYP_IRQn 0 */

  /* USER CODE END CRYP_IRQn 0 */
  HAL_CRYP_IRQHandler(&hcryp);
  /* USER CODE BEGIN CRYP_IRQn 1 */

  /* USER CODE END CRYP_IRQn 1 */
}

/** 
* AES256执行完成回调. 
* 设置标志位，加解密函数等待这个标志位. 
* @param[in]   hcryp:AES256句柄. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Kend Deng于2019-07-26创建 
*/
void HAL_CRYP_OutCpltCallback(CRYP_HandleTypeDef *hcryp){
	crypt_complete = 1;
}

/** 
* 阻塞式AES256加密函数. 
* 判断是不16字节倍数，启动硬件加密并等待结果. 
* @param[in]   key:秘钥，in:输入明文数据，out:输出密文数据，size:长度，必需16的倍数. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Kend Deng于2019-07-26创建 
*/
void crypt_encryption_block(uint8_t *key, uint8_t *in, uint8_t *out, uint16_t size){
	
	if( size % 16 == 0 && size!=0 ){
		MX_CRYP_Init(key);
		HAL_CRYP_AESECB_Encrypt(&hcryp, in, size, out,10);
	}
}

/** 
* AES256加密函数. 
* 判断是不16字节倍数，启动硬件加密并等待结果. 
* @param[in]   key:秘钥，in:输入明文数据，out:输出密文数据，size:长度，必需16的倍数. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Kend Deng于2019-07-26创建 
*/
void crypt_encryption(uint8_t *key, uint8_t *in, uint8_t *out, uint16_t size){
	
	if( size % 16 == 0 && size!=0 ){
		MX_CRYP_Init(key);
//		__HAL_CRYP_RESET_HANDLE_STATE(&hcryp);
		crypt_complete = 0;
		HAL_CRYP_AESECB_Encrypt_IT(&hcryp, in, size, out);
		while(crypt_complete==0){
			vTaskDelay(1);
		}
//		xSemaphoreTake(crypt_semaphore,1000);
	}
}

/** 
* AES256解密函数. 
* 判断是不16字节倍数，启动硬件解密并等待结果. 
* @param[in]   key:秘钥，in:输入明文数据，out:输出密文数据，size:长度，必需16的倍数. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Kend Deng于2019-07-26创建 
*/
void crypt_decryption(uint8_t *key, uint8_t *in, uint8_t *out, uint16_t size){
	
	if( size % 16 == 0 && size!=0 ){
		MX_CRYP_Init(key);
//		__HAL_CRYP_RESET_HANDLE_STATE(&hcryp);
		crypt_complete = 0;
		HAL_CRYP_AESECB_Decrypt_IT(&hcryp, in, size, out);
		while(crypt_complete==0){
			vTaskDelay(1);
		}
//		xSemaphoreTake(crypt_semaphore,100);
	}
}

/** 
* 加密写. 
* 由于AES256算法是16字节区域加密，对写起始地址及结束地址进行16倍数的调整，使用读-解密-改-加密-写方案. 
* 用于智能卡，备份存储卡，内部加密存储的加密
* @param[in]   dev:设备类型，addr:起始地址，len:长度，ptr:数据缓冲. 
* @param[out]  无.  
* @retval  0,写失败，其它：写入长度
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Kend Deng于2019-07-26创建 
*/
uint16_t crypt_write(CARD_MANAGE_T *card_manage,uint16_t addr, uint16_t len, uint8_t *ptr){
	
	xSemaphoreTake(crypt_semaphore,portMAX_DELAY);
	
	uint16_t(*write_func)(CARD_MANAGE_T *card_manage,uint16_t addr, uint16_t len, uint8_t *ptr); 
	uint16_t(*read_func)(CARD_MANAGE_T *card_manage,uint16_t addr, uint16_t len, uint8_t *ptr); 
	
	CARD_MANAGE_T *card_ptr;
	int8_t	reval=0;
	
	if( card_manage==NULL ){
		write_func = NULL;//at88sc_write_storage;
		read_func = NULL;//at88sc_read_storage;
		card_ptr = NULL;
	}
	else{
		write_func = sc_write_storage;
		read_func = sc_read_storage;
		card_ptr = card_manage;
	}
	
	uint16_t start_block, end_block;
	start_block = addr/CRYPT_BLOCK_SIZE;
	end_block = (addr+len)/CRYPT_BLOCK_SIZE;
	uint16_t size = (end_block-start_block+1)*CRYPT_BLOCK_SIZE;
	
	uint8_t *read_buf = malloc(size);
	uint8_t	*write_buf = malloc(size);
	
	if( read_buf != NULL && write_buf != NULL ){
	
		memset(read_buf,0,size);
		memset(write_buf,0,size);
		
		uint8_t start_addr = addr%CRYPT_BLOCK_SIZE;
						
		//memcpy(read_block,temp+start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE);
		if( read_func(card_ptr,start_block*CRYPT_BLOCK_SIZE,size,read_buf) != FUN_OK){
			reval = 1;
		}
//		crypt_log("write encrypt:");
//		crypt_dump(read_buf,size);
		if( !reval )
			crypt_decryption((uint8_t*)&default_password.encryption_key,read_buf,write_buf,size);
//		crypt_log("write plaintext read:");
//		crypt_dump(write_buf,size);
		
		memcpy(write_buf + start_addr, ptr, len);
//		crypt_log("write plaintext cpy:");
//		crypt_dump(write_buf,size);
		
		if( !reval )
			crypt_encryption((uint8_t*)&default_password.encryption_key,write_buf,read_buf,size);
		//memcpy(temp+start_block*CRYPT_BLOCK_SIZE,write_block,CRYPT_BLOCK_SIZE);
//		crypt_log("write decrypt:");
//		crypt_dump(read_buf,size);
		
		if( !reval ){
			if( write_func(card_ptr, start_block*CRYPT_BLOCK_SIZE,size,read_buf) != FUN_OK ){
				reval = 1;
			}
		}
	}

	if( read_buf!=NULL )
		free(read_buf);
	if( write_buf!=NULL )
		free(write_buf);
		
	if( reval ){
		len = 0;
	}
	
	xSemaphoreGive(crypt_semaphore);
	return len;
}

/** 
* 加密读. 
* 由于AES256算法是16字节区域加密，对读起始地址及结束地址进行16倍数的调整，使用读-解密-拷贝方案. 
* 用于智能卡，备份存储卡，内部加密存储的加密
* @param[in]   dev:设备类型，addr:起始地址，len:长度，ptr:数据缓冲. 
* @param[out]  无.  
* @retval  0,读失败，其它：读取长度
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Kend Deng于2019-07-26创建 
*/
uint16_t crypt_read(CARD_MANAGE_T *card_manage,uint16_t addr, uint16_t len, uint8_t *ptr){
	
	xSemaphoreTake(crypt_semaphore,portMAX_DELAY);
	
	uint16_t(*write_func)(CARD_MANAGE_T *card_manage,uint16_t addr, uint16_t len, uint8_t *ptr); 
	uint16_t(*read_func)(CARD_MANAGE_T *card_manage,uint16_t addr, uint16_t len, uint8_t *ptr); 
	
	CARD_MANAGE_T *card_ptr;
	
	if( card_manage==NULL ){
		write_func = NULL;//at88sc_write_storage;
		read_func = NULL;//at88sc_read_storage;
		card_ptr = NULL;
	}
	else{
		write_func = sc_write_storage;
		read_func = sc_read_storage;
		card_ptr = card_manage;
	}
	
	uint16_t start_block, end_block;
	start_block = addr/CRYPT_BLOCK_SIZE;
	end_block = (addr+len)/CRYPT_BLOCK_SIZE;
	uint16_t size = (end_block-start_block+1)*CRYPT_BLOCK_SIZE;
	
	uint8_t *read_buf = malloc(size);
	uint8_t	*write_buf = malloc(size);
	
	if( read_buf != NULL && write_buf != NULL ){
		
		memset(read_buf,0,size);
		memset(write_buf,0,size);
	
		uint8_t start_addr = addr%CRYPT_BLOCK_SIZE;
						
		//memcpy(read_block,temp+start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE);
		read_func(card_ptr, start_block*CRYPT_BLOCK_SIZE,size,read_buf);
//		if( write_func == sc_write_storage ){
//			crypt_log("read encrypt:");
//			crypt_dump(read_buf,size);
//		}
		crypt_decryption((uint8_t*)&default_password.encryption_key,read_buf,write_buf,size);
//		crypt_log("read plaintext:");
//		crypt_dump(write_buf,size);
		
		memcpy(ptr, write_buf + start_addr, len);
		
		free(read_buf);
		free(write_buf);
	}
	else {
		if( read_buf!=NULL )
			free(read_buf);
		if( write_buf!=NULL )
			free(write_buf);
		len = 0;
	}
	
	xSemaphoreGive(crypt_semaphore);
	return len;
}

//void crypt_encryption(uint8_t *in, uint8_t *out){
//	
//	mbedtls_aes_context aes_ctx;
//	
//    mbedtls_aes_init(&aes_ctx);
//    //设置加密密钥  
//    mbedtls_aes_setkey_enc(&aes_ctx, default_password.encryption_key, 256);
//   
//    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, in, out);
//	
//	mbedtls_aes_free(&aes_ctx);
//	
//}	
//	
//void crypt_decryption(uint8_t *in, uint8_t *out){
//	
//	mbedtls_aes_context aes_ctx;
//	
//	mbedtls_aes_init(&aes_ctx);
//    //设置解密密钥  
//    mbedtls_aes_setkey_dec(&aes_ctx, default_password.encryption_key, 256);
//	
//    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, in, out);
//   
//    mbedtls_aes_free(&aes_ctx);
//}

//uint16_t crypt_write(CRYPT_DEV_T dev,uint16_t addr, uint16_t len, uint8_t *ptr){
//	
//	uint8_t	read_block[CRYPT_BLOCK_SIZE];
//	uint8_t	write_block[CRYPT_BLOCK_SIZE];
//	uint8_t	plaintext_block[CRYPT_BLOCK_SIZE];
//	
//	uint16_t(*write_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
//	uint16_t(*read_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
//	
//	if( dev==CRYPT_SC ){
//		write_func = sc_write_storage;
//		read_func = sc_read_storage;
//	}
//	else if( dev==CRYPT_AT ){
//		write_func = at88sc_write_storage;
//		read_func = at88sc_read_storage;
//	}
//	
//	uint16_t start_block, end_block;
//	start_block = addr/CRYPT_BLOCK_SIZE;
//	end_block = (addr+len)/CRYPT_BLOCK_SIZE;
//	uint8_t block_addr = addr%CRYPT_BLOCK_SIZE;
//	uint8_t block_len;
//	uint16_t pos=addr;
//	uint8_t	copy_len=0;
//	
//	while( pos<addr+len ){
//		
////		vTaskDelay(1);
//		
//		//memcpy(read_block,temp+start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE);
//		read_func(start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE,read_block);
//		crypt_decryption(read_block,plaintext_block);
//		
//		block_addr = pos%CRYPT_BLOCK_SIZE;
//		block_len = (block_addr%CRYPT_BLOCK_SIZE)?CRYPT_BLOCK_SIZE-block_addr:CRYPT_BLOCK_SIZE;
//		
//		if( (len+addr - pos) > block_len )
//			copy_len = block_len;
//		else
//			copy_len = (len+addr - pos);
//		
//		memcpy(plaintext_block + block_addr, ptr + pos - addr , copy_len);
//		crypt_encryption(plaintext_block,write_block);
//		//memcpy(temp+start_block*CRYPT_BLOCK_SIZE,write_block,CRYPT_BLOCK_SIZE);
//		write_func(start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE,write_block);
//		
//		pos += block_len;
//		
//		start_block++;
//	}
//}

//uint16_t crypt_read(CRYPT_DEV_T dev,uint16_t addr, uint16_t len, uint8_t *ptr){
//	
//	uint8_t	read_block[CRYPT_BLOCK_SIZE];
//	uint8_t	plaintext_block[CRYPT_BLOCK_SIZE];
//	
//	uint16_t(*write_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
//	uint16_t(*read_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
//	
//	if( dev==CRYPT_SC ){
//		write_func = sc_write_storage;
//		read_func = sc_read_storage;
//	}
//	else if( dev==CRYPT_AT ){
//		write_func = at88sc_write_storage;
//		read_func = at88sc_read_storage;
//	}
//	
//		uint16_t start_block, end_block;
//	start_block = addr/CRYPT_BLOCK_SIZE;
//	end_block = (addr+len)/CRYPT_BLOCK_SIZE;
//	uint8_t block_addr = addr%CRYPT_BLOCK_SIZE;
//	uint8_t block_len;
//	uint16_t pos=addr;
//	uint8_t	copy_len=0;
//	
//	while( pos<addr+len ){
//		
////		vTaskDelay(1);
//		
//		read_func(start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE,read_block);
//		//memcpy(read_block,temp+start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE);
//		crypt_decryption(read_block,plaintext_block);
//		
//		block_addr = pos%CRYPT_BLOCK_SIZE;
//		block_len = (block_addr%CRYPT_BLOCK_SIZE)?CRYPT_BLOCK_SIZE-block_addr:CRYPT_BLOCK_SIZE;
//		
//		if( (len+addr - pos) > block_len )
//			copy_len = block_len;
//		else
//			copy_len = (len+addr - pos);
//		
//		memcpy(ptr + pos - addr, plaintext_block + block_addr , copy_len);
//		
//		pos += block_len;
//		
//		start_block++;
//	}
//}

//uint8_t temp[128]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40};

//void crypt_write(CRYPT_DEV_T dev,uint8_t *ptr,uint16_t addr, uint16_t len){
//	
//	uint8_t	read_block[CRYPT_BLOCK_SIZE];
//	uint8_t	write_block[CRYPT_BLOCK_SIZE];
//	uint8_t	plaintext_block[CRYPT_BLOCK_SIZE];
//	
//	uint16_t(*write_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
//	uint16_t(*read_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
//	
//	if( dev==CRYPT_SC ){
//		write_func = sc_write_storage;
//		read_func = sc_read_storage;
//	}
//	else if( dev==CRYPT_AT ){
//		write_func = at88sc_write_storage;
//		read_func = at88sc_read_storage;
//	}
//	
//	uint16_t start_block, end_block;
//	start_block = addr/CRYPT_BLOCK_SIZE;
//	end_block = (addr+len)/CRYPT_BLOCK_SIZE;
//	uint8_t block_addr = addr%CRYPT_BLOCK_SIZE;
//	uint8_t block_len;
//	uint16_t pos=addr;
//	uint8_t	copy_len=0;
//	
//	while( pos<addr+len ){
//		
//		memcpy(read_block,temp+start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE);
//		crypt_decryption(read_block,plaintext_block);
//		
//		block_addr = pos%CRYPT_BLOCK_SIZE;
//		block_len = (block_addr%CRYPT_BLOCK_SIZE)?CRYPT_BLOCK_SIZE-block_addr:CRYPT_BLOCK_SIZE;
//		
//		if( (len+addr - pos) > block_len )
//			copy_len = block_len;
//		else
//			copy_len = (len+addr - pos);
//		
//		memcpy(plaintext_block + block_addr, ptr + pos - addr , copy_len);
//		crypt_encryption(plaintext_block,write_block);
//		//write_func(start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE,plaintext_block);
//		memcpy(temp+start_block*CRYPT_BLOCK_SIZE,write_block,CRYPT_BLOCK_SIZE);
//		
//		pos += block_len;
//		
//		start_block++;
//	}
//}

//void crypt_read(CRYPT_DEV_T dev,uint8_t *ptr,uint16_t addr, uint16_t len){
//	
//	uint8_t	read_block[CRYPT_BLOCK_SIZE];
//	uint8_t	plaintext_block[CRYPT_BLOCK_SIZE];
//	
//	uint16_t(*write_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
//	uint16_t(*read_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
//	
//	if( dev==CRYPT_SC ){
//		write_func = sc_write_storage;
//		read_func = sc_read_storage;
//	}
//	else if( dev==CRYPT_AT ){
//		write_func = at88sc_write_storage;
//		read_func = at88sc_read_storage;
//	}
//	
//		uint16_t start_block, end_block;
//	start_block = addr/CRYPT_BLOCK_SIZE;
//	end_block = (addr+len)/CRYPT_BLOCK_SIZE;
//	uint8_t block_addr = addr%CRYPT_BLOCK_SIZE;
//	uint8_t block_len;
//	uint16_t pos=addr;
//	uint8_t	copy_len=0;
//	
//	while( pos<addr+len ){
//		
//		memcpy(read_block,temp+start_block*CRYPT_BLOCK_SIZE,CRYPT_BLOCK_SIZE);
//		crypt_decryption(read_block,plaintext_block);
//		
//		block_addr = pos%CRYPT_BLOCK_SIZE;
//		block_len = (block_addr%CRYPT_BLOCK_SIZE)?CRYPT_BLOCK_SIZE-block_addr:CRYPT_BLOCK_SIZE;
//		
//		if( (len+addr - pos) > block_len )
//			copy_len = block_len;
//		else
//			copy_len = (len+addr - pos);
//		
//		memcpy(ptr + pos - addr, plaintext_block + block_addr , copy_len);
//		
//		pos += block_len;
//		
//		start_block++;
//	}
//}

/** 
* AES256初始化. 
* 初始化DAM及互斥信号量. 
* @param[in]   无. 
* @param[out]  无.  
* @par 标识符 
* @retval  无.
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Kend Deng于2019-07-26创建 
*/
FUN_STATUS_T crypt_init(void){
	
	MX_DMA_Init();
	//MX_CRYP_Init();
	
	crypt_semaphore = xSemaphoreCreateMutex();
	
	return FUN_OK;
}

void crypt_test(void){
	
	char str[]={0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xd0,0xd1,0xd2,0xd3};
	char read[33]={0};
	crypt_write(AT_SC,0,32,(uint8_t*)str);
	crypt_read(AT_SC,0,32,(uint8_t*)read);
}
