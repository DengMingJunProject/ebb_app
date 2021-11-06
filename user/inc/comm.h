#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "stdint.h"
#include "stm32f4xx.h"
#include "macro_def.h"

#pragma pack(push) //保存对齐状态
#pragma pack(1)   // 1 bytes对齐

#define	COMM_RECV_QUEUE_NUM		10

#define COMM_SEND_QUEUE_NUM		10
#define COMM_SEND_RETAIN_NUM	4

#define EBB_RF_DEFAULT_ADDR		0x1a470501
#define EVT_RF_TARGET_ADDR		0x1a470500
#define EBB_RF_PC_ADDR			0x1a470502

#define EVT_REG_CHN_INIT	0
#define EVT_REG_CHN_PROC	5

#define	EVT_ADDR_MIN		0x14
#define EVT_ADDR_MAX		0xFF

#define CRYPT_COMMUNICATON

#define EVT_NUM				20

#define REG_CHN_NUM			3

#define CANDIDATE_LEN		102

#define POLL_TIME			100

#define VOTE_TALLY_TIME		180000

#define EVT_ADDR_NULL		0x0A

#define EVT_RPY_RFID_LEN	13

#define EVT_MSG_WAIT_TIME	100

#define REG_MAX				1

#define UID_LEN				12

#define INTERACTION_TIMEOUT 700

#define CANDIDATE_MAX		21

#define HASH_KEY_LEN		20

#define ELECTION_NAME_LEN	50

typedef struct{
	void		*buf;
	uint16_t 	len;
	uint32_t	target_addr;
	uint32_t	source_addr;
	uint8_t		chn;
	uint32_t	msg_id;
	uint8_t		wait_reply;
}COMM_SEND_MSG_T;

typedef struct {

	uint32_t 	source_address;
	uint32_t	target_address;
	uint32_t	message_id;
	char 		command[3];
}COMM_HEADER_T;

typedef struct {
	uint8_t		ebb_id[UID_LEN];
	uint8_t		vote_deactivation_key[32];
}COMM_END_VOTING_T;

typedef struct {
	uint8_t		ebb_id[UID_LEN];
}COMM_ACTIVE_SEND_T;

typedef struct {
	uint32_t	time_stamp;
	uint16_t	total_cnt;
	char		separator1;
	uint16_t	total_voters;
	char		separator2;
	uint16_t	no_voters;
	char		separator3;
}COMM_TOTAL_TALLY_T;

typedef struct {
	char		state_char;
	uint16_t	inc_cnt;
	uint16_t	total_cnt;
	char		separator;
}COMM_CAND_TALLY_T;

typedef struct {
	uint8_t		sha[20];
	char		end_char;
}COMM_SHA_T;

typedef enum{
	EVT2EBB=1,
	EBB2EVT,
}COMM_DIR_T;

typedef struct {
	uint16_t 	len;
	void		*buf;
}COMM_RECV_MSG_T;

enum{
	COMM_NONE=0,
	COMM_YES,
	COMM_NO,
};

enum{
	REPY_NONE=0,
	REPY_FIRST,
	REPY_SECOND,
	REPY_ERROR,
};

enum{
	REG_CHN_1=1,
	REG_CHN_2=2,
	REG_CHN_3=4,
};

typedef enum{
	EVT_STEP_BEGIN=0,
	EVT_STEP_REG,
	EVT_STEP_SETTING,
	EVT_STEP_POLL,
	EVT_STEP_CLOSE,
}COMM_STEP_T;

typedef enum{
	EVT_TRAN_INIT=0,
	EVT_TRAN_TIMEOUT,
	EVT_TRAN_OK,
}COMM_TRAN_T;

typedef struct{
	uint8_t		evt_id[UID_LEN];	///<EVT发过来的EVT ID返回给EVT
	uint8_t		evt_set_id;			///<EBB分配给EVT的新地址
	uint8_t		evt_reg_chn;		///<EBB分配给EVT的注册通道号
}COMM_RPY_RFID_T;

