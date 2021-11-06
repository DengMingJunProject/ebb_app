/** 
* @file         rf_module.c 
* @brief        NRF24L01模块驱动. 
* @details  	数据分包及组包，重发处理,校验处理. 
* @author       ken deng 
* @date     	2019-07-29 
* @version  	A001 
* @par Copyright (c):  
*       laxton company 
* @par History:          
*   version: ken deng, 2019-07-29, 建立\n 
*/  
#include "rf_module.h"
#include "tm_stm32_delay.h"
#include "tm_stm32_nrf24l01.h"
#include "tm_stm32_exti.h"
#include "system.h"
#include "daemon.h"
#include "protocol.h"
#include "comm.h"
#include "crc32.h"
#include "config.h"
#include "main.h"
#include "config.h"
/*
rf24L01配置注意:
地址，地址宽度，频道，开启或关闭CRC，CRC8bit或16bit
*/

RF_MODULE_T		rf_module;

//#undef RF_MODULE_DEBUG

#ifdef RF_MODULE_DEBUG
    #define rf_module_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[RF_MODULE](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define rf_module_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[RF_MODULE] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define rf_module_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[RF_MODULE] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define rf_module_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define rf_module_log(...)
    #define rf_module_usr(...)
    #define rf_module_err(...)
    #define rf_module_dump(...)
#endif
	
/* NRF24L01+ registers*/
#define NRF24L01_REG_CONFIG			0x00	//Configuration Register
#define NRF24L01_REG_EN_AA			0x01	//Enable 慉uto Acknowledgment?Function
#define NRF24L01_REG_EN_RXADDR		0x02	//Enabled RX Addresses
#define NRF24L01_REG_SETUP_AW		0x03	//Setup of Address Widths (common for all data pipes)
#define NRF24L01_REG_SETUP_RETR		0x04	//Setup of Automatic Retransmission
#define NRF24L01_REG_RF_CH			0x05	//RF Channel
#define NRF24L01_REG_RF_SETUP		0x06	//RF Setup Register	
#define NRF24L01_REG_STATUS			0x07	//Status Register
#define NRF24L01_REG_OBSERVE_TX		0x08	//Transmit observe registerf
#define NRF24L01_REG_RPD			0x09	
#define NRF24L01_REG_RX_ADDR_P0		0x0A	//Receive address data pipe 0. 5 Bytes maximum length.
#define NRF24L01_REG_RX_ADDR_P1		0x0B	//Receive address data pipe 1. 5 Bytes maximum length.
#define NRF24L01_REG_RX_ADDR_P2		0x0C	//Receive address data pipe 2. Only LSB
#define NRF24L01_REG_RX_ADDR_P3		0x0D	//Receive address data pipe 3. Only LSB
#define NRF24L01_REG_RX_ADDR_P4		0x0E	//Receive address data pipe 4. Only LSB
#define NRF24L01_REG_RX_ADDR_P5		0x0F	//Receive address data pipe 5. Only LSB
#define NRF24L01_REG_TX_ADDR		0x10	//Transmit address. Used for a PTX device only
#define NRF24L01_REG_RX_PW_P0		0x11	
#define NRF24L01_REG_RX_PW_P1		0x12	
#define NRF24L01_REG_RX_PW_P2		0x13	
#define NRF24L01_REG_RX_PW_P3		0x14	
#define NRF24L01_REG_RX_PW_P4		0x15	
#define NRF24L01_REG_RX_PW_P5		0x16	
#define NRF24L01_REG_FIFO_STATUS	0x17	//FIFO Status Register
#define NRF24L01_REG_DYNPD			0x1C	//Enable dynamic payload length
#define NRF24L01_REG_FEATURE		0x1D

	
EventGroupHandle_t rf_module_event_handler = NULL;    /*定义事件标志组*/
#define EVENTBIT_TRANSMIT_OK  		(1<<0)              
#define EVENTBIT_TRANSMIT_TIMEOUT  	(1<<1)
#define EVENTBIT_TRANSMIT_BIT    	(EVENTBIT_TRANSMIT_OK|EVENTBIT_TRANSMIT_TIMEOUT)

