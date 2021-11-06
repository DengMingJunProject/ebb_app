#include "system.h"
#include "config.h"
#include "rl_fs.h"
#include "crc8.h"

#ifdef CONFIG_DEBUG
    #define	config_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[CONFIG](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define config_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[CONFIG] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define config_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[CONFIG] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define config_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define config_log(...)
    #define config_usr(...)
    #define config_err(...)
    #define config_dump(...)
#endif
	
const uint32_t 	SOFTWARE_VERSION __attribute__ ((at(0x08020000)))= 1024;
const uint32_t 	HARDWARE_VERSION __attribute__ ((at(0x08020004)))= 1000;
const char		DEVICE_MODEL[]="02.22A";
	
#define EBB_UID ((uint8_t *)0x1FFF7A10)
	
CONFIG_T	config;

const CONFIG_T config_default={
	
	.ebb_device_type = EBB_DEVICE_TYPE,	
	.rf_module_power = 1,
	.tamper_switch = 1,
	.ebb_id = {0},
	.electoral_commission = COMMISSION,
	.ebb_addr = 0x01,
	.evt_step={EVT_STEP_BEGIN},
	.date_commission={19,9,1},
	.country_name={"China"},
//	.electoral_commission_name=COMMISSION,
	.last_upload_date_time={15,8,5,13,25,16},
	.last_election_date={15,9,1},
	.programmed = 1,
	.election_state = ECECTION_NOTSTARTED,
	.evt_save.rf_id = 0x14,
	.evt_save.evt_num = {0},
	.menu_config.upload_settings = 0,
	.menu_config.deact_tmp_alarm = 0,
};

/** 
* 保存新的EVT注册数据. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无.  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-15创建 
*/
void config_evt_update(COMM_EVT_STATE_T *comm_evt_state_ptr)
{
	uint8_t seq =comm_evt_state_ptr - comm_state.evt_state;
	
	config.evt_save.evt_num = comm_state.evt_num;
	config.evt_save.rf_id = comm_state.rf_id;
	
	memcpy(&config.evt_save.evt_reg[seq].comm_evt_data,&comm_evt_state_ptr->comm_evt_data,sizeof(COMM_EVT_DATA_T));
	config.evt_save.evt_reg[seq].addr = comm_evt_state_ptr->addr;
	config.evt_save.evt_reg[seq].reg_ch = comm_evt_state_ptr->reg_chn;
	config.evt_save.evt_reg[seq].send_chn = comm_evt_state_ptr->send_chn;
	config.evt_save.evt_reg[seq].step = comm_evt_state_ptr->step;
	config.evt_save.evt_reg[seq].msg_process_begin = comm_evt_state_ptr->msg_process_begin;
	memcpy(config.evt_save.evt_reg[seq].site_config_file_name, comm_evt_state_ptr->site_config_file_name, sizeof(((COMM_EVT_STATE_T*)0)->site_config_file_name));
}

/** 
* 恢复EVT注册数据到内存. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无.  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-15创建 
*/
void config_evt_restore(void)
{
	comm_state.evt_num = config.evt_save.evt_num;
	comm_state.rf_id = config.evt_save.rf_id;
	
	COMM_EVT_STATE_T *comm_evt_state_ptr = comm_state.evt_state;
	
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( config.evt_save.evt_reg[i].step > EVT_STEP_BEGIN ){
			memcpy(&comm_evt_state_ptr->comm_evt_data,&config.evt_save.evt_reg[i].comm_evt_data,sizeof(COMM_EVT_DATA_T));
			comm_evt_state_ptr->addr = config.evt_save.evt_reg[i].addr;
			comm_evt_state_ptr->reg_chn = config.evt_save.evt_reg[i].reg_ch;
			comm_evt_state_ptr->send_chn = config.evt_save.evt_reg[i].send_chn;
			comm_evt_state_ptr->step = config.evt_save.evt_reg[i].step;
			if( comm_evt_state_ptr->step == EVT_STEP_SETTING ){
				comm_evt_state_ptr->msg_process_begin = config.evt_save.evt_reg[i].msg_process_begin;
				comm_evt_state_ptr->msg_process_ptr = config.evt_save.evt_reg[i].msg_process_begin;
			}
			memcpy( comm_evt_state_ptr->site_config_file_name, config.evt_save.evt_reg[i].site_config_file_name, sizeof(((COMM_EVT_STATE_T*)0)->site_config_file_name));
		}
		comm_evt_state_ptr++;
	}
}

