#ifndef __UART_CONSOLE_H__
#define __UART_CONSOLE_H__

#include "stdint.h"
#include "stdbool.h"
#include "usart.h"
#include "system.h"
#include "macro_def.h"

#define console_uart_init(baud)				usart_open( USART6_NUM, baud, 8, 0, 1, console_recv_buf, 1024, console_send_buf, 1024 )
#define console_uart_send(buf,len)			usart_send_timeout( USART6_NUM, buf, len, 1000 )
#define console_uart_recv(buf,len)			usart_pop_time( USART6_NUM, buf, len, 10 )
#define console_uart_get(byte)				usart_pop( USART6_NUM, byte, 1 )

extern FUN_STATUS_T uart_console_start( uint16_t usStackSize, UBaseType_t uxPriority );
//extern void console_uart_send(uint8_t *buf,uint16_t len);
//extern uint8_t console_uart_get(uint8_t *buf);
#endif

