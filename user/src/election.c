/** 
* @file         election.c 
* @brief        ѡ��״̬����. 
* @details  	. 
* @author   	Ken Deng 
* @date     	2019-10-11 
* @version  	A001 
* @par Copyright (c):  
*       laxton company 
* @par History:          
*   version: Ken Deng, 2019-10-11, ����\n 
*/  
#include "system.h"
#include "election.h"
#include "display.h"
#include "main.h"
#include "config.h"
#include "devfs.h"
#include "comm.h"
#include "tm_stm32_rtc.h"
#include "time.h"
#include "event_log.h"
#include "beep.h"
#include "ioc.h"

#ifdef ELECTION_DEBUG
    #define	election_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[ELECTION](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define election_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[ELECTION] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define election_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[ELECTION] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define election_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define election_log(...)
    #define election_usr(...)
    #define election_err(...)
    #define election_dump(...)
#endif

ELECTION_STATUS_T	election_status;

/** 
* �Ƚ�ѡ��ʱ���Ѿ���ʼ������ѡ��ʱ���Ѿ�����. 
* ��. 
* @param[in]   s_e��1�ȽϿ�ʼʱ�䣬 2�ȽϽ���ʱ��
* @param[out]  ��.  
* @retval  FUN_OK:�Ѿ���ʼ���Ѿ�����
* @retval  FUN_ERROR:��δ��ʼ��δ����
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-10-18���� 
*/
FUN_STATUS_T election_compare_time(uint8_t s_e)
{
	DEVFS_ECECTION_DATE_TIME_T	election_date_time;
	struct tm read_time={0};
	struct tm rtc_time={0};
	
	time_t read_time_l,rtc_time_l;
	
	if( s_e==1 ){
		devfs_read_election_time( 1, &election_date_time);
	}
	else if( s_e==2 ){
		devfs_read_election_time( 2, &election_date_time);
	}
	
	read_time.tm_year = election_date_time.year+2000-1900;
	read_time.tm_mon  = election_date_time.month;
	read_time.tm_mday = election_date_time.day;

	read_time.tm_hour = election_date_time.hour;
	read_time.tm_min  = election_date_time.minute;
	read_time.tm_sec  = election_date_time.second;
	
	read_time_l = mktime(&read_time);
	
	rtc_time.tm_year = sys_status.rtc.Year+2000-1900;
	rtc_time.tm_mon  = sys_status.rtc.Month;
	rtc_time.tm_mday = sys_status.rtc.Day;

	rtc_time.tm_hour = sys_status.rtc.Hours;
	rtc_time.tm_min  = sys_status.rtc.Minutes;
	rtc_time.tm_sec  = sys_status.rtc.Seconds;
		
	rtc_time_l = mktime(&rtc_time);
	
	if( s_e==1 ){
		
		if( rtc_time_l > read_time_l )
			return FUN_OK;
		else
			return FUN_ERROR;
	}
	else if( s_e==2 ){
		
		if( rtc_time_l > read_time_l )
			return FUN_OK;
		else
			return FUN_ERROR;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* ѡ��״̬�����߳�. 
* ��. 
* @param[in]   pvParameters���̲߳���. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-10-11���� 
*/
static void election_task( void *pvParameters )
{   
	wait_for_startup();
	
	while(1){
		
		switch( election_status.ebb_stus ){
			case EBB_STUS_START_UP:
				if( election_status.stus_first ){
					election_status.stus_first = 0;
					election_log("Election START UP\r\n");
				}
				if( (sys_status.self_test_progress+=8)>=100 ){
					vTaskDelay( 1000 );
							
					switch_election(SCREEN_PRE_SETTING);
				}
				break;
			case EBB_STUS_PRE_SETTING:
				if( election_status.stus_first ){
					election_status.stus_first = 0;
					election_log("Election PRE SETTING\r\n");
				}
				if( config.programmed ){
					vTaskDelay( 1000 );
					
					DEVFS_ECECTION_DATE_TIME_T	election_date_time;
					TM_RTC_AlarmTime_t 			date_time;
					
					///<��������ʱ��
					if( FUN_OK == devfs_read_election_time( 1, &election_date_time) ){
						date_time.Type = TM_RTC_AlarmType_DayInMonth;
						date_time.Day = election_date_time.day;
						date_time.Hours = election_date_time.hour;
						date_time.Minutes = election_date_time.minute;
						date_time.Seconds = election_date_time.second;
						TM_RTC_EnableAlarm(TM_RTC_Alarm_A, &date_time, TM_RTC_Format_BIN);
						election_log("Election set start alarm\r\n");
						
						///<���ý���ʱ��
						if( FUN_OK == devfs_read_election_time( 2, &election_date_time) ){
							date_time.Type = TM_RTC_AlarmType_DayInMonth;
							date_time.Day = election_date_time.day;
							date_time.Hours = election_date_time.hour;
							date_time.Minutes = election_date_time.minute;
							date_time.Seconds = election_date_time.second;
							TM_RTC_EnableAlarm(TM_RTC_Alarm_B, &date_time, TM_RTC_Format_BIN);
							election_log("Election set close alarm\r\n");
							
							switch_election(SCREEN_PRE_ELECTION);
						}
						else{
							display_info_in("Can not set election end time",0);
						}
					}
					else{
						display_info_in("Can not set election start time",0);
					}
				}
				break;
			case EBB_STUS_PRE_ELECTION:
				if( election_status.stus_first ){
					election_status.stus_first = 0;
					election_log("Election PRE ELECTION\r\n");
					if( config.election_state==ECECTION_DURING ){
						switch_election(SCREEN_DURING_ELECTION);
					}
					else if( config.election_state==ECECTION_CLOSED ){
						switch_election(SCREEN_POST_ELECTION);
					}
					///<�ж�ѡ��ʱ���Ƿ��Ѿ���ʼ
					if( config.election_state == ECECTION_NOTSTARTED ){
						///<������ʼʱ��δ������ʱ�䣬���ڴ���ͶƱʱ���
						if( election_compare_time(1)==FUN_OK && election_compare_time(2)!=FUN_OK ){
							switch_election(SCREEN_DURING_ELECTION);
							config.election_state = ECECTION_DURING;
						}
					}
				}
				break;
			case EBB_STUS_DEVICE_SETTING_UPLOAD:
				if( election_status.stus_first ){
					election_status.stus_first = 0;
					election_log("Election DEVICE SETTING UPLOAD\r\n");
				}
				break;
			case EBB_STUS_DURING_ELECTION:
				if( election_status.stus_first ){
					election_status.stus_first = 0;
					election_log("Election DURING ELECTION\r\n");
					
					if( config.election_time_up ){
						config.election_time_up = 0;
						
						if( FUN_OK == devfs_vote_tallies_init() ){
							election_log("vote tallies file init successful\r\n");
						}
						else{
							election_log("vote tallies file init failed\r\n");
						}
					}
					
					///<�ж�ѡ��ʱ���Ƿ��Ѿ�����
					if( config.election_state == ECECTION_DURING ){
						if( election_compare_time(2)==FUN_OK ){
							switch_election(EBB_STUS_POST_ELECTION);
							config.election_state = ECECTION_CLOSED;
						}
					}
				}
				break;
			case EBB_STUS_POST_ELECTION:
				if( election_status.stus_first ){
					election_status.stus_first = 0;
					election_log("Election POST ELECTION\r\n");
					config.programmed = 0;
					comm_end_election();
					
					if( FUN_OK == devfs_vote_records_create() ){
						display_info_in("Voting records create successfully",0);
					}
					else{
						display_info_in("Voting records create failed",0);
					}
				}
				break;
		}
		vTaskDelay( 100 );
	}
}

/** 
* ����ͶƱ״̬ΪԤ����״̬. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-11-27���� 
*/
void election_setting(void)
{
	switch_ebb_stus(EBB_STUS_PRE_SETTING);
	switch_screen(SCREEN_PRE_SETTING);
	config.election_state = ECECTION_NOTSTARTED;
}

/** 
* ����ͶƱ״̬Ϊ��ʼ״̬. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-11-27���� 
*/
void election_start(void)
{
	switch_ebb_stus(EBB_STUS_DURING_ELECTION);
	switch_screen(SCREEN_DURING_ELECTION);
	config.election_state = ECECTION_DURING;
	config.election_time_up = 1;
	election_log("Election Start tigger\r\n");
	event_log_write(SYSTEM_LEVEL, ELECTION_TYPE, "Election Time has begun");
	beep_set_alarm(BP_TIME_REACH);
	ioc_keypad_led_blink();
	switch_ebb_stus(EBB_STUS_DURING_ELECTION);
}

/** 
* ����ͶƱ״̬Ϊ����״̬. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-11-27���� 
*/
void election_stop(void)
{
	switch_ebb_stus(EBB_STUS_POST_ELECTION);
	switch_screen(SCREEN_POST_ELECTION);
	config.election_state = ECECTION_CLOSED;
	election_log("Election Close tigger\r\n");
	event_log_write(SYSTEM_LEVEL, ELECTION_TYPE, "Election Time has closed");
}

/** 
* ��ʼͶƱʱ�䵽�жϻص�����. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-10-11���� 
*/
void TM_RTC_AlarmAHandler(void)
{
//	switch_ebb_stus(EBB_STUS_DURING_ELECTION);
//	switch_screen(SCREEN_DURING_ELECTION);
//	config.election_state = ECECTION_DURING;
//	config.election_time_up = 1;
	
	TM_RTC_DisableAlarm(TM_RTC_Alarm_A);
	
	election_start();
	
//	election_log("Election Start tigger\r\n");
}

/** 
* ����ͶƱʱ�䵽�жϻص�����. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-10-11���� 
*/
void TM_RTC_AlarmBHandler(void)
{
//	switch_ebb_stus(EBB_STUS_POST_ELECTION);
//	switch_screen(SCREEN_POST_ELECTION);
//	config.election_state = ECECTION_CLOSED;
	
	TM_RTC_DisableAlarm(TM_RTC_Alarm_B);
	
	election_stop();
	
//	election_log("Election Close tigger\r\n");
}

/** 
* ѡ��״̬�����ʼ��. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-10-11���� 
*/
FUN_STATUS_T election_init(void)
{
	memset(&election_status,0,sizeof(ELECTION_STATUS_T));
	
	if( FUN_OK != devfs_settings_to_election() ){
		return FUN_ERROR;
	}
	
	if( xTaskCreate( election_task, "ELECTION", ELECTION_STACK_SIZE, NULL, ELECTION_TASK_PRIORITY, NULL	) == pdPASS ){
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}