/** 
* 分别设置3个通道的接收地址. 
* 无. 
* @param[in]   addr:地址，pipe:通道号. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
void rf_module_set_rx_addr(uint32_t addr, uint8_t pipe){
	
	uint8_t buf[5]={0};
	buf[0] = pipe;
	buf[1] = (addr&0xff);
	buf[2] = ((addr>>8)&0xff);
	buf[3] = ((addr>>16)&0xff);
	buf[4] = ((addr>>24)&0xff);
	switch( pipe ){
		case 1:
			NRF24L01_CE_LOW;
			TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, buf, 5);
			NRF24L01_CE_HIGH;
			break;
		case 2:
			NRF24L01_CE_LOW;
			TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P2, &pipe, 1);
			NRF24L01_CE_HIGH;
			break;
		case 3:
			NRF24L01_CE_LOW;
			TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P3, &pipe, 1);
			NRF24L01_CE_HIGH;
			break;
		case 4:
			NRF24L01_CE_LOW;
			TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P4, &pipe, 1);
			NRF24L01_CE_HIGH;
			break;
		case 5:
			NRF24L01_CE_LOW;
			TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P5, &pipe, 1);
			NRF24L01_CE_HIGH;
			break;
	}
}

/** 
* 设置发送通道的发送地址. 
* 无. 
* @param[in]   addr:地址. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
void rf_module_set_tx_addr(uint32_t addr, uint8_t pipe){
	
	uint8_t buf[5]={0};
	buf[0] = pipe;
	buf[1] = (addr&0xff);
	buf[2] = ((addr>>8)&0xff);
	buf[3] = ((addr>>16)&0xff);
	buf[4] = ((addr>>24)&0xff);
	TM_NRF24L01_SetTxAddress(buf);
}

/** 
* 写队列. 
* 申请内存成功后，写入队列. 
* @param[in]   queue:队，addr：地址指针，data：数据指针，len：数据长度，isr：是否在中断调用. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
FUN_STATUS_T rf_module_write_queue(QueueHandle_t queue, RF_MSG_T *rf_msg_ptr, void *data, uint8_t isr)
{	
	rf_msg_ptr->data = (uint8_t*)malloc(rf_msg_ptr->len);
		
	if( (rf_msg_ptr->data != NULL) && (queue != NULL) ){
		
		memcpy(rf_msg_ptr->data,data,rf_msg_ptr->len);

		BaseType_t xReturn;
		
		if( isr )
			xReturn = xQueueSendFromISR(queue,rf_msg_ptr,0);
		else
			xReturn = xQueueSend(queue,rf_msg_ptr,0);
				
		if( xReturn == pdTRUE )
			return FUN_OK;
		else{
			free(rf_msg_ptr->data);
			return FUN_ERROR;
		}
	}
	else{
		if( rf_msg_ptr->data != NULL ){
			free(rf_msg_ptr->data);
		}
		return FUN_ERROR;
	}
}

/** 
* 读队列. 
* 拷贝数据到数据指针，并释放内存区.
* @param[in]   queue:队，addr：返回地址，data：返回数据指针. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
uint16_t rf_module_read_queue(QueueHandle_t queue, uint32_t *addr, uint8_t *pipe, void *data){
	
	RF_MSG_T	recv_msg;
	if( queue != NULL ){
		if( xQueueReceive(queue,&recv_msg,0)==pdPASS ){
			
			memcpy(data,recv_msg.data,recv_msg.len);
			*addr = recv_msg.addr;
			*pipe = recv_msg.pipe;
			free(recv_msg.data);
					
			return recv_msg.len;
		}
		else{
			return 0;
		}
	}
	else{
		return 0;
	}
}

/** 
* NRF24L01中断处理函数. 
* 处理发送完成中断，发送超时中断，接收成功中断.
* @param[in]   无. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
void rf_module_isr_handle(void){
	
	TM_NRF24L01_IRQ_t NRF_IRQ;
	
	/* Read interrupts */
	TM_NRF24L01_Read_Interrupts(&NRF_IRQ);
	
	/* Check if transmitted OK */
	if (NRF_IRQ.F.DataSent) {
		/* Save transmission status */
		rf_module.transmissionStatus = TM_NRF24L01_Transmit_Status_Ok;
		
		/* Go back to RX mode */
		TM_NRF24L01_PowerUpRx();
		
		xEventGroupSetBits(rf_module_event_handler,EVENTBIT_TRANSMIT_OK);
		
//		rf_module_log("transmitted OK\r\n");
	}
	
	/* Check if max retransmission reached and last transmission failed */
	if (NRF_IRQ.F.MaxRT) {
		/* Save transmission status */
		rf_module.transmissionStatus = TM_NRF24L01_Transmit_Status_Lost;
		
		/* Go back to RX mode */
		TM_NRF24L01_PowerUpRx();
		
		xEventGroupSetBits(rf_module_event_handler,EVENTBIT_TRANSMIT_TIMEOUT);
		
		rf_module_log("last transmission failed\r\n");
	}
	
	/* If data is ready on NRF24L01+ */
	if (NRF_IRQ.F.DataReady) {
		/* Get data from NRF24L01+ */
		uint8_t dataIn[32];
//		uint32_t cnt;
		TM_NRF24L01_GetData(dataIn);
		uint32_t addr;
		addr = dataIn[3];
		
		RF_MSG_T	rf_msg;
		rf_msg.len = dataIn[0];
		rf_msg.wait_reply = 0;
		rf_msg.addr = dataIn[3];
		rf_msg.pipe = 0;
		
		if( FUN_OK != rf_module_write_queue(rf_module.recv_queue, &rf_msg, &dataIn[1], 1) ){
			rf_module_log("Recv data enqueue Error\r\n");
		}
//		rf_module_log("Get data from NRF24L01+\r\n");
		
		rf_module.send_sta.delay_time = 0;
				
//			rf_module_dump((uint8_t*)dataIn,32);
	}
}

