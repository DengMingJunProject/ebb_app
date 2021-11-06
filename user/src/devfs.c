#include "devfs.h"
#include "lefs.h"
#include "system.h"
#include "main.h"
#include "config.h"
#include "fs.h"
#include "stdlib.h"
#include "rl_fs.h"
#include "election.h"

#ifdef DEVFS_DEBUG
    #define	devfs_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DEVFS](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define devfs_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DEVFS] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define devfs_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[DEVFS] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define devfs_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define devfs_log(...)
    #define devfs_usr(...)
    #define devfs_err(...)
    #define devfs_dump(...)
#endif

const DEVFS_VOTE_SETTINGS_T vote_settings_default={
	.election_start_date_time={17,9,19,16,45,0},
	.election_end_date_time={17,9,19,17,0,0},
	.number_of_vote_types = {2},
	
	.candidate_type1.name_vote_type = "Biggest Drama Queen 1",
	.candidate_type1.number_of_votes_type = 3,
	.candidate_type1.number_of_candidate_type = 21,
	.candidate_type1.name_of_candidates_type[0] = "A01\\Anton Oosthuizen",
	.candidate_type1.name_of_candidates_type[1] = "A02\\Gawain Badcock",
	.candidate_type1.name_of_candidates_type[2] = "A03\\Justin Uren",
	.candidate_type1.name_of_candidates_type[3] = "A04\\Johan Fourie",
	.candidate_type1.name_of_candidates_type[4] = "A05\\Johan Van Vuuren",
	.candidate_type1.name_of_candidates_type[5] = "A06\\Warren Dingwall",
	.candidate_type1.name_of_candidates_type[6] = "A07\\Warwick Reeves",
	.candidate_type1.name_of_candidates_type[7] = "A08\\Hennie Jnr Meeding",
	.candidate_type1.name_of_candidates_type[8] = "A09\\Hennie Snr Meeding",
	.candidate_type1.name_of_candidates_type[9] = "A10\\Rocky Hattingh",
	.candidate_type1.name_of_candidates_type[10] = "A11\\Brenda Hattingh",
	.candidate_type1.name_of_candidates_type[11] = "A12\\Mathew Wolmarans",
	.candidate_type1.name_of_candidates_type[12] = "A13\\Heinrich Freislich",
	.candidate_type1.name_of_candidates_type[13] = "A14\\Jed Conolly",
	.candidate_type1.name_of_candidates_type[14] = "A15\\Ian McCutcheon",
	.candidate_type1.name_of_candidates_type[15] = "A16\\Paulo Lara",
	.candidate_type1.name_of_candidates_type[16] = "A17\\Christian Storm",
	.candidate_type1.name_of_candidates_type[17] = "A18\\David Hall",
	.candidate_type1.name_of_candidates_type[18] = "A19\\Donovan Hall",
	.candidate_type1.name_of_candidates_type[19] = "A20\\Seome Thutse",
	.candidate_type1.name_of_candidates_type[20] = "A21\\Ryan Otto",
	
	.candidate_type2.name_vote_type = "Biggest Drama Queen 2",
	.candidate_type2.number_of_votes_type = 3,
	.candidate_type2.number_of_candidate_type = 21,
	.candidate_type2.name_of_candidates_type[0] = "B01\\Anton Oosthuizen",
	.candidate_type2.name_of_candidates_type[1] = "B02\\Gawain Badcock",
	.candidate_type2.name_of_candidates_type[2] = "B03\\Justin Uren",
	.candidate_type2.name_of_candidates_type[3] = "B04\\Johan Fourie",
	.candidate_type2.name_of_candidates_type[4] = "B05\\Johan Van Vuuren",
	.candidate_type2.name_of_candidates_type[5] = "B06\\Warren Dingwall",
	.candidate_type2.name_of_candidates_type[6] = "B07\\Warwick Reeves",
	.candidate_type2.name_of_candidates_type[7] = "B08\\Hennie Jnr Meeding",
	.candidate_type2.name_of_candidates_type[8] = "B09\\Hennie Snr Meeding",
	.candidate_type2.name_of_candidates_type[9] = "B10\\Rocky Hattingh",
	.candidate_type2.name_of_candidates_type[10] = "B11\\Brenda Hattingh",
	.candidate_type2.name_of_candidates_type[11] = "B12\\Mathew Wolmarans",
	.candidate_type2.name_of_candidates_type[12] = "B13\\Heinrich Freislich",
	.candidate_type2.name_of_candidates_type[13] = "B14\\Jed Conolly",
	.candidate_type2.name_of_candidates_type[14] = "B15\\Ian McCutcheon",
	.candidate_type2.name_of_candidates_type[15] = "B16\\Paulo Lara",
	.candidate_type2.name_of_candidates_type[16] = "B17\\Christian Storm",
	.candidate_type2.name_of_candidates_type[17] = "B18\\David Hall",
	.candidate_type2.name_of_candidates_type[18] = "B19\\Donovan Hall",
	.candidate_type2.name_of_candidates_type[19] = "B20\\Seome Thutse",
	.candidate_type2.name_of_candidates_type[20] = "B21\\Ryan Otto",
};

