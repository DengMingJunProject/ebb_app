/** 
* @file         daemon.c 
* @brief        系统守护线程. 
* @details  	接收其它线程定时发出的心跳. 
* @author       kun 
* @date     	2018-05-11
* @version  	A001 
* @par Copyright (c):  
*        
* @par History:          
*   version: kun, 2018-05-11, 建立\n 
*/  
#include "daemon.h"
#include "wdg.h"
#include "macro_def.h"
#include <string.h>
//#include "log_file.h"
//#include "uart_console.h"

//#undef DAEMON_DEBUG
#ifdef DAEMON_DEBUG
    #define daemon_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DAEMON](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define daemon_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DAEMON] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define daemon_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DAEMON] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define daemon_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define daemon_log(...)
    #define daemon_usr(...)
    #define daemon_err(...)
    #define daemon_dump(...)
#endif


DAEMON_T			daemon;
EventGroupHandle_t 	daemon_event;
TimerHandle_t		daemon_timer;
/** 
* 向守护发送心跳. 
* 加入守护的线程，都需要在指定时间内发送心跳. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-11创建 
*/
void daemon_refresh(void)
{	
	TaskHandle_t  curr_thread_handle;
	
	curr_thread_handle = xTaskGetCurrentTaskHandle();
	
	for( uint8_t i=0; i<daemon.handle_num; i++ ){
		
		if( curr_thread_handle==daemon.thread_handle[i] ){
			
			if( daemon_event!=0 )
				xEventGroupSetBits( daemon_event, (1<<i) );
			break;
		}
	}
}

/** 
* 守护定时器回调. 
* 只有所有加入守护的线程，都收到的心跳才清独立看门狗. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-11创建 
*		2018-06-01 kun:增加打印没有收到心跳的线程名称
*		2018-06-19 kuh:增加没有收到心跳的线程名称写入日志文件
*/
void daemon_timer_callback(TimerHandle_t xTimer )
{	
	///<所有加入守护线程的事件标志
	uint32_t	all_bit = (0x00ffffff>>(24-daemon.handle_num));
	all_bit	&= daemon.mask_bit;
	
	static EventBits_t	thread_bit;
	thread_bit = xEventGroupWaitBits(daemon_event,all_bit,pdTRUE,pdTRUE,1);
	if( (thread_bit & all_bit)==all_bit ){
		wdg_iwdg_refresh();
	}
	else{
		uint32_t	xor_bit = (thread_bit ^ all_bit);
		for( uint8_t i=0; i<daemon.handle_num; i++ ){
			
			if( xor_bit&(1<<i) ){
				char buf[50];
				snprintf(buf,50,"\"%s\" thread No heartbeat\r\n",pcTaskGetTaskName(daemon.thread_handle[i]));
				daemon_log("%s",buf);
				//log_file_write("%s",buf);
				//uart_console_send((uint8_t*)buf,sizeof(buf));
			}
		}
	}
}

/** 
* 守护打开. 
* 重新打开关闭了的线程守护. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-11创建 
*/
void daemon_enable( TaskHandle_t handle )
{	
	for( uint8_t i=0; i<daemon.handle_num; i++){
		if( handle==daemon.thread_handle[i] ){
			set_bit(daemon.mask_bit,i);
			break;
		}
	}
}

/** 
* 守护关闭. 
* 守护定时器不再检测当前的线程,用于线程挂起、删除时不再需要要守护. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-11创建 
*/
void daemon_disable( TaskHandle_t handle )
{
	for( uint8_t i=0; i<daemon.handle_num; i++){
		if( handle==daemon.thread_handle[i] ){
			clr_bit(daemon.mask_bit,i);
			break;
		}
	}
}

/** 
* 启动守护. 
* 建立事件标志，独立看门狗溢出时间为3秒，
* 守护定时器2.5检测一次，启动看门狗
* 需在所有线程建立后调用. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-11创建 
*/
void daemon_run( void )
{	
	daemon_event = xEventGroupCreate();
	daemon_timer = xTimerCreate( "daemon",9000,pdTRUE,(void *)0,daemon_timer_callback);
	xTimerStart(daemon_timer,0);
	///<使能所有加入守护的线程
	daemon.mask_bit = (0x00ffffff>>(24-daemon.handle_num));
	wdg_init();
}

/** 
* 初始化守护结构. 
* 清空守护结构,需在所有线程建立前调用. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-11创建 
*/
FUN_STATUS_T daemon_init(void)
{	
	memset(&daemon,0,sizeof(DAEMON_T));
	return FUN_OK;
}