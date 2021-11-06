#include "secure_storage.h"
//#include "at88sc.h"
#include "stm32f4xx_hal.h"
#include "crc32.h"
#include "string.h"
//#include "election.h"
#include "crypt.h"
#include "main.h"
#include "config.h"

DEFAULT_PASSWORDS_T		default_password;
VOTE_STATUS_T			vote_status;

const DEFAULT_PASSWORDS_T		default_password_config={
	.voter_key={0x4D,0x18,0xDF,0x7A,0x01,0xFD,0xB5,0x94,0x80,0x18,0x47,0xA6,0xA8,0xAC,0xAD,0x35,0x54,0x0E,0x23,0x26,0x63,0xBA,0x8F,0x2F,0x92,0xA3,0xA1,0x25,0xDF,0x96,0x96,0x7A},
	.encryption_key={0xCC,0x48,0x91,0x4C,0x8C,0xC5,0x49,0x8B,0xF0,0x02,0x52,0x57,0x93,0x78,0xDA,0x13,0x4B,0x10,0xCD,0xB9,0x2E,0x44,0x37,0x29,0x41,0x8B,0x48,0x2E,0x29,0x92,0xDE,0x8F},
	.encryption_transfer_key={0x62,0xDD,0xB7,0x68,0xC0,0x0A,0x66,0x70,0xF3,0x54,0x50,0x4A,0x9D,0x6B,0x18,0xEE,0x96,0x62,0xB1,0x35,0xC9,0x22,0x45,0x47,0xA7,0x18,0x76,0xAF,0x42,0xD1,0x4B,0x4B},
	.administrator_key={0x23,0x1B,0x3C,0xE8,0x49,0x98,0xC7,0xC1,0x58,0x62,0xC5,0x4D,0xA4,0x46,0xCD,0xE2,0x25,0x7D,0xA9,0xC8,0x43,0xF3,0x10,0xC5,0x99,0x4F,0x24,0x47,0x96,0xE6,0x55,0x61},
	.maintenance_key={0xA7,0xC3,0x23,0x7E,0x97,0xF5,0xA3,0xB5,0x4A,0xAB,0x3D,0xD5,0xA1,0xF2,0xF1,0xE7,0x81,0x7E,0xD5,0x9B,0xF1,0xBD,0xC8,0xF2,0xE2,0x00,0x41,0x27,0x71,0x35,0x33,0x14},
	.system_administrator_key1={0x20,0xDD,0xDB,0xD3,0x05,0xAB,0xEA,0xF5,0x93,0x75,0x77,0x20,0xCE,0xDE,0x33,0xA3,0xD6,0xED,0x73,0x91,0x00,0x9A,0xCF,0x54,0x6F,0x36,0x52,0x78,0xE5,0xDA,0x92,0x06},
	.system_administrator_key2={0x84,0xFA,0x6D,0x46,0xBA,0x58,0xD9,0x68,0x1A,0x4A,0xED,0xAB,0x70,0x7E,0x6B,0x1D,0x54,0x26,0x3D,0xDE,0x96,0x92,0x93,0xDB,0x67,0x85,0xC9,0x88,0xA7,0x3A,0x0D,0x53},
	.pc_settings_key={0x39,0x7D,0x93,0xB6,0x04,0x7A,0x9D,0x4B,0x3E,0xD1,0xA6,0x86,0xE0,0xBD,0x36,0x73,0x59,0x61,0x56,0xFB,0xF5,0x76,0x7B,0xAC,0x72,0x24,0x1A,0xA8,0x4C,0x4A,0x13,0xFA},
	.vote_deactivation_key={0x97,0xD0,0x68,0x39,0x5C,0x4D,0xDA,0x16,0xC8,0xF7,0x49,0xFE,0xCB,0x0F,0x5F,0x72,0x4E,0x7B,0x53,0xC9,0x33,0xE6,0xB2,0x31,0x0E,0x67,0x28,0xDC,0x0F,0x95,0xF1,0x87},
	.system_administrator={},
	.sa_se_read_password2={},
	.se_rw_password_zone1={},
	.se_rw_password_zone2={},
	.se_rw_password_zone3={},
	.se_rw_password_zone4={},
	.se_authentication_key1={},
	.se_authentication_key2={},
	.bootloader_key={0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f},
	.hashing_key={0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f},
	.system_initial_authentication_key={},
	.system_authentication_key1={},
	.system_authentication_key2={},
};