/** 
* 生成默认的EBB信息文件. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
static void devfs_device_information_init(void)
{
	FILE *file;
	
	uint32_t size = sizeof(DEVFS_DEVICE_INFORMATION_T)/16;
	if( sizeof(DEVFS_DEVICE_INFORMATION_T)%16 )
		size += 1;
	
	size *= 16;
	
	file = fopen (CONECT_STR(DEVFS_DRIVER,"DeviceInformation"),"rb+");
	if( file == NULL ){
		file = fopen (CONECT_STR(DEVFS_DRIVER,"DeviceInformation"),"wb+");
	}
	if( file != NULL )
		fclose(file);
	
	if( fs_fsize(CONECT_STR(DEVFS_DRIVER,"DeviceInformation")) != size ){
		
		lefs_new(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), size );
		char buf[50];
		
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,hardware_version),\
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->hardware_version), (uint8_t*)"1.00.00");
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,firmwate_version),\
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->firmwate_version), (uint8_t*)"1.00.00");
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,device_type),\
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->device_type), (uint8_t*)"EBBV1");
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,unique_id), \
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->unique_id), (uint8_t*)sys_status.sys_id.uuid_str);
		
		sprintf(buf,"%02d%02d%02d",config.date_commission[2],config.date_commission[1],config.date_commission[0]);
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,data_of_commission),\
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->data_of_commission), (uint8_t*)buf);
		
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,name_of_country_system_is_used_in), \
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->name_of_country_system_is_used_in), (uint8_t*)config.country_name);
		
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,name_of_electoral_commission),\
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->name_of_electoral_commission), (uint8_t*)config.electoral_commission);
		
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,name_of_partner),\
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->name_of_partner), (uint8_t*)config.partner_name);
		
		sprintf(buf,"%02d%02d%02d%02d%02d%02d",config.last_upload_date_time[2],config.last_upload_date_time[1],config.last_upload_date_time[0],\
			config.last_upload_date_time[3],config.last_upload_date_time[4],config.last_upload_date_time[5]);
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,data_time_of_last_settings_upload),\
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->data_time_of_last_settings_upload), (uint8_t*)buf);
			
		sprintf(buf,"%02d%02d%02d",config.last_election_date[2],config.last_election_date[1],config.last_election_date[0]);
		lefs_write(CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), offsetof(DEVFS_DEVICE_INFORMATION_T,data_of_last_election),\
			sizeof(((DEVFS_DEVICE_INFORMATION_T*)0)->data_of_last_election), (uint8_t*)buf);
	}
}

///** 
//* 生成默认的EVD信息文件. 
//* 无. 
//* @param[in]   无. 
//* @param[out]  无.  
//* @retval  无  
//* @par 标识符 
//*      保留 
//* @par 其它 
//*      无 
//* @par 修改日志 
//*      ken deng于2019-09-23创建 
//*/
//static void devfs_site_configuration_init(void)
//{
//	FILE *file;
//	
//	uint32_t size = sizeof(DEVFS_SITE_CONFIGURATION_T)/16;
//	if( sizeof(DEVFS_SITE_CONFIGURATION_T)%16 )
//		size += 1;
//	
//	size *= 16;
//	
//	file = fopen ("M0:\\SiteConfiruration","rb+");
//	if( file == NULL ){
//		file = fopen ("M0:\\SiteConfiruration","wb+");
//	}
//	fclose(file);
//	
//	if( fs_fsize("M0:\\SiteConfiruration") != size ){
//		
//		lefs_new("M0:\\SiteConfiruration", size );
//	}
//}

FUN_STATUS_T devfs_records_file_init(char *file_name, uint8_t type )
{
	COMM_VOTE_SETTING_T vote_setting;
	if( FUN_OK != devfs_read_vote_settings( type, &vote_setting ) )
		return FUN_ERROR;
	
	char name_buf[VOTE_TYPE_NAME_LEN];
	for( uint8_t i=0; i<vote_setting.total_candidates; i++ ){
		
		if( FUN_OK == devfs_read_candidates_name(type,i,name_buf)){
			
		}
	}
}

