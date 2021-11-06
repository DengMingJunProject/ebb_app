/** 
* @file         comm.c 
* @brief        ͨѶӦ�������ģ��. 
* @details  	ͨѶ��������ݼӽ��ܣ�EVT�豸ͨѶ����. 
* @author       ken deng 
* @date     	2019-07-29 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-07-29, ����\n 
*/  
#include "comm.h"
#include "system.h"
#include "daemon.h"
#include "rf_module.h"
#include "secure_storage.h"
#include "main.h"
#include "stdlib.h"
#include "beep.h"
#include "menu_func.h"
#include "event_log.h"
#include "config.h"
#include "crypt.h"
#include "config.h"
#include "urg.h"
#include "hash.h"
#include "tm_stm32_rtc.h"
#include "devfs.h"
#include "hash.h"
#include "election.h"
#include "display.h"

//#undef COMM_DEBUG
#ifdef COMM_DEBUG
    #define	comm_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[COMM](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define comm_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[COMM] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define comm_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[COMM] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define comm_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define comm_log(...)
    #define comm_usr(...)
    #define comm_err(...)
    #define comm_dump(...)
#endif
	
const char	CMD_REQ_CLOSE_ELECTION[]=	"03Y";
const char	CMD_CLOSE_ELECTION_RPY[]=	"03Z";
const char  CMD_REQ_CUR_VOTE_TALLY[] =  "01Z";
const char	CMD_TRAN_CUR_VOTE_TALLY[]=	"020";
const char	CMD_REQ_FINAL_VOTE_TALLY[]=	"021";
const char	CMD_TRAN_FINAL_VOTE_TALLY[]="022";
const char  CMD_REQ_ALL_VOTE_TALLY[] =  "023";
const char  CMD_TRAN_ALL_VOTE_TALLY[] = "024";
const char	CMD_ASSISTANCE[]=			"02J";
const char	CMD_TAMPER[]=				"02I";
const char	CMD_COMFIRM[]=				"000";
const char	CMD_PAPER_LOW[]=			"02K";
const char	CMD_BATTERY_LOW[]=			"02L";
const char 	CMD_CLR_SET_CHG_TYPE[]=		"01R";
const char 	CMD_CLR_SET_DEREG[]=		"01S";
const char 	CMD_UPDATE_SEG[]=			"01T";
const char 	CMD_UPDATE_RPY[]=			"01U";
const char  CMD_STATUS_OK_RPY[]=		"02M";
const char  CMD_REQ_EVENT_LOG[]=		"02N";
const char  CMD_TRN_EVENT_LOG[]=		"02O";
const char	CMD_REQ_STATUS[]=			"02F";
QueueHandle_t	*send_queue=NULL;						///<send queue
QueueHandle_t	*recv_queue=NULL;						///<receive queue
COMM_STATE_T	comm_state;

//EventGroupHandle_t comm_event_handler = NULL;    /*�����¼���־��*/
#define EVENTBIT_RECEIVE_OK  		(1<<0)
#define EVENTBIT_RECEIVE_BIT    	(EVENTBIT_RECEIVE_OK)

uint32_t comm_candidate_recv_target(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	
	comm_evt_state_ptr->candidate_count = 0;
	return REPY_FIRST;
}

uint32_t comm_clear_candidate_name(void *handle, uint32_t value, uint8_t *p_data)
{
	
	char name[CANDIDATE_NAME_LEN]={0};
	for( uint8_t i=0; i<CANDIDATE_NUM; i++ ){
//		secure_storage_canddate_name( i, CANDIDATE_NAME_LEN, (uint8_t*)name, SC_WRITE );
	}
	
//	secure_storage_canddate_info( OP_NUM, (uint8_t*)&comm_register.vote_setting.total_candidates, SC_WRITE );
//	secure_storage_canddate_info( OP_AV, (uint8_t*)&comm_register.vote_setting.select_candidates, SC_WRITE );
//	secure_storage_canddate_info( OP_VTN, (uint8_t*)&comm_register.vote_setting.name_election, SC_WRITE );
	
	return REPY_FIRST;
}

uint32_t comm_evt_registration(void *handle, uint32_t value, uint8_t *p_data)
{
	if( is_enable_cnct_new_ect() )
		return REPY_FIRST;
	else
		return REPY_SECOND;
}

uint32_t comm_evt_ready(void *handle, uint32_t value, uint8_t *p_data)
{
	
	return REPY_FIRST;
}

uint32_t comm_ebb_auth(void *handle, uint32_t value, uint8_t *p_data)
{
	
	if( value==COMM_YES ){
	}
	else if( value==COMM_NO ){
	}
	
	return REPY_FIRST;
}

uint32_t comm_evt_auth(void *handle, uint32_t value, uint8_t *p_data)
{
	
	return REPY_FIRST;
}

/** 
*  �յ�EVT���͹�����ַ����. 
*  �����ǰ��UIDδע����������µ�ַ������Ѿ�ע���ʹ��ԭ����ַ. 
* @param[in]   handle:EVT״̬�����value��������ֵ�� p_data:����ָ��. 
* @param[out]  ��.  
* @retval  REPY_NONE:û��ʹ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-07���� 
*/
uint32_t comm_evt_req_addr(void *handle, uint32_t value, uint8_t *p_data)
{
	if( is_enable_cnct_new_ect() ){
		
		COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
		
		if( FUN_OK != comm_evt_is_registration_of_uid(p_data) ){
				
			//��ǰ��EVT IDû�з�����µ�ַ�������һ���µ�ַ
			comm_evt_state_ptr->comm_evt_data.rf_id = comm_state.rf_id++;
		}
		///<�Ƚϵ�ַ����������õ�ֵַΪ0
		else if( strncmp( comm_evt_state_ptr->msg_process_ptr->cmd_code,register_msg_code[0].cmd_code,3)==0 ){
			comm_evt_state_ptr->addr = 0;
		}
		//���浱ǰEVT��EVT ID
		memcpy(comm_evt_state_ptr->comm_evt_data.evt_id,p_data,sizeof(((COMM_EVT_DATA_T*)0)->evt_id));
		
		//�رյ�ǰ������ѯ����
		comm_state.enable_poll = 0;
		
		return REPY_NONE;
	}
	else{
		return REPY_ERROR;
	}
}

/** 
*  ����evt rfid �ص����� . 
* �����������EVT��ַ��ע�ᣬ���ش���.. 
* @param[in]   handle:EVT״̬�����value��������ֵ�� p_data:����ָ��. 
* @param[out]  ��.  
* @retval  REPY_ERROR:�����жϵ�ǰEVT��ע�����,
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-07���� 
*/
uint32_t comm_set_new_addr(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	
	if( FUN_OK == comm_is_evt_reging(comm_evt_state_ptr->addr) ){
		return REPY_ERROR;
	}
	else{
		
		comm_state.comm_rpy_rfid.evt_set_id = comm_evt_state_ptr->comm_evt_data.rf_id;
		memcpy(comm_state.comm_rpy_rfid.evt_id,comm_evt_state_ptr->comm_evt_data.evt_id,sizeof(((COMM_RPY_RFID_T*)0)->evt_id));
		comm_evt_state_ptr->send_chn = RF_EVT_CHN_REQ;
		///<��EVT����ע��ͨ��
		if( !is_chn1_reging() ){
			set_chn_reging(REG_CHN_1);
			comm_state.comm_rpy_rfid.evt_reg_chn = RF_EBB_CHN_REG1;
			comm_evt_state_ptr->reg_chn = REG_CHN_1;
		}
		else if( !is_chn2_reging() ){
			set_chn_reging(REG_CHN_2);
			comm_state.comm_rpy_rfid.evt_reg_chn = RF_EBB_CHN_REG2;
			comm_evt_state_ptr->reg_chn = REG_CHN_2;
		}
		else if( !is_chn3_reging() ){
			set_chn_reging(REG_CHN_3);
			comm_state.comm_rpy_rfid.evt_reg_chn = RF_EBB_CHN_REG3;
			comm_evt_state_ptr->reg_chn = REG_CHN_3;
		}
		
		return REPY_FIRST;
	}
	return REPY_NONE;
}

uint32_t comm_rpy_evt_rfid(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	comm_evt_state_ptr->addr = comm_evt_state_ptr->comm_evt_data.rf_id;
	memcpy(comm_state.comm_rpy_rfid.evt_id,comm_evt_state_ptr->comm_evt_data.evt_id,sizeof(((COMM_EVT_DATA_T*)0)->evt_id));
	comm_state.comm_rpy_rfid.evt_set_id = comm_evt_state_ptr->comm_evt_data.rf_id;
	comm_evt_state_ptr->send_chn = RF_EVT_CHN_REG;
	return REPY_NONE;
}

uint32_t comm_evt_device_type(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	memcpy(comm_evt_state_ptr->comm_evt_data.evt_device_type, p_data,sizeof(((COMM_EVT_DATA_T*)0)->evt_device_type));
	return REPY_NONE;
}

uint32_t comm_evt_hardware_version(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	comm_evt_state_ptr->comm_evt_data.hardware_version = (*p_data<<8)|(*(p_data+1));
	return REPY_NONE;
}

uint32_t comm_evt_firmware_version(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	comm_evt_state_ptr->comm_evt_data.firmware_version = (*p_data<<8)|(*(p_data+1));
	return REPY_NONE;
}

uint32_t comm_evt_update(void *handle, uint32_t value, uint8_t *p_data)
{
	
	if( value==COMM_YES ){
	}
	else if( value==COMM_NO ){
	}
	return REPY_FIRST;
}

uint32_t comm_evt_vote_type(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	comm_evt_state_ptr->comm_evt_data.vote_type = (*p_data);
	return REPY_NONE;
}

