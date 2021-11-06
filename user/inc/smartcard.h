#ifndef __SMARTCARD_H__
#define __SMARTCARD_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"
#include "stm32f4xx_hal.h"

#define		SC_USE_CRYPT

#define SC_USARTx                           USART3
#define SC_USARTx_IRQn                      USART3_IRQn
#define SC_USART_IRQ_PREPRI                 0
#define SC_USART_IRQ_SUBPRI                 0
#define SC_USART_PRESCALER                  20
#define SC_USART_BDRATE                     (HAL_RCC_GetPCLK1Freq() / (SC_USART_PRESCALER * 2) / 372)  
#define SC_USART_GUARDTIME                  16

#define SC_T0_PROTOCOL                      0x00    /* T0 protocol                                                    */
#define SC_DIRECT_MODE                      0x3B    /* Direct bit convention                                          */
#define SC_INDIRECT_MODE                    0x3F    /* Indirect bit convention                                        */
#define ATR_SETUP_LENGTH                      20
#define ATR_HIST_LENGTH                       20
#define SC_APDU_LC_MAX                       64
#define ATR_BUFFER_SIZE                      8
#define SC_RECEIVE_TIMEOUT                   400    /* Direction to reader                                            */
#define SC_TRANSMIT_TIMEOUT                  400    /* Direction to transmit   										*/

#define SMARTCARD_NUM						2

#define SC_BUF_MAX_SIZE				16				//<智能卡芯片缓存大小
#define SC_READ_BUF_MAX_SIZE		128				//<
//#define SC_BLOCK_SIZE				128				//<智能卡芯片每个区块大小
#define SC_RETYR_NUM				10				//<调用智能卡读写驱动不成功尝试次数

/*----------------------------------CryptoMemory Asynchronous Command Set---------------------------------------------*/ 
#define SC_CLA_CODE                         0x00    /* '00' is used in the AT88SCxx application as CLA                */
#define SC_WRITE_USER_ZONE_CMD              0xB0
#define SC_RANDOM_READ_CMD                  0xB1
#define SC_READ_USER_ZONE_CMD               0xB2
#define SC_SYSTEM_WRITE_CMD                 0xB4
#define SC_SYSTEM_READ_CMD                  0xB6
#define SC_VERIFY_CRYPTO_CMD                0xB8
#define SC_VERIFY_PASSWORD_CMD              0xBA

#define READ_FROM_SC                        0x01
#define WRITE_TO_SC                         0x02
#define NONE_WRITE_READ                     0x03

/*---------------------------------------SC ADPU Command: Operation Code----------------------------------------------*/
#define ATR_INDIRECT                        0x3F    /* Indirect bit convention                                        */
#define ATR_INVERSE_CONVERTION              0x03    /* The initial character TS pattern if use the inverse convention */
#define ATR_DIRECT_CONVERTION               0x3B    /* The initial character TS pattern if use the direct convention  */

/* The status bytes SW1 SW2 indicate the card status at the end of the command                                        */
#define SC_MEM_UNCHANGED                    0x6200  /* The memory is unchanged (waiting for checksum)                 */
#define SC_LEN_INCORRECT                    0x6700  /* The length is incorrect.                                       */
#define SC_CMD_NOT_SUPPORT                  0x6900  /* Command not supported and no precise diagnosis given           */
#define SC_ADDR_INCORRECT                   0x6B00  /* The address is incorrect.                                      */
#define SC_INS_NOT_SUPPORT                  0x6D00  /* The instruction code is invalid                                */
#define SC_OP_TERMINATED                    0x9000  /* The command was successfully executed.                         */

#define SC_CARD_TYPE_IDF					3

#ifdef __DEBUG_VERSION__
#define SC_CARD_COUNTER_MAX                 255
#else
#define SC_CARD_COUNTER_MAX                 4
#endif

#define IDENTITY_CARD_PORT					0
#define ACTIVATE_CARD_PORT					1

enum
{
	VOTE_CARD_CONFIG   = 0,
	ADMIN_CARD_CONFIG,
	MAINTE_CARD_CONFIG,
	SYSTEM_CARD_CONFIG,
	BACKUP_CARD_CONFIG,
	CARD_CONFIG_MAX,
};

typedef struct{
	uint8_t					auth_key[8];
	uint8_t					program_key[8];
	uint8_t					zone_read_pwd[4][3];
	uint8_t					zone_write_pwd[4][3];
	uint8_t					write7_pwd[3];
	uint8_t					id_number[7];
	uint8_t					issuer_code[16];
	uint8_t					answer_reset[8];
	uint8_t					block_num;
	uint16_t				block_size;
	uint32_t				password;
}SMARTCARD_CONFIG_T;

enum{
	SC_GC0_SEL=0,
	SC_GC1_SEL,
	SC_GC2_SEL,
	SC_GC3_SEL,
};

enum{
	SC_PW0_SEL=0,
	SC_PW1_SEL,
	SC_PW2_SEL,
	SC_PW3_SEL,
	SC_PW4_SEL,
	SC_PW5_SEL,
	SC_PW6_SEL,
	SC_PW7_SEL,
};

/* Smartcard state defination                                                                                         */
typedef enum
{
  SC_POWER_OFF      = 0,
  SC_POWER_ON       = 1,
  SC_GET_ATR        = 2,
  SC_DECODE_ATR     = 3,
  SC_IDEN_AND_AUTH  = 4,
  SC_ACTIVE         = 5,
  SC_WAIT_OUT       = 6,
  SC_REMOVED        = 7,
  SC_WRITE_ATR,
}SC_STATE_T;

