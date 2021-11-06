/** 
* @file         beep.c 
* @brief        ���������� 
* @details  	���ö�ʱ���ص�����. 
* @author       Ken Deng
* @date     	2019-08-12 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: Ken Deng, 2019-08-12, ����\n 
*/  
#include "system.h"
#include "beep.h"
#include "tm_stm32_gpio.h"
#include "urg.h"

TimerHandle_t		beep_timer=NULL;
uint8_t				last_beep=BP_OCCUR_MAX;
BEEP_STATUS_T		beep_status;

/** 
* ��������. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-12���� 
*/
static void beep_on(void)
{
	HAL_GPIO_WritePin(BEEP_PORT,BEEP_PIN,GPIO_PIN_SET);
	beep_status.buzzerStatus = ENABLE;
}

/** 
* ��������. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-12���� 
*/
static void beep_off(void)
{
	HAL_GPIO_WritePin(BEEP_PORT,BEEP_PIN,GPIO_PIN_RESET);
	beep_status.buzzerStatus = DISABLE;
}

/** 
* ��������ʱ�ص�����. 
* �����������ʱ��. 
* @param[in]   xTimer:��ʱ�����. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-12���� 
*/
static void beep_timer_callback(TimerHandle_t xTimer )
{
	if (beep_status.cycle > 0){
		
		if (beep_status.counter > 0){
			
			if (--beep_status.counter <= 0){
				
				if (beep_status.buzzerStatus == ENABLE){
					beep_off();
					beep_status.counter = beep_status.period - beep_status.time; 
				}
				else if (beep_status.buzzerStatus == DISABLE){
					
					if (--beep_status.cycle != 0){
						
						beep_status.counter = beep_status.time;
						beep_on();
					}
				}
			}
		}
	}
	else{
		xTimerStop(beep_timer,0);
		last_beep = BP_OCCUR_MAX;
	}
}

/** 
* �򿪷����������ö��ڼ���������. 
* ��. 
* @param[in]   bzAlarm:��������. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-12���� 
*/
static void beep_control(uint32_t enableTime,uint32_t cycleNum,uint32_t period)
{
	beep_on();
	beep_status.period      	= period;
	beep_status.counter     	= enableTime;
	beep_status.time  			= enableTime;
	beep_status.cycle 			= cycleNum;
}

/** 
* ���÷�������������. 
* ��. 
* @param[in]   bzAlarm:��������. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-12���� 
*/
void beep_set_alarm(BP_TypeDef bzAlarm)
{
	uint16_t bzPeriod   = 0;
	uint16_t bzEnableTm = 0;
	uint32_t bzCycleCnt = 0;
	uint8_t	 alarm_flag = 1;
	
	if (bzAlarm == BP_STOP){
		
		xTimerStop(beep_timer,0);
		last_beep = BP_OCCUR_MAX;
		beep_off();
		return ;
	}
	
	if( last_beep==bzAlarm )
		return;
	
	if (!is_tamper_detect()){
		
		switch (bzAlarm){
			
			case BP_START_UP    	: bzPeriod =  6; bzEnableTm =  5; bzCycleCnt = 0x01; break;
			case BP_SETTING_DONE  	: bzPeriod = 20; bzEnableTm = 10; bzCycleCnt = 0x03; break;
			case BP_TIME_REACH  	: bzPeriod = 21; bzEnableTm = 20; bzCycleCnt = 0x01; break;
			case BP_ASSIST_REQ  	: bzPeriod = 20; bzEnableTm = 10; bzCycleCnt = 0x02; break;
			case BP_VOTE_CARD_ACT  	: bzPeriod = 6;  bzEnableTm =  5; bzCycleCnt = 0x01; break;
			case BP_TAMPER_ACT  	: bzPeriod = 11; bzEnableTm = 10; bzCycleCnt = 0x01; break;
			case BP_TAMPER_DETECT  	: bzPeriod = 11; bzEnableTm = 10; bzCycleCnt = 0x03; break;
			case BP_BATTERY_LOW 	: bzPeriod = 20; bzEnableTm = 10; bzCycleCnt = 0x03; break;
			default 				: alarm_flag = 0; break;
		}
		if( alarm_flag ){
    
			xTimerStart(beep_timer,0);
			beep_control(bzEnableTm,bzCycleCnt,bzPeriod);
		}
	}
	else if (bzAlarm == BP_TAMPER_DETECT){
		
		bzPeriod   = 30;
		bzEnableTm = 10;
		bzCycleCnt = 0xFFFFFFFF;
    
		beep_control(bzEnableTm,bzCycleCnt,bzPeriod);
		xTimerStart(beep_timer,0);
	}
	last_beep=bzAlarm;
}

/** 
* ������������ʼ��. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-12���� 
*/
FUN_STATUS_T beep_init(void){
	
	beep_timer = xTimerCreate( "beep",100,pdTRUE,(void *)0,beep_timer_callback);
	if( beep_timer == NULL )
		return FUN_ERROR;
	TM_GPIO_Init(BEEP_PORT, BEEP_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
	
	return FUN_OK;
}