/** 
* 投票结束后，所有投票人相应投票类型投票数据汇总. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  FUN_OK:成功建立所有类型的记录文件  
* @retval  FUN_ERROR:建立所有类型的记录文件失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-10-11创建 
*/
FUN_STATUS_T devfs_vote_records_create(void)
{
	FILE *file;
	
	uint8_t types;
	char file_name[100];
	uint8_t curr_vote_type;
	COMM_VOTE_SETTING_T vote_settings;
	
	devfs_read_type_numbers(&types);
	
	///<处理设置的投票类型
	for( curr_vote_type=1; curr_vote_type<=types; curr_vote_type++ ){
		
		devfs_read_vote_settings( curr_vote_type, &vote_settings );
		snprintf(file_name,sizeof(file_name),"%s%4d%02d%02d_Vote%dRecords",DEVFS_DRIVER,\
			vote_settings.start_year,vote_settings.start_month,vote_settings.start_day,curr_vote_type);
		
			
		if( FUN_OK != lefs_write(file_name, offsetof(DEVFS_VOTE_RECORDS_T,vote_name),\
			sizeof(((DEVFS_VOTE_RECORDS_T*)0)->vote_name), (uint8_t*)vote_settings.name_election))
				return FUN_ERROR;
		
		char date_buf[10];
		snprintf(date_buf,sizeof(date_buf),"%4d%02d%02d",vote_settings.start_year,vote_settings.start_month,vote_settings.start_day);
		if( FUN_OK != lefs_write(file_name, offsetof(DEVFS_VOTE_RECORDS_T,vote_date),\
			sizeof(((DEVFS_VOTE_RECORDS_T*)0)->vote_date), (uint8_t*)date_buf))
				return FUN_ERROR;
		
		char name_buf[CANDIDATE_NAME_LEN];
		char tally_buf[10];
		uint16_t total_tally,evt_tally;
		for( uint8_t i=0; i<CANDIDATE_MAX; i++ ){
			
			///<读取设置文件中的候选人名并写入记录文件中
			devfs_read_candidates_name(curr_vote_type, i, name_buf);
			if( FUN_OK != lefs_write(file_name, offsetof(DEVFS_VOTE_RECORDS_T,vote_records_item)+i*sizeof(DEVFS_VOTE_RECORDS_ITEM_T)+\
				offsetof(DEVFS_VOTE_RECORDS_ITEM_T,name),sizeof(((DEVFS_VOTE_RECORDS_ITEM_T*)0)->name), (uint8_t*)name_buf))
					return FUN_ERROR;
			
			///<读出所有EVT设备当前投票类型的投票数据
			total_tally = 0;
			COMM_EVT_STATE_T *comm_evt_state_ptr;
			comm_evt_state_ptr = comm_state.evt_state;

			for( uint8_t j=0; j<EVT_NUM; j++ ){
				
				if( comm_evt_state_ptr->comm_evt_data.vote_type == curr_vote_type ){
					evt_tally = 0;
					devfs_site_config_read_tally(comm_evt_state_ptr->site_config_file_name,i,&evt_tally);
					
					total_tally += evt_tally;
				}
				
				comm_evt_state_ptr++;
				
				vTaskDelay( 1 );
			}
//			devfs_log("%d\r\n",total_tally);
			snprintf(tally_buf,sizeof(tally_buf),"%06d",total_tally);
			if( FUN_OK != lefs_write(file_name, offsetof(DEVFS_VOTE_RECORDS_T,vote_records_item)+i*sizeof(DEVFS_VOTE_RECORDS_ITEM_T)+\
				offsetof(DEVFS_VOTE_RECORDS_ITEM_T,tally),sizeof(((DEVFS_VOTE_RECORDS_ITEM_T*)0)->tally), (uint8_t*)tally_buf))
					return FUN_ERROR;
		}
		vTaskDelay( 100 );
	}
	return FUN_OK;
}

FUN_STATUS_T devfs_save_vote_records(uint8_t type, uint8_t index, uint16_t total)
{
	FILE *file;
	char file_name[20];
	char total_buf[7];
	snprintf(file_name,20,"%sVoteRecords-Vote%d",DEVFS_DRIVER,type);
	
	snprintf(total_buf,7,"%06d",total);
	
	file = fopen(file_name,"rb+");
	if( file != NULL ){
		return lefs_write(file_name, offsetof(DEVFS_VOTE_RECORDS_T,vote_records_item)+index*sizeof(DEVFS_VOTE_RECORDS_ITEM_T),\
					sizeof(((DEVFS_VOTE_RECORDS_ITEM_T*)0)->tally), (uint8_t*)total_buf);
	}
	else{
		return FUN_ERROR;
	}
}

