/** 
* @file         menu_func.c 
* @brief        菜单操作回调函数. 
* @details  	This is the detail description. 
* @author       Ken Deng
* @date     	2019-10-08 
* @version  	A001 
* @par Copyright (c):  
*       Laxton company 
* @par History:          
*   version: Ken Deng, 2019-10-08, 建立\n 
*/  
#include "system.h"
#include "menu_func.h"
#include "display.h"
#include "lcdxx.h"
#include "smartcard.h"
#include "ltc1955.h"
#include "election.h"
#include "main.h"
#include "comm.h"
#include "urg.h"
#include "event_log.h"
#include "config.h"
#include "rf_module.h"
#include "devfs.h"
#include "beep.h"

MENU_FUNC_VAR_T menu_func_var;

#ifdef MENU_FUNC_DEBUG
    #define	menu_func_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[MENU FUNC](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define menu_func_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[MENU FUNC] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define menu_funcn_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[MENU FUNC] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define menu_func_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define menu_func_log(...)
    #define menu_func_usr(...)
    #define menu_func_err(...)
    #define menu_func_dump(...)
#endif
	
#define menu_rst_bar()	{lcdxx_clear_row(menu_status.process_row);menu_status.process_step = 0;}
#define menu_row_bar(r)	{menu_status.process_row = 4;}

void menu_func_nocard_exit(void){
	
//	display_switch_menu(NULL);
//	election_status.card_type = CARD_PRESENT_NONE;
//	menu_status.menu_func = NULL;
//	menu_status.state = MENU_INFO;
//	
//	lcd1602_ClearScreen();
}

void menu_func_exit(uint8_t key){
	
	if( menu_status.func_fitst ){
		
		menu_status.func_fitst = 0;
		
		display_switch_menu(NULL);
		menu_status.menu_func = NULL;
		menu_status.state = MENU_INFO;
		menu_status.screen_refresh = 1;
		
		lcdxx_clear_screen();
	}
}

void menu_func_version(uint8_t key){
	
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		char buf[DISP_BUF_SIZE];
		snprintf(buf,DISP_BUF_SIZE,"Software V%d.%d.%d\nHardware %d",SOFTWARE_VERSION/1000, SOFTWARE_VERSION%100/10, SOFTWARE_VERSION%10,HARDWARE_VERSION);
		display_info_in(buf,0);
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
}

void menu_func_download_settings(uint8_t key){
	
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
//		lcd1602_ClearScreen();
		
	}
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
}

//void menu_view_logs(uint8_t key)
//{
//	static EVENT_LOG_HEADER_T event_log_header;
//	static uint8_t seg,seg_max,last50=0,file_record;
//	static uint16_t pos;
//	
//	if( menu_status.func_fitst ){
//		
//		menu_status.func_fitst = 0;
//		lcdxx_clear_screen();
//		
//		event_log_header_read(&event_log_header);
//		
//		menu_func_var.buf = malloc(EVNET_LOG_SIZE);
//		if(menu_func_var.buf){
//			seg = 0;
//			memset(menu_func_var.buf,0,EVNET_LOG_SIZE);
//			
//			if( event_log_header.logs>0 )
//			{
//				pos = event_log_get_newest_pos();					//get newest log pos
//				last50=0;
//				file_record = event_log_header.file_record-1;
//				
//				event_log_read(pos, menu_func_var.buf);
//				seg_max = strlen(menu_func_var.buf)/32;
//				if( strlen(menu_func_var.buf)%32 )
//					seg_max += 1;
//				display_info_roll(seg,menu_func_var.buf);
//			}
//			else{
//				display_info_in("no new event log\n",0);
//			}		
//		}
//		else{
//			
//		}
//	}
//	
//	if( key==ENTER_KEY_MSG ){								//paging:next page
//		if( seg<seg_max-1 ){
//			seg ++;
//			display_info_roll(seg,menu_func_var.buf);
//		}
//	}
//	
//	if( key==CANCEL_KEY_MSG ){								//paging:last page
//		
//		if( seg==0 ){
//			free(menu_func_var.buf);
//			display_exit_to_menu();
//		}
//		else{
//			if( seg>0 ){
//				seg--;
//				display_info_roll(seg,menu_func_var.buf);
//			}
//		}
//	}
//	
//	uint8_t refresh=0;
//	if( key==UP_KEY_MSG )									//last record
//	{	
//		if( last50<50 && file_record>0 )
//		{			
//			if(pos != event_log_get_oldest_pos())
//			{
//				if( pos==0 )									
//					pos = EVNET_LOG_SITE_MAX-1;
//				else
//					pos--;	
//			}
//			
//			file_record--;
//			last50++;
//			refresh = 1;
//		}
//	}
//	
//	if( key==DOWN_KEY_MSG ){								//next record
//		
//		if( last50>0 ){
//			pos++;
//			pos = pos%EVNET_LOG_SITE_MAX;				
//			
//			file_record++;
//			last50--;
//			refresh = 1;
//		}
//	}
//	
//	if( refresh ){
//		
//		refresh = 0;
//		memset(menu_func_var.buf,0,EVNET_LOG_SIZE);
//		event_log_read(pos, menu_func_var.buf);
//		seg = 0;
//		seg_max = strlen(menu_func_var.buf)/32;
//		if( strlen(menu_func_var.buf)%32 )
//			seg_max += 1;
//		display_info_roll(seg,menu_func_var.buf);
//	}
//}