uint32_t comm_register_end(void *handle, uint32_t value, uint8_t *p_data)
{
	
//	rf_module_set_rx_addr(config.rf_id+0x1a470500, 2);
//	char buf[100];
//	snprintf(buf,100,"Receive address changed(0x1a4705%02x change to 0x1a4705%02x)",comm_state.old_addr,config.rf_id);
//	event_log_write(SYSTEM_LEVEL, COMMUNICATION_TYPE, buf);
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	comm_evt_state_ptr->addr = comm_evt_state_ptr->comm_evt_data.rf_id;
	comm_state.enable_poll = 1;
	clr_chn_reging(comm_evt_state_ptr->reg_chn);
	return REPY_FIRST;
}

uint32_t comm_setting_end(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	
	memset(&vote_status,0,sizeof(VOTE_STATUS_T));
	uint16_t temp=0;
//	secure_storage_canddate_info( OP_TOTALTV, (uint8_t*)&temp, SC_WRITE );
//	secure_storage_canddate_info( OP_TOTALVOTERS, (uint8_t*)&temp, SC_WRITE );
//	secure_storage_canddate_info( OP_NOVOTERS, (uint8_t*)&temp, SC_WRITE );
	beep_set_alarm(BP_SETTING_DONE);
	
	char buf[100];
	snprintf(buf,100,"Settings Downloaded from EBB(%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x)",config.ebb_id[0],config.ebb_id[1],\
		config.ebb_id[2],config.ebb_id[3],config.ebb_id[4],config.ebb_id[5],config.ebb_id[6],\
		config.ebb_id[7],config.ebb_id[8],config.ebb_id[9],config.ebb_id[10],config.ebb_id[11]);
	event_log_write(SYSTEM_LEVEL, SETTINGS_TYPE, buf);
	
	clr_chn_reging(comm_evt_state_ptr->reg_chn);
	
	//�½�EVD�����ļ�
	devfs_site_config_new(&comm_evt_state_ptr->comm_evt_data);
	
	comm_state.enable_poll = 1;
	
	return REPY_FIRST;
}

uint32_t comm_download_process(void *handle, uint32_t value, uint8_t *p_data)
{
	return REPY_NONE;
}

/** 
* EVT����RFID��Ӧ�ص�����. 
* ����EBB���ص�ַ��. 
* @param[in]   handle:EVT״̬�����value��������ֵ�� p_data:����ָ��. 
* @param[out]  ��.  
* @retval  REPY_NONE: û��ѡ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-08-08���� 
*/
uint32_t comm_reply_evt_set_rfid(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
//	comm_evt_state_ptr->addr = comm_evt_state_ptr->comm_evt_data.rf_id;
	return REPY_NONE;
}

uint32_t comm_get_rtc(void *handle, uint32_t value, uint8_t *p_data)
{
	TM_RTC_t rtc;
	TM_RTC_GetDateTime(&rtc, TM_RTC_Format_BIN);
	
	comm_state.rtc_setting.year = rtc.Year;
	comm_state.rtc_setting.month = rtc.Month;
	comm_state.rtc_setting.day = rtc.Day;
	comm_state.rtc_setting.hour = rtc.Hours;
	comm_state.rtc_setting.minute = rtc.Minutes;
	comm_state.rtc_setting.second = rtc.Seconds;
	return REPY_NONE;
}

uint32_t comm_test(void *handle, uint32_t value, uint8_t *p_data)
{
//	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
//	comm_evt_state_ptr->addr = comm_evt_state_ptr->comm_evt_data.rf_id;
	return REPY_NONE;
}

/** 
* ��ʼ����ע����������. 
* ��. 
* @param[in]   handle:EVT״̬�����value��������ֵ�� p_data:����ָ��. 
* @param[out]  ��.  
* @retval  REPY_NONE: û��ѡ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-12-27���� 
*/
uint32_t comm_start_reg(void *handle, uint32_t value, uint8_t *p_data)
{
	display_screen_info_in(menu_func_cnct_new_evt,"\n\n\nEVT REG...");
	return REPY_NONE;
}

/** 
* ��ʼ����������������. 
* ��. 
* @param[in]   handle:EVT״̬�����value��������ֵ�� p_data:����ָ��. 
* @param[out]  ��.  
* @retval  REPY_NONE: û��ѡ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-12-27���� 
*/
uint32_t comm_start_settings(void *handle, uint32_t value, uint8_t *p_data)
{
	display_screen_info_in(menu_func_cnct_new_evt,"\n\n\nEVT SET...");
	return REPY_NONE;
}

uint32_t comm_send_vote_settings(void *handle, uint32_t value, uint8_t *p_data)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = (COMM_EVT_STATE_T *)handle;
	uint8_t vote_type;
	
	vote_type = comm_evt_state_ptr->comm_evt_data.vote_type;
	
	devfs_read_vote_settings( vote_type, &comm_state.vote_setting );
}

/* EVT2EBB �����Ƚ���������ִ�лص�����*/
/* EBB2EVT ������ִ�лص������ٷ�������*/
/**  ע����������  */  
const COMM_MSG_PROCESS_T register_msg_code[]={
	1 ,EVT2EBB,"05B",NULL,0,comm_evt_req_addr,//1
	2 ,EBB2EVT,"01P",&comm_state.comm_rpy_rfid,sizeof(((COMM_STATE_T*)0)->comm_rpy_rfid),comm_set_new_addr,//50
	3 ,EVT2EBB,"000",NULL,0,comm_rpy_evt_rfid,//51
	4 ,EVT2EBB,"002",NULL,0,comm_start_reg,//1
	5 ,EBB2EVT,"000",NULL,0,NULL,//2
	6 ,EBB2EVT,"003/004",NULL,0,comm_evt_registration,//3
	7 ,EVT2EBB,"000",NULL,0,NULL,//4
	8 ,EBB2EVT,"01A",default_password.encryption_transfer_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->encryption_transfer_key),NULL,//5
	9 ,EVT2EBB,"000",NULL,0,NULL,//6
	10,EBB2EVT,"00M",NULL,0,NULL,//7
	11,EVT2EBB,"000",NULL,0,NULL,//8
	12,EVT2EBB,"00N/00O",NULL,0,comm_evt_ready,//9
	13,EBB2EVT,"000",NULL,0,NULL,//10
	
	//EBB2EVT,"000",NULL,0,NULL,//11
	14,EBB2EVT,"00P/00Q",NULL,0,comm_ebb_auth,//12
	15,EVT2EBB,"000",NULL,0,NULL,//13
	16,EVT2EBB,"00P/00Q",NULL,0,comm_evt_auth,//14
	17,EBB2EVT,"000",NULL,0,NULL,//15
	18,EBB2EVT,"01B",NULL,0,NULL,//16
	19,EVT2EBB,"000",NULL,0,NULL,//17
	20,EVT2EBB,"01C",NULL,0,comm_evt_req_addr,//18
	21,EBB2EVT,"000",NULL,0,NULL,//19
	22,EVT2EBB,"01B",NULL,0,NULL,//20
	
	23,EBB2EVT,"000",NULL,0,NULL,//21
	24,EBB2EVT,"01C",config.ebb_id,sizeof(((CONFIG_T*)0)->ebb_id),NULL,//22
	25,EVT2EBB,"000",NULL,0,NULL,//23
	26,EBB2EVT,"01D",NULL,0,NULL,//24
	27,EVT2EBB,"000",NULL,0,NULL,//25
	28,EVT2EBB,"01E",NULL,0,comm_evt_device_type,//26
	29,EBB2EVT,"000",NULL,0,NULL,//27
	30,EVT2EBB,"01D",NULL,0,NULL,//28
	31,EBB2EVT,"000",NULL,0,NULL,//29
	32,EBB2EVT,"01E",config.ebb_device_type,sizeof(((CONFIG_T*)0)->ebb_device_type),NULL,//30
	
	33,EVT2EBB,"000",NULL,0,NULL,//31
	34,EBB2EVT,"01H",&comm_state.rtc_setting,sizeof(((COMM_STATE_T*)0)->rtc_setting),comm_get_rtc,//32
	35,EVT2EBB,"000",NULL,0,NULL,//33
	36,EBB2EVT,"01N",NULL,0,NULL,//34
	37,EVT2EBB,"000",NULL,0,NULL,//35
	38,EVT2EBB,"01O",NULL,0,comm_evt_hardware_version,//36
	39,EBB2EVT,"000",NULL,0,NULL,//37
	40,EBB2EVT,"01J",NULL,0,NULL,//38
	41,EVT2EBB,"000",NULL,0,NULL,//39
	42,EVT2EBB,"01K",NULL,0,comm_evt_firmware_version,//40
	
	43,EBB2EVT,"000",NULL,0,NULL,//41
	44,EBB2EVT,"01L/01M",NULL,0,comm_evt_update,//42
	45,EVT2EBB,"000",NULL,0,NULL,//43
	46,EBB2EVT,"01F",NULL,0,NULL,//44
	47,EVT2EBB,"000",NULL,0,NULL,//45
	48,EVT2EBB,"01G",NULL,0,comm_evt_vote_type,//46
	49,EBB2EVT,"000",NULL,0,NULL,//47
	50,EBB2EVT,"01I",&comm_state.vote_setting,sizeof(((COMM_STATE_T*)0)->vote_setting),comm_send_vote_settings,//48
	51,EVT2EBB,"000",NULL,0,NULL,//49
//	EBB2EVT,"01P",&comm_state.evt_set_id,sizeof(((COMM_STATE_T*)0)->evt_set_id),comm_set_evt_rfid,//50
//	
//	EVT2EBB,"000",NULL,0,NULL,//51
	52,EBB2EVT,"008",NULL,0,NULL,//52
	53,EVT2EBB,"000",NULL,0,comm_register_end,//53
	54,0,"",NULL,0,NULL,
};