FUN_STATUS_T devfs_read_vote_records(uint8_t type, uint8_t index, uint16_t *total)
{
	FILE *file;
	char file_name[20];
	char total_buf[7];
	snprintf(file_name,20,"%sVoteRecords-Vote%d",DEVFS_DRIVER,type);
	
	file = fopen(file_name,"rb");
	
	if( file != NULL ){
		if( lefs_read(file_name, offsetof(DEVFS_VOTE_RECORDS_T,vote_records_item)+index*sizeof(DEVFS_VOTE_RECORDS_ITEM_T),\
					sizeof(((DEVFS_VOTE_RECORDS_ITEM_T*)0)->tally), (uint8_t*)total_buf) == FUN_OK){
			*total = atoi(total_buf);
			return FUN_OK;
		}
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 生成默认的投票设置文件. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-23创建 
*/
static void devfs_vote_settings_init(void)
{
	FILE *file;
	
	uint32_t size = sizeof(DEVFS_VOTE_SETTINGS_T)/16;
	if( sizeof(DEVFS_VOTE_SETTINGS_T)%16 )
		size += 1;
	
	size *= 16;
	
//	fdelete("M0:\\VoteSettings",NULL);
	file = fopen (CONECT_STR(DEVFS_DRIVER,"VoteSettings"),"rb+");
	if( file == NULL ){
		file = fopen (CONECT_STR(DEVFS_DRIVER,"VoteSettings"),"wb+");
	}
	if( file != NULL )
		fclose(file);
	
	if( fs_fsize(CONECT_STR(DEVFS_DRIVER,"VoteSettings")) != size ){
		lefs_new( CONECT_STR(DEVFS_DRIVER,"VoteSettings"), size );
		
		lefs_write(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), 0,sizeof(DEVFS_VOTE_SETTINGS_T), &vote_settings_default);
	}
}

/** 
* 生成默认的投票记数文件. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-23创建 
*/
FUN_STATUS_T devfs_vote_tallies_init(void)
{
	if( fdelete (CONECT_STR(DEVFS_DRIVER,"VoteTallies"), NULL) != fsOK ){
		return FUN_ERROR;
	}
	
	DEVFS_VOTE_TALLIES_T vote_tally;
	memset(&vote_tally,0,sizeof(DEVFS_VOTE_TALLIES_T));
	return lefs_write(CONECT_STR(DEVFS_DRIVER,"VoteTallies"), 0, sizeof(DEVFS_VOTE_TALLIES_T), (uint8_t*)&vote_tally);
}

/** 
* 生成默认的验证文件. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-23创建 
*/
static void devfs_file_verification_init(void)
{
	FILE *file;
	
	uint32_t size = sizeof(DEVFS_FILE_VERIFICATION_T)/16;
	if( sizeof(DEVFS_FILE_VERIFICATION_T)%16 )
		size += 1;
	
	size *= 16;
	
	file = fopen (CONECT_STR(DEVFS_DRIVER,"FileVerification"),"rb+");
	if( file == NULL ){
		file = fopen (CONECT_STR(DEVFS_DRIVER,"FileVerification"),"wb+");
	}
	if( file != NULL )
		fclose(file);
	
	if( fs_fsize(CONECT_STR(DEVFS_DRIVER,"FileVerification")) != size ){
		lefs_new( CONECT_STR(DEVFS_DRIVER,"FileVerification"), size );
	}
}

FUN_STATUS_T devfs_site_config_write_tally(char *file_name, uint8_t idx, uint16_t tally)
{
	if( FUN_OK != lefs_write(file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,tally)+idx*sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->tally)/CANDIDATE_MAX,\
		sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->tally)/CANDIDATE_MAX, (uint8_t*)&tally))
		return FUN_ERROR;
	
	return FUN_OK;
}

FUN_STATUS_T devfs_site_config_read_tally(char *file_name, uint8_t idx, uint16_t *tally)
{
	if( FUN_OK != lefs_read(file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,tally)+idx*sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->tally)/CANDIDATE_MAX,\
		sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->tally)/CANDIDATE_MAX, (uint8_t*)tally))
		return FUN_ERROR;
	
	return FUN_OK;
}

FUN_STATUS_T devfs_site_config_save_tally(char *file_name, COMM_VOTE_TALLY_T *comm_vote_tally_ptr)
{
	uint16_t	tally[CANDIDATE_MAX];
	COMM_VOTE_CANDIDATE_T *comm_vote_candidate_ptr = comm_vote_tally_ptr->vote_candidate;
	
	for( uint8_t i=0; i<CANDIDATE_MAX; i++ ){
		tally[i] = comm_vote_candidate_ptr->total;
		comm_vote_candidate_ptr++;
	}
	
	if( FUN_OK != lefs_write(file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,novote),\
		sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->novote), (uint8_t*)&comm_vote_tally_ptr->no_voters)){
		return FUN_ERROR;
	}
	
	if( FUN_OK != lefs_write(file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,tally),\
		sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->tally), (uint8_t*)&tally)){
		return FUN_ERROR;
	}
	
//	memset(&tally,0,CANDIDATE_MAX*2);
//	lefs_read(file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,tally),\
//		sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->tally), (uint8_t*)&tally);
		
//	uint16_t evt_tally;
//	for( uint8_t i=0; i<CANDIDATE_MAX; i++ ){
//		devfs_site_config_read_tally(file_name,i,&evt_tally);
//		devfs_log("%d\r\n",evt_tally);
//	}
		
	return FUN_OK;
}

FUN_STATUS_T devfs_site_config_restore_tally(char *file_name, COMM_VOTE_CANDIDATE_T *comm_vote_candidate_ptr)
{
	uint16_t	tally[CANDIDATE_MAX];
	
	if( FUN_OK != lefs_write(file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,tally),\
		sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->tally), (uint8_t*)&tally))
		return FUN_ERROR;
	
	for( uint8_t i=0; i<CANDIDATE_MAX; i++ ){
		comm_vote_candidate_ptr->total = tally[i];
		comm_vote_candidate_ptr++;
	}
	return FUN_OK;
}