typedef struct {
	uint8_t		step;
	COMM_DIR_T	direction;			///<命令发送方向
	char		cmd_code[8];		///<命令代码
	void 		*message;			///<发送或接收的消息缓冲
	uint8_t		msg_len;			///<发送或接收的消息长度
	uint32_t(*func)(void *handle, uint32_t, uint8_t *);				///<命令回调函数，执行于发送前或执行于接收后
}COMM_MSG_PROCESS_T;

typedef struct {
	uint8_t		day;
	uint8_t		month;
	uint16_t	year;
	uint8_t		hour;
	uint8_t		minute;
	uint8_t		second;
}COMM_RTC_SETTING_T;

typedef struct {
	uint8_t		total_candidates;
	uint8_t		select_candidates;
	char 		name_election[ELECTION_NAME_LEN];
	uint8_t		start_day;
	uint8_t		start_month;
	uint16_t	start_year;
	uint8_t		start_hour;
	uint8_t		start_minute;
	uint8_t		start_second;
}COMM_VOTE_SETTING_T;

typedef struct{
	uint8_t			evt_id[UID_LEN];
	uint8_t			rf_id;
	uint8_t			evt_device_type[6];
	uint32_t		hardware_version;
	uint32_t		firmware_version;
	uint8_t			vote_type;
}COMM_EVT_DATA_T;

typedef struct{
	uint32_t		msg_id;
	uint16_t		msg_time;
	uint8_t			msg_reply;
}COMM_WAIT_MSG_STA_T;

typedef struct{
	uint32_t		poll_send;
	uint32_t		poll_recv;
	uint32_t		poll_timeout;
	uint32_t		poll_id_err;
	uint32_t		poll_max_time;
	COMM_TRAN_T		tran_state;
}COMM_STATUS_T;

typedef struct{
	char			sta;
	uint16_t		increased;
	uint16_t		total;
}COMM_VOTE_CANDIDATE_T;

typedef struct{
	uint16_t		total_cnt;
	uint16_t		total_voters;
	uint16_t		no_voters;
	
	COMM_VOTE_CANDIDATE_T	vote_candidate[CANDIDATE_MAX];
}COMM_VOTE_TALLY_T;

typedef union{
	uint8_t			byte;
	struct{
		uint8_t		tamper_detect:1;
		uint8_t		help_detect:1;
		uint8_t		state_ok:1;
		uint8_t		low_paper:1;
		uint8_t		low_power:1;
		uint8_t		reserved:3;
	}bit;
}COMM_EVT_FALG_T;

typedef struct{
	uint8_t			poll_sending;
	uint16_t		sending_time;
}COMM_POLL_STA_T;

typedef struct{
	
	const COMM_MSG_PROCESS_T		*msg_process_ptr;
	const COMM_MSG_PROCESS_T		*msg_process_begin;
	COMM_EVT_DATA_T			comm_evt_data;
	uint16_t				timeout;				///<通讯计时
	uint8_t					cnt;
	uint8_t					err;
	uint8_t					addr;					///<保存EVT地址
	COMM_STEP_T				step;					///<EVT通讯状态，0：等待注册，1：等待设置，2：正常轮询
	uint32_t				poll_time;
	void*					recv_data;
	COMM_WAIT_MSG_STA_T		comm_wait_msg_sta;
	COMM_STATUS_T			comm_status;
	COMM_POLL_STA_T			comm_poll_sta;
	uint8_t					send_chn;				///<发送到对方EVT的接收通道号
	uint8_t					reg_chn;				///<保存当前正在注册通道号，用于设置完成后清空通道标志位
	uint8_t					candidate_count;		///<传输候选人计数
	uint16_t				interaction_timeout;
	char					site_config_file_name[50];
	
	COMM_VOTE_TALLY_T		vote_tally;
	COMM_EVT_FALG_T			evt_flag;
	COMM_EVT_FALG_T			last_evt_flag;

	uint8_t					worked;
	
	uint32_t				time_stamp;

}COMM_EVT_STATE_T;

typedef struct{
	
	uint8_t					comm_mode;
	uint8_t					cnct_new_evt;
}COMM_MENU_SETTING_T;

typedef struct{
	void*					evt_state_handle;
	uint8_t					chn;
	uint8_t					time;
}COMM_REG_CHN_T;

