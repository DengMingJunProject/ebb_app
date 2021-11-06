#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stdint.h"
#include "stdbool.h"
#include "comm.h"
#include "macro_def.h"
#include "election.h"

#pragma pack(push) //±£´æ¶ÔÆë×´Ì¬
#pragma pack(1)   // 1 bytes¶ÔÆë

#define	EBB_DEVICE_TYPE  	"EBBBV1"
#define COMMISSION			"IEC"

#define CONFIG_DRIVER		"N0:\\"

//typedef struct
//{
//	uint16_t	tally[CANDIDATE_MAX];
//}VOTE_TALLY_T;

typedef enum{
	
	BACKUP_DEVICE,
	VOTE_TYPE1=1,
	VOTE_TYPE2,
	VOTE_TYPE3,
	VOTE_TYPE4,
	VOTE_TYPE_MAX=4,
	VOTE_TYPE_ALL=8,
}VOTE_TYPE_T;

enum{
	PROCESS_STRT=0,
	PROCESS_INIT,
	PROCESS_SETTING,
	PROCESS_COMP,
};

typedef struct{
	COMM_EVT_DATA_T		comm_evt_data;
	uint8_t				step;
	uint8_t				addr;
	uint8_t				send_chn;
	uint8_t				reg_ch;
	char				site_config_file_name[50];
	const COMM_MSG_PROCESS_T		*msg_process_begin;
}EVT_REG_T;

typedef struct{
	COMM_EVT_NUM_T		evt_num;
	uint8_t				rf_id;
	EVT_REG_T			evt_reg[EVT_NUM];
}EVT_SAVE_T;

typedef struct{
	uint8_t				upload_settings;
	uint8_t				deact_tmp_alarm;
}MENU_CONFIG_T;

typedef struct{
	
	uint8_t				rf_module_power;
	uint8_t				tamper_switch;
	uint8_t				ebb_id[12];
	char				ebb_device_type[6];
	char				electoral_commission[50];
	uint8_t				ebb_addr;
	uint8_t				evt_step[EVT_NUM];
	char				date_commission[3];
	char				country_name[50];
	uint8_t				programmed;
	char				partner_name[50];
	uint8_t				last_upload_date_time[6];
	uint8_t				last_election_date[3];
	ELECTION_STATE_T	election_state;
	EVT_SAVE_T			evt_save;
	uint8_t				election_time_up;
	MENU_CONFIG_T		menu_config;
	uint8_t				crc;
}CONFIG_T;

#define rf_module_on()			{config.rf_module_power=1;TM_NRF24L01_PowerUpRx();}
#define rf_module_off()			{config.rf_module_power=2;TM_NRF24L01_PowerDown();}
#define is_rf_on()				(config.rf_module_power==1)
#define is_rf_off()				(config.rf_module_power==2)
#define get_election_state()	(config.election_state)

#define is_tamper_en()			(config.menu_config.deact_tmp_alarm==0)
#define tamper_enable()			(config.menu_config.deact_tmp_alarm=0)
#define tamper_disable()		(config.menu_config.deact_tmp_alarm=1)

#pragma pack(pop)//»Ö¸´¶ÔÆë×´Ì¬

extern CONFIG_T	config;
extern const uint32_t 	SOFTWARE_VERSION;
extern const uint32_t 	HARDWARE_VERSION;
//extern const char 		EVD_DEVICE_TYPE[6];
extern const char		DEVICE_MODEL[];

extern FUN_STATUS_T config_init(void);
extern void config_timming_save(void);
extern void config_evt_update(COMM_EVT_STATE_T *comm_evt_state_ptr);
extern void config_evt_restore(void);

#endif