/** 
* 保存EVD设备信息到文件. 
* 无. 
* @param[in]   comm_evt_data_ptr:EVD设备数据句柄. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
FUN_STATUS_T devfs_site_config_new(COMM_EVT_DATA_T *comm_evt_data_ptr)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = comm_state.evt_state;
	uint8_t i;
	for( i=0; i<EVT_NUM; i++ ){
		if( memcmp(comm_evt_data_ptr->evt_id,comm_evt_state_ptr->comm_evt_data.evt_id,UID_LEN)==0 ){
			snprintf(comm_evt_state_ptr->site_config_file_name,sizeof(((COMM_EVT_STATE_T*)0)->site_config_file_name),\
				"%sSiteConfig_%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",DEVFS_DRIVER,\
				comm_evt_data_ptr->evt_id[11],comm_evt_data_ptr->evt_id[10],comm_evt_data_ptr->evt_id[9],comm_evt_data_ptr->evt_id[8],\
				comm_evt_data_ptr->evt_id[7],comm_evt_data_ptr->evt_id[6],comm_evt_data_ptr->evt_id[5],comm_evt_data_ptr->evt_id[4],\
				comm_evt_data_ptr->evt_id[3],comm_evt_data_ptr->evt_id[2],comm_evt_data_ptr->evt_id[1],comm_evt_data_ptr->evt_id[0]);
				
				break;
		}
		comm_evt_state_ptr++;
	}
	
	if( i==EVT_NUM ){
		return FUN_ERROR;
	}
	
	FILE *file;
	
	char str_buf[50];
	
	file = fopen (comm_evt_state_ptr->site_config_file_name,"wb+");
	
	if( file!=NULL ){
		
//		fseek (file, sizeof(DEVFS_SITE_CONFIGURATION_T), SEEK_SET); 
		
		fclose(file);
					
		if( FUN_OK != lefs_write(comm_evt_state_ptr->site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,evd_device_type),\
						sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->evd_device_type), (uint8_t*)comm_evt_data_ptr->evt_device_type))
			return FUN_ERROR;
		
		sprintf(str_buf,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",comm_evt_data_ptr->evt_id[0],comm_evt_data_ptr->evt_id[1],\
			comm_evt_data_ptr->evt_id[2],comm_evt_data_ptr->evt_id[3],comm_evt_data_ptr->evt_id[4],comm_evt_data_ptr->evt_id[5],\
			comm_evt_data_ptr->evt_id[6],comm_evt_data_ptr->evt_id[7],comm_evt_data_ptr->evt_id[8],comm_evt_data_ptr->evt_id[9],\
			comm_evt_data_ptr->evt_id[10],comm_evt_data_ptr->evt_id[11]);
		if( FUN_OK != lefs_write(comm_evt_state_ptr->site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,evd_unipue_id),sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->evd_unipue_id), (uint8_t*)str_buf))
			return FUN_ERROR;
		
		sprintf(str_buf,"%d",comm_evt_data_ptr->vote_type);
		if( FUN_OK != lefs_write(comm_evt_state_ptr->site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,evd_vote_type),sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->evd_vote_type), (uint8_t*)str_buf))
			return FUN_ERROR;
		
		sprintf(str_buf,"%u%u%u", comm_evt_data_ptr->firmware_version / 100, comm_evt_data_ptr->firmware_version / 10 % 10,comm_evt_data_ptr->firmware_version % 10);
		if( FUN_OK != lefs_write(comm_evt_state_ptr->site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,evd_firmware_version),sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->evd_firmware_version), (uint8_t*)str_buf))
			return FUN_ERROR;
		
		sprintf(str_buf,"%u%u%u", comm_evt_data_ptr->hardware_version / 100, comm_evt_data_ptr->hardware_version / 10 % 10,comm_evt_data_ptr->hardware_version % 10);
		if( FUN_OK != lefs_write(comm_evt_state_ptr->site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,evd_hardware_version),sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->evd_hardware_version), (uint8_t*)str_buf))
			return FUN_ERROR;
		
		sprintf(str_buf,"%02d", comm_evt_data_ptr->rf_id);
		if( FUN_OK != lefs_write(comm_evt_state_ptr->site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,evd_address),sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->evd_address), (uint8_t*)str_buf))
			return FUN_ERROR;
		
		for( uint8_t i=0; i<CANDIDATE_MAX; i++){
			devfs_site_config_write_tally(comm_evt_state_ptr->site_config_file_name,i,0);
		}
		
		uint16_t no_vote=0;
		if( FUN_OK != lefs_write(comm_evt_state_ptr->site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,novote),sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->novote), (uint8_t*)&no_vote))
			return FUN_ERROR;
			
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 读取候选人名字. 
* 无. 
* @param[in]   type:类型,idx:序号,out_ptr:输出指针. 
* @param[out]  无.  
* @retval  0：成功
* @retval  -1：失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
FUN_STATUS_T devfs_read_candidates_name(uint8_t type, uint8_t idx, char *name_ptr)
{
	uint32_t type_pos=0,name_pos;
	switch( type ){
		case 1:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type1);
			break;
		case 2:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type2);
			break;
		case 3:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type3);
			break;
		case 4:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type4);
			break;
	}
	name_pos = offsetof(DEVFS_CANDIDATE_TYPE_T,name_of_candidates_type);
	
	return lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), type_pos+name_pos+idx*CANDIDATE_NAME_LEN, CANDIDATE_NAME_LEN, (uint8_t*)name_ptr);
}

/** 
* 读取候选人是否已经结束. 
* 无. 
* @param[in]   type:类型,idx:序号. 
* @param[out]  无.  
* @retval  0： 未结束
* @retval  1： 结束
* @retval  -1：失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
FUN_STATUS_T devfs_candidates_end(uint8_t type, uint8_t idx)
{
	uint32_t type_pos=0,name_pos,num;
	switch( type ){
		case 1:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type1);
			break;
		case 2:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type2);
			break;
		case 3:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type3);
			break;
		case 4:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type4);
			break;
	}
	name_pos = offsetof(DEVFS_CANDIDATE_TYPE_T,number_of_candidate_type);
	
	if( FUN_OK == lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), type_pos+offsetof(DEVFS_CANDIDATE_TYPE_T,number_of_candidate_type),\
					sizeof(((DEVFS_CANDIDATE_TYPE_T*)0)->number_of_candidate_type), &num) ){
		
		if( idx>=num )
			return FUN_END;
		else
			return FUN_NEND;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 读取选举的开始时间或结束时间. 
* 无. 
* @param[in]   ts_te:1开始时间、2结束时间,date_time:时间指针. 
* @param[out]  无.  
* @retval  0： 成功
* @retval  -1：失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
FUN_STATUS_T devfs_read_election_time( uint8_t ts_te, DEVFS_ECECTION_DATE_TIME_T *date_time)
{
	if( ts_te==1 ) {
		
		if( FUN_OK == lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), offsetof(DEVFS_VOTE_SETTINGS_T,election_start_date_time),\
				sizeof(((DEVFS_VOTE_SETTINGS_T*)0)->election_start_date_time), (uint8_t*)date_time) ){
			
			return FUN_OK;
		}
		else{
			return FUN_ERROR;
		}
	}
	else if( ts_te==2 ){
		
		if( FUN_OK == lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), offsetof(DEVFS_VOTE_SETTINGS_T,election_end_date_time),\
				sizeof(((DEVFS_VOTE_SETTINGS_T*)0)->election_end_date_time), (uint8_t*)date_time) ){
			
			return FUN_OK;
		}
		else{
			return FUN_ERROR;
		}
	}
	return FUN_ERROR;
}

/** 
* 读取选举类型数. 
* 无. 
* @param[in]   numbers:选举类型个数. 
* @param[out]  无.  
* @retval  FUN_OK:成功
* @retval  FUN_ERROR:失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-10-10创建 
*/
FUN_STATUS_T devfs_read_type_numbers( uint8_t *numbers )
{
	return lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), offsetof(DEVFS_VOTE_SETTINGS_T,number_of_vote_types),\
				sizeof(((DEVFS_VOTE_SETTINGS_T*)0)->number_of_vote_types), numbers);
}