/**  ������������  */  
const COMM_MSG_PROCESS_T setting_msg_code[]={
	1 ,EVT2EBB,"005",NULL,0,comm_start_settings,//1
	2 ,EBB2EVT,"000",NULL,0,NULL,//2
//	2 ,EBB2EVT,"01P",&comm_state.comm_rpy_rfid,sizeof(((COMM_STATE_T*)0)->comm_rpy_rfid),comm_set_new_addr,//50
	3 ,EBB2EVT,"006",NULL,0,NULL,//3
	4 ,EVT2EBB,"000",NULL,0,NULL,//4
	5 ,EBB2EVT,"01B",NULL,0,NULL,//5
	6 ,EVT2EBB,"000",NULL,0,NULL,//6
	7 ,EVT2EBB,"01C",NULL,0,comm_evt_req_addr,//7
	8 ,EBB2EVT,"000",NULL,0,NULL,//8
	9 ,EVT2EBB,"01B",NULL,0,NULL,//9
	10,EBB2EVT,"000",NULL,0,NULL,//10
	
	48,EVT2EBB,"01G",NULL,0,comm_evt_vote_type,//46
	49,EBB2EVT,"000",NULL,0,NULL,//47
	11,EBB2EVT,"01C",config.ebb_id,sizeof(((CONFIG_T*)0)->ebb_id),NULL,//11
	12,EVT2EBB,"000",NULL,0,NULL,//12
	13,EBB2EVT,"00M",NULL,0,NULL,//13
	14,EVT2EBB,"000",NULL,0,NULL,//14
	15,EVT2EBB,"00N/00O",NULL,0,comm_evt_ready,//15
	16,EBB2EVT,"000",NULL,0,NULL,//16
	17,EBB2EVT,"00P",NULL,0,NULL,//17
	18,EVT2EBB,"000",NULL,0,NULL,//18
	19,EVT2EBB,"00P",NULL,0,NULL,//19
	20,EBB2EVT,"000",NULL,0,NULL,//20
	
	21,EBB2EVT,"03D",default_password.voter_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->voter_key),NULL,//21
	22,EVT2EBB,"000",NULL,0,NULL,//22
	23,EBB2EVT,"03E",default_password.maintenance_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->maintenance_key),NULL,//23
	24,EVT2EBB,"000",NULL,0,NULL,//24
	25,EBB2EVT,"03F",default_password.administrator_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->administrator_key),NULL,//25
	26,EVT2EBB,"000",NULL,0,NULL,//26
	27,EBB2EVT,"03G",default_password.system_administrator_key1,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator_key1),NULL,//27
	28,EVT2EBB,"000",NULL,0,NULL,//28
	29,EBB2EVT,"03C",&comm_state.vote_setting,sizeof(((COMM_STATE_T*)0)->vote_setting),comm_send_vote_settings,//29
	30,EVT2EBB,"000",NULL,0,NULL,//30
	
	31,EBB2EVT,"03H",config.electoral_commission,sizeof(((CONFIG_T*)0)->electoral_commission),NULL,//31
	32,EVT2EBB,"000",NULL,0,comm_candidate_recv_target,//32
	
	33,EBB2EVT,"   ",NULL,0,NULL,//33
	34,EVT2EBB,"   ",NULL,0,NULL,//34

	35,EBB2EVT,"008",NULL,0,NULL,//35
	36,EVT2EBB,"000",NULL,0,comm_setting_end,//36
	37,0,"",NULL,0,NULL,
};

/**  ����������������  */
static uint16_t setting_msg_code_num = sizeof(setting_msg_code)/sizeof(COMM_MSG_PROCESS_T);
/**  ע��������������  */
static uint16_t register_msg_code_num = sizeof(register_msg_code)/sizeof(COMM_MSG_PROCESS_T);

/** 
* RFģ�齫���յ������ݷ���ͨѶģ�������. 
* ��RF�����̣߳�USB�̣߳�UDP�̵߳���. 
* @param[in]   data:����ָ��,len:�������ݳ���. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
void comm_recv_in_queue(void *data, uint16_t len){
	
	COMM_RECV_MSG_T msg;
	msg.len = len;
	msg.buf = data;
	if( msg.buf!=NULL )
		xQueueSend(recv_queue,&msg,0);
}

/** 
* ��ͨѶģ�����ݴӶ�����ȡ��. 
* �ɽ��ܽ��պ�������. 
* @param[in]   *len:�������ݳ��ȣ�û��������Ϊ0. 
* @param[out]  ��.  
* @retval  ����ָ�룬û���ݷ��ؿ�ָ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
static void* comm_recv_out_queue(uint16_t *len){
	
	COMM_RECV_MSG_T	comm_msg;
	if( xQueueReceive(recv_queue,&comm_msg,0)==pdPASS ){
		
		*len = comm_msg.len;
		return comm_msg.buf;
	}
	else{
		return NULL;
	}
}

/** 
* ��Ҫ������Ϣ�����. 
* ״̬��Ϣ���û��������룬������Ϣ���뵽����ǰͷ. 
* @param[in]   addr:���͵�ַ��chn������ͨ���ţ�data���������ݵ�ַ��len�����ͳ��ȣ�msg_id:��ϢID��msg_type:��Ϣ����. 
* @param[out]  ��.  
* @retval  ����FUN_STATUS_T.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-10-28���� 
*/
FUN_STATUS_T comm_send_in_queue(uint32_t target_addr, uint32_t source_addr, uint8_t chn, void *data, uint16_t len, uint32_t msg_id,uint8_t msg_type )
{	
	COMM_SEND_MSG_T msg;
	msg.len = len;
	msg.buf = data;
	msg.target_addr = target_addr;
	msg.source_addr = source_addr;
	msg.chn = chn;
	msg.msg_id = msg_id;
	
	if( msg.buf!=NULL ){
		
		if( xQueueSend(send_queue,&msg,0) == pdPASS){
			return FUN_OK;
		}
		else{
			free(data);
			return FUN_ERROR;
		}
		
//		if( msg_type==0 ){
//			if( uxQueueMessagesWaiting(send_queue) < (COMM_SEND_QUEUE_NUM-COMM_SEND_RETAIN_NUM) ){
//				if( xQueueSend(send_queue,&msg,0) == pdPASS )
//					return FUN_OK;
//				else{
//					free(data);
//					return FUN_ERROR;
//				}	
//			}
//			else{
//				free(data);
//				return FUN_ERROR;
//			}	
//		}
//		else if( msg_type==1 ){
//			if( xQueueSend(send_queue,&msg,0) == pdPASS){
//				return FUN_OK;
//			}
//			else{
//				free(data);
//				return FUN_ERROR;
//			}	
//		}
	}
	return FUN_ERROR;
}