/** 
* 激活投票卡. 
* 可以单独选择投票类型或者根据EBB设置类型激活卡. 
* @param[in]   key:按键. 
* @param[out]  输出.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-11-14创建 
*/
void menu_activate_vote_card(uint8_t key)
{
	
	const MENU_SELECT_ITEM_T	select_item[]={
		
		0,"<Vote EBB>",
		1,"<Vote 1>",
		2,"<Vote 2>",
		3,"<Vote 3>",
		4,"<Vote 4>",
	};
	static uint8_t vote_type_insert;
		
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		menu_row_bar(4);

		lcdxx_clear_screen();
		
		display_info_in("Select vote type to activate",0);
		menu_func_var.items = 0;
		char buf[DISP_BUF_SIZE];
		snprintf(buf,DISP_BUF_SIZE,"\n\n    %s",select_item[menu_func_var.items].itme_str);
		display_info_in(buf,0);
		
		vote_type_insert = 0;
	}
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==ENTER_KEY_MSG ){
		menu_rst_bar();
		
		if( sc_activate_vote_card(select_item[menu_func_var.items].num) == FUN_OK ){
					
			display_info_in("\n\n\nActivate OK",0);
			event_log_write(STANDARD_LEVEL, KEY_CARD_TYPE, "Vote Card Activated");
			beep_set_alarm(BP_VOTE_CARD_ACT);
		}
		else{
			display_info_in("\n\n\nActivate ERROR",0);
		}
	}
	
	if( key==UP_KEY_MSG ){
		
		if( menu_func_var.items<sizeof(select_item)/sizeof(MENU_SELECT_ITEM_T)-1 ){
			menu_func_var.items++;
		}
		else{
			menu_func_var.items=0;
		}
		
		char buf[DISP_BUF_SIZE];
		snprintf(buf,DISP_BUF_SIZE,"\n\n    %s",select_item[menu_func_var.items].itme_str);
		display_info_in(buf,0);
	}
	
	if( key==DOWN_KEY_MSG ){

		if( menu_func_var.items>0){
			menu_func_var.items--;
		}
		else{
			menu_func_var.items=sizeof(select_item)/sizeof(MENU_SELECT_ITEM_T)-1;
		}
		
		char buf[DISP_BUF_SIZE];
		snprintf(buf,DISP_BUF_SIZE,"\n\n    %s",select_item[menu_func_var.items].itme_str);
		display_info_in(buf,0);
	}
	
	if( vote_type_insert==0 ){
		if( sc_card_type_insert(CARD_B,VOTE_CARD_PRESENT) == FUN_OK ){
			vote_type_insert = 1;
			display_info_in("\n\n\nVOTE CARD IN",0);
		}
	}
	else{
		if( sc_card_type_insert(CARD_B,VOTE_CARD_PRESENT) != FUN_OK ){
			vote_type_insert = 0;
			display_info_in("\n\n\nVOTE CARD OUT",0);
		}
	}
}

/** 
* 选择通信方式. 
* 无. 
* @param[in]   key:按键. 
* @param[out]  输出.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-11-14创建 
*/
void menu_func_sel_comm_mode(uint8_t key)
{
	
	const MENU_SELECT_ITEM_T	select_item[]={
		
		0,"< RF   >",
		1,"< USB  >",
		2,"< Wifi >",
	};
	
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		menu_row_bar(4);

		lcdxx_clear_screen();
		
		display_info_in("Select communication mode",0);
		menu_func_var.items = 0;
		char buf[DISP_BUF_SIZE];
		snprintf(buf,DISP_BUF_SIZE,"\n\n    %s",select_item[menu_func_var.items].itme_str);
		display_info_in(buf,0);
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==ENTER_KEY_MSG ){
		menu_rst_bar();
		
		char buf[DISP_BUF_SIZE],buf1[5]={0};
		strncpy(buf1,&select_item[menu_func_var.items].itme_str[2],4);
		snprintf(buf,DISP_BUF_SIZE,"\n\n\nSelect %s OK",buf1);
		display_info_in(buf,0);
		
		comm_state.menu_setting.comm_mode = select_item[menu_func_var.items].num;
	}
	
	if( key==UP_KEY_MSG ){
		if( menu_func_var.items<sizeof(select_item)/sizeof(MENU_SELECT_ITEM_T)-1 ){
			menu_func_var.items++;
		}
		else{
			menu_func_var.items=0;
		}
		
		char buf[DISP_BUF_SIZE];
		snprintf(buf,DISP_BUF_SIZE,"\n\n    %s",select_item[menu_func_var.items].itme_str);
		display_info_in(buf,0);
	}
	
	if( key==DOWN_KEY_MSG ){
		if( menu_func_var.items>0){
			menu_func_var.items--;
		}
		else{
			menu_func_var.items=sizeof(select_item)/sizeof(MENU_SELECT_ITEM_T)-1;
		}
		
		char buf[DISP_BUF_SIZE];
		snprintf(buf,DISP_BUF_SIZE,"\n\n    %s",select_item[menu_func_var.items].itme_str);
		display_info_in(buf,0);
	}
}

/** 
* 允许连接新的EVT. 
* 无. 
* @param[in]   key:按键. 
* @param[out]  输出.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-11-14创建 
*/
void menu_func_cnct_new_evt(uint8_t key)
{
	static uint8_t last_evt_num,last_reg;
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;

		lcdxx_clear_screen();
		
		display_info_in("Connect new EVT",0);
		
		comm_state.menu_setting.cnct_new_evt = 1;
		last_evt_num = 0xff;
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
		
		comm_state.menu_setting.cnct_new_evt = 0;
	}
	
	if( key==ENTER_KEY_MSG ){
		
	}
	
	if( key==UP_KEY_MSG ){

	}
	
	if( key==DOWN_KEY_MSG ){

	}
	
	static uint8_t time=0xff;
	if( time++>100 ){
		time = 0;
		if( (last_evt_num != comm_state.evt_num.total) || (last_reg!=comm_state.evt_num.reg) ){
			last_evt_num = comm_state.evt_num.total;
			last_reg = comm_state.evt_num.reg;
			char buf[DISP_BUF_SIZE];
			snprintf(buf,DISP_BUF_SIZE,"\nT1=%d T2=%d T3=%d T4=%d Total=%d RG=%d",comm_state.evt_num.t1,comm_state.evt_num.t2,\
			comm_state.evt_num.t3,comm_state.evt_num.t4,comm_state.evt_num.total,comm_state.evt_num.reg);
			display_info_in(buf,0);
		}
	}
}

/** 
* 使用系统管理员卡结束投票. 
* 无. 
* @param[in]   key:键值. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2011-11-27创建 
*/
void menu_func_sys_end_election(uint8_t key)
{
	static uint8_t end_sel=0;
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		end_sel = 0;
		
		lcdxx_clear_screen();
		
		display_info_in("  END ELECTION",0);
		
		if( get_election_state() != ECECTION_DURING ){
			display_info_in("\n\nNot in the election",0);
		}
		else{
			display_info_in("\n\n      <NO>",0);
		}
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( get_election_state() == ECECTION_DURING ){
		
		if( key==ENTER_KEY_MSG ){
			if( end_sel ){
				display_info_in("\n\n\nSend END cmd...",0);
				if( comm_end_election()==FUN_OK ){
					election_stop();
					display_info_in("\n\n\nEnd complete",0);
				}
				else{
					display_info_in("\n\n\nEnd not execute",0);
				}
			}
		}
		
		if( key==UP_KEY_MSG ){
			if( end_sel==1 ){
				end_sel = 0;
				display_info_in("\n\n      <NO>",0);
			}
			else{
				end_sel = 1;
				display_info_in("\n\n      <YES>",0);
			}
		}
		
		if( key==DOWN_KEY_MSG ){
			if( end_sel==1 ){
				end_sel = 0;
				display_info_in("\n\n      <NO>",0);
			}
			else{
				end_sel = 1;
				display_info_in("\n\n      <YES>",0);
			}
		}
	}
}