void secure_storage_default(void){
	
	const CANDIDATE_T candidate_test={
		.vote_date.day = 28,
		.vote_date.month = 1,
		.vote_date.year = 2019,
		.vtn = {"presidential"},
		.av = 1,
		.ebb_id={1,2,3,4,5,6,7,8,9,10,11,12},
		.total_tv = 0,
		.candidate_num = 5,
		.total_voters = 0,
		.vote_data[0].name="Aabbye",
		.vote_data[0].tv = 0,
		.vote_data[1].name="Cadence",
		.vote_data[1].tv = 0,
		.vote_data[2].name="Gabe",
		.vote_data[2].tv = 0,
		.vote_data[3].name="LaRue",
		.vote_data[3].tv = 0,
		.vote_data[4].name="Qiana",
		.vote_data[4].tv = 0,
//		.vote_data[5].name="Chad",
//		.vote_data[5].tv = 0,
//		.vote_data[6].name="Drew",
//		.vote_data[6].tv = 0,
//		.vote_data[7].name="Egbert",
//		.vote_data[7].tv = 0,
//		.vote_data[8].name="Grover",
//		.vote_data[8].tv = 0,
//		.vote_data[9].name="Hogan",
//		.vote_data[9].tv = 0,
	};
	

//	uint16_t temp;
//	secure_storage_canddate_count(4,&temp,SC_READ);
//	temp++;
//	secure_storage_canddate_count(4,&temp,SC_WRITE);
//	
//	char buf[20];
//	secure_storage_canddate_name( 4, 20, buf, SC_READ );
	
	static uint8_t write_flag=0;
//	CANDIDATE_T				candidate_readout;
	if( write_flag )
		crypt_write(AT_SC,0,sizeof(CANDIDATE_T),(uint8_t *)&candidate_test);	
//	crypt_read(AT_SC,0,sizeof(CANDIDATE_T),(uint8_t *)&candidate_readout);
}

void secure_storage_canddate_info( OP_TYPE_T op_type, uint8_t *ptr, SC_RW_T rw ){
	
	#ifdef SE_USE_CRYPT
	uint16_t(*at88sc_func)(CARD_MANAGE_T *card_manage, uint16_t addr, uint16_t len, uint8_t *ptr); 
	#else
	uint16_t(*at88sc_func)(CARD_MANAGE_TypeDef *card_manage, uint16_t addr, uint16_t len, uint8_t *ptr); 
	#endif
	
	#ifdef SE_USE_CRYPT
	if( rw==SC_READ )
		at88sc_func = crypt_read;
	else if( rw==SC_WRITE )
		at88sc_func = crypt_write;
	#else
	if( rw==SC_READ )
		at88sc_func = at88sc_read_storage;
	else if( rw==SC_WRITE )
		at88sc_func = at88sc_write_storage;
	#endif
	
	switch( op_type ){
		case OP_DATE:
			at88sc_func( AT_SC, offsetof(CANDIDATE_T,vote_date), sizeof(((CANDIDATE_T*)0)->vote_date), ptr);
		break;
		
		case OP_VTN:
			at88sc_func( AT_SC, offsetof(CANDIDATE_T,vtn), sizeof(((CANDIDATE_T*)0)->vtn), ptr);
		break;
		
		case OP_AV:
			at88sc_func( AT_SC, offsetof(CANDIDATE_T,av), sizeof(((CANDIDATE_T*)0)->av), ptr);
		break;
		
		case OP_EBBID:
			at88sc_func( AT_SC, offsetof(CANDIDATE_T,ebb_id), sizeof(((CANDIDATE_T*)0)->ebb_id), ptr);
		break;
		
		case OP_TOTALTV:
			at88sc_func( AT_SC, offsetof(CANDIDATE_T,total_tv), sizeof(((CANDIDATE_T*)0)->total_tv), ptr);
		break;
		
		case OP_NUM:
			at88sc_func( AT_SC, offsetof(CANDIDATE_T,candidate_num), sizeof(((CANDIDATE_T*)0)->candidate_num), ptr);
		break;
		
		case OP_TOTALVOTERS:
			at88sc_func( AT_SC, offsetof(CANDIDATE_T,total_voters), sizeof(((CANDIDATE_T*)0)->total_voters), ptr);
		break;
		
		case OP_NOVOTERS:
			at88sc_func( AT_SC, offsetof(CANDIDATE_T,no_voters), sizeof(((CANDIDATE_T*)0)->no_voters), ptr);
		break;
		
		default:
			*ptr = 0;
		break;
	}
}

