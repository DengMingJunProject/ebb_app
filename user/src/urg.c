/** 
* @file         urg.c 
* @brief        ��ȫ������⴦��. 
* @details  	����ȫ��⣬��������¼��. 
* @author       Ken Deng
* @date     	2019-12-05 
* @version  	A001 
* @par Copyright (c):  
*       Laxton company 
* @par History:          
*   version: Ken Deng, 2019-12-05, ����\n 
*/
#include "system.h"
#include "urg.h"
#include "daemon.h"
#include "ioc.h"
#include "urg.h"
#include "config.h"
#include "event_log.h"
#include "display.h"
#include "beep.h"

URG_STATUS_T	urg_status;

/** 
* ���㱨������
* ���TAMPER�ϼ�Ϊһ��. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��������
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-12-09���� 
*/
uint8_t urg_cnt_err(void)
{
	uint8_t cnt=0;
	if( urg_status.urg.bit.low_power )
		cnt++;
	if( urg_status.urg.bit.tamper_detect )
		cnt++;
	return cnt;
}

/** 
* ��ȫ��������߳�
* ��. 
* @param[in]   pvParameters:�̲߳���.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-12-05���� 
*/
void urg_task(void *pvParameters)
{
	
	wait_for_startup();
	
	uint16_t tamper_enable_timer=0;
	uint8_t	 last_open_dect = 1,last_tilted=1;
	
	while(1){
		
		if( !is_tamper_en() ){
			if( urg_status.urg.bit.tamper_detect==0 ){
				///<����۸ļ��ر��˼�⵽�۸Ļָ���������10�����������۸ı���
				if( tamper_enable_timer++ > 10 ){
					tamper_enable();
					event_log_write(SYSTEM_LEVEL, SECURITY_TYPE, "Tamper Re-Activated");
				}
			}
			///<���۸ļ��5������û�лָ����������ٴ������۸ı���
			else if( tamper_enable_timer++ > 300 ) {
				tamper_enable();
				event_log_write(SYSTEM_LEVEL, SECURITY_TYPE, "Tamper Re-Activated");
			}
		}
		else{
			tamper_enable_timer = 0;
		}
		
		///<�򿪼��
		if( last_open_dect != ioc_read_dect(OPEN_DECT) ){
			
			last_open_dect = ioc_read_dect(OPEN_DECT);
			
			if( last_open_dect==1 ){
				
				urg_status.tamper.bit.device_opened = 1;
				if( is_tamper_en() ){
					
				}
			}
		}
		
		///<��б���
		if( last_tilted != ioc_read_dect(TILT_DECT) ){
			
			last_tilted = ioc_read_dect(TILT_DECT);
			
			if( last_tilted==1 ){
				
				urg_status.tamper.bit.device_tilted = 1;
				if( is_tamper_en() ){
					
				}
			}
		}
		
		urg_status.urg.bit.tamper_detect = (urg_status.tamper.byte!=0);
		
		//<bug:���һ���۸�һֱ�����ã���һ���۸ĺ������ò����¼����ʾ��Һ����
		if( is_tamper_en() ){
			
			if( urg_status.urg.bit.tamper_detect ){
				
				static uint16_t tamper_keep_time=0;
				if( urg_status.tamper_alarm==0 ){
					
					urg_status.tamper_alarm = 1;
					tamper_keep_time = 0;
					beep_set_alarm(BP_TAMPER_DETECT);
					
					if( urg_status.tamper.bit.device_tilted ){
						display_info_in("Tamper Detected (Tilt)", 1);
						event_log_write(CRITICAL_LEVEL, SECURITY_TYPE, "Tamper Detected with the encloseure (Tilt)");
					}
					
					if( urg_status.tamper.bit.device_opened ){
						event_log_write(CRITICAL_LEVEL, SECURITY_TYPE, "Tamper Detected with the encloseure (Open)");
						display_info_in("Tamper Detected (Open)", 1);
					}
				}
				///<����۸�û�н������ÿ5���Ӻ��ٴα���
				else if( tamper_keep_time<0xffff ){
					tamper_keep_time++;
					if( tamper_keep_time>300 ){
						tamper_keep_time = 0;
						beep_set_alarm(BP_TAMPER_DETECT);
					}
				}
			}
			else{
				urg_status.tamper_alarm = 0;
			}
		}
		
		vTaskDelay(1000);
	}
}

/** 
* ��ȫ��������ʼ��. 
* ��. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2019-12-05���� 
*/
FUN_STATUS_T urg_init(void){
	
	memset(&urg_status,0,sizeof(URG_STATUS_T));
	
	if( xTaskCreate( urg_task, "URG", URG_STACK_SIZE, NULL, URG_TASK_PRIORITY, daemon_from_create ) != pdPASS ){
		return FUN_ERROR;
	}
	
	return FUN_OK;
}