/** 
* 使用2张管理员卡结束投票. 
* 无. 
* @param[in]   key:键值. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2011-11-27创建 
*/
void menu_func_admin_end_election(uint8_t key)
{
	static uint8_t end_sel=0;
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		end_sel = 0;
		
		lcdxx_clear_screen();
		
		display_info_in("  END ELECTION",0);
		
		if( get_election_state() != ECECTION_DURING ){
			display_info_in("\n\nNot in the election",0);
		}
		else{
			display_info_in("\n\n      <NO>",0);
		}
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( get_election_state() == ECECTION_DURING ){
		
		if( key==ENTER_KEY_MSG ){
			if( end_sel ){
				
				if( is_card_b_type(ADMIN_CARD_PRESENT) ){
				
					if( comm_end_election()==FUN_OK ){
						election_stop();
						display_info_in("\n\n\nEnd complete",0);
					}
					else{
						display_info_in("\n\n\nEnd error",0);
					}
				}
				else{
					display_info_in("\n\n\nAdmin not in B",0);
				}
			}
		}
		
		if( key==UP_KEY_MSG ){
			if( end_sel==1 ){
				end_sel = 0;
				display_info_in("\n\n      <NO>",0);
			}
			else{
				end_sel = 1;
				display_info_in("\n\n      <YES>",0);
			}
		}
		
		if( key==DOWN_KEY_MSG ){
			if( end_sel==1 ){
				end_sel = 0;
				display_info_in("\n\n      <NO>",0);
			}
			else{
				end_sel = 1;
				display_info_in("\n\n      <YES>",0);
			}
		}
	}
}

void menu_func_retrieve_tally(uint8_t key)
{
	static COMM_EVT_STATE_T *evt_state_ptr,*last_evt_state_ptr;
	char buf[70];
	
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		lcdxx_clear_screen();
		
		if( get_election_state() == ECECTION_DURING ){
			
			uint8_t i=0;
			
			while( i<EVT_NUM ){
				if( comm_state.evt_state[i].step == EVT_STEP_POLL ){
					evt_state_ptr = &comm_state.evt_state[i];
					last_evt_state_ptr = &comm_state.evt_state[i];
					break;
				}
				i++;
			}

			if( i<EVT_NUM ){
				if( evt_state_ptr->step == EVT_STEP_POLL ){
					snprintf(buf,64,"retrieve %02X tally",evt_state_ptr->addr);
					display_info_in(buf,0);
				}
			}
			else{
				display_info_in("no EVT device registered EBB",0);
				evt_state_ptr = NULL;
				last_evt_state_ptr = NULL;
			}
		}
		else{
			display_info_in("Not during election can not execute this function",0);
		}
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( get_election_state() == ECECTION_DURING ){
		
		if( key==ENTER_KEY_MSG ){
			if( evt_state_ptr != NULL ){
				snprintf(buf,64,"\n\nReq %02X tally",evt_state_ptr->addr);
				display_info_in(buf,0);
				comm_pack(evt_state_ptr->addr, config.ebb_addr, RF_EVT_CHN_POLL, (char*)CMD_REQ_CUR_VOTE_TALLY, NULL, 0);
			}
		}
		
		if( key==UP_KEY_MSG ){
			if( evt_state_ptr != NULL ){
				do{
					
					if( evt_state_ptr == &comm_state.evt_state[0] ){
						evt_state_ptr = &comm_state.evt_state[EVT_NUM-1];
					}
					else{
						evt_state_ptr--;
					}
					
					if( evt_state_ptr->step == EVT_STEP_POLL ){
						last_evt_state_ptr = evt_state_ptr;
						snprintf(buf,64,"retrieve %02X tally",evt_state_ptr->addr);
						display_info_in(buf,0);
					}
				}while( last_evt_state_ptr != evt_state_ptr );
			}
		}
		
		if( key==DOWN_KEY_MSG ){
			if( evt_state_ptr != NULL ){
				do{
					
					if( evt_state_ptr == &comm_state.evt_state[EVT_NUM-1] ){
						evt_state_ptr = &comm_state.evt_state[0];
					}
					else{
						evt_state_ptr++;
					}
					
					if( evt_state_ptr->step == EVT_STEP_POLL ){
						last_evt_state_ptr = evt_state_ptr;
						snprintf(buf,64,"retrieve %02X tally",evt_state_ptr->addr);
						display_info_in(buf,0);
					}
				}while( last_evt_state_ptr != evt_state_ptr );
			}
		}
	}
}

/** 
* 管理员卡复位设备. 
* 按下确认复位后，5秒倒计时可取消. 
* @param[in]   key:按键消息. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-12-04创建 
*/
void menu_func_admin_reset_device(uint8_t key)
{
	static uint8_t reset_flag=0;
	static uint8_t rsting_flag=0,rsting_time;
	static uint8_t sec=0;
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		
		reset_flag = 0;
		lcdxx_clear_screen();
		
		display_info_in("  RESET DEVICE",0);
		display_info_in("\n\n       NO",0);
	}
	
	if( key==CANCEL_KEY_MSG ){
		if( rsting_flag ){
			rsting_flag = 0;
			display_info_in("\n\n\n ",0);
		}
		else{
			display_exit_to_menu();
		}
	}
	
	if( key==ENTER_KEY_MSG ){
		if( is_card_b_type(ADMIN_CARD_PRESENT) ){
			if( reset_flag ){
				rsting_flag = 1;
				rsting_time = 5;
				lcdxx_display_string(0,LCDXX_ROW4,"RESET WITHIN",1);
				
				char buf[3];
				snprintf(buf,3,"%d",rsting_time);
				lcdxx_display_string(13,LCDXX_ROW4,buf,0);
			}
			else{
				display_exit_to_menu();
			}
		}
		else{
			display_info_in("\n\n\nAdmin Card In B",0);
		}
	}
	
	if( key==UP_KEY_MSG ){
		if( reset_flag ){
			reset_flag = 0;
			display_info_in("\n\n       NO",0);
		}
		else{
			reset_flag = 1;
			display_info_in("\n\n       YES",0);
		}
	}
	
	if( key==DOWN_KEY_MSG ){
		if( reset_flag ){
			reset_flag = 0;
			display_info_in("\n\n       NO",0);
		}
		else{
			reset_flag = 1;
			display_info_in("\n\n       YES",0);
		}
	}
	
	if( rsting_flag ){
		if( sec++ >= 100 ){
			sec = 0;
			if( rsting_time > 0 ){
				rsting_time--;
				
				char buf[3];
				snprintf(buf,3,"%d",rsting_time);
				lcdxx_display_string(13,LCDXX_ROW4,buf,1);
				
				if( rsting_time==0 ){
					HAL_NVIC_SystemReset();
				}
			}
		}
	}
}

