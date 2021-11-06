/** 
* @file         daemon.c 
* @brief        ϵͳ�ػ��߳�. 
* @details  	���������̶߳�ʱ����������. 
* @author       kun 
* @date     	2018-05-11
* @version  	A001 
* @par Copyright (c):  
*        
* @par History:          
*   version: kun, 2018-05-11, ����\n 
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
* ���ػ���������. 
* �����ػ����̣߳�����Ҫ��ָ��ʱ���ڷ�������. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-11���� 
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
* �ػ���ʱ���ص�. 
* ֻ�����м����ػ����̣߳����յ�����������������Ź�. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-11���� 
*		2018-06-01 kun:���Ӵ�ӡû���յ��������߳�����
*		2018-06-19 kuh:����û���յ��������߳�����д����־�ļ�
*/
void daemon_timer_callback(TimerHandle_t xTimer )
{	
	///<���м����ػ��̵߳��¼���־
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
* �ػ���. 
* ���´򿪹ر��˵��߳��ػ�. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-11���� 
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
* �ػ��ر�. 
* �ػ���ʱ�����ټ�⵱ǰ���߳�,�����̹߳���ɾ��ʱ������ҪҪ�ػ�. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-11���� 
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
* �����ػ�. 
* �����¼���־���������Ź����ʱ��Ϊ3�룬
* �ػ���ʱ��2.5���һ�Σ��������Ź�
* ���������߳̽��������. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-11���� 
*/
void daemon_run( void )
{	
	daemon_event = xEventGroupCreate();
	daemon_timer = xTimerCreate( "daemon",9000,pdTRUE,(void *)0,daemon_timer_callback);
	xTimerStart(daemon_timer,0);
	///<ʹ�����м����ػ����߳�
	daemon.mask_bit = (0x00ffffff>>(24-daemon.handle_num));
	wdg_init();
}

/** 
* ��ʼ���ػ��ṹ. 
* ����ػ��ṹ,���������߳̽���ǰ����. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-11���� 
*/
FUN_STATUS_T daemon_init(void)
{	
	memset(&daemon,0,sizeof(DAEMON_T));
	return FUN_OK;
}