/** 
* 定时检查数据变化，更新到配置文件. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无.  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-15创建 
*/
void config_timming_save(void)
{	
	FILE *f;
	uint8_t chk;
	int res;
	f = fopen (CONECT_STR(CONFIG_DRIVER,"config"),"r");
	if (f == NULL){
		f = fopen (CONECT_STR(CONFIG_DRIVER,"config"),"w");
		if( f != NULL ){
			memcpy( &config, &config_default, sizeof(CONFIG_T) );
			
			uint8_t *ebb_id = EBB_UID;
			for( uint8_t i=0; i<12; i++ ){
				config.ebb_id[i] = *ebb_id++;
			}
			config.crc = crc8((uint8_t*)&config,sizeof(CONFIG_T)-sizeof(((CONFIG_T*)0)->crc));
			
			fseek(f,0,SEEK_SET);
			fwrite (&config, sizeof (uint8_t), sizeof(CONFIG_T), f);
			fclose (f);
			
			config_log("create new file!\n");
		}
	}
	else{
		fclose (f);
		uint8_t crc;
		crc = crc8((uint8_t*)&config,sizeof(CONFIG_T)-sizeof(((CONFIG_T*)0)->crc));
		if( crc != config.crc ){
			config.crc = crc;
			f = fopen (CONECT_STR(CONFIG_DRIVER,"config"),"r+");
			if( f != NULL ){
				fseek(f,0,SEEK_SET);
				fwrite (&config, sizeof (uint8_t), sizeof(CONFIG_T), f);
				fclose (f);
			}
		}
	}
}

/** 
* 赋值配置数据结构. 
* 读取保存的配置文件或使用用默认的配置数据. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无.  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-15创建 
*/
FUN_STATUS_T config_init(void)
{
	memset(&config,0,sizeof(CONFIG_T));
		
	FILE *f;
	f = fopen (CONECT_STR(CONFIG_DRIVER,"config"),"r");
	if( f == NULL ){
		memcpy( &config, &config_default, sizeof(CONFIG_T) );
		uint8_t *ebb_id = EBB_UID;
		for( uint8_t i=0; i<12; i++ ){
			config.ebb_id[i] = *ebb_id++;
		}
		
		uint8_t crc;
		crc = crc8((uint8_t*)&config,sizeof(CONFIG_T)-sizeof(((CONFIG_T*)0)->crc));
		if( crc != config.crc ){
			config.crc = crc;
			f = fopen (CONECT_STR(CONFIG_DRIVER,"config"),"w");
			fseek(f,0,SEEK_SET);
			fwrite (&config, sizeof (uint8_t), sizeof(CONFIG_T), f);
			fclose (f);
		}
	}
	else{
		fseek(f,0,SEEK_SET);
		fread (&config, sizeof (uint8_t), sizeof(CONFIG_T), f);
		fclose(f);
		uint8_t crc;
		crc = crc8((uint8_t*)&config,sizeof(CONFIG_T)-sizeof(((CONFIG_T*)0)->crc));
		if( crc != config.crc ){
			memcpy(&config,&config_default,sizeof(CONFIG_T));
			config.crc = crc8((uint8_t*)&config_default,sizeof(CONFIG_T)-sizeof(((CONFIG_T*)0)->crc));;
			f = fopen (CONECT_STR(CONFIG_DRIVER,"config"),"w");
			fseek(f,0,SEEK_SET);
			fwrite (&config, sizeof(uint8_t), sizeof(CONFIG_T), f);
			fclose (f);
		}
	}
	
//	config_timming_save();
	
//	config.ebb_addr = 1;
//	config.electoral_commission_name = "IEC";
//	rf_module_on();
	
	return FUN_OK;
}