/** 
* @file         lefs 
* @brief        生成加密文件及解密文件数据. 
* @details  	对文件的指定区域加密及解密. 
* @author       ken deng 
* @date     	2019-09-10 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-09-10, 建立\n 
*/  
#include "lefs.h"
#include "crypt.h"
#include "system.h"
#include "fs.h"
#include "rl_fs.h"
#include "secure_storage.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "config.h"
#include "comm.h"
#include "math.h"

#undef LEFS_DEBUG
#ifdef LEFS_DEBUG
    #define	lefs_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[EFS](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define lefs_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[EFS] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define lefs_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[EFS] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define lefs_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define lefs_log(...)
    #define lefs_usr(...)
    #define lefs_err(...)
    #define lefs_dump(...)
#endif
static xSemaphoreHandle lefs_xSemaphore=NULL;
/** 
* 文件加密写. 
* 将文件指定部分读出解密，拷贝相应的数据，加密写回去,如果写入位置超出文件长度，中间会写入未加密的0数据. 
* @param[in]   file_name:文件名,pos：写入位置，len：写入长度，ptr:写入数据指针. 
* @param[out]  无.  
* @retval  0  成功 
* @retval  -1  错误  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-10创建 
*/
FUN_STATUS_T lefs_write(char *file_name, uint32_t pos, uint32_t len, uint8_t *ptr)
{
	if( lefs_xSemaphore==NULL ){
		lefs_xSemaphore = xSemaphoreCreateMutex();
	}
	
	xSemaphoreTake(lefs_xSemaphore,portMAX_DELAY);
	
	FILE *file;
	file = fopen (file_name,"rb+");
	if( file == NULL ){
		file = fopen (file_name,"wb+");
	}
	if( file!=NULL ){
		
		uint16_t start_block, end_block;
		start_block = pos/CRYPT_BLOCK_SIZE;
		end_block = (pos+len)/CRYPT_BLOCK_SIZE;
		uint16_t size = (end_block-start_block+1)*CRYPT_BLOCK_SIZE;
		
		uint8_t *read_buf = malloc(size);
		uint8_t	*write_buf = malloc(size);
		
		if( read_buf != NULL && write_buf != NULL ){
		
			memset(read_buf,0,size);
			memset(write_buf,0,size);
			
			uint8_t start_addr = pos%CRYPT_BLOCK_SIZE;
							
			fseek (file, start_block*CRYPT_BLOCK_SIZE, SEEK_SET); 
			fread (read_buf, sizeof(uint8_t), size, file);
			lefs_log("read encrypt:");
			lefs_dump(read_buf,size);
			crypt_decryption((uint8_t*)&default_password.encryption_key,read_buf,write_buf,size);
			lefs_log("write plaintext read:");
			lefs_dump(write_buf,size);
			
			memcpy(write_buf + start_addr, ptr, len);
			lefs_log("write plaintext cpy:");
			lefs_dump(write_buf,size);
			
			crypt_encryption((uint8_t*)&default_password.encryption_key,write_buf,read_buf,size);
			lefs_log("write decrypt:");
			lefs_dump(read_buf,size);
			
			fseek (file, start_block*CRYPT_BLOCK_SIZE, SEEK_SET); 
			fwrite (read_buf, sizeof(uint8_t), size, file);
			
			free(read_buf);
			free(write_buf);
		}
		else{
			if( read_buf!=NULL )
				free(read_buf);
			if( write_buf!=NULL )
				free(write_buf);
			len = 0;
		}
		
		fclose(file);
		
		xSemaphoreGive(lefs_xSemaphore);
		return FUN_OK;
	}
	else{
		
		xSemaphoreGive(lefs_xSemaphore);
		return FUN_ERROR;
	}
}