/** 
* 读取选举日期时间及候选人参数. 
* 无. 
* @param[in]   type:选举类型,vote_settings_ptr:投票设置指针. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
FUN_STATUS_T devfs_read_vote_settings( uint8_t type, COMM_VOTE_SETTING_T *vote_settings_ptr )
{
	uint32_t type_pos=0,num;
	DEVFS_ECECTION_DATE_TIME_T data_time;
	switch( type ){
		case 1:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type1);
			break;
		case 2:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type2);
			break;
		case 3:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type3);
			break;
		case 4:
			type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type4);
			break;
	}
	
	memset(vote_settings_ptr,sizeof(COMM_VOTE_SETTING_T),0);
	
	if( FUN_OK != lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), offsetof(DEVFS_VOTE_SETTINGS_T,election_start_date_time),\
		sizeof(((DEVFS_VOTE_SETTINGS_T*)0)->election_start_date_time), (uint8_t*)&data_time))
			return FUN_ERROR;
	vote_settings_ptr->start_year = data_time.year+2000;
	vote_settings_ptr->start_month = data_time.month;
	vote_settings_ptr->start_day = data_time.day;
	vote_settings_ptr->start_hour = data_time.hour;
	vote_settings_ptr->start_minute = data_time.minute;
	vote_settings_ptr->start_second = data_time.second;
	
	if( FUN_OK != lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), type_pos+offsetof(DEVFS_CANDIDATE_TYPE_T,name_vote_type),\
		sizeof(((DEVFS_CANDIDATE_TYPE_T*)0)->name_vote_type), (uint8_t*)&vote_settings_ptr->name_election))
			return FUN_ERROR;
	
	if( FUN_OK != lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), type_pos+offsetof(DEVFS_CANDIDATE_TYPE_T,number_of_candidate_type),\
		sizeof(((DEVFS_CANDIDATE_TYPE_T*)0)->number_of_candidate_type), (uint8_t*)&vote_settings_ptr->total_candidates))
			return FUN_ERROR;
	
	if( FUN_OK != lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), type_pos+offsetof(DEVFS_CANDIDATE_TYPE_T,number_of_votes_type),\
		sizeof(((DEVFS_CANDIDATE_TYPE_T*)0)->number_of_votes_type), (uint8_t*)&vote_settings_ptr->select_candidates))
			return FUN_ERROR;
		
	return FUN_OK;
}

/** 
* 读取选选举人数量及可选人数及选举类型数量. 
* 无. 
* @param[in]   type:选举类型,vote_settings_ptr:投票设置指针. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
FUN_STATUS_T devfs_settings_to_election( void )
{
	devfs_read_type_numbers(&election_status.election_config.number_of_vote_types);
	
	uint32_t type_pos;
	uint8_t i;
	
	for( i=0; i<election_status.election_config.number_of_vote_types; i++ ){
		
		switch( i ){
			case 0:
				type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type1);
				break;
			case 1:
				type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type2);
				break;
			case 2:
				type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type3);
				break;
			case 3:
				type_pos = offsetof(DEVFS_VOTE_SETTINGS_T,candidate_type4);
				break;
		}
		
		if( FUN_OK != lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), type_pos+offsetof(DEVFS_CANDIDATE_TYPE_T,number_of_candidate_type),\
			sizeof(((DEVFS_CANDIDATE_TYPE_T*)0)->number_of_candidate_type), (uint8_t*)&election_status.election_config.number_of_candidate_type[i]))
				return FUN_ERROR;
		
		if( FUN_OK != lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), type_pos+offsetof(DEVFS_CANDIDATE_TYPE_T,number_of_votes_type),\
			sizeof(((DEVFS_CANDIDATE_TYPE_T*)0)->number_of_votes_type), (uint8_t*)&election_status.election_config.number_of_votes_type[i]))
				return FUN_ERROR;
	}
	return FUN_OK;
}

/** 
* 写入选举的开始时间或结束时间. 
* 无. 
* @param[in]   ts_te:1开始时间，2结束时间,data_time_ptr:日期时间. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-10-14创建 
*/
FUN_STATUS_T devfs_write_election_time( uint8_t ts_te, DEVFS_ECECTION_DATE_TIME_T *data_time_ptr )
{

	if( ts_te==1 ){
		if( FUN_OK != lefs_write(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), offsetof(DEVFS_VOTE_SETTINGS_T,election_start_date_time),\
			sizeof(((DEVFS_VOTE_SETTINGS_T*)0)->election_start_date_time), (uint8_t*)data_time_ptr))
				return FUN_ERROR;
	}
	else if( ts_te==2 ){
		if( FUN_OK != lefs_write(CONECT_STR(DEVFS_DRIVER,"VoteSettings"), offsetof(DEVFS_VOTE_SETTINGS_T,election_end_date_time),\
			sizeof(((DEVFS_VOTE_SETTINGS_T*)0)->election_end_date_time), (uint8_t*)data_time_ptr))
				return FUN_ERROR;
	}
	else{
		return FUN_ERROR;
	}
	return FUN_OK;
}

