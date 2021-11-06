#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "system.h"
#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

/** 
 * �����ػ����߳̾��������.
 * �����߳�ʱ����. 
 */  
typedef struct{
	
	TaskHandle_t	thread_handle[24];			 ///<freertosһ���¼���־���24��.
	uint8_t			handle_num;
	uint32_t		mask_bit;
}DAEMON_T;

extern DAEMON_T		daemon;
///<�����߳�ʱ����
#define daemon_from_create		&daemon.thread_handle[daemon.handle_num++]
///<�Ѿ����߳̾����ֱ�Ӹ�ֵʹ��
#define daemon_from_value		daemon.thread_handle[daemon.handle_num++]

extern FUN_STATUS_T daemon_init( void );
extern void daemon_run( void );
extern void daemon_refresh(void);
extern void daemon_enable( TaskHandle_t handle );
extern void daemon_disable( TaskHandle_t handle );

#endif