uint8_t secure_storage_canddate_name( uint16_t num, uint8_t len, uint8_t *ptr, SC_RW_T rw ){
	
	uint16_t(*at88sc_func)(uint16_t addr, uint16_t len, uint8_t *ptr);
	uint8_t op_len;
	
	if( (num<CANDIDATE_NUM) ){
		
		#ifdef SE_USE_CRYPT
		uint16_t(*at88sc_func)(CARD_MANAGE_T *card_manage, uint16_t addr, uint16_t len, uint8_t *ptr); 
		#else
		uint16_t(*at88sc_func)(CARD_MANAGE_TypeDef *card_manage, uint16_t addr, uint16_t len, uint8_t *ptr); 
		#endif
		
		#ifdef SE_USE_CRYPT
		if( rw==SC_READ )
			at88sc_func = crypt_read;
		else if( rw==SC_WRITE )
			at88sc_func = crypt_write;
		#else
		if( rw==SC_READ )
			at88sc_func = at88sc_read_storage;
		else if( rw==SC_WRITE )
			at88sc_func = at88sc_write_storage;
		#endif
		
		op_len = len>(sizeof(((VOTE_DATA_T*)0)->name))?(sizeof(((VOTE_DATA_T*)0)->name)):len;
		
		
		at88sc_func( AT_SC, offsetof(CANDIDATE_T,vote_data)+(num)*sizeof(VOTE_DATA_T), op_len, ptr);
		
		strlen(ptr)>sizeof(((VOTE_DATA_T*)0)->name)?(len = (sizeof(((VOTE_DATA_T*)0)->name))):(len = strlen(ptr));
		ptr[len] = 0;
		return len;
		
	}
	return 0;
}

void secure_storage_canddate_count( uint16_t num, uint8_t *ptr, SC_RW_T rw ){
	
	uint16_t(*at88sc_func)(uint16_t addr, uint16_t len, uint8_t *ptr); 
	
	if( num<CANDIDATE_NUM){
		
		#ifdef SE_USE_CRYPT
		uint16_t(*at88sc_func)(CARD_MANAGE_T *card_manage, uint16_t addr, uint16_t len, uint8_t *ptr); 
		#else
		uint16_t(*at88sc_func)(CARD_MANAGE_TypeDef *card_manage, uint16_t addr, uint16_t len, uint8_t *ptr); 
		#endif
		
		#ifdef SE_USE_CRYPT
		if( rw==SC_READ )
			at88sc_func = crypt_read;
		else if( rw==SC_WRITE )
			at88sc_func = crypt_write;
		#else
		if( rw==SC_READ )
			at88sc_func = at88sc_read_storage;
		else if( rw==SC_WRITE )
			at88sc_func = at88sc_write_storage;
		#endif
		
		
		at88sc_func( AT_SC, offsetof(CANDIDATE_T,vote_data)+offsetof(VOTE_DATA_T,tv)+(num)*sizeof(VOTE_DATA_T), sizeof(((VOTE_DATA_T*)0)->tv), ptr);
		
	}
}

void secure_storage_canddate_inc(uint16_t num, uint8_t value){
	
	uint16_t cnt=0;

	secure_storage_canddate_count( num, &cnt, SC_READ );
	cnt += value;
	secure_storage_canddate_count( num, &cnt, SC_WRITE );
}

void secure_storage_totaltv_inc(uint8_t value){
	
	uint16_t cnt=0;
	secure_storage_canddate_info( OP_TOTALTV, &cnt, SC_READ );
	cnt += value;
	secure_storage_canddate_info( OP_TOTALTV, &cnt, SC_WRITE );
}

void secure_storage_voter_inc(uint8_t value){
	
	uint16_t cnt=0;
	secure_storage_canddate_info( OP_TOTALVOTERS, &cnt, SC_READ );
	cnt += value;
	secure_storage_canddate_info( OP_TOTALVOTERS, &cnt, SC_WRITE );
}

void secure_storage_novoter_inc(uint8_t value){
	
	uint16_t cnt=0;
	secure_storage_canddate_info( OP_NOVOTERS, &cnt, SC_READ );
	cnt += value;
	secure_storage_canddate_info( OP_NOVOTERS, &cnt, SC_WRITE );
}

void secure_storage_flash_read(uint8_t *ptr , uint16_t num){
	
	uint32_t addr = FLASH_STORAGE_ADDR;
	while( num-- ){
		*ptr = *(__IO uint32_t*)(addr);
		ptr++;
		addr++;
	}
}

