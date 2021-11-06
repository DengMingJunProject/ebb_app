#include "system.h"
#include "event_log.h"
#include "main.h"
#include "tm_stm32_string.h"
#include "daemon.h"
#include "config.h"
#include "smartcard.h"
#include "dwt.h"

//#undef EVENTLOG_DEBUG

#ifdef EVENTLOG_DEBUG
    #define	event_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[EVENTLOG](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define event_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[EVENTLOG] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define event_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[EVENTLOG] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define event_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define event_log(...)
    #define event_usr(...)
    #define event_err(...)
    #define event_dump(...)
#endif
	
static xSemaphoreHandle event_log_xSemaphore=NULL;
static QueueHandle_t	*event_log_queue=NULL;	
static xSemaphoreHandle header_xSemaphore=NULL;

const char* level_string[]={
	"",
	"Critical Event",
	"System Event",
	"Standard Event",
};

const char* type_string[]={
	"",
	"Memory",
	"Power",
	"Communication",
	"Security",
	"System Admin",
	"System",
	"Printer",
	"Election",
	"Settings",
	"Firmware",
	"Key Card",
	"Maintenance",
};

/** 
* 读出log file记录头部. 
* 无. 
* @param[in]   无. 
* @param[out]  event_log_header_ptr:返回log file头部.  
* @retval  FUN_STATUS_T:函数状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-11-13创建 
*/
FUN_STATUS_T event_log_header_read(EVENT_LOG_HEADER_T *event_log_header_ptr){
	
	if( header_xSemaphore==NULL ){
		header_xSemaphore = xSemaphoreCreateMutex();
	}
	xSemaphoreTake(header_xSemaphore,portMAX_DELAY);
	
	FILE *file;
	int size;
	file = fopen (CONECT_STR(EVENT_LOG_DRIVER,"log_header.txt"),"rb+");
	if( file ){
		
		fseek (file, 0, SEEK_SET); 
		size = fread (event_log_header_ptr, sizeof(uint8_t), sizeof(EVENT_LOG_HEADER_T), file);
		fclose(file);
		
		if( size != sizeof(EVENT_LOG_HEADER_T) ){
			xSemaphoreGive(header_xSemaphore);
			return FUN_ERROR;
		}
		else{
			xSemaphoreGive(header_xSemaphore);
			return FUN_OK;
		}
	}
	xSemaphoreGive(header_xSemaphore);
	return FUN_ERROR;
}

/** 
* 写入log file记录头部. 
* 无. 
* @param[in]   event_log_header_ptr:写入log file头部. 
* @param[out]  无.  
* @retval  FUN_STATUS_T:函数状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-11-13创建 
*/
FUN_STATUS_T event_log_header_write(EVENT_LOG_HEADER_T *event_log_header_ptr){
	
	if( header_xSemaphore==NULL ){
		header_xSemaphore = xSemaphoreCreateMutex();
	}
	xSemaphoreTake(header_xSemaphore,portMAX_DELAY);
	
	FILE *file;
	int size;
	file = fopen (CONECT_STR(EVENT_LOG_DRIVER,"log_header.txt"),"rb+");
	if( file ){
		fseek (file, 0, SEEK_SET); 
		size = fwrite (event_log_header_ptr, sizeof(uint8_t), sizeof(EVENT_LOG_HEADER_T), file);
		fclose(file);

		if( size != sizeof(EVENT_LOG_HEADER_T) ){
			xSemaphoreGive(header_xSemaphore);
			return FUN_ERROR;
		}
		else{
			xSemaphoreGive(header_xSemaphore);
			return FUN_OK;
		}
	}
	xSemaphoreGive(header_xSemaphore);
	return FUN_ERROR;
}

/** 
* 通过记录序号计算出文件名及所在的存放位置. 
* 无. 
* @param[in]   item:记录序号. 
* @param[out]  file_name:通道号,pos:返回的存位置.  
* @retval  FUN_STATUS_T:函数状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-11-13创建 
*/
FUN_STATUS_T event_log_file_pos(uint16_t item, char *file_name, uint16_t *pos)
{
	FILE 		*file;
	
	if( item>EVENT_LOG_MAX ){
		return FUN_ERROR;
	}
	snprintf(file_name,EVENT_FILE_NAME_LEN,CONECT_STR(EVENT_LOG_DRIVER,"system_log%d.txt"),(item/EVENT_FILE_LOG_MAX+1));
	
	*pos = item%EVENT_FILE_LOG_MAX;
	
	return FUN_OK;
}
	
void event_display_file(void){
//	FILE_HANDLER fp;
//	int res;
//	uint32_t size;
//	uint8_t *ptr;
//	res = midfs_fopen( &fp, "system_log.txt", "r" );
//	if( !res ){
//		size = midfs_fsize( &fp );
//		ptr = malloc(size);
//		midfs_fseek( &fp, 0, MIDFS_SEEK_SET );
//		midfs_fread(&fp,size,ptr);
////		event_log("DISPLAY FILE\r\n");
////		event_dump(ptr,size);
////		event_log("\r\n");
//		free(ptr);
//		midfs_fclose(&fp);
//	}
}

