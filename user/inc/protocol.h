#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include "rf_module.h"

#pragma pack(push) //�������״̬
#pragma pack(1)   // 1 bytes����

#if defined(USE_USB_RF_MODULE)
#define MSG_DATA_LEN		27
#else
#define MSG_DATA_LEN		28
#endif
#define RESEND_TIMERS		3
#define WAIT_REPLY_TIME		10	

enum{
	SEG_OK=0,	///<receive segment ok
	FRAME_OK,	///<receive frame ok
	NO_ERR,		///<receive segment no error
	CRC_ERR,	///<receive frame crc error
}SEG_STUS_T;

typedef union _SEG_STA{
	uint8_t		byte;
	struct{
		uint8_t		data_len:5;
		uint8_t		reserve:3;
	}transmit;
	struct{
		uint8_t		reserve:6;
		uint8_t		status:2;			///<0:seg_ok,1:frame_ok,2:seg_no error,3:crc error
	}reply;

}SEG_STA_T;

typedef union _MSG_ID {
	uint8_t		byte;
	uint8_t		id_no;
	uint8_t		reply_seg;
}MSG_ID_T;

typedef struct MSG_T{
	uint8_t		seg_no;					///<��ǰ�κ�
	uint8_t		seg_total;				///<�ܶ���
	uint8_t		source_addr;			///<Դ��ַ
	SEG_STA_T	seg_sta;				///<��״̬
	uint8_t		seg_data[MSG_DATA_LEN];	///<�ֶ�����
	
}MSG_T;

#pragma pack(pop)//�ָ�����״̬

#endif
