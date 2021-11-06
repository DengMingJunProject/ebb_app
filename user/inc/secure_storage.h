#ifndef __SECURE_STORAGE_H__
#define __SECURE_STORAGE_H__

#include "stdint.h"
#include "macro_def.h"

#pragma pack(push) //保存对齐状态
#pragma pack(1)   // 1 bytes对齐

#define 	CANDIDATE_NAME_LEN		100
#define		CANDIDATE_NUM			30
#define 	FLASH_STORAGE_ADDR		0x080E0000

#define		SE_USE_CRYPT
#define		SE_DEV					CRYPT_AT,

typedef struct{
	
	uint8_t			day;
	uint8_t			month;
	uint16_t		year;
	
}VOTE_DATE_T;

typedef struct{
	
	char			name[CANDIDATE_NAME_LEN];
	uint16_t		tv;
	
}VOTE_DATA_T;

typedef struct {
	VOTE_DATE_T		vote_date;
	uint8_t			vtn[50];
	uint8_t			av;
	uint8_t			ebb_id[12];
	uint16_t		total_tv;	
	uint8_t			candidate_num;
	uint16_t		total_voters;
	uint16_t		no_voters;
	VOTE_DATA_T		vote_data[CANDIDATE_NUM];
}CANDIDATE_T;

typedef enum{
	OP_DATE=0,
	OP_VTN,
	OP_AV,
	OP_EBBID,
	OP_TOTALTV,
	OP_NUM,
	OP_TOTALVOTERS,
	OP_NOVOTERS,
}OP_TYPE_T;

typedef enum{
	SC_WRITE=0,
	SC_READ,
}SC_RW_T;

typedef struct {
	char			name[100];
	uint8_t			reserve[28];
}CANDIDATE_ITEM_T;

typedef struct {
	uint16_t		last_tv[CANDIDATE_NUM];
}VOTE_STATUS_T;

typedef struct{
	uint8_t 		voter_key[32];					///<投票卡密钥
	uint8_t			encryption_key[32];				///<存储数据加密键
	uint8_t			encryption_transfer_key[32];	///<通讯数据加密键
	uint8_t			administrator_key[32];			///<管理都键
	uint8_t 		maintenance_key[32];			///<维修键
	uint8_t			system_administrator_key1[32];	///<系统管理员键1
	uint8_t			system_administrator_key2[32];	///<系统管理员键2
	uint8_t			system_administrator[32];
	uint8_t			sa_se_read_password2[0];
	uint8_t			se_rw_password_zone1[0];
	uint8_t			se_rw_password_zone2[0];
	uint8_t			se_rw_password_zone3[0];
	uint8_t			se_rw_password_zone4[0];
	uint8_t			se_authentication_key1[0];
	uint8_t			se_authentication_key2[0];
	uint8_t			bootloader_key[32];
	uint8_t			vote_deactivation_key[32];
	uint8_t			pc_settings_key[32];
	uint8_t			hashing_key[32];
	uint8_t			system_initial_authentication_key[32];
	uint8_t			system_authentication_key1[0];
	uint8_t			system_authentication_key2[0];
	uint32_t		crc;
}DEFAULT_PASSWORDS_T;

enum{
	VOTER_KEY=0,
	ENCRYPTION_KEY,
	ADMINISTRATOR_KEY,
	MAINTENANCE_KEY,
	SYSTEM_ADMINISTRATOR_KEY1,
	SYSTEM_ADMINISTRATOR_KEY2,
	SYSTEM_ADMINISTRATOR,
	SA_SE_READ_PASSWORD2,
	SE_RW_PASSWORD_ZONE1,
	SE_RW_PASSWORD_ZONE2,
	SE_RW_PASSWORD_ZONE3,
	SE_RW_PASSWORD_ZONE4,
	SE_AUTHENTICATION_KEY1,
	SE_AUTHENTICATION_KEY2,
	BOOTLOADER_KEY,
	VOTE_DEACTIVATION_KEY,
	PC_SETTINGS_KEY,
	HASHING_KEY,
	SYSTEM_INITIAL_AUTHENTICATION_KEY,
	SYSTEM_AUTHENTICATION_KEY1,
	SYSTEM_AUTHENTICATION_KEY2,
};

extern DEFAULT_PASSWORDS_T		default_password;
extern const DEFAULT_PASSWORDS_T		default_password_config;
extern VOTE_STATUS_T			vote_status;

extern void secure_storage_canddate_info( OP_TYPE_T op_type, uint8_t *ptr, SC_RW_T rw );
extern uint8_t secure_storage_canddate_name( uint16_t num, uint8_t len, uint8_t *ptr, SC_RW_T rw );
extern void secure_storage_canddate_count( uint16_t num, uint8_t *ptr, SC_RW_T rw );
extern void secure_storage_canddate_inc(uint16_t num, uint8_t value);
extern void secure_storage_voter_inc(uint8_t value);
extern void secure_storage_novoter_inc(uint8_t value);
extern void secure_storage_totaltv_inc(uint8_t value);
extern int8_t secure_storage_init(void);
extern void secure_storage_all_key(void);
extern void secure_storage_read_key( uint8_t op_type, uint8_t *ptr );
extern void secure_storage_write_key( uint8_t op_type, uint8_t *ptr );

#pragma pack(pop)//恢复对齐状态

#endif