/** 
* 将RF发送队列的数据读出并向NRF24L01模块写入. 
* 最大重发3次.
* @param[in]   pvParameters：线程参数. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
void rf_module_send_task( void *pvParameters )
{
	wait_for_startup();
	
	TickType_t ticks = xTaskGetTickCount();
	MSG_T		msg;
//	uint8_t		wait_reply;
	
	while(1){
		
		if( rf_module.send_sta.delay_time < 255 ){
			rf_module.send_sta.delay_time++;
		}
		if( (rf_module.send_sta.delay_time >= RF_SEND_DELAY_TIME) && is_rf_on() ){
			
			uint16_t len;
			len=rf_module_read_queue(rf_module.send_queue, &rf_module.send_sta.addr, &rf_module.send_sta.pipe, &msg);
			if( len>0 ){
				
				uint8_t		dataOut[32];
				
//				uint8_t cd;
				
				rf_module_set_target(rf_module.send_sta.addr, rf_module.send_sta.pipe);

				#if defined(USE_USB_RF_MODULE)
				memcpy(&dataOut[1],&msg,len);
				dataOut[0] = len;
				#else
				memcpy(dataOut,&msg,len);
				#endif
				
				EventBits_t event_bits=0;
				rf_module.send_sta.resend_times = 0;
				
				do{
//					do{
//						cd = TM_NRF24L01_ReadRegister(NRF24L01_REG_RPD);
//					}while( cd==1 );
//					rf_module_log("seg send %d\r\n",rf_module.send_sta.resend_times);
										
					TM_NRF24L01_Transmit(dataOut);
					event_bits = xEventGroupWaitBits(rf_module_event_handler,EVENTBIT_TRANSMIT_BIT,pdTRUE,pdFALSE,50);
					rf_module.send_sta.resend_times++;
					
				}while(rf_module.send_sta.resend_times<=RESEND_TIMES && (event_bits&EVENTBIT_TRANSMIT_OK)!=EVENTBIT_TRANSMIT_OK );
				
				if( rf_module.send_sta.resend_times>=RESEND_TIMES ){
					rf_module_log("resend times maxim\r\n");
				}	
				
			}
			rf_module.send_sta.delay_time = 0;
		}
		vTaskDelayUntil( &ticks, 10 );
	}
}

/** 
* 根据接收的地址，初始化接收缓冲区. 
* 最大重发3次.
* @param[in]   addr：接收的数据包地址. 
* @param[out]  无.  
* @retval  RF_RECV_STA_T *:返回接收数据处理数据句柄  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-08-19创建 
*/
RF_RECV_STA_T * rf_module_find_recv_ptr( uint32_t addr )
{
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( rf_module.recv_sta[i].sur_addr == addr ){
			return &rf_module.recv_sta[i];
		}
	}
	
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( rf_module.recv_sta[i].sur_addr == 0 ){
			memset( &rf_module.recv_sta[i], 0, sizeof(RF_RECV_STA_T) );
			rf_module.recv_sta[i].sur_addr = addr;
			return &rf_module.recv_sta[i];
		}
	}
	
	return NULL;
}