/** 
* ������Ϣ����. 
* �ɽ��ܽ��պ�������. 
* @param[in]   ��. 
* @param[out]  msg_ptr:��Ϣ���.  
* @retval  ����FUN_STATUS_T.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-10-28���� 
*/
static FUN_STATUS_T comm_send_out_queue(COMM_SEND_MSG_T *msg_ptr)
{
	if( xQueueReceive(send_queue,msg_ptr,0)==pdPASS ){
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* ��RFģ�飬USBģ�飬����ģ�鷢������. 
* ��ͨѶ���ܷ��ͺ�������. 
* @param[in]   addr:��ַ, chn: ͨ�� *p_send_data:��������ָ�룬 len:���ͳ���. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
static void comm_send_msg(uint32_t target_addr, uint32_t source_addr, uint8_t chn, uint8_t *p_send_data, uint16_t len){
	
	rf_module_send(target_addr, source_addr, chn, p_send_data, len );
}

/** 
* ��ͨѶģ����������ݶ�����������. 
* ��ͨѶ������̵߳���. 
* @param[in]   *len:�������ݳ��ȣ�û��������Ϊ0. 
* @param[out]  ��.  
* @retval  ����ָ�룬û���ݷ��ؿ�ָ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
void* comm_crypt_recv( uint16_t *len ){
	
	void *recv_data=NULL;
	uint16_t buf_len;
	uint16_t data_len;
	recv_data=comm_recv_out_queue(&buf_len);
	if( recv_data!=NULL ){
		if( buf_len>sizeof(COMM_HEADER_T) ){
			data_len = buf_len-sizeof(COMM_HEADER_T);
			uint8_t *plaintext_buf = malloc(data_len);
			if( plaintext_buf!=NULL ){
				
				crypt_decryption(default_password.encryption_transfer_key, recv_data+sizeof(COMM_HEADER_T), plaintext_buf, data_len);
				memcpy(recv_data+sizeof(COMM_HEADER_T),plaintext_buf,data_len);
				free(plaintext_buf);
				
				*len = buf_len;
				return recv_data; 
			}
			else{
				*len = 0;
				free(recv_data);
				return NULL;
			}
		}
		else{
			*len = buf_len;
			return recv_data;
		}
	}
	else{
		*len = 0;
		return NULL;
	}
}

/** 
* �����ݼ�����ͨѶ�ӿڷ���. 
* ��ͨѶ������̵߳���. 
* @param[in]   addr�����յ�ַ��*ptr:������������ָ�룬size:�����������ݳ���. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
void comm_crypt_send(uint32_t target_addr, uint32_t source_addr, uint8_t chn, uint8_t *ptr, uint16_t size ){
	
	if( size>sizeof(COMM_HEADER_T) ){
		
		uint8_t seg;
		seg = size/16;
		if( size%16 ){
			seg += 1;
		}
		void *send_data,*plaintext_buf;
		uint16_t data_len = seg*16;
		send_data = malloc(data_len+sizeof(COMM_HEADER_T));
		plaintext_buf = malloc(data_len);
		if( plaintext_buf && send_data ){
			
			memcpy(send_data, ptr, sizeof(COMM_HEADER_T));
			memset(plaintext_buf,0,data_len);
			memcpy(plaintext_buf, ptr+sizeof(COMM_HEADER_T), size-sizeof(COMM_HEADER_T));
			crypt_encryption(default_password.encryption_transfer_key, plaintext_buf, send_data+sizeof(COMM_HEADER_T), data_len);
			comm_send_msg(target_addr, source_addr, chn, send_data, data_len+sizeof(COMM_HEADER_T) );
		}
		free(plaintext_buf);
		free(send_data);
	}
	else{
		comm_send_msg(target_addr, source_addr, chn, ptr, size );
	}
	
	free(ptr);
}

COMM_EVT_STATE_T* comm_save_curr_msgid(uint8_t addr,uint32_t msg_id)
{
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( comm_state.evt_state[i].addr == addr ){
			comm_state.evt_state[i].comm_wait_msg_sta.msg_id = msg_id;
			comm_state.evt_state[i].comm_wait_msg_sta.msg_time = 0;
			comm_state.evt_state[i].comm_wait_msg_sta.msg_reply = 1;
			return &comm_state.evt_state[i];
		}
	}
	return NULL;
}

/** 
* ������ݵ��ü��ܷ��ͺ�������. 
* ��. 
* @param[in]   target_addr:Ŀ���ַ��source_addr;Դ��ַ��cmd:�����룬data;���ݣ�len������. 
* @param[out]  ��.  
* @retval  1�����ͳɹ���0������ʧ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-08-05���� 
*/
FUN_STATUS_T comm_pack(uint8_t target_addr, uint8_t source_addr, uint8_t pipe, char *cmd, uint8_t *data, uint16_t len)
{
	uint8_t *ptr = malloc(sizeof(COMM_HEADER_T) + len);
	if (ptr != NULL) {
		
		memset(ptr,0,sizeof(COMM_HEADER_T) + len);

		//<���Ŀ���ַ��Դ��ַ,��ϢID
		COMM_HEADER_T *comm_frame = (COMM_HEADER_T *)ptr;
		comm_frame->source_address = source_addr+EVT_RF_TARGET_ADDR;
		comm_frame->target_address = target_addr+EVT_RF_TARGET_ADDR;
		memcpy(comm_frame->command,cmd,3);
		comm_frame->message_id = comm_state.message_id++;
		
		memcpy((uint8_t*)ptr+sizeof(COMM_HEADER_T),data, len);

		///<���ܷ��ͺ���
//		comm_crypt_send(target_addr+EVT_RF_TARGET_ADDR, pipe,  ptr, sizeof(COMM_HEADER_T) + len);
		comm_send_in_queue(target_addr+EVT_RF_TARGET_ADDR,  source_addr, pipe,  ptr, sizeof(COMM_HEADER_T) + len, comm_frame->message_id,1 );
//		if( strcmp(CMD_REQ_STATUS,cmd)==0 ) 
//			comm_send_in_queue(target_addr+EVT_RF_TARGET_ADDR, pipe,  ptr, sizeof(COMM_HEADER_T) + len, comm_frame->message_id,0 );
//		else
//			comm_send_in_queue(target_addr+EVT_RF_TARGET_ADDR, pipe,  ptr, sizeof(COMM_HEADER_T) + len, comm_frame->message_id,1 );
		
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* �ж�EVT��ַ�Ƿ��Ѿ�ע��. 
* ��. 
* @param[in]   addr:��ַ. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
FUN_STATUS_T comm_evt_is_register(uint16_t addr)
{
	if( addr>=EVT_ADDR_MIN && addr<=EVT_ADDR_MAX ){
		
		for( uint8_t i=0; i<EVT_NUM; i++ ){
			if( comm_state.evt_state[i].addr == addr ){
				if( comm_state.evt_state[i].step > EVT_STEP_REG ){
					return FUN_OK;
				}
			}
		}
	}
	return FUN_ERROR;
}

/** 
* ����EVT UID�Ƿ��Ѿ�ע��. 
* ��. 
* @param[in]   uid:IDָ��. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-08-27���� 
*/
FUN_STATUS_T comm_evt_is_registration_of_uid(uint8_t *uid)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = comm_state.evt_state;
	
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( memcmp(comm_evt_state_ptr->comm_evt_data.evt_id, uid, UID_LEN) == 0 ){
			return FUN_OK;
		}
		comm_evt_state_ptr++;
	}
	
	return FUN_ERROR;
}

/** 
* ����EVT ��ַ����EVT STATE���. 
* ��. 
* @param[in]   addr:��ַ. 
* @param[out]  ��.  
* @retval  COMM_EVT_STATE_T *:EVT STATE ���.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-08-27���� 
*/
COMM_EVT_STATE_T *comm_get_evt_handle_of_addr(uint8_t addr)
{
	COMM_EVT_STATE_T *comm_evt_state_ptr = comm_state.evt_state;
	
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( comm_evt_state_ptr->addr == addr ){
			return comm_evt_state_ptr;
		}
		comm_evt_state_ptr++;
	}
	
	return NULL;
}

/** 
* �ж�EVT��ַ�Ƿ��Ѿ�ע��. 
* ��. 
* @param[in]   addr:��ַ. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
FUN_STATUS_T comm_evt_is_setting(uint16_t addr)
{
	if( addr>=EVT_ADDR_MIN && addr<=EVT_ADDR_MAX ){
		
		for( uint8_t i=0; i<EVT_NUM; i++ ){
			if( comm_state.evt_state[i].addr == addr ){
				if( comm_state.evt_state[i].step > EVT_STEP_SETTING ){
					return FUN_OK;
				}
			}
		}
	}
	return FUN_ERROR;
}

/** 
* ͨ����ַ����EVT���ݾ����û�з���NULL. 
* ��. 
* @param[in]   addr:��ַ,uid_ptr:uid����ָ��. 
* @param[out]  COMM_EVT_STATE_Tָ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-08-05���� 
*	   ken deng 2019-08-28 : �޸������ַΪ0���Ƿ�ע���
*/
COMM_EVT_STATE_T * comm_find_evt_addr(uint16_t addr, uint8_t *uid_ptr)
{
	
	///<�����ǰ�ĵ�ַΪ0�����Ȳ����Ƿ��Ѿ�ע���
	if( addr==0x00 ){
		
		COMM_EVT_STATE_T *comm_evt_state_ptr = comm_state.evt_state;
	
		for( uint8_t i=0; i<EVT_NUM; i++ ){
			if( memcmp(comm_evt_state_ptr->comm_evt_data.evt_id, uid_ptr, UID_LEN) == 0 ){
				return comm_evt_state_ptr;
			}
			comm_evt_state_ptr++;
		}
	}
	
	if( (addr>=EVT_ADDR_MIN && addr<=EVT_ADDR_MAX) || addr==0x00 ){
		
		for( uint8_t i=0; i<EVT_NUM; i++ ){
			if( (comm_state.evt_state[i].addr == addr) || (comm_state.evt_state[i].comm_evt_data.rf_id == addr) ){
				return &comm_state.evt_state[i];
			}
		}
	}
	return NULL;
}

/** 
* �����µ�EVT�����ݵ�EBB�������ַ��״̬����. 
* ��. 
* @param[in]   addr:��ַ,uid_ptr:UIDָ��. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-08-06���� 
*/
void comm_fill_evt_addr(uint16_t addr, uint8_t *uid_ptr)
{
	if( (addr>=EVT_ADDR_MIN && addr<=EVT_ADDR_MAX) || addr==0x00 ){
		
		for( uint8_t i=0; i<EVT_NUM; i++ ){
			///<�Ѿ����ڵ�ǰ�������ݵĵ�ַ
			if( comm_state.evt_state[i].addr == addr ){
				return;
			}
		}
		
		///<���ӵ���һ��EVT״̬������
		if( FUN_OK != comm_evt_is_registration_of_uid(uid_ptr) ){
			for( uint8_t i=0; i<EVT_NUM; i++ ){
				if( (comm_state.evt_state[i].addr == EVT_ADDR_NULL) || comm_state.evt_state[i].addr==0x00 ){
					comm_state.evt_state[i].addr = addr;
					comm_log("set evt new addr %02x\r\n",comm_state.evt_state[i].addr);
					return;
				}
			}
		}
	}
}

/** 
* �ж��Ƿ�ǰ���������Ƿ����. 
* ��. 
* @param[in]   msg_process_ptr:��������ָ��. 
* @param[out]  ��.  
* @retval  FUN_OK:������FUN_ERROR��û�н���.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-08-06���� 
*/
FUN_STATUS_T comm_is_cmd_sequence_complete( COMM_MSG_PROCESS_T * msg_process_ptr)
{
	if( msg_process_ptr->direction==0 && (strcmp(msg_process_ptr->cmd_code,"")==0 ))
		return FUN_OK;
	else
		return FUN_ERROR;
}

//uint8_t comm_is_candidate_end(uint16_t num)
//{
//	if( num >= sizeof(candidate_test)/sizeof(candidate_test[0]) ){
//		return 1;
//	}
//	else{
//		return 0;
//	}
//}

/** 
* �ж�EVT�豸�б����Ƿ�������EVT��ַ��ע��. 
* ��. 
* @param[in]   addr: ��ǰ��ѯ��ַ. 
* @param[out]  ��.  
* @retval  1����EVT�豸����ע�ᣬ0:û��EVT�豸��ע��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-08-23���� 
*/
FUN_STATUS_T comm_is_evt_reging(uint8_t addr)
{
	uint8_t reg_cnt=0;
	COMM_EVT_STATE_T *comm_evt_state_ptr;
	
	///<�����Ѿ�ʹ�õ�ע��ͨ����
	for( uint8_t i=0; i<REG_MAX; i++ ){
		if( comm_state.reging_chn&(1<<i))
			reg_cnt ++;
	}
//	if( reg_cnt >= REG_MAX )
//		return 1;
//	
//	reg_cnt = 0;
//	comm_evt_state_ptr = comm_state.evt_state;
//	for( uint8_t i=0; i<EVT_NUM; i++ ){
//		if( (comm_evt_state_ptr->step > EVT_STEP_BEGIN && comm_evt_state_ptr->step < EVT_STEP_POLL ) && (comm_evt_state_ptr->addr!=addr) ){
//			reg_cnt++;
//		}
//	}
	
	if( reg_cnt >= REG_MAX )
		return FUN_OK;
	else
		return FUN_ERROR;
}

/** 
* �����е�EVT���ͽ���ͶƱ����. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  FUN_STATUS_T״̬.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-11-29���� 
*/
FUN_STATUS_T comm_end_election(void)
{
	COMM_END_VOTING_T end_voting;
	memcpy(end_voting.ebb_id, config.ebb_id, sizeof(((COMM_END_VOTING_T*)0)->ebb_id) );
	memcpy(end_voting.vote_deactivation_key, default_password.vote_deactivation_key, sizeof(((COMM_END_VOTING_T*)0)->vote_deactivation_key) );
	
	COMM_EVT_STATE_T *comm_evt_state_ptr;
	
	comm_evt_state_ptr = comm_state.evt_state;
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		
		if( comm_evt_state_ptr->step == EVT_STEP_POLL ){
		
//			///<�ر�ͶƱǰ��ȡ����ͶƱ����
//			comm_pack(comm_evt_state_ptr->addr, config.ebb_addr, RF_EVT_CHN_POLL, (char*)CMD_REQ_CUR_VOTE_TALLY, NULL, 0);
//			vTaskDelay( 500 );
//			comm_pack(comm_evt_state_ptr->addr, config.ebb_addr, comm_evt_state_ptr->send_chn, (char*)CMD_REQ_CLOSE_ELECTION,\
//				(uint8_t*)&end_voting, sizeof(COMM_END_VOTING_T));
//			vTaskDelay( 500 );
//			
//			uint8_t time=0;
//			while( (comm_evt_state_ptr->step != EVT_STEP_CLOSE) && (time++<100) ){
//				vTaskDelay( 10 );
//			}

			//<�ر�ͶƱǰ��ȡ����ͶƱ����
			uint8_t cnt;
			uint8_t time;
			do{
				comm_pack(comm_evt_state_ptr->addr, config.ebb_addr, comm_evt_state_ptr->send_chn, (char*)CMD_REQ_FINAL_VOTE_TALLY,\
					(uint8_t*)&end_voting, sizeof(COMM_END_VOTING_T));
				
				///�ȴ������е�������������
				while( uxQueueMessagesWaiting(send_queue)!=0 ){
					vTaskDelay( 200 );
				}

				time = 0;
				while( (comm_evt_state_ptr->step != EVT_STEP_CLOSE) && (time++<100) ){
					vTaskDelay( 10 );
				}
			}while( (cnt++<3) && (comm_evt_state_ptr->step != EVT_STEP_CLOSE) );
		}
		comm_evt_state_ptr++;
	}
	
	return FUN_OK;
}