void secure_storage_flash_write(void){
	
	uint32_t HalfWord;
//	Byte_Num = Byte_Num/2;
	uint8_t *source_addr;
	uint32_t	addr;
	HAL_FLASH_Unlock();
	
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector        = FLASH_SECTOR_11;
	EraseInitStruct.NbSectors     = 1;
    //设置PageError
    uint32_t PageError = 0;
    //调用擦除函数
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

	addr = FLASH_STORAGE_ADDR;
	source_addr = (uint8_t *)&default_password;
	
	uint32_t crc;
	crc = crc32(&default_password,sizeof(DEFAULT_PASSWORDS_T)-sizeof(((DEFAULT_PASSWORDS_T*)0)->crc));
	default_password.crc = crc;
	
	for( uint16_t i=0; i<sizeof(DEFAULT_PASSWORDS_T); i++ ){
		
		HAL_FLASH_Program(TYPEPROGRAM_BYTE, addr, *source_addr);
		addr ++;
		source_addr++;
	}
	
	HAL_FLASH_Lock();
}

void secure_storage_write_key( uint8_t op_type, uint8_t *ptr ){
	
	uint8_t flag=1;
	switch(op_type){
		
		case VOTER_KEY:
			memcpy((void*)&default_password.voter_key,(void *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->voter_key));
			break;
		
		case ENCRYPTION_KEY:
			memcpy((void*)&default_password.encryption_key,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->encryption_key));
			break;
		
		case ADMINISTRATOR_KEY:
			memcpy((void*)&default_password.administrator_key,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->administrator_key));
			break;
		
		case MAINTENANCE_KEY:
			memcpy((void*)&default_password.maintenance_key,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->maintenance_key));
			break;
		
		case SYSTEM_ADMINISTRATOR_KEY1:
			memcpy((void*)&default_password.system_administrator_key1,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator_key1));
			break;	
		
		case SYSTEM_ADMINISTRATOR_KEY2:
			memcpy((void*)&default_password.system_administrator_key2,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator_key2));
			break;
		
		case SYSTEM_ADMINISTRATOR:
			memcpy((void*)&default_password.system_administrator,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator));
			break;
		
		case SA_SE_READ_PASSWORD2:
			memcpy((void*)&default_password.sa_se_read_password2,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->sa_se_read_password2));
			break;
		
		case SE_RW_PASSWORD_ZONE1:
			memcpy((void*)&default_password.se_rw_password_zone1,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_rw_password_zone1));
			break;
		
		case SE_RW_PASSWORD_ZONE2:
			memcpy((void*)&default_password.se_rw_password_zone2,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_rw_password_zone2));
			break;
		
		case SE_RW_PASSWORD_ZONE3:
			memcpy((void*)&default_password.se_rw_password_zone3,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_rw_password_zone3));
			break;
		
		case SE_RW_PASSWORD_ZONE4:
			memcpy((void*)&default_password.se_rw_password_zone4,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_rw_password_zone4));
			break;
		
		case SE_AUTHENTICATION_KEY1:
			memcpy((void*)&default_password.se_authentication_key1,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_authentication_key1));
			break;
		
		case SE_AUTHENTICATION_KEY2:
			memcpy((void*)&default_password.se_authentication_key2,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_authentication_key2));
			break;
		
		case BOOTLOADER_KEY:
			memcpy((void*)&default_password.bootloader_key,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->bootloader_key));
			break;
		
		case VOTE_DEACTIVATION_KEY:
			memcpy((void*)&default_password.vote_deactivation_key,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->vote_deactivation_key));
			break;
		
		case PC_SETTINGS_KEY:
			memcpy((void*)&default_password.pc_settings_key,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->pc_settings_key));
			break;
		
		case HASHING_KEY:
			memcpy((void*)&default_password.hashing_key,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->hashing_key));
			break;
		
		case SYSTEM_INITIAL_AUTHENTICATION_KEY:
			memcpy((void*)&default_password.system_initial_authentication_key,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_initial_authentication_key));
			break;
		
		case SYSTEM_AUTHENTICATION_KEY1:
			memcpy((void*)&default_password.system_authentication_key1,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_authentication_key1));
			break;
		
		case SYSTEM_AUTHENTICATION_KEY2:
			memcpy((void*)&default_password.system_authentication_key2,(uint8_t *)ptr,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_authentication_key2));
			break;
		
		default:
			flag = 0;
			break;
	}
	if( flag )
		secure_storage_flash_write();
}