typedef struct{
	uint8_t					t1;						///<T1类型数量
	uint8_t					t2;						///<T2类型数量
	uint8_t					t3;						///<T3类型数量
	uint8_t					t4;						///<T4类型数量
	uint8_t					reg;					///<注册数量
	uint8_t					total;					///<已设置总数
}COMM_EVT_NUM_T;

typedef struct{
	COMM_EVT_NUM_T			evt_num;				///<EVT数量
	uint32_t				message_id;				///<消息ID
	uint8_t					rf_id;
	COMM_RPY_RFID_T			comm_rpy_rfid;
	uint8_t					reging_chn;
	uint8_t					enable_poll;
	
	COMM_RTC_SETTING_T		rtc_setting;
	COMM_VOTE_SETTING_T		vote_setting;
	COMM_EVT_STATE_T		evt_state[EVT_NUM];
	COMM_REG_CHN_T			reg_chn[REG_CHN_NUM];
	COMM_MENU_SETTING_T		menu_setting;
	
//	COMM_EVT_STATE_T		*evt_token;
}COMM_STATE_T;

#pragma pack(pop)//恢复对齐状态

#define is_chn1_reging()	(READ_BIT(comm_state.reging_chn,REG_CHN_1))
#define is_chn2_reging()	(READ_BIT(comm_state.reging_chn,REG_CHN_2))
#define is_chn3_reging()	(READ_BIT(comm_state.reging_chn,REG_CHN_3))
#define set_chn_reging(B)	(SET_BIT(comm_state.reging_chn, B))
#define clr_chn_reging(B)	{CLEAR_BIT(comm_state.reging_chn, B);B=0;}

#define is_enable_cnct_new_ect()	(comm_state.menu_setting.cnct_new_evt==1)

#define is_interaction_err()			(comm_state.interaction_err==1)
#define clr_interaction_err()			(comm_state.interaction_err=0)
#define clr_interaction_cnt()			(comm_state.interaction_cnt=0)

extern const char	CMD_REQ_CLOSE_ELECTION[];
extern const char   CMD_REQ_CUR_VOTE_TALLY[];
extern const char	CMD_TRAN_CUR_VOTE_TALLY[];
extern const char	CMD_REQ_FINAL_VOTE_TALLY[];
extern const char	CMD_TRAN_FINAL_VOTE_TALLY[];
extern const char   CMD_REQ_ALL_VOTE_TALLY[];
extern const char   CMD_TRAN_ALL_VOTE_TALLY[];
extern const char	CMD_ASSISTANCE[];
extern const char	CMD_TAMPER[];
extern const char	CMD_COMFIRM[];
extern const char	CMD_PAPER_LOW[];
extern const char	CMD_BATTERY_LOW[];
extern const char 	CMD_CLR_SET_CHG_TYPE[];
extern const char 	CMD_CLR_SET_DEREG[];
extern const char 	CMD_UPDATE_SEG[];
extern const char 	CMD_UPDATE_RPY[];
extern const char   CMD_STATUS_OK_RPY[];
extern const char   CMD_REQ_EVENT_LOG[];
extern const char   CMD_TRN_EVENT_LOG[];
extern const char	CMD_REQ_STATUS[];

extern const COMM_MSG_PROCESS_T register_msg_code[];
extern const COMM_MSG_PROCESS_T setting_msg_code[];
extern uint16_t setting_msg_code_num;
extern uint16_t register_msg_code_num;
extern COMM_STATE_T	comm_state;

extern FUN_STATUS_T comm_init(void);
extern void comm_recv_complete(void);
extern void comm_recv_in_queue(void *data, uint16_t len);
extern void* comm_recv_out_queue(uint16_t *len);
extern void comm_active_send(const char *cmd);
extern void comm_start_cmd_sequences(const COMM_MSG_PROCESS_T *msg);
extern uint32_t comm_convert_addr(uint8_t *ptr);
extern void comm_upload_event_log(void);
extern FUN_STATUS_T comm_is_evt_reging(uint8_t addr);
extern FUN_STATUS_T comm_evt_is_registration_of_uid(uint8_t *uid);
extern FUN_STATUS_T comm_end_election(void);
extern FUN_STATUS_T comm_pack(uint8_t target_addr, uint8_t source_addr, uint8_t pipe, char *cmd, uint8_t *data, uint16_t len);
#endif