/** 
* 系统管理员卡复位设备. 
* 按下确认复位后，5秒倒计时可取消. 
* @param[in]   key:按键消息. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-10-09创建 
*/
void menu_func_sys_reset_device(uint8_t key)
{
	static uint8_t reset_flag=0;
	static uint8_t rsting_flag=0,rsting_time;
	static uint8_t sec=0;
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		
		reset_flag = 0;
		lcdxx_clear_screen();
		
		display_info_in("  RESET DEVICE",0);
		display_info_in("\n\n       NO",0);
	}
	
	if( key==CANCEL_KEY_MSG ){
		if( rsting_flag ){
			rsting_flag = 0;
			display_info_in("\n\n\n ",0);
		}
		else{
			display_exit_to_menu();
		}
	}
	
	if( key==ENTER_KEY_MSG ){
		if( reset_flag ){
			rsting_flag = 1;
			rsting_time = 5;
			lcdxx_display_string(0,LCDXX_ROW4,"RESET WITHIN",1);
			
			char buf[3];
			snprintf(buf,3,"%d",rsting_time);
			lcdxx_display_string(13,LCDXX_ROW4,buf,0);
		}
		else{
			display_exit_to_menu();
		}
	}
	
	if( key==UP_KEY_MSG ){
		if( reset_flag ){
			reset_flag = 0;
			display_info_in("\n\n       NO",0);
		}
		else{
			reset_flag = 1;
			display_info_in("\n\n       YES",0);
		}
	}
	
	if( key==DOWN_KEY_MSG ){
		if( reset_flag ){
			reset_flag = 0;
			display_info_in("\n\n       NO",0);
		}
		else{
			reset_flag = 1;
			display_info_in("\n\n       YES",0);
		}
	}
	
	if( rsting_flag ){
		if( sec++ >= 100 ){
			sec = 0;
			if( rsting_time > 0 ){
				rsting_time--;
				
				char buf[3];
				snprintf(buf,3,"%d",rsting_time);
				lcdxx_display_string(13,LCDXX_ROW4,buf,1);
				
				if( rsting_time==0 ){
					HAL_NVIC_SystemReset();
				}
			}
		}
	}
}

/** 
* 显示当前类型的投票数据. 
* 无. 
* @param[in]   idx:序号，max:最大候选人数，type:投票类型. 
* @param[out]  输出.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-11-14创建 
*/
static void menu_sub_view_crt_tally_disp(uint8_t idx, uint8_t max, uint8_t type)
{
	uint16_t tally[3];
	char buf[50];
	
	if( max>0 ){
		if( (idx+1)<=(max) ){
			devfs_vote_tallies_read(type, idx, &tally[0]);
			snprintf(buf,sizeof(buf),"\n  %3d. - %5d",idx+1,tally[0]);
			display_info_in(buf,0);
		}
		else{
			display_info_in("\n ",0);
		}
		
		if( (idx+2)<=(max) ){
			devfs_vote_tallies_read(type, idx+1, &tally[1]);
			snprintf(buf,sizeof(buf),"\n\n  %3d. - %5d",idx+2,tally[1]);
			display_info_in(buf,0);
		}
		else{
			display_info_in("\n\n ",0);
		}
		
		if( (idx+3)<=(max) ){
			devfs_vote_tallies_read(type, idx+2, &tally[2]);
			snprintf(buf,sizeof(buf),"\n\n\n  %3d. - %5d",idx+3,tally[2]);
			display_info_in(buf,0);
		}
		else{
			display_info_in("\n\n\n ",0);
		}
	}
}

/** 
* 显示当前类型的投票数据，上下翻处理. 
* 无. 
* @param[in]   key：按键输入. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-11-14创建 
*/
void menu_func_view_crt_tally(uint8_t key)
{
	const MENU_SELECT_ITEM_T	select_item[]={
		
		1,"<VOTE TYPE1>",
		2,"<VOTE TYPE2>",
		3,"<VOTE TYPE3>",
		4,"<VOTE TYPE4>",
	};
	
	static uint8_t sel_types;
	static uint8_t idx;
	static uint8_t candidate_max;
	char buf[50];
	uint16_t tally[3];
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		sel_types = 0;
		menu_func_var.items = 0;
		lcdxx_clear_screen();
		display_info_in(" VIEW CRT TALLY",0);
		snprintf(buf,sizeof(buf),"\n  %s",select_item[menu_func_var.items].itme_str);
		display_info_in(buf,0);
	}
	
	if( key==CANCEL_KEY_MSG ){
		
		if( sel_types ){
			sel_types = 0;
			display_info_in("\n \n \n ",0);
			snprintf(buf,sizeof(buf),"\n  %s",select_item[menu_func_var.items].itme_str);
			display_info_in(buf,0);
		}
		else{
			display_exit_to_menu();
		}
	}
	
	if( key==ENTER_KEY_MSG ){
		
		if( sel_types ){
			
		}
		else{
			sel_types = 1;
			idx = 0;
			
			COMM_VOTE_SETTING_T vote_settings;
			devfs_read_vote_settings( select_item[menu_func_var.items].num, &vote_settings );
			
			candidate_max = vote_settings.total_candidates;
			
			display_info_in("\n \n \n ",0);
			
			menu_sub_view_crt_tally_disp(idx,candidate_max,select_item[menu_func_var.items].num);
		}
	}
	
	if( key==UP_KEY_MSG ){
		
		if( sel_types ){
			
			if( candidate_max>0 ){
				if( idx+3 < candidate_max-1 )
					idx += 3;

				menu_sub_view_crt_tally_disp(idx,candidate_max,select_item[menu_func_var.items].num);
			}
		}
		else{
			if( menu_func_var.items<sizeof(select_item)/sizeof(MENU_SELECT_ITEM_T)-1 ){
				menu_func_var.items++;
			}
			else{
				menu_func_var.items=0;
			}
			
			char buf[DISP_BUF_SIZE];
			snprintf(buf,DISP_BUF_SIZE,"\n  %s",select_item[menu_func_var.items].itme_str);
			display_info_in(buf,0);
		}
	}
	
	if( key==DOWN_KEY_MSG ){

		if( sel_types ){
			if( candidate_max>0 ){
				if( idx > 3 )
					idx -= 3;
				else
					idx = 0;

				menu_sub_view_crt_tally_disp(idx,candidate_max,select_item[menu_func_var.items].num);
			}
		}
		else{
			if( menu_func_var.items>0){
				menu_func_var.items--;
			}
			else{
				menu_func_var.items=sizeof(select_item)/sizeof(MENU_SELECT_ITEM_T)-1;
			}
			
			char buf[DISP_BUF_SIZE];
			snprintf(buf,DISP_BUF_SIZE,"\n  %s",select_item[menu_func_var.items].itme_str);
			display_info_in(buf,0);
		}
	}
}

