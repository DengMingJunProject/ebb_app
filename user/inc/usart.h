#ifndef __USART_H__
#define __USART_H__

#include "stdint.h"
#include "stdbool.h"

//#define	USART1_EN
//#define USART2_EN
//#define USART3_EN
//#define USART4_EN
//#define USART5_EN
#define USART6_EN

enum{
	#ifdef USART1_EN
	USART1_NUM,
	#endif
	#ifdef USART2_EN
	USART2_NUM,
	#endif
	#ifdef USART3_EN
	USART3_NUM,
	#endif
	#ifdef USART4_EN
	USART4_NUM,
	#endif
	#ifdef USART5_EN
	USART5_NUM,
	#endif
	#ifdef USART6_EN
	USART6_NUM,
	#endif
	MAX_UART_NUM,
};

extern int usart_open( uint32_t uartx, uint32_t bps, uint32_t word_len, int even, int stop,
                uint8_t *rbuf, uint32_t rbuf_size, uint8_t *tbuf, uint32_t tbuf_size );
extern void usart_send( uint32_t uartx, uint8_t *ptr, uint32_t len );
extern int usart_send_timeout( uint32_t uartx, uint8_t *ptr, uint32_t len, uint32_t timeout );
extern uint32_t usart_num( uint32_t uartx );
extern uint32_t usart_pop( uint32_t uartx, uint8_t *buf, uint32_t len );
extern uint8_t usart_pop_byte( uint32_t uartx );
extern uint32_t usart_pop_time( uint32_t uartx, uint8_t *buf, uint32_t len, uint32_t timeout );
extern void usart_reset( uint32_t uartx );
extern void usart_dump_rxbuf( uint32_t uartx );
#endif