/** 
* ִ��EVTע�ἰ������������. 
* ��. 
* @param[in]   comm_evt_state_ptr:����EVT������ݽṹָ�룬comm_recv:�������ݻ���ָ��. 
* @param[out]  ��.  
* @retval  1����������ִ�н�����0:����ִ������û�����.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
uint8_t comm_evt_cmd_sequence(COMM_EVT_STATE_T	*comm_evt_state_ptr)
{
	
	COMM_MSG_PROCESS_T		*msg_process_ptr;
	msg_process_ptr = (COMM_MSG_PROCESS_T*)comm_evt_state_ptr->msg_process_ptr;
	
	COMM_HEADER_T		*comm_recv;
	
	if( msg_process_ptr==NULL ){
		free( comm_evt_state_ptr->recv_data );
		comm_evt_state_ptr->recv_data = NULL;
		return 0;
	}
	
	comm_recv = (COMM_HEADER_T*)comm_evt_state_ptr->recv_data;
	if (msg_process_ptr->direction == EVT2EBB) {

		if (comm_evt_state_ptr->recv_data != NULL) {

			///<���ͺ�ѡ���������ճɹ�
			if (strncmp("   ", msg_process_ptr->cmd_code, 3) == 0){

				if (strncmp(comm_recv->command, "000", 3) == 0){

					comm_log("addr %02x candidate recv comp \r\n",comm_evt_state_ptr->addr);
					if( FUN_NEND==devfs_candidates_end(comm_evt_state_ptr->comm_evt_data.vote_type,comm_evt_state_ptr->candidate_count) )
						msg_process_ptr--;
					else
						msg_process_ptr++;
					
					comm_evt_state_ptr->interaction_timeout = 0;
				}
			}
			///<�Ƚ�EVT��������ǰ���
			else if (strncmp(comm_recv->command, msg_process_ptr->cmd_code,3) == 0) {

				///<��������
				if (msg_process_ptr->message != NULL && msg_process_ptr->msg_len != 0) {
					memcpy(msg_process_ptr->message, (uint8_t*)comm_evt_state_ptr->recv_data + sizeof(COMM_HEADER_T), msg_process_ptr->msg_len);
				}
				
				if (msg_process_ptr->msg_len == 0) {
				}
				else {

				}

				///<ִ������ص�����
				if (msg_process_ptr->func != NULL) {
					uint8_t reply;
					reply = msg_process_ptr->func(comm_evt_state_ptr,COMM_YES,(uint8_t*)comm_evt_state_ptr->recv_data + sizeof(COMM_HEADER_T));
					if( reply == REPY_NONE ){
						msg_process_ptr++;
					}
					else if( reply == REPY_FIRST ){
						msg_process_ptr++;
					}
					else if( reply == REPY_SECOND ){
						msg_process_ptr++;
					}
					else if( reply == REPY_ERROR ){
						comm_log("addr %02x cmd sequence interrupt\r\n",comm_recv->source_address&0xff);
						if( comm_evt_state_ptr->step != EVT_STEP_SETTING ){
							clr_chn_reging(comm_evt_state_ptr->reg_chn);
							free(comm_evt_state_ptr->recv_data);
							memset(comm_evt_state_ptr,0,sizeof(COMM_EVT_STATE_T));
							comm_evt_state_ptr->addr = EVT_ADDR_NULL;
						}
						else{
							clr_chn_reging(comm_evt_state_ptr->reg_chn);
							free(comm_evt_state_ptr->recv_data);
							comm_evt_state_ptr->msg_process_ptr = comm_evt_state_ptr->msg_process_begin;
						}
						return 0;
					}
				}
				else{
					msg_process_ptr++;
				}
				
				comm_evt_state_ptr->interaction_timeout = 0;
			}
			///<�Ƚ�EVT�����������
			else if (strncmp(comm_recv->command, &msg_process_ptr->cmd_code[4], 3) == 0) {

				///<��������
				if (msg_process_ptr->message != NULL && msg_process_ptr->msg_len != 0) {
					memcpy(msg_process_ptr->message, (uint8_t*)comm_evt_state_ptr->recv_data + sizeof(COMM_HEADER_T), msg_process_ptr->msg_len);
				}
				
				if (msg_process_ptr->msg_len == 0) {
				}
				else {

				}

				///<ִ������ص�����
				if (msg_process_ptr->func != NULL) {
					uint8_t reply;
					reply = msg_process_ptr->func(comm_evt_state_ptr,COMM_NO,(uint8_t*)comm_evt_state_ptr->recv_data + sizeof(COMM_HEADER_T));
					if( reply == REPY_NONE ){
						msg_process_ptr++;
					}
					else if( reply == REPY_FIRST ){
						msg_process_ptr++;
					}
					else if( reply == REPY_SECOND ){
						msg_process_ptr++;
					}
					else if( reply == REPY_ERROR ){
						comm_log("addr %02x cmd sequence interrupt\r\n",comm_recv->source_address&0xff);
						free(comm_evt_state_ptr->recv_data);
						memset(comm_evt_state_ptr,0,sizeof(COMM_EVT_STATE_T));
						comm_evt_state_ptr->addr = EVT_ADDR_NULL;
						clr_chn_reging(comm_evt_state_ptr->reg_chn);
						comm_state.enable_poll = 1;
						return 0;
					}
				}
				else{
					msg_process_ptr++;
				}
				
				comm_evt_state_ptr->interaction_timeout = 0;
			}
			char cmd_buf[4]={0};
			memcpy(cmd_buf,comm_recv->command,3 );
			comm_log("addr %02x recv cmd %3s msgid %d\r\n",comm_recv->source_address&0xff,cmd_buf,comm_recv->message_id);

			free(comm_evt_state_ptr->recv_data);
			comm_evt_state_ptr->recv_data = NULL;
		}
	}
	else if (msg_process_ptr->direction == EBB2EVT) {
		
		///<���ͺ�ѡ������
		if (strncmp("   ", msg_process_ptr->cmd_code, 3) == 0) {

			if ( FUN_NEND == devfs_candidates_end(comm_evt_state_ptr->comm_evt_data.vote_type,comm_evt_state_ptr->candidate_count) ) {

				if (msg_process_ptr->func != NULL) {
					msg_process_ptr->func(comm_evt_state_ptr,COMM_NONE,NULL);
				}
				
				char candidate_buf[CANDIDATE_NAME_LEN];
				if( FUN_OK == devfs_read_candidates_name(comm_evt_state_ptr->comm_evt_data.vote_type, comm_evt_state_ptr->candidate_count, candidate_buf)){
					
					char cmd[3]={0};
					memcpy(cmd,candidate_buf,3);
					char name[CANDIDATE_NAME_LEN+1];
					strncpy(name,&candidate_buf[4],CANDIDATE_NAME_LEN);
					
					///<������ͺ���
					comm_pack(comm_evt_state_ptr->addr, config.ebb_addr, comm_evt_state_ptr->send_chn, cmd, name, strlen(name));
					
					comm_log("addr %02x candidate %d \"%s\" msgid %d send comp \r\n",comm_evt_state_ptr->addr,comm_evt_state_ptr->candidate_count,name,comm_state.message_id-1);
					
					comm_evt_state_ptr->candidate_count++;
					msg_process_ptr ++;
				}
			}
			else
				msg_process_ptr += 2;
			
			comm_evt_state_ptr->interaction_timeout = 0;
		}
		else {
			uint32_t reply = REPY_FIRST;
			char cmd[4]={0};

			///<���лص����������÷����ж�״̬
			if (msg_process_ptr->func != NULL) {
				reply = msg_process_ptr->func(comm_evt_state_ptr,COMM_NONE,NULL);
			}

			if (reply == REPY_NONE){
				memcpy(cmd, msg_process_ptr->cmd_code, 3);
			}
			///<��EVT����ǰ�������
			else if( reply == REPY_FIRST ){
				memcpy(cmd, msg_process_ptr->cmd_code, 3);
			}
			///<��EVT���غ�������
			else if (reply == REPY_SECOND){
				memcpy(cmd, &msg_process_ptr->cmd_code[4], 3);
			}
			else if (reply == REPY_ERROR){
				comm_log("The registration channel is occupied\r\n");
//				memset(comm_evt_state_ptr,0,sizeof(COMM_EVT_STATE_T));
				comm_evt_state_ptr->addr = EVT_ADDR_NULL;
				comm_state.enable_poll = 1;
				return 0;
			}

			///<������ͺ���
			comm_pack(comm_evt_state_ptr->addr, config.ebb_addr, comm_evt_state_ptr->send_chn, cmd, msg_process_ptr->message, msg_process_ptr->msg_len);
			
			comm_log("addr %02x chn %02x send cmd %3s msgid %d\r\n", comm_evt_state_ptr->addr, comm_evt_state_ptr->send_chn, cmd, comm_state.message_id-1);
				
			msg_process_ptr++;
			
			comm_evt_state_ptr->interaction_timeout = 0;
		}
	}
	else if (msg_process_ptr->direction == 0) {
		
		msg_process_ptr = NULL;
	}
	
	///<����EVT״̬�ṹָ��
	comm_evt_state_ptr->msg_process_ptr = msg_process_ptr;
	
	///<���������ʱ����λ����ָ��
	if( (comm_evt_state_ptr->interaction_timeout++ > INTERACTION_TIMEOUT) && (comm_evt_state_ptr->msg_process_ptr!=comm_evt_state_ptr->msg_process_begin) ){
		
		comm_evt_state_ptr->interaction_timeout = 0;
		comm_log("reset the interaction pointer\r\n");
		char cmd[3]={0};
		memcpy(cmd,msg_process_ptr->cmd_code,3);
		comm_log("Curr step %d cmd %s\r\n",msg_process_ptr->step,cmd);
		comm_evt_state_ptr->msg_process_ptr = comm_evt_state_ptr->msg_process_begin;
		clr_chn_reging(comm_evt_state_ptr->reg_chn);
		
		char buf[DISP_BUF_SIZE];
		if( comm_evt_state_ptr->msg_process_begin==register_msg_code ){
			snprintf(buf,DISP_BUF_SIZE,"\n\n\n%d REG TIMEOUT",comm_evt_state_ptr->comm_evt_data.rf_id);
			display_screen_info_in(menu_func_cnct_new_evt,buf);
		}
		else if( (comm_evt_state_ptr->msg_process_begin==setting_msg_code) && (comm_evt_state_ptr->comm_evt_data.vote_type!=BACKUP_DEVICE) ){
			snprintf(buf,DISP_BUF_SIZE,"\n\n\n%d SET TIMEOUT",comm_evt_state_ptr->comm_evt_data.rf_id);
			display_screen_info_in(menu_func_cnct_new_evt,buf);
		}
	}
	
	///<�ж�����ִ�������Ƿ����
	if( FUN_OK == comm_is_cmd_sequence_complete((COMM_MSG_PROCESS_T*)comm_evt_state_ptr->msg_process_ptr) ){
		comm_evt_state_ptr->msg_process_ptr = NULL;
		
		char buf[DISP_BUF_SIZE];
		if( comm_evt_state_ptr->msg_process_begin==register_msg_code ){
			snprintf(buf,DISP_BUF_SIZE,"\n\n\nREG COMPLETE");
		}
		else if( comm_evt_state_ptr->msg_process_begin==setting_msg_code ){
			snprintf(buf,DISP_BUF_SIZE,"\n\n\nSET COMPLETE");
		}
		display_screen_info_in(menu_func_cnct_new_evt,buf);
		
		return 1;
	}
	else{
		return 0;
	}
}

/** 
* ��ȡ�Ѿ�ע��ɹ���������ѯ״̬�µ�EV��ַ. 
* ��. 
* @param[in]   ��. 
* @param[out]  �����ַָ��.  
* @retval  ��ַ����.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-09-16���� 
*/
uint8_t	comm_get_addr_list(uint8_t *list_ptr)
{
	uint8_t cnt=0;
	for(uint8_t i=0; i<EVT_NUM; i++){
		if( comm_state.evt_state[i].step >= EVT_STEP_POLL ){
			cnt++;
			*list_ptr = comm_state.evt_state[i].comm_evt_data.rf_id;
			list_ptr++;
		}
	}
	return cnt;
}