/** 
* 将RF发送队列的数据读出并向NRF24L01模块写入. 
* 最大重发3次.
* @param[in]   pvParameters：线程参数. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
/*
void rf_module_recv_task( void *pvParameters ){
	
	TickType_t ticks = xTaskGetTickCount();
	MSG_T		msg;
//	uint8_t		wait_reply;
	uint8_t		seg_cnt=0,total_cnt=0;
	while(1){
		
		if( rf_module_read_queue(rf_module.recv_queue, &rf_module.recv_sta.sur_addr, &msg) ){
			
			switch(rf_module.recv_sta.step){
				
				case 0:
					
					if( msg.seg_no==1 && msg.seg_total<=RECV_QUEUE_LENGTH ){
						rf_module.recv_sta.buf = malloc(MSG_DATA_LEN*msg.seg_total);
						if( rf_module.recv_sta.buf!=NULL ){
							rf_module.recv_sta.len = 0;
							memcpy((void*)(rf_module.recv_sta.buf + rf_module.recv_sta.len),msg.seg_data,msg.seg_sta.transmit.data_len);
							rf_module.recv_sta.len += msg.seg_sta.transmit.data_len;
							
							rf_module.recv_sta.step = 1;
							rf_module.recv_sta.timeout = 0;
							seg_cnt = 1;
							total_cnt = msg.seg_total;
						}
					}
					break;
					
				case 1:
					if( seg_cnt<total_cnt ){
						seg_cnt++;
						memcpy(rf_module.recv_sta.buf + rf_module.recv_sta.len,msg.seg_data,msg.seg_sta.transmit.data_len);
						rf_module.recv_sta.len += msg.seg_sta.transmit.data_len;					
						
						rf_module.recv_sta.timeout = 0;
					}
					else{
						free(rf_module.recv_sta.buf);
						rf_module.recv_sta.step=0;
					}
					break;
			}
			
			if( msg.seg_no == msg.seg_total ){
						
				int cnt;
				uint32_t crc,*recv_crc;
				recv_crc = (uint32_t*)(rf_module.recv_sta.buf+rf_module.recv_sta.len-4);
				crc = crc32(rf_module.recv_sta.buf,rf_module.recv_sta.len-4);
				if( crc == *recv_crc ){
//					cbuf_clear(&rf_module.cbsta);
//					cbuf_writebk(&rf_module.cbsta,rf_module.recv_sta.buf,rf_module.recv_sta.len-4,&cnt);
					comm_recv_in_queue(rf_module.recv_sta.buf, rf_module.recv_sta.len-4);
//					comm_recv_complete();
					rf_module.recv_sta.step = 0;
//					free(rf_module.recv_sta.buf);
					rf_module.recv_sta.buf = NULL;
					
				}
			}
		}
		
		///<receive timout reset receive process
		if( rf_module.recv_sta.step!=0 ){
			
			if( rf_module.recv_sta.timeout++ > RF_RECV_TIMEOUT_TIME ){
				free(rf_module.recv_sta.buf);
				rf_module.recv_sta.step=0;
			}
		}
		vTaskDelayUntil( &ticks, 10 );
	}
}
*/
void rf_module_recv_task( void *pvParameters )
{

	wait_for_startup();
	
	TickType_t ticks = xTaskGetTickCount();
	MSG_T		msg;
//	uint8_t		wait_reply;
	uint8_t		seg_cnt=0,total_cnt=0,pipe;
	uint32_t 	recv_addr;
	RF_RECV_STA_T * recv_ptr;
	while(1){
		
		if( rf_module_read_queue(rf_module.recv_queue, &recv_addr, &pipe, &msg) ){
			
			recv_ptr = rf_module_find_recv_ptr(recv_addr);
			
			if( recv_ptr!=NULL ){
			
				switch(recv_ptr->step){
					
					case 0:
						
						if( msg.seg_no==1 && msg.seg_total<=RECV_QUEUE_LENGTH ){
							recv_ptr->buf = malloc(MSG_DATA_LEN*msg.seg_total);
							if( recv_ptr->buf!=NULL ){
								recv_ptr->len = 0;
								memcpy((void*)(recv_ptr->buf + recv_ptr->len),msg.seg_data,msg.seg_sta.transmit.data_len);
								recv_ptr->len += msg.seg_sta.transmit.data_len;
								
								recv_ptr->step = 1;
								recv_ptr->timeout = 0;
								seg_cnt = 1;
								total_cnt = msg.seg_total;
							}
						}
						break;
						
					case 1:
						if( seg_cnt<total_cnt ){
							seg_cnt++;
							memcpy(recv_ptr->buf + recv_ptr->len,msg.seg_data,msg.seg_sta.transmit.data_len);
							recv_ptr->len += msg.seg_sta.transmit.data_len;					
							
							recv_ptr->timeout = 0;
						}
						else{
							free(recv_ptr->buf);
							recv_ptr->step=0;
							//段错误计数
							recv_ptr->err_sta.seg_err++;
						}
						break;
				}
				
				if( msg.seg_no == msg.seg_total ){
							
					int cnt;
					uint32_t crc,*recv_crc;
					recv_crc = (uint32_t*)(recv_ptr->buf+recv_ptr->len-4);
					//rf_module_log("len=%d\r\n",recv_ptr->len);
					if( recv_ptr->len > 4 ){
						crc = crc32(recv_ptr->buf,recv_ptr->len-4);
						if( crc == *recv_crc ){
							comm_recv_in_queue(recv_ptr->buf, recv_ptr->len-4);
							
	//						rf_module_dump(recv_ptr->buf, recv_ptr->len-4);
	//						char buf[20];
	//						memcpy(buf,recv_ptr->buf,recv_ptr->len-4);
	//						rf_module_log("%s",buf);
							
							//memset( recv_ptr, 0, sizeof(RF_RECV_STA_T) );
							recv_ptr->step = 0;
							recv_ptr->buf = NULL;
							recv_ptr->len= 0;
							recv_ptr->timeout = 0;
						}
						else{
							//CRC错误计数
							recv_ptr->err_sta.crc_err++;
						}
					}
					else{
						recv_ptr->err_sta.size_err++;
					}
				}
			}
		}
		
		for( uint8_t i=0; i<EVT_NUM; i++ ){
			if( rf_module.recv_sta[i].sur_addr != 0 ){
				if( rf_module.recv_sta[i].timeout++ > RF_RECV_TIMEOUT_TIME ){
					free(rf_module.recv_sta[i].buf);
					memset( &rf_module.recv_sta[i], 0, sizeof(RF_RECV_STA_T) );
				}
			}
		}
		
		vTaskDelayUntil( &ticks, 10 );
	}
}