void menu_func_deactivate_current_alarm(uint8_t key)
{
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
//		display_info_in("all alarm has \nbeen Deactivate",0);
//		bsp_RelieveTamperAlarm();
//		//reset_tamper_detect();
//		//urg_tamper_disable();
//		urg_status.tamper.byte= 0;
//		urg_status.urg.bit.tamper_detect = 0;
//		clr_tamper_alarm();
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==ENTER_KEY_MSG ){
	}
	
	if( key==UP_KEY_MSG ){
	}
	
	if( key==DOWN_KEY_MSG ){
	}
}

void menu_func_deactivate_alarm(uint8_t key){
	
	static uint8_t alarm_switch;
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		
//		alarm_switch = is_tamper_enable();
//		if( alarm_switch ){
//			display_info_in("Alarm state is \n <ENABLE>",0);
//		}
//		else{
//			display_info_in("Alarm state is \n <DISABLE>",0);
//		}
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==UP_KEY_MSG || key==DOWN_KEY_MSG ){
		if( alarm_switch ){
			alarm_switch = 0;
			display_info_in("\n <DISABLE>",0);
		}
		else{
			alarm_switch = 1;
			display_info_in("\n <ENABLE>",0);
		}
	}
	
	if( key==ENTER_KEY_MSG ){
//		bsp_SetBuzzerAlarm(BZ_EVENT_TAMPER_ACT);
//		if( alarm_switch ){
//			event_log_write(SYSTEM_LEVEL, SECURITY_TYPE, "Tamper Re-Activated");
//			urg_tamper_enable();
//		}
//		else{
//			event_log_write(SYSTEM_LEVEL, SECURITY_TYPE, "Tamper deactivated");
//			urg_tamper_disable();
//		}
//		display_exit_to_menu();
	}
}

void menu_func_disable_rf(uint8_t key){
	
		if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		
		if( is_rf_on() ){
			rf_module_on();
			display_info_in("RF \n <ON>",0);
		}
		else{
			rf_module_off();
			display_info_in("RF \n <OFF>",0);
		}
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==ENTER_KEY_MSG ){
		if( is_rf_on() ){
			rf_module_off();
			display_info_in("\n <OFF>",0);
		}
		else{
			rf_module_on();
			display_info_in("\n <ON>",0);
		}
	}
}

void menu_func_battery_status(uint8_t key){
	
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
//		lcd1602_ClearScreen();
//		
//		if( is_bat_plug_in() ){
//			char buf[DISP_BUF_SIZE];
//			snprintf(buf,DISP_BUF_SIZE,"Voltage: %2.2fV\nCurrent: 0.0mA",measure_ac_voltage());
//			display_info_in(buf,0);
//		}
//		else{
//			display_info_in("no battery \nplug in",0);
//		}
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
}

void menu_func_memory_space(uint8_t key){
	
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
//		lcd1602_ClearScreen();
//		
//		uint16_t secure_total = ZONE_MAX*AT88SC_BLOCK_SIZE;
//		uint16_t secure_usage = offsetof(CANDIDATE_T,vote_data)+election_status.candidates_num*CANDIDATE_NAME_LEN;
//		float secure_percent = (float)(secure_usage)/(float)(secure_total);
//		
//		uint32_t flash_usage = midfs_lfs_usage();
//		
//		float flash_percent = (float)(flash_usage)/(float)(sys_status.sys_run_para.flash_total);
//		
//		char buf[DISP_BUF_SIZE];
//		snprintf(buf,DISP_BUF_SIZE,"Flash: %2.1f%%\nSecure: %2.1f%%",flash_percent*100,secure_percent*100);
//		display_info_in(buf,0);
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
}

void menu_func_test_printer(uint8_t key){
	
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
//		lcd1602_ClearScreen();
//		display_info_in("Confirm key\n printer test",0);
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==ENTER_KEY_MSG ){
//		if( printer_PrintReceiptTest(0,0,true)==HAL_OK){
//			display_info_in("Printer test \nOK",0);
//		}
//		else{
//			display_info_in("Printer test \nError",0);
//		}
	}
}

void menu_func_test_vote(uint8_t key){
	
	static uint8_t test_enable=0;
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
//		lcd1602_ClearScreen();
//		if( is_election_start() ){
//			test_enable = 0;
//			display_info_in("During election\ncan not test",0);
//		}
//		else if( !is_configured() ){
//			test_enable = 0;
//			display_info_in("No configuration\ncan not test",0);
//		}
//		else{
//			test_enable = 1;
//			display_info_in("Press vote key\nvote test",0);
//		}
//		set_vote_step(VOTE_NOT_START);
	}
	
	if( key==CANCEL_KEY_MSG ){
//		led_buf_clr();
//		set_vote_step(VOTE_FINISHED);
//		display_exit_to_menu();
	}
	
//	if( test_enable )
//		app_TestVoteKpEventHandle();
}

//char text[5][EVNET_LOG_SIZE];

