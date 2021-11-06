/** 
* @file         display.c 
* @brief        显示处理函数. 
* @details  	菜单处理，显示信息处理，菜单回调函数处理. 
* @author       ken deng
* @date     	2019-08-16
* @version  	A001 
* @par Copyright (c):  
*       laxton company 
* @par History:          
*   version: ken deng, 2019-08-16, 建立\n 
*/  
#include "system.h"
#include "display.h"
#include "daemon.h"
#include "main.h"
#include "ioc.h"
#include "menu_func.h"
#include "lcdxx.h"
#include "config.h"
#include "pm.h"
#include "main.h"

#ifdef DISPLAY_DEBUG
    #define	display_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DISPLAY](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define display_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DISPLAY] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define display_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DISPLAY] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define display_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define display_log(...)
    #define display_usr(...)
    #define display_err(...)
    #define display_dump(...)
#endif
	
DISPLAY_MENU_STATUS_T	menu_status;
TaskHandle_t			display_handle;
extern const DISPLAY_MENU_T admin_card_menu[];

///<菜单项中同时存在回调函数及子菜单，优先执行回调函数
const DISPLAY_MENU_T device_menu_admin[]={
	"RF"			 ,admin_card_menu,NULL,NULL,
	"USB"			 ,admin_card_menu,NULL,NULL,
	"WI-FI"			 ,admin_card_menu,NULL,NULL,
};

const DISPLAY_MENU_T device_menu_system[]={
	"RF"			 ,system_card_menu,NULL,NULL,
	"USB"			 ,system_card_menu,NULL,NULL,
	"WI-FI"			 ,system_card_menu,NULL,NULL,
};

///<管理员卡菜单
const DISPLAY_MENU_T admin_card_menu[]={
	"GENERATE KEYS  ",NULL,NULL,NULL,
	"UPL SETTINGS   ",NULL,NULL,menu_func_upl_settings,
	"UPL AUDIO - SD ",NULL,NULL,NULL,
	"RETRIEVE LOGS  ",NULL,NULL,NULL,
	"VIEW LOGS      ",NULL,NULL,menu_func_view_log,
	"SAVE LOGS TO SD",NULL,NULL,NULL,
	"EABLE CARD UNIT",NULL,NULL,menu_activate_vote_card,
	"DABLE CARD UNIT",NULL,NULL,NULL,
	"VIEW CRT TALLY ",NULL,NULL,menu_func_view_crt_tally,
	"VIEW CRT ALARM ",NULL,NULL,menu_func_view_crt_alarm,
	"DEACT CRT ALARM",NULL,NULL,menu_func_deact_crt_alarm,
	"DEACT TMP-ALARM",NULL,NULL,menu_func_deact_tmp_alarm,
	"BATTERY STATUS ",NULL,NULL,NULL,
	"RETRIEVE TALLY ",NULL,NULL,menu_func_retrieve_tally,
	"END ELECTION   ",NULL,NULL,menu_func_admin_end_election,
	"TALLY TO SD    ",NULL,NULL,NULL,
	"RESET DEVICE   ",NULL,NULL,menu_func_admin_reset_device,
	"SEL COMM MODE  ",NULL,NULL,menu_func_sel_comm_mode,
	"EXIT           ",NULL,NULL,menu_func_exit,
};

///<系统管理员卡菜单
const DISPLAY_MENU_T system_card_menu[]={
	"UPL SETTINGS   " ,NULL,NULL,NULL,
	"SCAN DEVICES   " ,NULL,NULL,NULL,
	"CNCT TO NEW EVT" ,NULL,NULL,menu_func_cnct_new_evt,
	"RETRIEVE TALLY " ,NULL,NULL,menu_func_retrieve_tally,
	"RESET DEVICE   " ,NULL,NULL,menu_func_sys_reset_device,
	"DLD NEW SETTING" ,NULL,NULL,NULL,
	"END ELECTION   ",NULL,NULL,menu_func_sys_end_election,
	"SEL COMM MODE  ",NULL,NULL,menu_func_sel_comm_mode,
	"EXIT           ",NULL,NULL,menu_func_exit,
};