/** 
* 读取投票记数. 
* 无. 
* @param[in]   type:选举类型,index:候选人序号,tallies_ptr:投票记数. 
* @param[out]  无.  
* @retval   0:成功
* @retval  -1:失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
FUN_STATUS_T devfs_vote_tallies_read(uint8_t vote_type, uint8_t index, uint16_t *tallies_ptr)
{
	
	if( FUN_OK == lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteTallies"), offsetof(DEVFS_VOTE_TALLIES_T,vote1_tally) + \
		sizeof(DEVFS_CANDIDATE_TALLIES_T)*(vote_type-1)+\
		sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally)/CANDIDATE_MAX*index,\
		sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally)/CANDIDATE_MAX, (uint8_t*)tallies_ptr)){
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 写入投票记数. 
* 无. 
* @param[in]   type:选举类型,index:候选人序号,tallies:投票记数. 
* @param[out]  无.  
* @retval   0:成功
* @retval  -1:失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-16创建 
*/
FUN_STATUS_T devfs_vote_tallies_write(uint8_t vote_type, uint8_t index, uint16_t tallies)
{
	
	if( FUN_OK == lefs_write(CONECT_STR(DEVFS_DRIVER,"VoteTallies"), offsetof(DEVFS_VOTE_TALLIES_T,vote1_tally) + \
		sizeof(DEVFS_CANDIDATE_TALLIES_T)*(vote_type-1)+\
		sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally)/CANDIDATE_MAX*index,\
		sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally)/CANDIDATE_MAX, (uint8_t*)&tallies)){
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 累加投票数. 
* 无. 
* @param[in]   vote_type:选举类型,index:候选人序号,tallies:增加投票数. 
* @param[out]  无.  
* @retval   FUN_OK:成功
* @retval  FUN_ERROR:失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-10-12创建 
*/
FUN_STATUS_T devfs_vote_tallies_acc(uint8_t vote_type, uint8_t index, uint16_t tallies)
{
	uint16_t	tally;
	if( FUN_OK != devfs_vote_tallies_read(vote_type, index, &tally))
		return FUN_ERROR;
	
	tally += tallies;
	if( FUN_OK != devfs_vote_tallies_write(vote_type, index, tally))
		return FUN_ERROR;
	
	return FUN_OK;
}

FUN_STATUS_T devfs_vote_tallies_save_all(uint8_t vote_type, COMM_VOTE_TALLY_T *comm_vote_tally_ptr)
{
	if( vote_type >= VOTE_TYPE1 && vote_type <= VOTE_TYPE4 ){
		
		DEVFS_CANDIDATE_TALLIES_T candidate_tallies;
		
		if( FUN_OK != lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteTallies"), offsetof(DEVFS_VOTE_TALLIES_T,vote1_tally) + \
			sizeof(DEVFS_CANDIDATE_TALLIES_T)*(vote_type-1),\
			sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally), (uint8_t*)&candidate_tallies)){
			
			return FUN_ERROR;
		}
		
		for( uint8_t i=0; i<CANDIDATE_MAX; i++ ){
			candidate_tallies.candidate_tally[i] += comm_vote_tally_ptr->vote_candidate[i].increased;
		}
		
		if( FUN_OK == lefs_write(CONECT_STR(DEVFS_DRIVER,"VoteTallies"), offsetof(DEVFS_VOTE_TALLIES_T,vote1_tally) + \
			sizeof(DEVFS_CANDIDATE_TALLIES_T)*(vote_type-1),\
			sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally), (uint8_t*)&candidate_tallies)){
				
//			memset(&candidate_tallies,0,sizeof(DEVFS_CANDIDATE_TALLIES_T));
//				
//			lefs_read("M0:\\VoteTallies", offsetof(DEVFS_VOTE_TALLIES_T,vote1_tally) + \
//			sizeof(DEVFS_CANDIDATE_TALLIES_T)*(vote_type-1),\
//			sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally), (uint8_t*)&candidate_tallies);
//			
//			for( uint8_t i=0; i<CANDIDATE_MAX; i++ ){
//				devfs_log("%d\r\n",candidate_tallies.candidate_tally[i]);
//			}
			
			return FUN_OK;
		}
		else{
			return FUN_ERROR;
		}
	}
	else{
		return FUN_ERROR;
	}
}