/** 
* log翻阅. 
* 上\下键->上下一条记录，确认\取消->一条记录超出一屏时翻屏. 
* @param[in]   key:键值. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-12-03创建 
*/
void menu_func_view_log(uint8_t key)
{
	
	static uint8_t seg,seg_max,file_record;
	static uint16_t pos;
	static char	*event_buf=NULL;
	static EVENT_LOG_HEADER_T	event_log_header;
	uint8_t refresh=0;
	
	if( menu_status.func_fitst ){
		
		menu_status.func_fitst = 0;
		lcdxx_clear_screen();
		
		event_log_header_read(&event_log_header);
		
		if( event_buf==NULL )
			event_buf = malloc(EVNET_LOG_SIZE);
		if(event_buf){
			seg = 0;
			memset(event_buf,0,EVNET_LOG_SIZE);
			
			if( event_log_header.logs>0 ){
				
				if( event_log_header.write_pos==0 )
					pos = event_log_header.logs-1;
				else
					pos = event_log_header.write_pos-1;
				
				file_record = event_log_header.logs-1;
				
				refresh = 1;
			}
			else{
				display_info_in("no new event log\n",0);
			}		
		}
		else{
			
		}
	}
	
	//<下翻屏
	if( key==ENTER_KEY_MSG ){
		if( seg<seg_max-1 ){
			seg ++;
			lcdxx_clear_screen();
			display_info_seg(seg,event_buf);
		}
	}
	
	//<上翻屏，第一屏时按下时返回菜单
	if( key==CANCEL_KEY_MSG ){
		
		if( seg==0 ){
//			led_buf_clr();
			free(event_buf);
			event_buf = NULL;
			display_exit_to_menu();
		}
		else{
			if( seg>0 ){
				seg--;
				lcdxx_clear_screen();
				display_info_seg(seg,event_buf);
			}
		}
	}
	
	///<上一条记录
	if( key==UP_KEY_MSG ){
		
		if( file_record>0 ){
			
			file_record--;
			
			if( pos>0 )	
				pos--;
			else
				pos = event_log_header.logs-1;
			
			refresh = 1;
		}
	}
	
	///<下一条记录
	if( key==DOWN_KEY_MSG ){
		
		if( file_record < event_log_header.logs-1 ){
			
			file_record++;
			
			if( pos < (event_log_header.logs-1) )	
				pos++;
			else
				pos = 0;
			
			refresh = 1;
		}
	}
	
	if( refresh ){
		
		refresh = 0;
		lcdxx_clear_screen();
		memset(event_buf,0,EVNET_LOG_SIZE);
		event_log_read(pos, event_buf);
		seg = 0;
		seg_max = strlen(event_buf)/DSIP_TEXT_LEN;
		if( strlen(event_buf)%DSIP_TEXT_LEN )
			seg_max += 1;
		display_info_seg(seg,event_buf);
	}
}

/** 
* 选择上传设置方式. 
* .
* @param[in]   key:键值. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-12-04创建 
*/
void menu_func_upl_settings(uint8_t key)
{
	const MENU_SELECT_ITEM_T	select_item[]={
		
		0,"\n\n <     RF    >",
		1,"\n\n <Remote Port>",
		2,"\n\n <    WIFI   >",
		3,"\n\n <    USB    >",
	};
	static uint8_t sel=0;
	
	if( menu_status.func_fitst ){
		
		menu_status.func_fitst = 0;
		lcdxx_clear_screen();
		
		display_info_in("Upload Settings",0);
		if( config.menu_config.upload_settings > sizeof(select_item) )
			config.menu_config.upload_settings = 0;
		sel = config.menu_config.upload_settings;
		display_info_in(select_item[sel].itme_str,0);
	}
	
	#define select_num 	sizeof(select_item)/sizeof(MENU_SELECT_ITEM_T)
	
	if( key==ENTER_KEY_MSG ){
		config.menu_config.upload_settings = sel;
		display_exit_to_menu();
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==UP_KEY_MSG ){
		if( sel < select_num-1 ){
			sel ++;
		}
		else{
			sel  = 0;
		}
		display_info_in(select_item[sel].itme_str,0);
	}
	
	if( key==DOWN_KEY_MSG ){
		if( sel>0 ){
			sel --;
		}
		else{
			sel = select_num-1;
		}
		display_info_in(select_item[sel].itme_str,0);
	}
}

/** 
* 设置攥改开闭. 
* .
* @param[in]   key:键值. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-12-04创建 
*/
void menu_func_deact_tmp_alarm( uint8_t key )
{
	const MENU_SELECT_ITEM_T	deact_tmp_item[]={
		
		0,"\n\n <    ON     >",
		1,"\n\n <    OFF    >",
	};
	#define deact_num 	sizeof(deact_tmp_item)/sizeof(MENU_SELECT_ITEM_T)
	static uint8_t sel=0;
	
	if( menu_status.func_fitst ){
		
		menu_status.func_fitst = 0;
		lcdxx_clear_screen();
		
		display_info_in("  Deactivate \n     Tamper",0);
		
		if( config.menu_config.deact_tmp_alarm > deact_num )
			config.menu_config.deact_tmp_alarm = 0;
		
		sel = config.menu_config.deact_tmp_alarm;
		display_info_in(deact_tmp_item[sel].itme_str,0);
	}
	
	if( key==ENTER_KEY_MSG ){
		if( sel==0 ){
			event_log_write(SYSTEM_LEVEL, SECURITY_TYPE, "Tamper Re-Activated");
		}
		else if( sel==1 ){
			event_log_write(SYSTEM_LEVEL, SECURITY_TYPE, "Tamper deactivated");
		}
		config.menu_config.deact_tmp_alarm = sel;
		beep_set_alarm(BP_TAMPER_ACT);
		display_exit_to_menu();
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==UP_KEY_MSG ){
		if( sel < deact_num-1 ){
			sel ++;
		}
		else{
			sel  = 0;
		}
		display_info_in(deact_tmp_item[sel].itme_str,0);
	}
	
	if( key==DOWN_KEY_MSG ){
		if( sel>0 ){
			sel --;
		}
		else{
			sel = deact_num-1;
		}
		display_info_in(deact_tmp_item[sel].itme_str,0);
	}
}