///<菜单中的项数
const DISPLAY_MENU_SIZE_T		menu_size[]={
	&admin_card_menu[0],		sizeof(admin_card_menu)/sizeof(DISPLAY_MENU_T),
	&device_menu_admin[0],		sizeof(device_menu_admin)/sizeof(DISPLAY_MENU_T),
	&system_card_menu[0],		sizeof(system_card_menu)/sizeof(DISPLAY_MENU_T),
	&device_menu_system[0],		sizeof(device_menu_system)/sizeof(DISPLAY_MENU_T),
};

/** 
* 显示公司名称及设备型号. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_start_up(void){
	
	char buf[DISP_BUF_SIZE];
	snprintf(buf,DISP_BUF_SIZE,"     LAXTON     \n%s",config.electoral_commission);
	display_info_in(buf,0);
	
	snprintf(buf,DISP_BUF_SIZE,"\n%s",DEVICE_MODEL);
	display_info_in(buf,0);
}

/** 
* 切换到信息显示. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
void display_exit_to_info(void){
	
	menu_status.curr_menu = NULL;
	menu_status.state = MENU_INFO;
	menu_status.screen_refresh = 1;
}

/** 
* 切换到菜单显示. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
void display_exit_to_menu(void){
	
	menu_status.menu_func = NULL;
	menu_status.refresh_flag = 1;
	menu_status.state = MENU_SELECT;
}

/** 
* 切换菜单. 
* 无. 
* @param[in]   menu:菜单句柄. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_switch_menu(const DISPLAY_MENU_T *menu){
	
	if( menu != NULL ){
		menu_status.init_flag = 1;
	}
	menu_status.menu_func = NULL;
	menu_status.curr_menu = (DISPLAY_MENU_T	*)menu;
	menu_status.state = MENU_SELECT;
}

/** 
* 获取当前菜单的显示项数. 
* 无. 
* @param[in]   menu:菜单句柄. 
* @param[out]  无.  
* @retval  返回菜单项数
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
uint8_t display_menu_size(DISPLAY_MENU_T *menu){
	
	for( uint8_t i=0; i<sizeof(menu_size)/sizeof(DISPLAY_MENU_SIZE_T); i++ ){
		if( menu_size[i].menu==menu ){
			return menu_size[i].num;
		}
	}
	return 0;
}

/** 
* 显示消息入队列. 
* 无. 
* @param[in]   info:显示消息,confirm:1需按键确认、0：不需按键确认. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*	   2019-10-30 ken deng:入队列不成功，申请内存空间丢失
*	   2019-11-19 ken deng:增加是否按键确认
*/
void display_info_in(char *info, uint8_t confirm)
{
	
	if( strlen(info)>DISP_BUF_SIZE )
		info[DISP_BUF_SIZE-1] = 0;
	else
		info[strlen(info)] = 0;
	
	uint8_t *buf;
	buf = malloc(DISP_BUF_SIZE);
	
	if( buf ){
		DISPLAY_MSG_T msg;
		msg.buf = buf;
		msg.confirm = confirm;
		memcpy(buf, info, DISP_BUF_SIZE);
		if( xQueueSend(menu_status.disp_queue,&msg,0) !=pdPASS ){
			free(buf);
		}
	}
}

