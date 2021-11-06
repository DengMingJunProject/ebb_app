#ifndef __ELECTION_H__
#define __ELECTION_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

typedef enum
{
  EVT_WAIT_SETUP = 0,
  EVT_WAIT_VOTE  = 1,
  EVT_ALLOW_VOTE = 2,
  EVT_VOTING     = 3,
  EVT_FINISH     = 4,
}EVT_STEP_T;

typedef enum
{
	VOTE_NOT_START  = 0,
	VOTE_IN_PROCESS,
	VOTE_FINISHED,
}VOTE_STEP_T;

typedef enum
{
	EBB_STUS_INIT=0,
	EBB_STUS_START_UP,
	EBB_STUS_PRE_SETTING,
	EBB_STUS_PRE_ELECTION,
	EBB_STUS_DEVICE_SETTING_UPLOAD,
	EBB_STUS_DURING_ELECTION,
	EBB_STUS_POST_ELECTION,
	EBB_STUS_MAX,
}ELECTION_EBB_STUS_T;

typedef enum{
	ECECTION_NOTSTARTED=0,
	ECECTION_DURING,
	ECECTION_CLOSED,
}ELECTION_STATE_T;

typedef struct {
	union{
		uint8_t			byte;
		struct{
			uint8_t		start:1;
			uint8_t		configured:1;
			uint8_t		closed:1;
			uint8_t		init:1;
			uint8_t		settings:1;
			uint8_t		reserve:3;
		}bit;
	};
}ELECTION_FLAG_T;

typedef struct{
	uint8_t		number_of_vote_types;
	uint8_t		number_of_votes_type[4];
	uint8_t		number_of_candidate_type[4];
}ELECTION_CONFIG_T;

typedef struct {
	
	ELECTION_FLAG_T		flag;
	
	uint8_t				card_id[7];
	
	uint8_t				candidates_num;
//	uint8_t				av_num;
//	
//	uint8_t				io_borad_num;
//	
//	uint8_t				blink_num;
//	uint8_t				blink_step;
//	
//	char				header[20];
//	char				type_name[50];
//		
//	EVT_STEP_T			rfEvtStep;
//	VOTE_STEP_T  		voteStep;
	
	ELECTION_CONFIG_T	election_config;
	ELECTION_EBB_STUS_T	ebb_stus;
	uint8_t				stus_first;				///<状态切换第一次进入
}ELECTION_STATUS_T;

#define switch_ebb_stus(stus)		{election_status.ebb_stus=stus;election_status.stus_first=1;}
#define get_ebb_stus()				(election_status.ebb_stus)

#define switch_election(stus)		{switch_ebb_stus(stus);switch_screen(stus);}

#define wait_for_startup()			while( get_ebb_stus() < EBB_STUS_START_UP ){ vTaskDelay(1000); }

#define election_votes_num(type)		(election_status.election_config.number_of_votes_type[type-1])
#define election_candidate_num(type)	(election_status.election_config.number_of_candidate_type[type-1])
#define election_vote_types()			(election_status.election_config.number_of_vote_types)

extern ELECTION_STATUS_T	election_status;
extern FUN_STATUS_T election_init(void);
extern void election_start(void);
extern void election_stop(void);
extern void election_setting(void);
#endif