/** 
* 关闭当前蜂鸣器报警. 
* .
* @param[in]   key:键值. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-12-04创建 
*/
void menu_func_deact_crt_alarm(uint8_t key)
{
	if( menu_status.func_fitst ){
		
		menu_status.func_fitst = 0;
		lcdxx_clear_screen();
		
		display_info_in("  Deactivate \ncurrent tamper",0);
	}
	
	if( key==ENTER_KEY_MSG ){
		
		beep_set_alarm(BP_STOP);
		display_info_in("\n\nDEACTIVATED",0);
		display_exit_to_menu();
	}
	
	if( key==CANCEL_KEY_MSG ){
		display_exit_to_menu();
	}
	
	if( key==UP_KEY_MSG ){

	}
	
	if( key==DOWN_KEY_MSG ){
	}
}

uint8_t menu_func_cnt_err(COMM_EVT_FALG_T flag)
{
	uint8_t cnt=0;
	if( flag.bit.help_detect==1 )
		cnt++;
	if( flag.bit.low_paper==1 )
		cnt++;
	if( flag.bit.low_power==1 )
		cnt++;
	if( flag.bit.tamper_detect==1 )
		cnt++;
	return cnt;
}

/** 
* 查看当前的报警信息. 
* .
* @param[in]   key:键值. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-12-06创建 
*/
void menu_func_view_crt_alarm(uint8_t key)
{
	static uint8_t 	cnt;
	static uint8_t	sel_flag;
	static uint8_t	sel_max;
	static uint8_t 	curr_sel;
	char buf[DISP_BUF_SIZE];
	if( menu_status.func_fitst ){
		menu_status.func_fitst = 0;
		
		lcdxx_clear_screen();
		
		display_info_in(" CURRENT ALARM",0);
		
		snprintf(buf,DISP_BUF_SIZE,"\n\n  <   EBB  >  %d",urg_cnt_err());
		display_info_in(buf,0);
		
		sel_flag = 0;
		curr_sel = 0;
		sel_max = comm_state.evt_num.total+1;
	}
	
	///<选择EBB或EVT
	if( sel_flag==0 ){
		
		if( key==CANCEL_KEY_MSG ){
			display_exit_to_menu();
		}
		
		if( key==ENTER_KEY_MSG ){
			
		}
		
		if( key==UP_KEY_MSG ){
			if( curr_sel < sel_max-1 ){
				curr_sel++;
			}
			if( curr_sel==0 ){
				snprintf(buf,DISP_BUF_SIZE,"\n\n  <   EBB  >  %d",urg_cnt_err());
				display_info_in(buf,0);
			}
			else{
				snprintf(buf,DISP_BUF_SIZE,"\n\n  < EVT.%2d >  %d",(int)comm_state.evt_state[curr_sel-1].addr,menu_func_cnt_err(comm_state.evt_state[curr_sel-1].evt_flag));
				display_info_in(buf,0);
			}
		}
		
		if( key==DOWN_KEY_MSG ){
			if( curr_sel > 0 ){
				curr_sel--;
			}
			if( curr_sel==0 ){
				snprintf(buf,DISP_BUF_SIZE,"\n\n  <   EBB  >  %d",urg_cnt_err());
				display_info_in(buf,0);
			}
			else{
				snprintf(buf,DISP_BUF_SIZE,"\n\n  < EVT.%2d >  %d",(int)comm_state.evt_state[curr_sel-1].addr,menu_func_cnt_err(comm_state.evt_state[curr_sel-1].evt_flag));
				display_info_in(buf,0);
			}
		}
	}
	///<查看报警信息
	else{
		if( key==CANCEL_KEY_MSG ){
		}
		
		if( key==ENTER_KEY_MSG ){
			
			while( cnt<10 ){
				if( cnt<8 ){
					if( urg_status.tamper.byte & (1<<cnt) ){
						switch( cnt ){
							case 0:
								display_info_in("Device Opened\n\n",0);
								break;
							case 1:
								display_info_in("Device Tilted\n",0);
								break;
							case 2:
								display_info_in("Auth Failed - RF\n",0);
								break;
							case 3:
								display_info_in("Auth Failed - \nRemote Port",0);
								break;
							case 4:
								display_info_in("Memory Mismatch\n",0);
								break;
							case 5:
								display_info_in("Incorrect Voter \nKey present",0);
								break;
							case 6:
								display_info_in("Vote Card Tried \ntoo many times",0);
								break;
						}
						break;
					}
				}
				else if( cnt==8 ){
					if( urg_status.urg.bit.low_power ){
						
						display_info_in("Battery low\nplug into AC",0);
					}
				}
				cnt++;
			}
			if( cnt>=10 )
				cnt = 0;
		}
	}
}

void menu_func_deregister(uint8_t key)
{
	
//	if( menu_status.func_fitst ){
//		
//		menu_status.func_fitst = 0;
//		lcd1602_ClearScreen();
//		if( is_election_start() ){
//			display_info_in("can't operation \nduring election",0);
//		}
//		else{
//			display_info_in("OK to confirm \nderegister",0);
//		}
//	}
//	
//	if( key==ENTER_KEY_MSG && !is_election_start() ){
//		
//		display_info_in("deregister had \nbeen execute",0);
//		config.comm_process=PROCESS_STRT;
//		comm_active_send(CMD_CLR_SET_DEREG);
//	}
//	
//	if( key==CANCEL_KEY_MSG ){
//		
//		display_exit_to_menu();
//	}
}

void menu_func_chg_vote(uint8_t key){
	
//	if( menu_status.func_fitst ){
//		
//		menu_status.func_fitst = 0;
//		lcd1602_ClearScreen();
//		if( is_election_start() ){
//			display_info_in("can't operation \nduring election",0);
//		}
//		else{
//			display_info_in("OK to confirm \nclear settings",0);
//		}
//	}
//	
//	if( key==ENTER_KEY_MSG && !is_election_start() ){
//		
//		display_info_in("settings has\n been clear",0);
//		config.comm_process=PROCESS_INIT;
//		config.vote_type = BACKUP_DEVICE;
//		comm_active_send(CMD_CLR_SET_CHG_TYPE);
//	}
//	
//	if( key==CANCEL_KEY_MSG ){
//		
//		display_exit_to_menu();
//	}
}