/** 
* 只有指定菜单回调下才入队列显示消息. 
* 无. 
* @param[in]   func:菜单回调函数,info：显示消息. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-12-27创建 
*/
FUN_STATUS_T display_screen_info_in( void(*func)(uint8_t), char *info )
{
	if( is_menu_func(func) ){
		display_info_in(info,0);
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 显示消息出队列. 
* 无. 
* @param[in]   info:显示消息. 
* @param[out]  无.  
* @retval  1:有新显示消息，0：没有新显示消息
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
FUN_STATUS_T display_info_out(char *info, uint8_t *confirm)
{
	DISPLAY_MSG_T msg;
	if( xQueueReceive(menu_status.disp_queue,&msg,0)==pdPASS ){
		memcpy(info,msg.buf,DISP_BUF_SIZE);
		*confirm = msg.confirm;
		free(msg.buf);
		return FUN_OK;
	}
	return FUN_NONE;
}

/** 
* 按键消息入队列. 
* 无. 
* @param[in]   button_msg:按键消息. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_button_in(BUTTON_MSG_T *button_msg)
{
	xQueueSend(menu_status.button_queue,button_msg,0);
//	ioc_led_blink_cnt(button_msg->value,100,1);
}

/** 
* 按键消息出队列. 
* 无. 
* @param[in]   button_msg:按键消息. 
* @param[out]  无.  
* @retval  FUN_OK:有按键消息，
* @retval  0： 没有新消息
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
FUN_STATUS_T display_button_out(BUTTON_MSG_T *button_msg)
{
	if( xQueueReceive(menu_status.button_queue,button_msg,0)==pdPASS ){
		return FUN_OK;
	}
	return FUN_NONE;
}

/** 
* 分屏显示长字符串消息. 
* 无. 
* @param[in]   seg:段号,info:消息字符串. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*	   Ken Deng于2019-12-03 改为全屏拷贝
*/
void display_info_seg(uint8_t seg, char *info)
{
	uint8_t len,pos=0,line=0;;
	len = strlen(info);
	char buf[DISP_BUF_SIZE];
	memset(buf,0,DISP_BUF_SIZE);
	
	if( (seg*DSIP_TEXT_LEN + DSIP_TEXT_LEN) < len ){
		
		memcpy(buf,info+seg*DSIP_TEXT_LEN,DSIP_TEXT_LEN);
	}
	else{
		
		memcpy(buf,info+seg*DSIP_TEXT_LEN,len%DSIP_TEXT_LEN);
	}
	
	display_info_in(buf,0);
}

/** 
* 解码字符格式. 
* 将解码的数据放入相应的行显示缓冲区. 
* @param[in]   str:源字符串. 
* @param[out]  p_disp_dec_str:输出显示字符串缓冲区.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
void display_decode_string(char *str, DISP_DEC_STR_T *p_disp_dec_str)
{	
	memset(p_disp_dec_str,0,sizeof(DISP_DEC_STR_T));
	//sscanf(str,"%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]\n",p_disp_dec_str->str[0],p_disp_dec_str->str[1],p_disp_dec_str->str[2],p_disp_dec_str->str[3]);
	uint8_t idx=0;
	uint8_t substr_len;
	while( *str ){
		substr_len = 0;
		while( *str != '\n' && *str && substr_len<LCDXX_COLUMN_NUMBER ){
			p_disp_dec_str->str[idx][substr_len++] = *str;
			str++;
		}
		if( *str == '\n' ){
			str++;
		}
		
		p_disp_dec_str->len[idx] = substr_len;
		
		idx ++;
		
		if( idx >= LCDXX_ROW_NUMBER ) 
			break;
	}
}

/** 
* 进度条显示个数. 
* 如果中间有几个没有设置，会自动补上. 
* @param[in]   step:显示第几个. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
void display_process_bar(uint8_t step)
{
	if( step>LCDXX_COLUMN_NUMBER )
		return;
	while( step>menu_status.process_step ){
		lcdxx_progress_bar(4,menu_status.process_step++);
	}
}

/** 
* 判断菜单的范围. 
* 无. 
* @param[in]   curr_item_ptr:菜单当前项指针. 
* @param[out]  DISPLAY_MENU_T 菜单指针.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-12-04创建 
*/
DISPLAY_MENU_T * disp_menu_rang(DISPLAY_MENU_T *curr_item_ptr,int8_t value)
{
	static int32_t len = admin_card_menu+sizeof(admin_card_menu)/sizeof(DISPLAY_MENU_T);
	if( (curr_item_ptr >= admin_card_menu) && (curr_item_ptr < (admin_card_menu+sizeof(admin_card_menu)/sizeof(DISPLAY_MENU_T))) ){
		
		///<超出菜单底部
		if( (curr_item_ptr+value) >= (admin_card_menu+sizeof(admin_card_menu)/sizeof(DISPLAY_MENU_T)) ){
			return (DISPLAY_MENU_T *)((curr_item_ptr-admin_card_menu+value)%sizeof(admin_card_menu)/sizeof(DISPLAY_MENU_T)+admin_card_menu);
		}
		///<超出菜单顶部
		else if( (curr_item_ptr+value) < admin_card_menu ){
			int8_t num;
			num = (curr_item_ptr-admin_card_menu)+value+sizeof(admin_card_menu)/sizeof(DISPLAY_MENU_T);
			return (DISPLAY_MENU_T *)(admin_card_menu+num);
		}
		else{
			return curr_item_ptr+value;
		}
	}
	else if( curr_item_ptr >= system_card_menu && curr_item_ptr < (system_card_menu+sizeof(system_card_menu)/sizeof(DISPLAY_MENU_T)) ){
		
		///<超出菜单底部
		if( (curr_item_ptr+value) >= (system_card_menu+sizeof(system_card_menu)/sizeof(DISPLAY_MENU_T)) ){
			return (DISPLAY_MENU_T *)((curr_item_ptr-system_card_menu+value)%sizeof(system_card_menu)/sizeof(DISPLAY_MENU_T)+system_card_menu);
		}
		///<超出菜单顶部
		else if( (curr_item_ptr+value) < system_card_menu ){
			int8_t num;
			num = (curr_item_ptr-system_card_menu)+value+sizeof(system_card_menu)/sizeof(DISPLAY_MENU_T);
			return (DISPLAY_MENU_T *)(system_card_menu+num);
		}
		else{
			return curr_item_ptr+value;
		}
	}
}

/** 
* 显示菜单项. 
* 无. 
* @param[in]   curr_item_ptr:菜单当前项指针，max_item:菜单的最大项数,dir：正向或反向显示. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_menu_item(DISPLAY_MENU_T *curr_item_ptr, uint8_t cursor, uint8_t max_item, uint8_t dir)
{
	uint8_t max;
	int8_t pos,new_cursor;
	DISPLAY_MENU_T *new_item_ptr;
	if( max_item > DISP_MENU_ITEMS ){
		max = DISP_MENU_ITEMS;
	}
	else{
		max = max_item;
	}
	lcdxx_clear_screen();
	new_cursor = cursor;
	if( dir ){
		for( uint8_t i=0; i<max; i++ ){
			
			if( i%cursor )
				pos = ((max-i)%cursor)*(-1);
			else
				pos = (i);
			
			new_item_ptr = disp_menu_rang(curr_item_ptr,pos);
			lcdxx_display_string(1, new_cursor, new_item_ptr->item_str,0);
			
			new_cursor++;	
			if(new_cursor==5)
				new_cursor=1;
		}
	}
	else{
		for( uint8_t i=0; i<max; i++ ){
			
			if( i/cursor && i%cursor )
				pos = (i%cursor)*(-1);
			else
				pos = (i);
			
			new_item_ptr = disp_menu_rang(curr_item_ptr,pos);
			lcdxx_display_string(1, new_cursor, new_item_ptr->item_str,0);
			
			new_cursor++;	
			if(new_cursor==5)
				new_cursor=1;
		}
	}
}

/** 
* 显示菜单光标位置. 
* 无. 
* @param[in]   curror:光标位置. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_menu_curror(uint8_t curror)
{
	for( uint8_t i=1; i<=DISP_MENU_ITEMS; i++ ){
		if( i==curror )
			lcdxx_display_string(0, curror, ">",0);
		else
			lcdxx_display_string(0, i, " ",0);
	}
}

/** 
* 解释文本数据并显示在相应的行上. 
* 无. 
* @param[in]   text:文本指针,clear_screen:1清屏，0只清行. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-10-22创建 
*/
void display_text(char *text,uint8_t clear_screen)
{
	DISP_DEC_STR_T	disp_dec_str;
	
	if( clear_screen )
		lcdxx_clear_screen();
			
	display_decode_string(text,&disp_dec_str);
	
	if( disp_dec_str.len[0]!=0 ){
		
		if( clear_screen==0 )
			lcdxx_clear_row(LCDXX_ROW1);
		lcdxx_dispaly_data(LCDXX_ROW1,0,(uint8_t*)&disp_dec_str.str[0],disp_dec_str.len[0]);
	}
	
	if( disp_dec_str.len[1]!=0 ){
		
		if( clear_screen==0 )
			lcdxx_clear_row(LCDXX_ROW2);
		lcdxx_dispaly_data(LCDXX_ROW2,0,(uint8_t*)&disp_dec_str.str[1],disp_dec_str.len[1]);
	}
	
	if( disp_dec_str.len[2]!=0 ){
		
		if( clear_screen==0 )
			lcdxx_clear_row(LCDXX_ROW3);
		lcdxx_dispaly_data(LCDXX_ROW3,0,(uint8_t*)&disp_dec_str.str[2],disp_dec_str.len[2]);
	}
	
	if( disp_dec_str.len[3]!=0 ){
		
		if( clear_screen==0 )
			lcdxx_clear_row(LCDXX_ROW4);
		lcdxx_dispaly_data(LCDXX_ROW4,0,(uint8_t*)&disp_dec_str.str[3],disp_dec_str.len[3]);
	}
}

/** 
* 处理按键消息上菜单选择，菜单回调函数处理. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_menu_select(void)
{
	static DISPLAY_MENU_T	*curr_item_ptr;
	
	uint8_t	display_msg[DISP_BUF_SIZE];
	static uint8_t confirm = 0;
	DISP_DEC_STR_T	disp_dec_str;
	
	if( menu_status.disp_time>DISP_STAY_TIME*5 ){
				
		if( FUN_OK == display_info_out(&display_msg,&confirm) ){
			
			menu_status.disp_time = 0;
			menu_status.rtc_time = 0;
			menu_status.init_flag = 1;
			
			display_text((char*)display_msg,1);
			
			//<如需按键确认显示信息
			if( confirm ){
				lcdxx_clear_row(LCDXX_ROW4);
				lcdxx_dispaly_data(LCDXX_ROW4,0,(uint8_t*)"CON|CAN KEY RTN", 15);
			}
		}
		else{

			if( menu_status.curr_menu != NULL && menu_status.menu_func==NULL ){//&& uxQueueMessagesWaiting(menu_status.disp_queue)==0 ){
					
				if( menu_status.init_flag ){
					menu_status.init_flag = 0;
					menu_status.max_item = display_menu_size(menu_status.curr_menu);
					menu_status.max_item_less = menu_status.max_item>DISP_MENU_ITEMS?DISP_MENU_ITEMS:menu_status.max_item;
					menu_status.curr_item = 0;
					menu_status.cursor = 1;

					curr_item_ptr = &menu_status.curr_menu[menu_status.curr_item];
					display_menu_item(curr_item_ptr, menu_status.cursor, menu_status.max_item,1);
					lcdxx_display_string(0, 1, ">",0);
				}
				
				if( menu_status.refresh_flag ){
					
					menu_status.refresh_flag = 0;
					lcdxx_clear_screen();
					curr_item_ptr = &menu_status.curr_menu[menu_status.curr_item];
					
					if( menu_status.curr_item == 0 ){
						display_menu_item(curr_item_ptr, menu_status.cursor, menu_status.max_item,0);
						lcdxx_display_string(0, 1, ">",0);
					}
					else if( menu_status.curr_item == menu_status.max_item-1 ){
						
						display_menu_item(curr_item_ptr, menu_status.cursor, menu_status.max_item,1);
						lcdxx_display_string(0, menu_status.cursor, ">",0);
					}
					else{
						display_menu_item(curr_item_ptr, menu_status.cursor, menu_status.max_item,0);
						lcdxx_display_string(0, menu_status.cursor, ">",0);
					}
				}
				
				
				BUTTON_MSG_T button_msg;
				if(FUN_OK == display_button_out(&button_msg)){
					
					switch( button_msg.value ){
						
						case UP_KEY_MSG:
						
							if( menu_status.cursor>1 )
								menu_status.cursor--;
						
							if( menu_status.curr_item>0 )
								menu_status.curr_item--;
							else{
								menu_status.curr_item = menu_status.max_item-1;
								menu_status.cursor = menu_status.max_item_less;
								///<小于屏幕行数菜单不刷新
								if( menu_status.max_item>DISP_MENU_ITEMS )
									menu_status.refresh_flag = 1;
							}
							
							curr_item_ptr = &menu_status.curr_menu[menu_status.curr_item];
							
							display_menu_curror(menu_status.cursor);

							if( (menu_status.curr_item <= menu_status.max_item-DISP_MENU_ITEMS) && (menu_status.cursor==1) ){

								lcdxx_display_string(1, 1, curr_item_ptr->item_str,1);
								lcdxx_display_string(1, 2, (curr_item_ptr+1)->item_str,1);
								lcdxx_display_string(1, 3, (curr_item_ptr+2)->item_str,1);
								lcdxx_display_string(1, 4, (curr_item_ptr+3)->item_str,1);
								
		//						display_menu_item(curr_item_ptr, DISP_MENU_ITEMS, 1);
							}
							
							break;
						
						case DOWN_KEY_MSG:
						
							if( menu_status.cursor<DISP_MENU_ITEMS )
								menu_status.cursor++;
						
							if( menu_status.curr_item<menu_status.max_item-1 )
								menu_status.curr_item++;
							else{
								menu_status.cursor = 1;
								menu_status.curr_item = 0;
								///<小于屏幕行数菜单不刷新
								if( menu_status.max_item>DISP_MENU_ITEMS )
									menu_status.refresh_flag = 1;
							}
							
							curr_item_ptr = &menu_status.curr_menu[menu_status.curr_item];
							DISPLAY_MENU_T	*item_ptr = curr_item_ptr;
							
							if( (menu_status.curr_item >= DISP_MENU_ITEMS) && (menu_status.cursor>=4) ){
								
								lcdxx_display_string(1, 1, (curr_item_ptr-3)->item_str,1);
								lcdxx_display_string(1, 2, (curr_item_ptr-2)->item_str,1);
								lcdxx_display_string(1, 3, (curr_item_ptr-1)->item_str,1);
								lcdxx_display_string(1, 4, curr_item_ptr->item_str,1);
								
		//						display_menu_item(curr_item_ptr, DISP_MENU_ITEMS, 0);
							}
							display_menu_curror(menu_status.cursor);
							break;
						
						case CANCEL_KEY_MSG:
							if( curr_item_ptr->parent_menu )
								display_switch_menu(curr_item_ptr->parent_menu);
							break;
						
						case ENTER_KEY_MSG:
							if( curr_item_ptr->func!=NULL ){
								menu_status.menu_func = curr_item_ptr->func;
								menu_status.state = MENU_FUNC;
								display_func_refresh();
							}
							else if( curr_item_ptr->child_menu )
								display_switch_menu(curr_item_ptr->child_menu);
							break;
						
						default:
							break;
					}
				}
			}
		}
	}
	else if( menu_status.disp_time<255 ){
		
		if( confirm ){
			BUTTON_MSG_T button_msg;
			if( (FUN_OK == display_button_out(&button_msg)) ){
				if( button_msg.value == CANCEL_KEY_MSG || button_msg.value == ENTER_KEY_MSG ){
					menu_status.disp_time = DISP_STAY_TIME*5+1;
				}
			}
		}
		else{
			menu_status.disp_time++;
		}
	}
}

/** 
* 显示队列中消息显示. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_menu_info(void)
{
	uint8_t	display_msg[DISP_BUF_SIZE];
	static uint8_t confirm;
	DISP_DEC_STR_T	disp_dec_str;
	
	if( menu_status.disp_time>DISP_STAY_TIME ){
				
		if( FUN_OK == display_info_out(&display_msg,&confirm) ){
			
			menu_status.disp_time = 0;
			menu_status.rtc_time = 0;
			menu_status.screen_refresh = 1;
			
			display_text((char*)&display_msg,1);
			
			//<如需按键确认显示信息
			if( confirm ){
				lcdxx_clear_row(LCDXX_ROW4);
				lcdxx_dispaly_data(LCDXX_ROW4,0,(uint8_t*)"CON|CAN KEY RTN", 15);
			}
		}
		else{
			
			if( menu_status.curr_menu != NULL )
				menu_status.state = MENU_SELECT;
			
			if( menu_status.rtc_time++>=10 ){
				
				menu_status.rtc_time = 0;
				if( 1 ){
				
					switch( menu_status.info_screen ){
						case SCREEN_START_UP:
						{
							char disp_buf[17];
							snprintf(disp_buf,17,"%02d:%02d:%02d %02d%02d%02d ",sys_status.rtc.Hours,sys_status.rtc.Minutes,sys_status.rtc.Seconds,\
							sys_status.rtc.Day, sys_status.rtc.Month, sys_status.rtc.Year);
							lcdxx_display_string(0,LCDXX_ROW1,disp_buf,1);
							
							if( menu_status.screen_refresh ){
								menu_status.screen_refresh = 0;
								lcdxx_display_string(0,LCDXX_ROW2,config.electoral_commission,1);
								
								sprintf(disp_buf,"EBB V %s",DEVICE_MODEL);
								lcdxx_display_string(0,LCDXX_ROW3,disp_buf,1);
								
								//sprintf(disp_buf,);
								lcdxx_display_string(0,LCDXX_ROW4,"SELF TEST OK",1);
							}
							
							break;
						}
						case SCREEN_PRE_SETTING:
						{
							char disp_buf[17];
							snprintf(disp_buf,17,"%02d:%02d:%02d %02d%02d%02d ",sys_status.rtc.Hours,sys_status.rtc.Minutes,sys_status.rtc.Seconds,\
								sys_status.rtc.Day, sys_status.rtc.Month, sys_status.rtc.Year);
							lcdxx_display_string(0,LCDXX_ROW1,disp_buf,1);
							
							if( menu_status.screen_refresh ){
								menu_status.screen_refresh = 0;
								lcdxx_display_string(0,LCDXX_ROW2,"Unprogrammed",1);
								
								lcdxx_clear_row(LCDXX_ROW3);
								lcdxx_clear_row(LCDXX_ROW4);
							}
							break;
						}
						case SCREEN_PRE_ELECTION:
						{
							char disp_buf[17];
							snprintf(disp_buf,17,"%02d:%02d:%02d BAT:%d",sys_status.rtc.Hours,sys_status.rtc.Minutes,sys_status.rtc.Seconds,pm_read_bat());
							lcdxx_display_string(0,LCDXX_ROW1,disp_buf,1);
							
							if( menu_status.screen_refresh ){
								menu_status.screen_refresh = 0;
								lcdxx_display_string(0,LCDXX_ROW2,config.electoral_commission,1);
								
								lcdxx_display_string(0,LCDXX_ROW3,"  PRE-ELECTION",1);
								lcdxx_clear_row(LCDXX_ROW4);
							}
							break;
						}
						case SCREEN_DEVICE_SETTING_UPLOAD:
						{
							char disp_buf[17];
							snprintf(disp_buf,17,"%02d:%02d:%02d BAT:%d",sys_status.rtc.Hours,sys_status.rtc.Minutes,sys_status.rtc.Seconds,pm_read_bat());
							lcdxx_display_string(0,LCDXX_ROW1,disp_buf,1);
							
							if( menu_status.screen_refresh ){
								menu_status.screen_refresh = 0;
								lcdxx_display_string(0,LCDXX_ROW2,"EVD DEVICE REG",1);
								
								lcdxx_clear_row(LCDXX_ROW3);
								
								lcdxx_display_string(0,LCDXX_ROW4,"Upload Settings",1);
							}
							break;
						}
						case SCREEN_DURING_ELECTION:
						{
							char disp_buf[17];
							snprintf(disp_buf,17,"%02d:%02d:%02d %02d%02d%02d ",sys_status.rtc.Hours,sys_status.rtc.Minutes,sys_status.rtc.Seconds,\
							sys_status.rtc.Day, sys_status.rtc.Month, sys_status.rtc.Year);
							lcdxx_display_string(0,LCDXX_ROW1,disp_buf,1);
							
							sprintf(disp_buf,"Bat: %d%%",pm_read_bat());
							lcdxx_display_string(0,LCDXX_ROW2,disp_buf,1);
							
							if( menu_status.screen_refresh ){
								menu_status.screen_refresh = 0;
							
								lcdxx_display_string(0,LCDXX_ROW3,"CAU: Active",1);
							
								lcdxx_display_string(0,LCDXX_ROW4,"Election Running",1);
							}
							break;
						}
						case SCREEN_POST_ELECTION:
						{
							char disp_buf[17];
							snprintf(disp_buf,17,"%02d:%02d:%02d %02d%02d%02d ",sys_status.rtc.Hours,sys_status.rtc.Minutes,sys_status.rtc.Seconds,\
							sys_status.rtc.Day, sys_status.rtc.Month, sys_status.rtc.Year);
							lcdxx_display_string(0,LCDXX_ROW1,disp_buf,1);
							
							if( menu_status.screen_refresh ){
								menu_status.screen_refresh = 0;
								lcdxx_clear_row(LCDXX_ROW2);
								
								lcdxx_display_string(0,LCDXX_ROW3,"ELECTION CLOSED",1);
								
								lcdxx_clear_row(LCDXX_ROW4);
							}
							break;
						}
					}
				}
			}
		}
	}
	else if( menu_status.disp_time<255 ){
		
		if( confirm ){
			BUTTON_MSG_T button_msg;
			if( (FUN_OK == display_button_out(&button_msg)) ){
				if( button_msg.value == CANCEL_KEY_MSG || button_msg.value == ENTER_KEY_MSG ){
					menu_status.disp_time = DISP_STAY_TIME*5+1;
				}
			}
		}
		else{
			menu_status.disp_time++;
		}
	}
}

/** 
* 菜单回调函数处理. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_menu_func(void)
{
	uint8_t	display_msg[DISP_BUF_SIZE];
	static uint8_t confirm;
	DISP_DEC_STR_T	disp_dec_str;
	
	if( menu_status.disp_time>DISP_MENU_TIME ){
		if( menu_status.menu_func!=NULL && menu_status.info_complete==1 ){
				
			BUTTON_MSG_T button_msg;
			if( FUN_OK != display_button_out(&button_msg) )
				button_msg.value = 0xff;
			menu_status.menu_func(button_msg.value);
		}
	}
	
	if( menu_status.disp_time>DISP_MENU_TIME ){
				
		if( FUN_OK == display_info_out(&display_msg,&confirm) ){
			
			menu_status.disp_time = 0;
			
			menu_status.info_complete = 0;
			
			display_text((char*)&display_msg,0);

			//<如需按键确认显示信息
			if( confirm ){
				lcdxx_clear_row(LCDXX_ROW4);
				lcdxx_dispaly_data(LCDXX_ROW4,0,(uint8_t*)"CON|CAN KEY RTN", 15);
			}
		}
		else{
			menu_status.info_complete = 1;
		}
	}
	else if( menu_status.disp_time<255 ){
		
		if( confirm ){
			BUTTON_MSG_T button_msg;
			if( (FUN_OK == display_button_out(&button_msg)) ){
				if( button_msg.value == CANCEL_KEY_MSG || button_msg.value == ENTER_KEY_MSG ){
					menu_status.disp_time = DISP_STAY_TIME+1;
					display_func_refresh();
				}
			}
		}
		else{
			menu_status.disp_time++;
		}
	}
}

/**
* 显示主线程. 
* 处理菜单操作，消息显示，菜单回调函数. 
* @param[in]   pvParameters:线程参数. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
void display_task( void *pvParameters )
{	
	wait_for_startup();
	
	TickType_t ticks = xTaskGetTickCount();
	uint8_t		refresh_time=100;
	while(1){
		
		//if( urg_info_display() ){
		if( 1 ){
			
			switch( menu_status.state ){
				case MENU_SELECT:
					display_menu_select();
					refresh_time = 10;
					break;
				case MENU_FUNC:
					display_menu_func();
					refresh_time=10;
					break;
				case MENU_INFO:
					display_menu_info();
					refresh_time = 50;
					break;
			}
		}
		vTaskDelay(refresh_time);
	}
}

/** 
* 显示初始化. 
* 液晶接口，液晶初始化，显示队列，按键队列初始化. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  0  成功 
* @retval  1   错误  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-16创建 
*/
FUN_STATUS_T display_init(void)
{	
	lcdxx_init();
//	HAL_Delay(1000);
	
	memset(&menu_status,0,sizeof(DISPLAY_MENU_STATUS_T));
	menu_status.disp_queue = xQueueCreate(DISP_QUEUE_MAX,sizeof(DISPLAY_MSG_T));
	if( menu_status.disp_queue ==0 ){
		
		display_log("can not create display queue!!!\r\n");
		return FUN_ERROR;
	}
	menu_status.button_queue = xQueueCreate(KEY_QUEUE_MAX,sizeof(BUTTON_MSG_T));
	if( menu_status.button_queue ==0 ){
		
		display_log("can not create key queue!!!\r\n");
		return FUN_ERROR;
	}
	menu_status.disp_time = 255;
	if( xTaskCreate( display_task, "DISPLAY", DISPLAY_STACK_SIZE, NULL, DISPLAY_TASK_PRIORITY, &display_handle ) != pdPASS ){
		return FUN_ERROR;
	}
	daemon_from_value = display_handle;
	
	display_exit_to_info();
	
//	static DISP_DEC_STR_T	disp_dec_str;
//	display_decode_string("hello world\n1234567890abcdefghijklmnopqrstuvwxyz12345678901234567890",&disp_dec_str);
	
	return FUN_OK;
}