/** 
* 通过记录序号计算出文件名及所在的存放位置. 
* 无. 
* @param[in]   item:记录序号. 
* @param[out]  file_name:通道号,pos:返回的存位置.  
* @retval  FUN_STATUS_T:函数状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-11-13创建 
*/
FUN_STATUS_T event_log_write(EVENT_LEVEL_T level, EVENT_TYPE_T type, char *message){
	
	char *buf;
		
	buf = malloc(EVNET_LOG_SIZE);
	if( buf!=NULL && event_log_queue!=NULL ){
		
		memset(buf,0,EVNET_LOG_SIZE);
		snprintf(buf,EVNET_LOG_SIZE,"%02d/%02d/%02d/%02d/%02d/%02d-%s-%s-%s",sys_status.rtc.Hours,sys_status.rtc.Minutes,sys_status.rtc.Seconds,\
			sys_status.rtc.Day,sys_status.rtc.Month,sys_status.rtc.Year,level_string[level],type_string[type],message);
		
		EVENT_LOG_MSG_T msg;
		msg.addr = (uint8_t*)buf;
		
		if( pdPASS == xQueueSend(event_log_queue,&msg,0) )
			return FUN_OK;
		else{
			free(buf);
			return FUN_ERROR;
		}
	}
	else{
		if( buf!=NULL )
			free(buf);
		return FUN_ERROR;
	}
}

/** 
* 读取event log一条记录将修改读指针. 
* 调用一次返回1条记录. 
* @param[in]   无. 
* @param[out]  log_ptr:返回记录指针. 
* @retval  FUN_STATUS_T:函数状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-11-13创建 
*/
FUN_STATUS_T event_log_read_upload( char *log_ptr)
{
	FILE 		*file=NULL;
	uint32_t 	size;
	uint16_t	pos;
	char 		file_name[EVENT_FILE_NAME_LEN];
	
	EVENT_LOG_HEADER_T	event_log_header;		
	event_log_header_read(&event_log_header);
	
//	en_send_log(event_log_header.total);
	if( event_log_header.total>0 ){
		
		if( FUN_OK == event_log_file_pos(event_log_header.read_pos,file_name,&pos) ){
	
			xSemaphoreTake(event_log_xSemaphore,portMAX_DELAY);
			file = fopen(file_name,"rb");
		}
		
		if( file ){
				
			fseek(file, pos*EVNET_LOG_SIZE, SEEK_SET);
			size = fread (log_ptr, sizeof(uint8_t), EVNET_LOG_SIZE, file);
			fclose(file);
			
			event_log("read out .No %d event log\r\n",event_log_header.read_pos+1);
			
			event_log_header.total--;
			event_log_header.read_pos =  (event_log_header.read_pos+1)%EVENT_LOG_MAX;
			
			event_log_header_write(&event_log_header);
			
			xSemaphoreGive(event_log_xSemaphore);
			return FUN_OK;
		}
		else{
			xSemaphoreGive(event_log_xSemaphore);
			return FUN_ERROR;
		}
	}
	else{
		xSemaphoreGive(event_log_xSemaphore);
		return FUN_ERROR;
	}
}

/** 
* 读取event log一条记录. 
* 无. 
* @param[in]   log_num:记录序号. 
* @param[out]  log_ptr:返回记录指针. 
* @retval  FUN_STATUS_T:函数状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-11-13创建 
*/
FUN_STATUS_T event_log_read(uint16_t log_num, char *log_ptr)
{
	FILE 	*file=NULL;
	int res;
	uint32_t size;
	uint16_t pos;
	char file_name[EVENT_FILE_NAME_LEN];
	
	EVENT_LOG_HEADER_T	event_log_header;
	if( log_num>=EVENT_LOG_MAX ){
		
		return FUN_ERROR;
	}	
	
	if( FUN_OK == event_log_file_pos(log_num,file_name,&pos) ){
		
		xSemaphoreTake(event_log_xSemaphore,portMAX_DELAY);
		
		file = fopen(file_name,"rb");
	}
	if( file ){

		fseek(file, pos*EVNET_LOG_SIZE, SEEK_SET);
		size = fread (log_ptr, sizeof(uint8_t), EVNET_LOG_SIZE, file);
		fclose(file);
		
		event_log("read out .No %d event log\r\n",log_num+1);
		if( size==EVNET_LOG_SIZE ){
			xSemaphoreGive(event_log_xSemaphore);
			return FUN_OK;
		}
		else{
			xSemaphoreGive(event_log_xSemaphore);
			return FUN_ERROR;
		}
	}
	else{
		xSemaphoreGive(event_log_xSemaphore);
		return FUN_ERROR;
	}
}