void menu_func_clr_backup_mem(uint8_t key){
	
//	if( menu_status.func_fitst ){
//		menu_status.func_fitst = 0;
//		
//		display_info_in("Press OK to \nstart",0);
//	}
//	
//	if( key==CANCEL_KEY_MSG ){
//		display_exit_to_menu();
//	}
//	
//	if( key==ENTER_KEY_MSG ){
//		if( is_election_start() || is_election_stop() ){
//		
//			display_info_in("op prior to the \nelection begin",0);
//		}
//		else{
//			uint16_t len;
//			len = sc_clear_storage(SC_BACKUP, 0, g_smartCardManage[1].storage_size);
//			if( len > 0 ){
//				char buf[DISP_BUF_SIZE];
//				snprintf(buf,DISP_BUF_SIZE,"clear complete \n %d bytes",len);
//				display_info_in(buf,0);
//			}
//			else
//				display_info_in("clear fault\n\n",0);
//		}
//	}
}

void menu_func_deactivate_evt(uint8_t key){
	
//	if( menu_status.func_fitst ){
//		menu_status.func_fitst = 0;
//		display_info_in("please insert \nadmin card2");
//		set_req_admin_card2();
//		clr_admin_card2_present();
//	}
//	
//	if( key==CANCEL_KEY_MSG ){
//		display_exit_to_menu();
//		clr_req_admin_card2();
//	}
//	
//	if( key==ENTER_KEY_MSG ){
//		display_exit_to_menu();
//		clr_req_admin_card2();
//		display_info_in("evt deactivate\n return menu");
//		vTaskDelay(1000);
//	}
//	
//	if( is_admin_card2_present() ){
//		clr_admin_card2_present();
//		display_info_in("admin card2 \nConfirm deactivate");
//	}
}

void menu_func_disable_evt(uint8_t key){
	
//	if( menu_status.func_fitst ){
//		menu_status.func_fitst = 0;
//		display_info_in("please insert \nadmin card2");
//	}
//	
//	if( key==CANCEL_KEY_MSG ){
//		display_exit_to_menu();
//	}
//	
//	if( key==ENTER_KEY_MSG ){
//		stop_election();
//		display_info_in("end the voting\n");
//	}
}

void menu_func_ovr_backup_mem(uint8_t key){
	
//	if( menu_status.func_fitst ){
//		menu_status.func_fitst = 0;
//		display_info_in("ovr backup mem \ncomplete future");
//	}
//	
//	if( key==CANCEL_KEY_MSG ){
//		display_exit_to_menu();
//	}
//	
//	if( key==ENTER_KEY_MSG ){
//		display_exit_to_menu();
//	}
//	
//	if( key==UP_KEY_MSG ){
//		display_info_in("enable charger \n");
//		en_charger();
//	}
//	
//	if( key==DOWN_KEY_MSG ){
//		display_info_in("disable charger \n");
//		db_charger();
//	}
}

void menu_func_connect_new_ebb(uint8_t key){
	
//	static uint8_t addr;
//	if( menu_status.func_fitst ){
//		menu_status.func_fitst = 0;
//		char buf[DISP_BUF_SIZE];
//		if( config.ebb_addr==1 || (config.ebb_addr>=0x14 && config.ebb_addr<=0xff) )
//			addr = config.ebb_addr;
//		else
//			addr = config.ebb_addr = 1;
//		snprintf(buf,DISP_BUF_SIZE,"set EBB addr:\n0x1A4705%02X",addr);
//		display_info_in(buf);
//	}
//	
//	if( key==CANCEL_KEY_MSG ){
//		display_exit_to_menu();
//	}
//	
//	if( key==ENTER_KEY_MSG ){
//		config.ebb_addr = addr;
//		rf_module_set_target(config.ebb_addr);
//		display_info_in("connect to EBB\n");
//		config.comm_process = 0;
//		election_status.flag.byte = 0;
//	}
//	
//	if( key==UP_KEY_MSG ){
//		if( addr==1 ){
//			char buf[3];
//			addr=0x14;
//			snprintf(buf,3,"%02X",addr);
//			lcd1602_DispalyData(LCD1602_ROW2,8,(const uint8_t*)buf, 2);
//		}
//		else if( addr<0xff ){
//			char buf[3];
//			addr++;
//			snprintf(buf,3,"%02X",addr);
//			lcd1602_DispalyData(LCD1602_ROW2,8,(const uint8_t*)buf, 2);
//		}
//	}
//	
//	if( key==DOWN_KEY_MSG ){
//		if( addr>0x14 ){
//			char buf[3];
//			addr--;
//			snprintf(buf,3,"%02X",addr);
//			lcd1602_DispalyData(LCD1602_ROW2,8,(const uint8_t*)buf, 2);
//		}
//		else if( addr==0x14 ){
//			char buf[3];
//			addr=1;
//			snprintf(buf,3,"%02X",addr);
//			lcd1602_DispalyData(LCD1602_ROW2,8,(const uint8_t*)buf, 2);
//		}
//	}
}

void menu_func_send_log(uint8_t key){
	
//	static uint16_t log_num;
//	static uint8_t	cancel_send=0;
//	static uint8_t	refresh=0;
//	if( menu_status.func_fitst ){
//		menu_status.func_fitst = 0;

//		EVENT_LOG_HEADER_T	event_log_header;		
//		event_log_header_read(&event_log_header);
//		
//		char buf[33];
//		snprintf( buf,33, "have %d new log\nConfirm to send",event_log_header.total );
//		display_info_in(buf);
//		
//		log_num = event_log_header.total;
//		cancel_send=0;
//	}
//	
//	if( key==CANCEL_KEY_MSG ){
//		
//		if( !is_send_log_en() ){
//			display_exit_to_menu();
//		}
//		else if( cancel_send ){
//			en_send_log(0);
//			cancel_send = 0;
//			refresh = 0;
//			display_info_in("event log send\nbreak");
//		}
//		else{
//			display_exit_to_menu();
//		}
//	}
//	
//	if( key==ENTER_KEY_MSG ){
//		
//		if( !is_send_log_en() ){
//			cancel_send = 1;
//			en_send_log(log_num);
//			display_info_in("event log send\n ");
//			refresh = 1;
//		}
//	}
//	
//	if( key==UP_KEY_MSG ){

//	}
//	
//	if( key==DOWN_KEY_MSG ){
//	}
//	
//	if( refresh ){
//		if( is_send_log_en() ){
//			char buf[5];
//			snprintf(buf,5,"%4d",send_log_remain());
//			lcd1602_DispalyData(LCD1602_ROW2,0,(const uint8_t*)buf, 4);
//		}
//		else{
//			refresh = 0;
//			display_exit_to_menu();
//		}
//	}
}