/** 
* ���ö�Ӧ��EVT״̬. 
* ��. 
* @param[in]   addr:�����õ�EVT��ַ��sta������״̬,EVT_TRAN_INIT,EVT_TRAN_TIMEOUT,EVT_TRAN_OK. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-09-16���� 
*/
void comm_evt_tran_state(uint8_t addr, COMM_TRAN_T sta)
{
	static xSemaphoreHandle tran_state_xSemaphore=NULL;
	if( tran_state_xSemaphore==NULL ){
		tran_state_xSemaphore = xSemaphoreCreateMutex();
	}
	
	xSemaphoreTake(tran_state_xSemaphore,portMAX_DELAY);
	
	for(uint8_t i=0; i<EVT_NUM; i++){
		if( comm_state.evt_state[i].comm_evt_data.rf_id == addr ){
			if( sta > comm_state.evt_state[i].comm_status.tran_state ){
				comm_state.evt_state[i].comm_status.tran_state = sta;
			}
		}
	}
	
	xSemaphoreGive(tran_state_xSemaphore);
}

/** 
* ��ʼ��EVT����״̬. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-09-16���� 
*/
void comm_evt_tran_state_init(void)
{
	for(uint8_t i=0; i<EVT_NUM; i++){
		if( comm_state.evt_state[i].step == EVT_STEP_POLL ){
			comm_state.evt_state[i].comm_status.tran_state = EVT_TRAN_INIT;
		}
	}
}

//void comm_send_token( void )
//{
//	COMM_EVT_STATE_T *evt_state_ptr = comm_state.evt_token;
//	uint8_t		cnt=0;
//	do{
//		evt_state_ptr++;
//		cnt++;
//		
//		if( (evt_state_ptr - &comm_state.evt_state[0]) == EVT_NUM ){
//			evt_state_ptr = &comm_state.evt_state[0];
//		}
//		
//		if( evt_state_ptr->step == EVT_STEP_POLL ){
//			comm_state.evt_token = evt_state_ptr;
//			return ;
//		}
//	}while( (evt_state_ptr != comm_state.evt_token) && (cnt<EVT_NUM) );
//}

