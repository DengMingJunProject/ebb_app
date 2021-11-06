#ifndef __MENU_FUNC_H__
#define __MENU_FUNC_H__

#include "stdint.h"

#define PROCESS_BAR		16

typedef struct MENU_FUNC_VAR_T{
	
	uint8_t	process_bar;
	uint8_t	items;
	uint8_t	downloading;
}MENU_FUNC_VAR_T;

typedef struct MENU_SELECT_ITEM_T{
	uint8_t		num;
	char*		itme_str;
}MENU_SELECT_ITEM_T;

extern MENU_FUNC_VAR_T menu_func_var;

#define menu_func_bar_inc()			(menu_func_var.process_bar++)
#define menu_func_bar_clr()			(menu_func_var.process_bar=0)

extern void menu_func_exit(uint8_t);
extern void menu_view_logs(uint8_t key);
extern void menu_activate_vote_card(uint8_t key);
extern void menu_func_sel_comm_mode(uint8_t key);
extern void menu_func_cnct_new_evt(uint8_t key);
extern void menu_func_sys_end_election(uint8_t key);
extern void menu_func_admin_end_election(uint8_t key);
extern void menu_func_retrieve_tally(uint8_t key);
extern void menu_func_sys_reset_device(uint8_t key);
extern void menu_func_admin_reset_device(uint8_t key);
extern void menu_func_view_crt_tally(uint8_t key);
extern void menu_func_view_log(uint8_t key);
extern void menu_func_upl_settings(uint8_t key);
extern void menu_func_deact_tmp_alarm( uint8_t key );
extern void menu_func_deact_crt_alarm(uint8_t key);
extern void menu_func_view_crt_alarm(uint8_t key);
#endif

