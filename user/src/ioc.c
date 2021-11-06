/** 
* @file         ioc.c 
* @brief        ����IO��⼰LED��ʾ. 
* @details  	IOC�����߳�. 
* @author       Ken Deng 
* @date     	2019-11-15
* @version  	A001 
* @par Copyright (c):  
*       Laxton company 
* @par History:          
*   version: Ken Deng, 2019-11-15, ����\n 
*/  
#include "system.h"
#include "ioc.h"
#include "daemon.h"
#include "button.h"
#include "election.h"

EXTIO_INPUT_T		g_extio_input;
EXTIO_LED_T			g_extio_led;

/** 
* ����LED״̬. 
* ��. 
* @param[in]   led:LED��ţ�sta������.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-11-15���� 
*/
void ioc_led_set( uint8_t led, uint8_t sta )
{
	///<LED��·Ϊ������ȡ��
	sta = (sta==1)?0:1;
	
	switch( led ){
		
		case CON_LED:	led1_set(sta);break;
		case CAN_LED:	led2_set(sta);break;
		case UP_LED:	led3_set(sta);break;
		case DOWN_LED:	led4_set(sta);break;
	}
}

/** 
* LED��˸��IO���. 
* ��. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-11-15���� 
*/
static void ioc_led_io(void)
{
	//��������IO���˲�
	g_extio_input.read_io.bit.gpio1 = gpio1_read();
	g_extio_input.read_io.bit.gpio2 = gpio2_read();
	
	uint8_t io;
	io = g_extio_input.read_io.byte^g_extio_input.save_io.byte;
//	if( io!=0 )
	{
		
		uint8_t i;
		for( i=0; i<EXTIO_INPUT_NUM; i++ ){
			
			if( io&(1<<i) ){
				if( g_extio_input.timer[i]++>EXTIO_INPUT_TIME ){
					g_extio_input.timer[i] = 0;
					if( g_extio_input.read_io.byte&(1<<i) )
						g_extio_input.save_io.byte |= (1<<i);
					else
						g_extio_input.save_io.byte &= ~(1<<i);
				}
			}
			else{
				g_extio_input.timer[i] = 0;
			}
		}
	}
	
	//����LED����˸ʱ�䣬������������
	uint8_t i;
	for(i=0; i<EXTIO_LED_NUM; i++){
		if( g_extio_led.state[i]!=LED_BLINK ){
			
			if( g_extio_led.state[i] != g_extio_led.last_state[i] ){
				g_extio_led.last_state[i] = g_extio_led.state[i];
				ioc_led_set(i,g_extio_led.state[i]);
			}
			
			if( g_extio_led.state==LED_OFF ){
				g_extio_led.cnt[i] = 0;
				g_extio_led.timer[i] = 0;
				g_extio_led.blink_flag[i] = 0;
			}
		}
		else{
			if( g_extio_led.timer[i]++>g_extio_led.blink_time[i] ){
				g_extio_led.timer[i] = 0;
				if( g_extio_led.blink_flag[i]==1 ){
					g_extio_led.blink_flag[i]=0;
					ioc_led_set(i,LED_OFF);
					
					if( g_extio_led.blink_cnt[i]!=0xff ){
						if( ++g_extio_led.cnt[i]>=g_extio_led.blink_cnt[i] ){
							
							g_extio_led.state[i] = LED_OFF;
							g_extio_led.cnt[i] = 0;
						}
					}
				}
				else{
					g_extio_led.blink_flag[i]=1;
					ioc_led_set(i,LED_ON);
				}
			}
		}
	}
}

/** 
* ����LED. 
* ��. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-11-15���� 
*/
static void ioc_blink_led(void)
{
	if( get_ebb_stus()>EBB_STUS_INIT ){
		static uint8_t time;
		if( time++>50 ){
			time = 0;
			TM_GPIO_TogglePinValue(LED_RED_PORT, LED_RED_PIN);
		}
	}
}

/** 
* IOC�߳�. 
* ��. 
* @param[in]   pvParameters:�̲߳���.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-11-15���� 
*/
static void ioc_task( void *pvParameters )
{
	wait_for_startup();
	
	TickType_t ticks = xTaskGetTickCount();
	
	uint8_t cnt=0;
	while(1){
		
		ioc_blink_led();
		ioc_led_io();
		button_scan();
		vTaskDelayUntil( &ticks, 10);
	}
}

/** 
* ���û���LED����. 
* ��. 
* @param[in]   led:LED��ţ�sta������.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-11-15���� 
*/
void ioc_led(uint8_t led, uint8_t sta)
{
	sta = (sta==1)?0:1;
	if( led==1 ){
		TM_GPIO_SetPinValue(LED_GREEN_PORT, LED_GREEN_PIN,sta);
	}
	else if( led==2 ){
		TM_GPIO_SetPinValue(LED_YELLOW_PORT, LED_YELLOW_PIN,sta);
	}
}

/** 
* IOC��ʼ��. 
* ��. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-11-15���� 
*/
FUN_STATUS_T ioc_init(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	TM_GPIO_Init(LED_GREEN_PORT, LED_GREEN_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
	TM_GPIO_SetPinValue(LED_GREEN_PORT, LED_GREEN_PIN,1);
	TM_GPIO_Init(LED_YELLOW_PORT, LED_YELLOW_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
	TM_GPIO_SetPinValue(LED_YELLOW_PORT, LED_YELLOW_PIN,1);
	TM_GPIO_Init(LED_RED_PORT, LED_RED_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
	
	TM_GPIO_Init(TILT_DECT_PORT, TILT_DECT_PIN, TM_GPIO_Mode_IN, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Low);
	TM_GPIO_Init(OPEN_DECT_PORT, OPEN_DECT_PIN, TM_GPIO_Mode_IN, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Low);
	
	memset(&g_extio_input,0,sizeof(EXTIO_INPUT_T));
	memset(&g_extio_led,0,sizeof(EXTIO_LED_T));
	
	ioc_keypad_led_on();
	
//	ioc_led_blink_cnt(CON_LED,100,1);
//	ioc_led_blink_cnt(CAN_LED,100,1);
//	ioc_led_blink_cnt(UP_LED,100,1);
//	ioc_led_blink_cnt(DOWN_LED,100,1);
//	
//	ioc_led_set(0,LED_ON);
//	ioc_led_set(0,LED_OFF);
//	ioc_led_set(1,LED_ON);
//	ioc_led_set(1,LED_OFF);
//	ioc_led_set(2,LED_ON);
//	ioc_led_set(2,LED_OFF);
//	ioc_led_set(3,LED_ON);
//	ioc_led_set(3,LED_OFF);
		
	if( xTaskCreate( ioc_task, "IOC", IOC_STACK_SIZE, NULL, IOC_TASK_PRIORITY, daemon_from_create ) == pdPASS ){
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}
