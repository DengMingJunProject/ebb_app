#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "stdint.h"
#include "stdbool.h"
#include "system.h"
#include "macro_def.h"

#define		DISP_QUEUE_MAX		10
#define		KEY_QUEUE_MAX		10
#define 	DISP_BUF_SIZE		68
#define		DISP_STAY_TIME		20
#define 	DISP_MENU_TIME		5
#define		DISP_MENU_ITEMS		4
#define 	DSIP_TEXT_LEN		64
#define 	DISP_LINE_LEN		16

enum{
	ENTER_KEY_MSG=0,
	CANCEL_KEY_MSG,
	DOWN_KEY_MSG,
	UP_KEY_MSG,
};

typedef enum{
	LOGO_UI,
	VOTE_UI,
	SETTING_UI,
}UI_T;

typedef struct {
	char		*item_str;
	void const  *parent_menu;
	void const  *child_menu;
	void		(*func)(uint8_t);
}DISPLAY_MENU_T;

typedef struct {
	const DISPLAY_MENU_T	*menu;
	uint8_t			num;
}DISPLAY_MENU_SIZE_T;

typedef struct {
	//char		disp_buf[DISP_BUF_SIZE];
	uint8_t		confirm;
	void*		buf;
}DISPLAY_MSG_T;

typedef struct {
	uint8_t 	value;
	uint8_t		event;
}BUTTON_MSG_T;

typedef enum {
	MENU_INFO,
	MENU_SELECT,
	MENU_FUNC,
}MENU_STATE_T;

typedef enum{
	SCREEN_INIT=0,
	SCREEN_START_UP,
	SCREEN_PRE_SETTING,
	SCREEN_PRE_ELECTION,
	SCREEN_DEVICE_SETTING_UPLOAD,
	SCREEN_DURING_ELECTION,
	SCREEN_POST_ELECTION,
}DISP_SCREEN_T;

typedef struct{
	char		str[4][16];
	uint8_t		len[4];
}DISP_DEC_STR_T;

typedef struct {
	
	DISPLAY_MENU_T	*curr_menu;
	
	void			(*menu_func)(uint8_t);
	
	uint8_t			curr_item;
	uint8_t			cursor;
	uint8_t			max_item;
	uint8_t			max_item_less;
	uint8_t			init_flag;
	uint8_t			refresh_flag;
	uint8_t			disp_time;
	uint8_t			rtc_time;
	
	uint8_t			process_step;
	uint8_t			process_row;
	
	uint8_t			req_admin_card2;
	uint8_t			admin_card2_present;
	
	uint8_t			info_complete;				///<信息显示完成
	
	uint8_t			func_fitst;
	
	MENU_STATE_T	state;
	
	DISP_SCREEN_T	info_screen;
	uint8_t			screen_refresh;
	
	QueueHandle_t	*disp_queue;
	QueueHandle_t	*button_queue;
	
}DISPLAY_MENU_STATUS_T;

#define is_req_admin_card2()	(menu_status.req_admin_card2==1)			///<请求第二个管理员卡
#define set_req_admin_card2()	(menu_status.req_admin_card2=1)				///<设置请求第二个管量员卡
#define clr_req_admin_card2()	(menu_status.req_admin_card2=0)				///<清空请求第二个管量员卡

#define set_admin_card2_present()	(menu_status.admin_card2_present=1)		///<设置第二张管理员卡插入
#define clr_admin_card2_present()	(menu_status.admin_card2_present=0)		///<删除第二张管理员卡插入
#define is_admin_card2_present()	(menu_status.admin_card2_present==1)	///<判断第二张管理员卡插入

#define is_menu_state(sta)			(menu_status.state==sta)

#define switch_screen(sce)			{menu_status.info_screen=sce;menu_status.screen_refresh=1;}

#define display_func_refresh()		(menu_status.func_fitst = 1)

#define is_menu_func(func)			(menu_status.menu_func==func)

extern const DISPLAY_MENU_T admin_card_menu[];
extern const DISPLAY_MENU_T maintenance_card_menu[];
extern const DISPLAY_MENU_T system_card_menu[];
extern DISPLAY_MENU_STATUS_T	menu_status;

extern void display_switch_menu(const DISPLAY_MENU_T *item);
extern FUN_STATUS_T display_init(void);
extern void display_info_in(char *info, uint8_t confirm);
extern FUN_STATUS_T display_screen_info_in( void(*func)(uint8_t), char *info );
extern void display_log_version(void);
extern void display_exit_to_menu(void);
extern void display_exit_to_info(void);
extern void display_button_in(BUTTON_MSG_T *button_msg);
extern FUN_STATUS_T display_button_out(BUTTON_MSG_T *button_msg);
extern void display_info_seg(uint8_t seg, char *info);
extern void display_card_pull_out(void);
extern void display_process_bar(uint8_t step);
extern void display_text(char *text,uint8_t clear_screen);
#endif