/** 
* 向RF模块发送队列写入发送数据. 
* 将数据分包写入RF模块发送队列. 
* @param[in]   addr:发送地址,ptr:发送数据指针,size;发送数据大小. 
* @param[out]  无.  
* @retval  成功发送数据大小
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
uint32_t rf_module_send(uint32_t target_addr, uint32_t source_addr,  uint8_t chn, uint8_t *ptr, uint16_t size )
{	
	uint16_t 	send_len,seg=0,send_ptr=0;
	uint32_t	crc;

	MSG_T		msg;
	memset(&msg,0,sizeof(MSG_T));
	
	crc = crc32(ptr,size);
	
	void *data;
	data = malloc(size+4);
	if( data!=NULL ){
		memcpy(data,ptr, size);
		memcpy((void*)(data+size),(void*)&crc, 4);
		
		size += 4;
		
		msg.seg_total = size/MSG_DATA_LEN;
		if( size%MSG_DATA_LEN )
			msg.seg_total += 1;
		///<发送EBB8位地址
		msg.source_addr = source_addr&0xff;
		
		while( send_ptr<size ){
			
			if( (size-send_ptr)>MSG_DATA_LEN ){
				memcpy((void*)msg.seg_data,(void*)(data+send_ptr),MSG_DATA_LEN);
				send_ptr += MSG_DATA_LEN;
				msg.seg_no = ++seg;
				msg.seg_sta.transmit.data_len= MSG_DATA_LEN;
				
				send_len = sizeof(MSG_T);
			}
			else{
				memcpy((void*)msg.seg_data,(void*)(data+send_ptr),(size-send_ptr));
				send_len = sizeof(MSG_T);
				send_len = sizeof(MSG_T) - MSG_DATA_LEN + (size-send_ptr);
				msg.seg_sta.transmit.data_len= (size-send_ptr);
				send_ptr += (size-send_ptr);
				msg.seg_no = ++seg;
				size = 0;
			}
			
			RF_MSG_T	rf_msg;
			rf_msg.len = send_len;
			rf_msg.wait_reply = 0;
			rf_msg.addr = target_addr;
			rf_msg.pipe = chn;
			
			if( FUN_OK != rf_module_write_queue(rf_module.send_queue, &rf_msg, &msg, 0) ){
				rf_module_log("Send data enqueue Error\r\n");
			}
		}
		
		free(data);
		return size;
	}
	return 0;
}

/** 
* 计算的目标的地址，并设置发送通道地目标地址. 
* 无. 
* @param[in]   target:目标地址. 
* @param[out]  无.  
* @retval  成功发送数据大小
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
void rf_module_set_target(uint32_t target, uint8_t pipe){
	
	rf_module_set_tx_addr(target,pipe);
}

/** 
* 初始化NRF24L01模块，并启动中断检测，启动发送及接收线程,建立发送队列及接收队列. 
* 无. 
* @param[in]   target:目标地址. 
* @param[out]  无.  
* @retval  1：初始化成功，0：初始化失败
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-29创建 
*/
FUN_STATUS_T rf_module_init(void){
	
	memset(&rf_module,0,sizeof(RF_MODULE_T));
	
	/* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
	/* By default 2Mbps data rate and 0dBm output power */
	/* NRF24L01 goes to RX mode by default */
	TM_NRF24L01_Init(0, 32);
	
	/* Set 2MBps data rate and -18dBm output power */
	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_2M, TM_NRF24L01_OutputPower_0dBm);
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_AW, 	3);
	
	rf_module_set_rx_addr(EBB_RF_DEFAULT_ADDR, RF_EBB_CHN_REQ);
	rf_module_set_rx_addr(EBB_RF_DEFAULT_ADDR, RF_EBB_CHN_REG1);
	rf_module_set_rx_addr(EBB_RF_DEFAULT_ADDR, RF_EBB_CHN_POLL);
	rf_module_set_rx_addr(EBB_RF_DEFAULT_ADDR, RF_EBB_CHN_REG2);
	rf_module_set_rx_addr(EBB_RF_DEFAULT_ADDR, RF_EBB_CHN_REG3);
	
	TM_NRF24L01_PowerUpRx();
