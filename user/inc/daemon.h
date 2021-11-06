#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "system.h"
#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

/** 
 * 加入守护的线程句柄及数量.
 * 建立线程时调用. 
 */  
typedef struct{
	
	TaskHandle_t	thread_handle[24];			 ///<freertos一个事件标志最大24个.
	uint8_t			handle_num;
	uint32_t		mask_bit;
}DAEMON_T;

extern DAEMON_T		daemon;
///<建立线程时调用
#define daemon_from_create		&daemon.thread_handle[daemon.handle_num++]
///<已经有线程句柄，直接赋值使用
#define daemon_from_value		daemon.thread_handle[daemon.handle_num++]

extern FUN_STATUS_T daemon_init( void );
extern void daemon_run( void );
extern void daemon_refresh(void);
extern void daemon_enable( TaskHandle_t handle );
extern void daemon_disable( TaskHandle_t handle );

#endif