/** 
* 文件加密读. 
* 将文件指定部分读出解密，拷贝相应的数据. 
* @param[in]   file_name:文件名,pos：读出位置，len：读出长度，ptr:读出数据指针. 
* @param[out]  无.  
* @retval  0  成功 
* @retval  -1  错误  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-10创建 
*/
FUN_STATUS_T lefs_read(char *file_name, uint32_t pos, uint32_t len, uint8_t *ptr)
{
	if( lefs_xSemaphore==NULL ){
		lefs_xSemaphore = xSemaphoreCreateMutex();
	}
	
	xSemaphoreTake(lefs_xSemaphore,portMAX_DELAY);
	
	FILE *file;
	int rtn;
	file = fopen (file_name,"rb");
	if( file!=NULL ){
		uint16_t start_block, end_block;
		start_block = pos/CRYPT_BLOCK_SIZE;
		end_block = (pos+len)/CRYPT_BLOCK_SIZE;
		uint16_t size = (end_block-start_block+1)*CRYPT_BLOCK_SIZE;
		
		uint8_t *read_buf = malloc(size);
		uint8_t	*write_buf = malloc(size);
		
		if( read_buf != NULL && write_buf != NULL ){
			
			memset(read_buf,0,size);
			memset(write_buf,0,size);
		
			uint8_t start_addr = pos%CRYPT_BLOCK_SIZE;
							
			fseek (file, start_block*CRYPT_BLOCK_SIZE, SEEK_SET); 
			rtn = fread (read_buf, sizeof(uint8_t), size, file);
			if( rtn != size ){
				free(read_buf);
				free(write_buf);
				return FUN_ERROR;
			}
				
			lefs_log("read encrypt:");
			lefs_dump(read_buf,size);
			crypt_decryption((uint8_t*)&default_password.encryption_key,read_buf,write_buf,size);
			lefs_log("read plaintext:");
			lefs_dump(write_buf,size);
			
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
		fclose(file);
		
		xSemaphoreGive(lefs_xSemaphore);
		return FUN_OK;
	}
	else{
		
		xSemaphoreGive(lefs_xSemaphore);
		return FUN_ERROR;
	}
}

/** 
* 新建加密文件. 
* 全部写入0的加密数据. 
* @param[in]   file_name:文件名,len：建立的文件长度. 
* @param[out]  无.  
* @retval  0  成功 
* @retval  -1  错误  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-10创建 
*/
FUN_STATUS_T lefs_new( char *file_name, uint32_t len )
{	
	const uint8_t plaintext_buf[16]={' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	uint8_t ciphertext_buf[16];
	
	crypt_encryption(default_password.encryption_key, (uint8_t*)plaintext_buf, ciphertext_buf, 16);

	fdelete(file_name,NULL);
	
	FILE *file;
	file = fopen (file_name,"wb+");
	uint16_t seg;
	if( file != NULL ){
		seg = len/16;
		if( len%16 )
			seg += 1;
		for( uint16_t i=0; i<seg; i++ ){
			fseek (file, i*16, SEEK_SET); 
			fwrite (&ciphertext_buf[0], sizeof(uint8_t), 16, file);
		}
		fclose(file);
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 加解密文件模块初始化. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  0  成功 
* @retval  -1  错误  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-10创建 
*/
FUN_STATUS_T lefs_init(void)
{
//	lefs_new("M0:\\test",45);
//	static uint32_t size ;
//	size =  fs_fsize("M0:\\test");

//	char hardware_version[7],firmwate_version[8],device_type[10],unique_id[24];
	
//	lefs_read("M0:\\DeviceInformation", offsetof(DEVICE_INFORMATION_T,hardware_version), sizeof(((DEVICE_INFORMATION_T*)0)->hardware_version), hardware_version);
//	lefs_read("M0:\\DeviceInformation", offsetof(DEVICE_INFORMATION_T,firmwate_version), sizeof(((DEVICE_INFORMATION_T*)0)->firmwate_version), firmwate_version);
//	lefs_read("M0:\\DeviceInformation", offsetof(DEVICE_INFORMATION_T,device_type), sizeof(((DEVICE_INFORMATION_T*)0)->device_type), device_type);
//	lefs_read("M0:\\DeviceInformation", offsetof(DEVICE_INFORMATION_T,unique_id), sizeof(((DEVICE_INFORMATION_T*)0)->unique_id), unique_id);
	return FUN_OK;
}