/** 
* event log 任务线程. 
* 从队列中读出记录，由于记录总数比较大，分成多个小文件保存. 
* @param[in]   pvParameters:线程参数. 
* @param[out]  无.  
* @retval  FUN_STATUS_T:函数状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-11-13创建 
*/
void event_log_task( void *pvParameters )
{
	wait_for_startup();
	
	FILE 	*file;
	int 	size;
	file = fopen(CONECT_STR(EVENT_LOG_DRIVER,"log_header.txt"),"rb");
	if( file )
		fclose(file);
//	file = 0;
	if( file==NULL ){
		file = fopen(CONECT_STR(EVENT_LOG_DRIVER,"log_header.txt"),"w");
		fseek(file, 0, SEEK_SET);
		EVENT_LOG_HEADER_T event_log_header;
		memset(&event_log_header,0,sizeof(EVENT_LOG_HEADER_T));
		size = fwrite (&event_log_header, sizeof(uint8_t), sizeof(EVENT_LOG_HEADER_T), file);
		fclose(file);
	}
			
	while(1){
			
		EVENT_LOG_MSG_T msg;
		UBaseType_t num;
		uint8_t 	save_flag=0;
		FILE 		*file;
		EVENT_LOG_HEADER_T	event_log_header;
		char file_name[EVENT_FILE_NAME_LEN];
		uint16_t pos;
		
		num = uxQueueMessagesWaiting((QueueHandle_t)event_log_queue);
		
		xSemaphoreTake(event_log_xSemaphore,portMAX_DELAY);
		if( num>0 ){
			
			event_log_header_read(&event_log_header);
			
			event_log_file_pos(event_log_header.write_pos,file_name,&pos);
			
			file = fopen(file_name,"rb");
			if( file==NULL ){
				file = fopen(file_name,"wb+");
				fclose(file);		
			}
			else{
				fclose(file);
			}
			
			file = fopen(file_name,"rb+");
			if( file )
				save_flag = 1;
			else
				save_flag = 0;
		}

		while( num>0 && save_flag ){
		
			if( xQueueReceive((QueueHandle_t)event_log_queue,&msg,0)==pdPASS ){
				
				int size;
				
//				TickType_t ticks = dwt_read();
				
				if( event_log_header.total<EVENT_LOG_MAX ){

					event_log_file_pos(event_log_header.write_pos,file_name,&pos);
					///<记录递增到新的记录文件
					if( pos==0 ){
						///<关闭旧文件
						fclose(file);
						///<打开新文件
						file = fopen(file_name,"rb+");
						if( file==NULL ){
							file = fopen(file_name,"wb+");
							fclose(file);		
						}
						
						if( file )
							save_flag = 1;
						else
							save_flag = 0;
					}
					
					if( save_flag ){
						
						fseek(file, pos*EVNET_LOG_SIZE, SEEK_SET);
						size = fwrite (msg.addr, sizeof(uint8_t), EVNET_LOG_SIZE, file);
						
						if( size == EVNET_LOG_SIZE ){
							event_log_header.total ++;
							if( event_log_header.logs<EVENT_LOG_MAX )
								event_log_header.logs ++;
							
							event_log_header.write_pos = (event_log_header.write_pos+1)%EVENT_LOG_MAX;
							event_log("Additional .No %d new event log\r\n",event_log_header.write_pos);
						}
						else{
							event_err("Can not additional .No %d new event log\r\n",event_log_header.write_pos);
						}
					}
				}

				free(msg.addr);
				msg.addr = NULL;
				
//				sys_status.rd_number = (dwt_read()-ticks)/168/1000;
			}
			num--;
		}
		
		if( save_flag ){
			
			fclose(file);
			event_log_header_write(&event_log_header);
		}
		
		xSemaphoreGive(event_log_xSemaphore);
		
		vTaskDelay( 1000 );
	}
}

/** 
* event log 初始化. 
* 建立互斥信号量，写入记录队列，线程. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  FUN_STATUS_T:函数状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-11-13创建 
*/
FUN_STATUS_T event_log_init(void)
{
	int res;
	static uint8_t delete_file=0;
	
	event_log_xSemaphore = xSemaphoreCreateMutex();
	
	if( event_log_xSemaphore == NULL ){
		event_err("can not create event log mutex!!!\r\n");
		return FUN_ERROR;
	}
	
	event_log_queue = xQueueCreate(EVENT_LOG_QUEUE_MAX,sizeof(EVENT_LOG_MSG_T));
	if( event_log_queue ==0 ){
		
		event_err("can not create event log queue!!!\r\n");
		return FUN_ERROR;
	}

//	static char str[EVNET_LOG_SIZE];
//	event_log_read_upload(str);
//	event_log_read(59, str);
	
	if( xTaskCreate( event_log_task, "EVENT LOG", EVENT_LOG_STACK_SIZE, NULL, EVENT_LOG_TASK_PRIORITY, daemon_from_create ) != pdPASS ){
		return FUN_ERROR;
	}
	
	return FUN_OK;
}