/** 
* EVTע�ᣬ���ã�״̬��ѯ. 
* ��. 
* @param[in]   comm_evt_state_ptr:����EVT������ݽṹָ��. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-25���� 
*/
void comm_evt_poll(COMM_EVT_STATE_T	*comm_evt_state_ptr)
{		
	if( (comm_evt_state_ptr->poll_time++)%POLL_TIME == 0 ){
		
		///<��÷�������
//			if( comm_state.evt_token == comm_evt_state_ptr && comm_evt_state_ptr->comm_wait_msg_sta.msg_reply==0 ){
		
		if( get_election_state() == ECECTION_DURING ){
			///<ÿ��30���ӷ��ͻ�ȡͶƱ��������
			if( (comm_evt_state_ptr->poll_time > VOTE_TALLY_TIME) && (comm_evt_state_ptr->comm_poll_sta.poll_sending==0) ){
				comm_evt_state_ptr->poll_time = 0;
				
				comm_evt_state_ptr->comm_poll_sta.poll_sending = 1;
				comm_evt_state_ptr->comm_poll_sta.sending_time = 0;
				
				if( FUN_OK == comm_pack(comm_evt_state_ptr->addr, config.ebb_addr, RF_EVT_CHN_POLL, (char*)CMD_REQ_CUR_VOTE_TALLY, NULL, 0) ){
					
					comm_evt_state_ptr->comm_status.poll_send++;
					comm_evt_state_ptr->comm_wait_msg_sta.msg_reply = 1;
					comm_log("poll evt %02x vote tally cmd\r\n",comm_evt_state_ptr->addr);
				}
			}
		}
		
		///<ÿ��1�뷢�ͻ�ȡEVT״̬����
		if( comm_evt_state_ptr->comm_poll_sta.poll_sending==0 ){
			
			comm_evt_state_ptr->comm_poll_sta.poll_sending = 1;
			comm_evt_state_ptr->comm_poll_sta.sending_time = 0;
			
			if( FUN_OK == comm_pack(comm_evt_state_ptr->addr, config.ebb_addr, RF_EVT_CHN_POLL, (char*)CMD_REQ_STATUS, NULL, 0) ){
				
				comm_evt_state_ptr->comm_status.poll_send++;
				comm_evt_state_ptr->comm_wait_msg_sta.msg_reply = 1;
				comm_log("poll evt %02x request status cmd\r\n",comm_evt_state_ptr->addr);
			}
		}
	}
	
	if( comm_evt_state_ptr->recv_data != NULL ){
		
		COMM_HEADER_T *comm_header_ptr = (COMM_HEADER_T*)comm_evt_state_ptr->recv_data;
		
		comm_evt_state_ptr->comm_wait_msg_sta.msg_reply = 0;
		
		//��¼EVTͨ����󷵻�ʱ��
		if( comm_evt_state_ptr->comm_wait_msg_sta.msg_time > comm_evt_state_ptr->comm_status.poll_max_time ){
			
			comm_evt_state_ptr->comm_status.poll_max_time = comm_evt_state_ptr->comm_wait_msg_sta.msg_time;
		}
		
		if( comm_header_ptr->message_id == comm_evt_state_ptr->comm_wait_msg_sta.msg_id ){
			comm_evt_state_ptr->comm_status.poll_recv++;
//			comm_send_token();
			comm_log("poll evt recv %02x OK\r\n",comm_header_ptr->source_address&0xff);
		}
		else{
			comm_log("poll evt recv %02x ERR\r\n",comm_header_ptr->source_address&0xff);
			comm_evt_state_ptr->comm_status.poll_id_err++;
		}
		
		///<���ͷ���ͶƱ���ݲ�������Ӧ��EVT�洢�ṹ
		if( strncmp(comm_header_ptr->command,CMD_TRAN_CUR_VOTE_TALLY,3)==0 || \
			strncmp(comm_header_ptr->command,CMD_TRAN_FINAL_VOTE_TALLY,3)==0 || \
			strncmp(comm_header_ptr->command,CMD_TRAN_ALL_VOTE_TALLY,3)==0)
		{
			comm_evt_state_ptr->comm_poll_sta.poll_sending = 0;
			
			COMM_TOTAL_TALLY_T *comm_total_tally_ptr = (COMM_TOTAL_TALLY_T *)((uint8_t*)comm_evt_state_ptr->recv_data + sizeof(COMM_HEADER_T));
			COMM_CAND_TALLY_T *comm_cand_tally_ptr = (COMM_CAND_TALLY_T *)((uint8_t*)comm_evt_state_ptr->recv_data + sizeof(COMM_HEADER_T) + sizeof(COMM_TOTAL_TALLY_T));
			COMM_SHA_T *comm_sha_ptr = (COMM_SHA_T *)((uint8_t*)comm_evt_state_ptr->recv_data + sizeof(COMM_HEADER_T) + sizeof(COMM_TOTAL_TALLY_T) + sizeof(COMM_CAND_TALLY_T)*CANDIDATE_MAX);
			
			if( comm_evt_state_ptr->time_stamp != comm_total_tally_ptr->time_stamp ){
				
				comm_evt_state_ptr->time_stamp = comm_total_tally_ptr->time_stamp;

				unsigned char decrypt[HASH_KEY_LEN];
				uint16_t sha_len = sizeof(COMM_TOTAL_TALLY_T) + sizeof(COMM_CAND_TALLY_T)*CANDIDATE_MAX;
				
				hash_count((uint8_t*)comm_total_tally_ptr, sha_len, decrypt);

	//			comm_dump(comm_evt_state_ptr->recv_data,sizeof(COMM_HEADER_T)+sizeof(COMM_TOTAL_TALLY_T)+sizeof(COMM_CAND_TALLY_T)*election_status.candidates_num+sizeof(COMM_SHA_T));
				
				if( strncmp(comm_header_ptr->command,CMD_TRAN_FINAL_VOTE_TALLY,3)==0 ){
					comm_evt_state_ptr->step = EVT_STEP_CLOSE;
				}
				
				if (memcmp(comm_sha_ptr->sha, decrypt, HASH_KEY_LEN) == 0){
					
					COMM_EVT_STATE_T *evt_state_ptr;
					evt_state_ptr = comm_get_evt_handle_of_addr(comm_header_ptr->source_address);
					if( evt_state_ptr != NULL ){
						COMM_VOTE_TALLY_T *vote_tally_ptr = &evt_state_ptr->vote_tally;

						vote_tally_ptr->total_cnt = comm_total_tally_ptr->total_cnt;
						vote_tally_ptr->total_voters = comm_total_tally_ptr->total_voters;
						vote_tally_ptr->no_voters = comm_total_tally_ptr->no_voters;

						for (uint8_t i = 0; i < CANDIDATE_MAX; i++) {
							
							///<����ͶƱ���ݵ��ڴ�
							vote_tally_ptr->vote_candidate[i].sta = comm_cand_tally_ptr->state_char;
							vote_tally_ptr->vote_candidate[i].increased = comm_cand_tally_ptr->inc_cnt;
							vote_tally_ptr->vote_candidate[i].total = comm_cand_tally_ptr->total_cnt;
							
							///<�ۼ�ͶƱ���ݵ�vote tally
	//						devfs_vote_tallies_acc(evt_state_ptr->comm_evt_data.vote_type,i,vote_tally_ptr->vote_candidate[i].increased);
	//						
	//						///<����ͶƱ���ݵ��ļ�
	//						devfs_site_config_write_tally(comm_evt_state_ptr->site_config_file_name,i,comm_cand_tally_ptr->total_cnt);

							comm_cand_tally_ptr++;
						}
						
						devfs_vote_tallies_save_all(evt_state_ptr->comm_evt_data.vote_type,vote_tally_ptr);
						devfs_site_config_save_tally(comm_evt_state_ptr->site_config_file_name,vote_tally_ptr);
					}
				}
			}
		}
		///<ͶƱ������Э��
		else if( strncmp( comm_header_ptr->command, CMD_ASSISTANCE,3 ) == 0 ){
			if( comm_evt_state_ptr->last_evt_flag.bit.help_detect==0 ){
				
				comm_evt_state_ptr->last_evt_flag.bit.help_detect = 1;
				
				comm_evt_state_ptr->evt_flag.bit.help_detect = 1;
				comm_evt_state_ptr->evt_flag.bit.state_ok = 0;
							
				char info[50];
				snprintf(info,50,"NO.%d EVT REQUEST ASSISTANCE",comm_header_ptr->source_address&0xff);
				display_info_in(info,1);
				
				event_log_write(STANDARD_LEVEL, COMMUNICATION_TYPE, "Assistance called for EVT");
				
				beep_set_alarm(BP_ASSIST_REQ);
			}
		}
		///<EVT�۸ļ��
		else if( strncmp( comm_header_ptr->command, CMD_TAMPER,3 ) == 0 ){
			if( comm_evt_state_ptr->last_evt_flag.bit.tamper_detect==0 ){
				
				comm_evt_state_ptr->last_evt_flag.bit.tamper_detect=1;
				
				comm_evt_state_ptr->evt_flag.bit.tamper_detect = 1;
				comm_evt_state_ptr->evt_flag.bit.state_ok = 0;
							
				char info[50];
				snprintf(info,50,"NO.%d EVT TAMPER DETECT",comm_header_ptr->source_address&0xff);
				display_info_in(info,1);
			}
		}
		///<��ӡ��ûֽ
		else if( strncmp( comm_header_ptr->command, CMD_PAPER_LOW,3 ) == 0 ){
			if( comm_evt_state_ptr->last_evt_flag.bit.low_paper==0 ){
				
				comm_evt_state_ptr->last_evt_flag.bit.low_paper = 1;
				
				comm_evt_state_ptr->evt_flag.bit.low_paper = 1;
				comm_evt_state_ptr->evt_flag.bit.state_ok = 0;
							
				char info[50];
				snprintf(info,50,"NO.%d EVT PAPER LOW",comm_header_ptr->source_address&0xff);
				display_info_in(info,1);
			}
		}
		///<��ص�����
		else if( strncmp( comm_header_ptr->command, CMD_BATTERY_LOW,3 ) == 0 ){
			if( comm_evt_state_ptr->last_evt_flag.bit.low_power==0 ){
				
				comm_evt_state_ptr->last_evt_flag.bit.low_power = 1;
				
				comm_evt_state_ptr->evt_flag.bit.low_power = 1;
				comm_evt_state_ptr->evt_flag.bit.state_ok = 0;
				
				char info[50];
				snprintf(info,50,"NO.%d EVT BATTERY LOW",comm_header_ptr->source_address&0xff);
				display_info_in(info,1);
			}
		}
		///<����״̬OK��մ���״̬��־
		else if( strncmp(comm_header_ptr->command,CMD_STATUS_OK_RPY,3)==0 ){
			comm_evt_state_ptr->evt_flag.byte = 0;
			comm_evt_state_ptr->last_evt_flag.byte = 0;
			comm_evt_state_ptr->evt_flag.bit.state_ok = 1;
		}
		else if( strncmp(comm_header_ptr->command,CMD_CLOSE_ELECTION_RPY,3)==0 ){
			comm_evt_state_ptr->step = EVT_STEP_CLOSE;
		}
		else{
			
		}
		
		///<�������EVT���������ͷ��ڴ棬�ý�������ָ��Ϊ��
		free( comm_evt_state_ptr->recv_data );
		comm_evt_state_ptr->recv_data = NULL;
	}
	
	///<����ʱ��ѯ״̬ʱ�䣬���ʱ����û���յ����ݲ�����µ���ѯ״̬����
	if( comm_evt_state_ptr->comm_poll_sta.poll_sending ){
		comm_evt_state_ptr->comm_poll_sta.sending_time++;
		if( comm_evt_state_ptr->comm_poll_sta.sending_time > 500 )
			comm_evt_state_ptr->comm_poll_sta.poll_sending = 0;
	}
	
	///<�����Ϣ�Ƿ�ʱ
	if( comm_evt_state_ptr->comm_wait_msg_sta.msg_reply ){
		
		if( comm_evt_state_ptr->comm_wait_msg_sta.msg_time++ > EVT_MSG_WAIT_TIME ){
			comm_evt_state_ptr->comm_wait_msg_sta.msg_reply = 0;
			comm_evt_state_ptr->comm_status.poll_timeout++;
		}
	}
}

/** 
* ��Ϣ���ʹ���. 
* ˳������Ϣ���ȴ��ظ�. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-10-29���� 
*/
void comm_send_cmd_process(void)
{
	COMM_SEND_MSG_T msg;
	static COMM_EVT_STATE_T* comm_evt_state_ptr;
	static uint8_t step=0;
	static uint16_t send_time=0;
	
	switch( step ){
		case 0:
			if( comm_send_out_queue(&msg)==FUN_OK ){
				
				///<���浱ǰEVT�������ݵ�MSG ID
				comm_evt_state_ptr = comm_save_curr_msgid(msg.target_addr,msg.msg_id);
				if( comm_evt_state_ptr != NULL ){
					comm_crypt_send(msg.target_addr, msg.source_addr, msg.chn, msg.buf, msg.len);
					step = 1;
					send_time = 0;
				}
			}
			break;
		case 1:
			if( comm_evt_state_ptr->comm_wait_msg_sta.msg_reply == 0 ){
				step = 0;
			}
			else if( send_time++>100 ){
				step = 0;
			}
			break;
		default:
			break;
	}
}