/* Smartcard status defination                                                                                        */
typedef enum
{
  CARD_NOT_INSERTED = 0,
  CARD_INSERTED     = 1,
}CARD_STATUS_T;

typedef enum
{
  ERR_CARD_ERROR         = 0,
  ERR_CARD_NOT_AUTH      = 1,
  ERR_SYSTEM_NOT_CFG     = 2,
  ERR_CARD_NOT_IN_TIME   = 3,
  ERR_CARD_VOTE_NOT_DONE = 4,
  ERR_VOTE_CRAD_REMOVE   = 5,
  ERR_CARD_NONE          = 6,
}CARD_ERROR_T;


typedef enum
{
	CARD_PRESENT_NONE   = 0,
	VOTE_CARD_PRESENT,
	ADMIN_CARD_PRESENT,
	MAINTE_CARD_PRESENT,
	SYSTEM_CARD_PRESENT,
//	BACKUP_CARD_PRESENT,
	VOTE_USED_PRESENT,
//	BACKUP_NO_CARD,
}CARD_PRESENT_T;

/* ATR(Answer To Reset) structure defination                                                                          */  
typedef struct
{
  uint8_t ts;                     /* Bit Convention (ATR values for this byte should read 0x3F or 0x3B)               */
  uint8_t t0;                     /* High 4 bits refer to the presence of TA1, TB1, TC1 and TD1 characters, 
                                     Low 4 bits refers to the size of historical data in bytes                        */
  uint8_t t[ATR_SETUP_LENGTH];    /* Setup array                                                                      */
  uint8_t h[ATR_HIST_LENGTH];     /* Historical array                                                                 */
  uint8_t t_length;                /* Setup array dimension                                                            */
  uint8_t h_length;                /* Historical array dimension                                                       */
}SC_ATR_T;

typedef struct
{
  uint8_t cla;                    /* Command class                                                                    */
  uint8_t ins;                    /* Operation code                                                                   */
  uint8_t p1;                     /* Selection Mode                                                                   */
  uint8_t p2;                     /* Selection Option                                                                 */
}SC_HEADER_T;

typedef struct
{
  uint8_t lc;                     /* Data field length (P3 in T=0)                                                    */
  uint8_t data[SC_APDU_LC_MAX];   /* Command parameters                                                               */
  uint8_t le;                     /* Expected length of data to be returned                                           */
}SC_BODY_T;

/* ADPU Command structure --------------------------------------------------------------------------------------------*/
typedef struct
{
  SC_HEADER_T		header;
  SC_BODY_T  		body;
}SC_APDU_CMD_T;

/* SC response structure ---------------------------------------------------------------------------------------------*/
typedef struct
{
  uint8_t data[SC_APDU_LC_MAX]; /* Data returned from the card                                                        */
  uint8_t sw1;                  /* Command Processing status                                                          */
  uint8_t sw2;                  /* Command Processing qualification                                                   */
}SC_APDU_RESP_T;

typedef struct{
	uint8_t					block[16];
}SMARTCARD_AUTH_PWD_T;

typedef struct
{
	SC_STATE_T    			card_state;
	CARD_STATUS_T		  	card_status;
	SC_ATR_T      			card_atr;
	SC_APDU_CMD_T  			card_apdu_cmd;
	SC_APDU_RESP_T 			card_apdu_resp;
	uint8_t              	sc_atr_table[ATR_BUFFER_SIZE];
	uint8_t              	sc_atr_len;
	bool                 	sc_inverse_flag;
	uint8_t			   		card_port;
	uint8_t					atr_type;
	uint8_t					block_num;
	uint16_t				block_size;
	uint16_t				storage_size;
	SMARTCARD_AUTH_PWD_T	*auth_pwd;
	SMARTCARD_HandleTypeDef handle;                  ///<智能卡硬件接口句柄
	
	CARD_PRESENT_T			card_present;			//<插入卡类型
	
}CARD_MANAGE_T;

typedef struct {
	uint8_t					unique_id[7];
	uint8_t					key[32];
	uint8_t					vote_count;
}SC_CARD_INFO_T;

typedef struct{
	uint8_t		curr_use_block;
//	uint8_t		atr_type;
//	uint8_t		card_port;
	uint8_t		error_code;
}SC_STA_T;

typedef struct {
	SC_CARD_INFO_T			card_info;
//	uint8_t					system_adminiator[2];
}SC_CARD_IND_T;

#define			is_card_a_type(type)				(type==smartcard_manage[0].card_present)
#define			is_card_b_type(type)				(type==smartcard_manage[1].card_present)
#define 		switch_smartcard_step(step)			(p_card_manage->card_state = step)

extern CARD_MANAGE_T	smartcard_manage[SMARTCARD_NUM];
extern FUN_STATUS_T sc_init(void);
extern FUN_STATUS_T sc_write_storage(CARD_MANAGE_T *card_manage, uint16_t addr, uint16_t len, uint8_t *ptr);
extern FUN_STATUS_T sc_read_storage(CARD_MANAGE_T *card_manage, uint16_t addr, uint16_t len, uint8_t *ptr);
extern FUN_STATUS_T sc_clear_storage(CARD_MANAGE_T *card_manage, uint16_t addr, uint16_t len);
extern FUN_STATUS_T sc_activate_vote_card(uint8_t vote_type);
extern FUN_STATUS_T sc_card_type_insert(uint8_t card_port, CARD_PRESENT_T card_type);
#endif
