#ifndef __RF_MODULE_H__
#define __RF_MODULE_H__

#include "stdint.h"
#include "tm_stm32_nrf24l01.h"
#include "system.h"
#include "comm.h"
#include "macro_def.h"

#define USE_USB_RF_MODULE

#define RF_EBB_CHN_REQ		1
#define RF_EBB_CHN_REG1		2
#define RF_EBB_CHN_POLL		3
#define RF_EBB_CHN_REG2		4
#define RF_EBB_CHN_REG3		5

#define RF_EVT_CHN_REQ		1
#define RF_EVT_CHN_REG		2
#define RF_EVT_CHN_POLL		3
#define RF_EVT_CHN_PC		4
#define RF_EVT_CHN_REV		5

typedef struct{
	uint32_t	seg_err;
	uint32_t	crc_err;
	uint32_t	size_err;
}RF_ERR_STA_T;

typedef struct{
	uint8_t 	step;
	void 		*buf;
	uint16_t	len;
	uint16_t 	timeout;
	uint32_t	sur_addr;
	RF_ERR_STA_T		err_sta;
}RF_RECV_STA_T;

typedef struct{
	uint8_t		msg_id;
	uint32_t	addr;
	uint8_t		pipe;
	uint8_t		delay_time;
	uint8_t		resend_times;
}RF_SEND_STA_T;

typedef struct{
	uint8_t				msg_id;
	
	TM_NRF24L01_Transmit_Status_t transmissionStatus;
	
	QueueHandle_t		recv_queue;						///<receive queue
	QueueHandle_t		send_queue;						///<receive queue
	
	uint8_t				send_msg_id;
	
	RF_RECV_STA_T		recv_sta[EVT_NUM];
	RF_SEND_STA_T		send_sta;

}RF_MODULE_T;

typedef struct{
	
	void 		*data;				///<发送数据地址
	uint32_t	addr;				///send or receive address
	uint8_t		pipe;
	uint8_t		len;
	uint8_t		wait_reply;
}RF_MSG_T;

/* Interrupt pin settings */
#define RF_IRQ_PORT    GPIOA
#define RF_IRQ_PIN     GPIO_PIN_3

#define RF_RECV_TIMEOUT_TIME		1000
#define RF_SEND_DELAY_TIME			3
#define SEND_QUEUE_LENGTH			36
#define RECV_QUEUE_LENGTH			36
#define RESEND_TIMES				3

extern RF_MODULE_T		rf_module;

extern FUN_STATUS_T rf_module_init(void);
extern void rf_module_test(void);
extern void rf_module_isr_handle(void);
extern uint32_t rf_module_recv_len(void);
extern uint32_t rf_module_read(uint8_t *ptr,uint16_t size);
extern uint32_t rf_module_send(uint32_t target_addr, uint32_t source_addr,  uint8_t chn, uint8_t *ptr, uint16_t size );
extern void rf_module_set_rx_addr(uint32_t addr, uint8_t pipe);
extern void rf_module_set_target(uint32_t target, uint8_t pipe);
#endif