FUN_STATUS_T devfs_vote_tallies_read_all(uint8_t vote_type, uint16_t *tally_ptr)
{
	if( vote_type >= VOTE_TYPE1 && vote_type <= VOTE_TYPE4 ){
		
		DEVFS_CANDIDATE_TALLIES_T candidate_tallies;
		
		if( FUN_OK == lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteTallies"), offsetof(DEVFS_VOTE_TALLIES_T,vote1_tally) + \
			sizeof(DEVFS_CANDIDATE_TALLIES_T)*(vote_type-1),\
			sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally), (uint8_t*)tally_ptr)){
			
			return FUN_OK;
		}
		else{
			return FUN_ERROR;
		}
	}
	else{
		return FUN_ERROR;
	}
}

FUN_STATUS_T devfs_init(void)
{
	
	devfs_device_information_init();
//	devfs_site_configuration_init();
//	devfs_vote_records_init();
	devfs_vote_settings_init();
	devfs_file_verification_init();
	
	static uint8_t flag=0;
	if( flag ){
		flag = 0;
		COMM_VOTE_TALLY_T comm_vote_tally;
		for( uint8_t i=0; i<CANDIDATE_MAX; i++ ){
			comm_vote_tally.vote_candidate[i].total = i;
		}
		
		devfs_site_config_save_tally(comm_state.evt_state[0].site_config_file_name, &comm_vote_tally.vote_candidate);
		devfs_site_config_save_tally(comm_state.evt_state[1].site_config_file_name, &comm_vote_tally.vote_candidate);
		
		devfs_vote_records_create();
	}
	
//	devfs_vote_records_create();
	
//	DEVFS_ECECTION_DATE_TIME_T date_time;
//	devfs_read_election_time( 1, &date_time);
//	devfs_read_election_time( 2, &date_time);
	
//	char buf[100];
//	devfs_read_candidates_name(1, 20, buf);
	
//	COMM_VOTE_TALLY_T comm_vote_tally;
//	
//	for( uint8_t i=0; i<CANDIDATE_MAX; i++ )
//		comm_vote_tally.vote_candidate[i].total = 0;
//	
//	devfs_vote_tallies_save_all(1,&comm_vote_tally);
//	
//	static uint16_t tally[CANDIDATE_MAX];
//	devfs_vote_tallies_read_all(1, tally);

//	COMM_VOTE_CANDIDATE_T vote_candidate[CANDIDATE_MAX];
//	for( uint8_t i=0; i<CANDIDATE_MAX; i++ ){
//		vote_candidate[i].total = i;
//	}
//	devfs_site_config_save_tally("M0:\\20191101_Vote1Records", &vote_candidate);
//	memset(vote_candidate,0,sizeof(COMM_VOTE_CANDIDATE_T)*CANDIDATE_MAX);
//	devfs_site_config_restore_tally("M0:\\20191101_Vote1Records", &vote_candidate);
		
	return FUN_OK;
}