#ifndef __DEVFS_H__
#define __DEVFS_H__

#include "stdint.h"
#include "stdbool.h"
#include "comm.h"
#include "macro_def.h"

#define CANDIDATE_NAME_LEN	30
#define VOTE_TYPE_NAME_LEN	50

#pragma pack(push) //±£´æ¶ÔÆë×´Ì¬
#pragma pack(1)   // 1 bytes¶ÔÆë

#define DEVFS_DRIVER		"M0:\\"

typedef struct
{
	char		hardware_version[7];
	char		firmwate_version[8];
	char		device_type[6];
	char		unique_id[24];
	char		data_of_commission[12];
	char		name_of_country_system_is_used_in[50];
	char		name_of_electoral_commission[50];
	char		name_of_partner[50];
	char		data_time_of_last_settings_upload[24];
	char		data_of_last_election[12];
}DEVFS_DEVICE_INFORMATION_T;

//typedef struct
//{
//	uint8_t		evd_unipue_id[12];
//	uint8_t		evd_firmware_version[8];
//	uint8_t		evd_hardware_version[7];
//	uint8_t 	evd_device_type[6];
//	uint8_t		evd_vote_type[1];
//	uint8_t		evd_address[1];
//	uint16_t	tally[CANDIDATE_MAX];
//}DEVFS_SITE_CONFIGURATION_ITEM_T;

typedef struct
{
	uint8_t		evd_unipue_id[12];
	uint8_t		evd_firmware_version[8];
	uint8_t		evd_hardware_version[7];
	uint8_t 	evd_device_type[6];
	uint8_t		evd_vote_type[1];
	uint8_t		evd_address[1];
	uint16_t	novote;
	uint16_t	tally[CANDIDATE_MAX];
//	DEVFS_SITE_CONFIGURATION_ITEM_T site_confiruration_item[EVT_NUM];
}DEVFS_SITE_CONFIGURATION_T;

typedef struct
{
	char		name[CANDIDATE_NAME_LEN];
	char		tally[6];
}DEVFS_VOTE_RECORDS_ITEM_T;

typedef struct
{
	char 		vote_name[ELECTION_NAME_LEN];
	char		vote_date[8];
	DEVFS_VOTE_RECORDS_ITEM_T	vote_records_item[CANDIDATE_MAX];
	char		evd_id_that_were_part_of_the_vote[24];
}DEVFS_VOTE_RECORDS_T;

typedef struct
{
	uint16_t						candidate_tally[CANDIDATE_MAX];
}DEVFS_CANDIDATE_TALLIES_T;

typedef struct
{
	DEVFS_CANDIDATE_TALLIES_T		vote1_tally;
	DEVFS_CANDIDATE_TALLIES_T		vote2_tally;
	DEVFS_CANDIDATE_TALLIES_T		vote3_tally;
	DEVFS_CANDIDATE_TALLIES_T		vote4_tally;
}DEVFS_VOTE_TALLIES_T;

typedef struct
{
	char		name_vote_type[VOTE_TYPE_NAME_LEN];
	uint8_t		number_of_votes_type[1];
	uint8_t		number_of_candidate_type[1];
	char		name_of_candidates_type[CANDIDATE_MAX][CANDIDATE_NAME_LEN];
}DEVFS_CANDIDATE_TYPE_T;

typedef struct
{
	uint8_t		year;
	uint8_t		month;
	uint8_t		day;
	uint8_t		hour;
	uint8_t		minute;
	uint8_t		second;
}DEVFS_ECECTION_DATE_TIME_T;

typedef struct
{
	DEVFS_ECECTION_DATE_TIME_T		election_start_date_time;
	DEVFS_ECECTION_DATE_TIME_T		election_end_date_time;
	uint8_t							number_of_vote_types[1];
	
	DEVFS_CANDIDATE_TYPE_T			candidate_type1;
	DEVFS_CANDIDATE_TYPE_T			candidate_type2;
	DEVFS_CANDIDATE_TYPE_T			candidate_type3;
	DEVFS_CANDIDATE_TYPE_T			candidate_type4;

}DEVFS_VOTE_SETTINGS_T;

typedef struct
{
	uint8_t							firmware[64];
	uint8_t							memory[64];
}DEVFS_FILE_VERIFICATION_T;

#pragma pack(pop)//»Ö¸´¶ÔÆë×´Ì¬

extern FUN_STATUS_T devfs_site_config_new(COMM_EVT_DATA_T *comm_evt_data_ptr);
extern FUN_STATUS_T devfs_read_candidates_name(uint8_t type, uint8_t idx, char *name_ptr);
extern FUN_STATUS_T devfs_candidates_end(uint8_t type, uint8_t cnt);
extern FUN_STATUS_T devfs_read_vote_settings( uint8_t type, COMM_VOTE_SETTING_T *vote_settings_ptr );
extern FUN_STATUS_T devfs_settings_to_election( void );
extern FUN_STATUS_T devfs_init(void);
extern FUN_STATUS_T devfs_read_election_time( uint8_t type, DEVFS_ECECTION_DATE_TIME_T *date_time);
extern FUN_STATUS_T devfs_write_election_time( uint8_t ts_te, DEVFS_ECECTION_DATE_TIME_T *data_time_ptr );
extern FUN_STATUS_T devfs_read_type_numbers( uint8_t *numbers );
extern FUN_STATUS_T devfs_site_config_write_tally(char *file_name, uint8_t idx, uint16_t tally);
extern FUN_STATUS_T devfs_site_config_read_tally(char *file_name, uint8_t idx, uint16_t *tally);
extern FUN_STATUS_T devfs_vote_records_create(void);
extern FUN_STATUS_T devfs_vote_tallies_acc(uint8_t vote_type, uint8_t index, uint16_t tallies);
extern FUN_STATUS_T devfs_vote_tallies_write(uint8_t vote_type, uint8_t index, uint16_t tallies);
extern FUN_STATUS_T devfs_vote_tallies_read(uint8_t vote_type, uint8_t index, uint16_t *tallies_ptr);
extern FUN_STATUS_T devfs_vote_tallies_init(void);
extern FUN_STATUS_T devfs_save_vote_records(uint8_t type, uint8_t index, uint16_t total);
extern FUN_STATUS_T devfs_read_vote_records(uint8_t type, uint8_t index, uint16_t *total);
extern FUN_STATUS_T devfs_site_config_save_tally(char *file_name, COMM_VOTE_TALLY_T *comm_vote_tally_ptr);
extern FUN_STATUS_T devfs_site_config_restore_tally(char *file_name, COMM_VOTE_CANDIDATE_T *comm_vote_candidate_ptr);
extern FUN_STATUS_T devfs_vote_tallies_save_all(uint8_t vote_type, COMM_VOTE_TALLY_T *comm_vote_tally_ptr);
extern FUN_STATUS_T devfs_vote_tallies_read_all(uint8_t vote_type, uint16_t *tally_ptr);
#endif