void secure_storage_read_key( uint8_t op_type, uint8_t *ptr ){
	
	switch(op_type){
		
		case VOTER_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.voter_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->voter_key));
			break;
		
		case ENCRYPTION_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.encryption_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->encryption_key));
			break;
		
		case ADMINISTRATOR_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.administrator_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->administrator_key));
			break;
		
		case MAINTENANCE_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.maintenance_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->maintenance_key));
			break;
		
		case SYSTEM_ADMINISTRATOR_KEY1:
			memcpy((uint8_t *)ptr,(void*)&default_password.system_administrator_key1,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator_key1));
			break;	
		
		case SYSTEM_ADMINISTRATOR_KEY2:
			memcpy((uint8_t *)ptr,(void*)&default_password.system_administrator_key2,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator_key2));
			break;
		
		case SYSTEM_ADMINISTRATOR:
			memcpy((uint8_t *)ptr,(void*)&default_password.system_administrator,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_administrator));
			break;
		
		case SA_SE_READ_PASSWORD2:
			memcpy((uint8_t *)ptr,(void*)&default_password.sa_se_read_password2,sizeof(((DEFAULT_PASSWORDS_T*)0)->sa_se_read_password2));
			break;
		
		case SE_RW_PASSWORD_ZONE1:
			memcpy((uint8_t *)ptr,(void*)&default_password.se_rw_password_zone1,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_rw_password_zone1));
			break;
		
		case SE_RW_PASSWORD_ZONE2:
			memcpy((uint8_t *)ptr,(void*)&default_password.se_rw_password_zone2,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_rw_password_zone2));
			break;
		
		case SE_RW_PASSWORD_ZONE3:
			memcpy((uint8_t *)ptr,(void*)&default_password.se_rw_password_zone3,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_rw_password_zone3));
			break;
		
		case SE_RW_PASSWORD_ZONE4:
			memcpy((uint8_t *)ptr,(void*)&default_password.se_rw_password_zone4,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_rw_password_zone4));
			break;
		
		case SE_AUTHENTICATION_KEY1:
			memcpy((uint8_t *)ptr,(void*)&default_password.se_authentication_key1,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_authentication_key1));
			break;
		
		case SE_AUTHENTICATION_KEY2:
			memcpy((uint8_t *)ptr,(void*)&default_password.se_authentication_key2,sizeof(((DEFAULT_PASSWORDS_T*)0)->se_authentication_key2));
			break;
		
		case BOOTLOADER_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.bootloader_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->bootloader_key));
			break;
		
		case VOTE_DEACTIVATION_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.vote_deactivation_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->vote_deactivation_key));
			break;
		
		case PC_SETTINGS_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.pc_settings_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->pc_settings_key));
			break;
		
		case HASHING_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.hashing_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->hashing_key));
			break;
		
		case SYSTEM_INITIAL_AUTHENTICATION_KEY:
			memcpy((uint8_t *)ptr,(void*)&default_password.system_initial_authentication_key,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_initial_authentication_key));
			break;
		
		case SYSTEM_AUTHENTICATION_KEY1:
			memcpy((uint8_t *)ptr,(void*)&default_password.system_authentication_key1,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_authentication_key1));
			break;
		
		case SYSTEM_AUTHENTICATION_KEY2:
			memcpy((uint8_t *)ptr,(void*)&default_password.system_authentication_key2,sizeof(((DEFAULT_PASSWORDS_T*)0)->system_authentication_key2));
			break;
		
		default:
			break;
	}
}

void secure_storage_all_key(void){
	
	uint32_t crc;
	secure_storage_flash_read((uint8_t*)&default_password,sizeof(DEFAULT_PASSWORDS_T));
	crc = crc32(&default_password, sizeof(DEFAULT_PASSWORDS_T)-sizeof(((DEFAULT_PASSWORDS_T*)0)->crc));
	if(  crc != default_password.crc ){
		memcpy(&default_password,&default_password_config,sizeof(DEFAULT_PASSWORDS_T));
		secure_storage_flash_write();
	}
}

int8_t secure_storage_init(void)
{	
	secure_storage_all_key();
	
//	secure_storage_default();
	
//	for( uint8_t i=0; i<CANDIDATE_NUM; i++ ){
//		secure_storage_canddate_count( i, &vote_status.last_tv[i], SC_READ);
//	}
	
	return 0;
}