//	rf_module_set_target(0);
	
//	if( config.comm_process>=PROCESS_INIT ){
//		uint32_t addr;
//		addr = RF_DEFAULT_ADDR+config.rf_id;
//		rf_module_set_rx_addr(addr,2);
////		comm_state.local_addr = addr;
//	}
//	else{
//		rf_module_set_rx_addr(0xffffffff, 2);
//	}
	
	/* Attach interrupt for NRF IRQ pin */
	TM_EXTI_Attach(RF_IRQ_PORT, RF_IRQ_PIN, TM_EXTI_Trigger_Falling);
	
	rf_module.send_queue = xQueueCreate(SEND_QUEUE_LENGTH,sizeof(RF_MSG_T));
	if( rf_module.send_queue ==0 ){
		
		rf_module_log("can not create rf send queue!!!\r\n");
		return FUN_ERROR;
	}
	
	rf_module.recv_queue = xQueueCreate(RECV_QUEUE_LENGTH,sizeof(RF_MSG_T));
	if( rf_module.recv_queue ==0 ){
		
		rf_module_log("can not create rf recv queue!!!\r\n");
		return FUN_ERROR;
	}
	
	rf_module_event_handler=xEventGroupCreate();      /*创建事件标志组*/
	
	xTaskCreate( rf_module_send_task, "RF SEND", RF_MODULE_SEND_STACK_SIZE, NULL, RF_MODULE_SEND_TASK_PRIORITY, daemon_from_create );
	xTaskCreate( rf_module_recv_task, "RF RECV", RF_MODULE_RECV_STACK_SIZE, NULL, RF_MODULE_RECV_TASK_PRIORITY, daemon_from_create );
	
	sys_status.have_rf_module = 1;
	
	return FUN_OK;
}
