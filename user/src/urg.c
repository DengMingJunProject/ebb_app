/** 
* @file         urg.c 
* @brief        安全紧急检测处理. 
* @details  	处理安全检测，报警，记录等. 
* @author       Ken Deng
* @date     	2019-12-05 
* @version  	A001 
* @par Copyright (c):  
*       Laxton company 
* @par History:          
*   version: Ken Deng, 2019-12-05, 建立\n 
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
* 计算报警数量
* 多个TAMPER合计为一个. 
* @param[in]   无.
* @param[out]  无.  
* @retval  报警数量
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-12-09创建 
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
* 安全紧急检测线程
* 无. 
* @param[in]   pvParameters:线程参数.
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-12-05创建 
*/
void urg_task(void *pvParameters)
{
	
	wait_for_startup();
	
	uint16_t tamper_enable_timer=0;
	uint8_t	 last_open_dect = 1,last_tilted=1;
	
	while(1){
		
		if( !is_tamper_en() ){
			if( urg_status.urg.bit.tamper_detect==0 ){
				///<如果篡改检测关闭了检测到篡改恢复正常，则10秒后重启允许篡改报警
				if( tamper_enable_timer++ > 10 ){
					tamper_enable();
					event_log_write(SYSTEM_LEVEL, SECURITY_TYPE, "Tamper Re-Activated");
				}
			}
			///<如查篡改检测5分钟内没有恢复正常，则再次重启篡改报警
			else if( tamper_enable_timer++ > 300 ) {
				tamper_enable();
				event_log_write(SYSTEM_LEVEL, SECURITY_TYPE, "Tamper Re-Activated");
			}
		}
		else{
			tamper_enable_timer = 0;
		}
		
		///<打开检测
		if( last_open_dect != ioc_read_dect(OPEN_DECT) ){
			
			last_open_dect = ioc_read_dect(OPEN_DECT);
			
			if( last_open_dect==1 ){
				
				urg_status.tamper.bit.device_opened = 1;
				if( is_tamper_en() ){
					
				}
			}
		}
		
		///<倾斜检测
		if( last_tilted != ioc_read_dect(TILT_DECT) ){
			
			last_tilted = ioc_read_dect(TILT_DECT);
			
			if( last_tilted==1 ){
				
				urg_status.tamper.bit.device_tilted = 1;
				if( is_tamper_en() ){
					
				}
			}
		}
		
		urg_status.urg.bit.tamper_detect = (urg_status.tamper.byte!=0);
		
		//<bug:如果一个篡改一直起作用，另一个篡改后起作用不会记录及显示到液晶上
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
				///<如果篡改没有解决，则每5分钟后再次报警
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
* 安全紧急检测初始化. 
* 无. 
* @param[in]   无.
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-12-05创建 
*/
FUN_STATUS_T urg_init(void){
	
	memset(&urg_status,0,sizeof(URG_STATUS_T));
	
	if( xTaskCreate( urg_task, "URG", URG_STACK_SIZE, NULL, URG_TASK_PRIORITY, daemon_from_create ) != pdPASS ){
		return FUN_ERROR;
	}
	
	return FUN_OK;
}