/** 
* ע��ɹ�EVT����. 
* ��. 
* @param[in]   i_d=1ע��EVT������1��i_d=0:ע��EVT������1. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-12-26���� 
*/
void comm_reg_evt_num(uint8_t i_d)
{
	if( i_d ){
		comm_state.evt_num.reg++;
	}
	else if( i_d==0 ){
		if( comm_state.evt_num.reg>0 ){
			comm_state.evt_num.reg--;
		}
	}
}


/** 
* ����Ӧ���Ѿ�ע�����õ�EVT������1,��������1. 
* ��. 
* @param[in]   comm_evt_state_ptr:EVT���ָ��. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-12-26���� 
*/
void comm_inc_evt_num(COMM_EVT_STATE_T	*comm_evt_state_ptr)
{
	switch( comm_evt_state_ptr->comm_evt_data.vote_type ){
		case VOTE_TYPE1:
			comm_state.evt_num.t1++;
			break;
		case VOTE_TYPE2:
			comm_state.evt_num.t2++;
			break;
		case VOTE_TYPE3:
			comm_state.evt_num.t3++;
			break;
		case VOTE_TYPE4:
			comm_state.evt_num.t4++;
			break;
	}
	comm_state.evt_num.total++;
}

/** 
* ע�ᣬ�����������ѯ���������߳�. 
* ��. 
* @param[in]   pvParameters:�̲߳���. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*	   ken deng 2019-08-28:ע�����EVT������ע�ᣬ��������EBB
*/
void comm_poll_task( void *pvParameters )
{
	wait_for_startup();
	
	TickType_t ticks = xTaskGetTickCount();
	void *recv_data=NULL;
	uint16_t 	len;
	uint8_t		evt_dev=0;
	COMM_EVT_STATE_T	*comm_evt_state_ptr;
	COMM_HEADER_T		*comm_header_ptr;
	
	while( 1 ){
		
//		rf_module_send(0x1a470501,1,"1234567890",10);
		
		if( (recv_data=comm_crypt_recv(&len))!=NULL ){
			
			comm_header_ptr = (COMM_HEADER_T*)recv_data;
			
			COMM_EVT_STATE_T *evt_handle;
			uint8_t addr = comm_header_ptr->source_address&0xff;
			
			///<����EVT״̬���еĵ�ַ
			comm_fill_evt_addr(addr,recv_data+sizeof(COMM_HEADER_T));
			
			///<����EVT���еĵ�ַ�����ý������ݵĻ���ָ��
			if( (evt_handle = comm_find_evt_addr(addr,recv_data+sizeof(COMM_HEADER_T)))!=NULL ){
				
				///<����Ӧ��EVT��������ָ�����ã�������ɺ���NULL
				if( evt_handle->recv_data == NULL ){
					evt_handle->recv_data = recv_data;
					
					///<ʶ���ȡ��ַ���EVT�����ظ�����ע��
					if( strncmp(comm_header_ptr->command,register_msg_code[0].cmd_code,3)==0 ){
						evt_handle->step = EVT_STEP_BEGIN;
					}
					
//					if( strncmp(comm_header_ptr->command,CMD_TRAN_CUR_VOTE_TALLY,3)==0 ){
//						evt_handle->recv_data = NULL;
//						free(recv_data);
//					}
				}
				///<�����ǰ��EVT���ݻ�û������ɣ����������ͷ��ڴ�
				else{
					free(recv_data);
					comm_log("free buf 1\r\n");
				}
			}
			///<�����ǰ���Ҳ�����Ӧ��EVT�ṹ�������ݣ����������ͷ��ڴ�
			else{
				free(recv_data);
				comm_log("free buf 2\r\n");
			}
		}
		
		///<ѭ��������EVT��״̬
		comm_evt_state_ptr = &comm_state.evt_state[0];
		
		for( evt_dev=0; evt_dev<EVT_NUM; evt_dev++ ){

			if( comm_evt_state_ptr->addr !=  EVT_ADDR_NULL  ){
				
				///<����EVTע��
				if( comm_evt_state_ptr->step == EVT_STEP_BEGIN ){
						
					if( FUN_OK != comm_evt_is_register(comm_evt_state_ptr->addr) ){
						
						comm_evt_state_ptr->msg_process_ptr = register_msg_code;
						comm_evt_state_ptr->msg_process_begin = register_msg_code;
						comm_evt_state_ptr->step = EVT_STEP_REG;
						comm_state.enable_poll = 0;
						
						config_evt_update(comm_evt_state_ptr);
					}
					else{
						
						comm_evt_state_ptr->msg_process_ptr = setting_msg_code;
						comm_evt_state_ptr->msg_process_begin = setting_msg_code;
						comm_evt_state_ptr->step = EVT_STEP_SETTING;
						comm_state.enable_poll = 0;
						
						config_evt_update(comm_evt_state_ptr);
					}
				}
				else if( comm_evt_state_ptr->step == EVT_STEP_REG ){
						
					if( comm_evt_cmd_sequence(comm_evt_state_ptr) ){
						comm_evt_state_ptr->msg_process_ptr = setting_msg_code;
						comm_evt_state_ptr->msg_process_begin = setting_msg_code;
						comm_evt_state_ptr->step = EVT_STEP_SETTING;
						comm_state.enable_poll = 0;
						comm_reg_evt_num(1);
						
						config_evt_update(comm_evt_state_ptr);
					}
				}
				///<����EVT����
				else if( comm_evt_state_ptr->step == EVT_STEP_SETTING ){
					///<��û����
					if( FUN_OK != comm_evt_is_setting(comm_evt_state_ptr->addr) ){
						if( comm_evt_cmd_sequence(comm_evt_state_ptr) ){
							comm_evt_state_ptr->step = EVT_STEP_POLL;
							///<��û���������ģʽ��EVT�����ۼ�
							if( comm_evt_state_ptr->worked==0 ){
								comm_inc_evt_num(comm_evt_state_ptr);
							}
							comm_reg_evt_num(0);
							config_evt_update(comm_evt_state_ptr);
							
							char buf[DISP_BUF_SIZE];
							uint32_t addr = EVT_RF_TARGET_ADDR+comm_evt_state_ptr->comm_evt_data.rf_id;
							snprintf(buf,DISP_BUF_SIZE,"\n\n\nNew Evt:%08X",addr);
							display_screen_info_in(menu_func_cnct_new_evt,buf);
						}
					}
					///<�Ѿ�����
					else{
						comm_evt_state_ptr->step = EVT_STEP_POLL;
						///<��û���������ģʽ��EVT�����ۼ�
						if( comm_evt_state_ptr->worked==0 ){
							comm_inc_evt_num(comm_evt_state_ptr);
						}
						comm_reg_evt_num(0);
						config_evt_update(comm_evt_state_ptr);
						
						char buf[DISP_BUF_SIZE];
						uint32_t addr = EVT_RF_TARGET_ADDR+comm_evt_state_ptr->comm_evt_data.rf_id;
						snprintf(buf,DISP_BUF_SIZE,"\n\n\nNew Evt:%08X",addr);
						display_screen_info_in(menu_func_cnct_new_evt,buf);
					}
				}
				///<��ѯEVT�豸
				else if( comm_evt_state_ptr->step == EVT_STEP_POLL ){
					
					comm_evt_state_ptr->worked = 1;
					comm_evt_poll(comm_evt_state_ptr);
				}
			}
			///<û�䵽��ַ,����ͨ��ʱ�䣬��λ��Ϣָ��
			else if( (comm_evt_state_ptr->interaction_timeout++ > 100) && (comm_evt_state_ptr->msg_process_ptr!=comm_evt_state_ptr->msg_process_begin) ){
				
				comm_evt_state_ptr->msg_process_ptr = comm_evt_state_ptr->msg_process_begin;
			}
			comm_evt_state_ptr++;
		}
		
		vTaskDelay( 10 );
	}
}

/** 
* ͨ��������߳�. 
* . 
* @param[in]   pvParameters:�̲߳���. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
void comm_send_task( void *pvParameters )
{
	wait_for_startup();
	
	TickType_t ticks = xTaskGetTickCount();
	while(1){
		comm_send_cmd_process();
		
		vTaskDelayUntil( &ticks, 1 );
	}	
}

/** 
* ͨѶģ���ʼ����������ʼ��. 
* . 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      ��
* @par �޸���־ 
*      ken deng��2019-07-29���� 
*/
FUN_STATUS_T comm_init(void)
{
	memset(&comm_state,0,sizeof(COMM_STATE_T));
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		comm_state.evt_state[i].addr = EVT_ADDR_NULL;
	}
//	comm_state.rf_id =  0x14;
	config_evt_restore();
	
	///<�����������Ƹ���һ��EVT�ռ�
//	comm_state.evt_token = &comm_state.evt_state[0];
	
	send_queue = xQueueCreate(COMM_SEND_QUEUE_NUM,sizeof(COMM_SEND_MSG_T));
	if( send_queue==NULL )
		return FUN_ERROR;
		
	recv_queue = xQueueCreate(COMM_RECV_QUEUE_NUM,sizeof(COMM_RECV_MSG_T));
	if( recv_queue==NULL )
		return FUN_ERROR;
	
	if( xTaskCreate( comm_poll_task, "COMM POLL", COMM_POLL_STACK_SIZE, NULL, COMM_POLL_TASK_PRIORITY, daemon_from_create ) != pdPASS)
		return FUN_ERROR;
	
	if( xTaskCreate( comm_send_task, "COMM SEND", COMM_SEND_STACK_SIZE, NULL, COMM_SEND_TASK_PRIORITY, daemon_from_create ) == pdPASS)
		return FUN_OK;
	else
		return FUN_ERROR;
}
