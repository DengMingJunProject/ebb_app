/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
 * NOTE:  This file uses a third party USB CDC driver.
 */

/* Standard includes. */
#include "string.h"
#include "stdio.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Example includes. */
#include "FreeRTOS_CLI.h"

/* Demo application includes. */
#include "console_uart.h"
#include "console_user.h"
#include "Driver_USART.h"
#include "system.h"

uint8_t		console_send_buf[1024];
uint8_t		console_recv_buf[1024];

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE		50

#define cmdQUEUE_LENGTH			25

/* DEL acts as a backspace. */
#define cmdASCII_DEL		( 0x7F )

#define cmdMAX_MUTEX_WAIT		( ( ( TickType_t ) 300 ) / ( portTICK_PERIOD_MS ) )

#ifndef configCLI_BAUD_RATE
	#define configCLI_BAUD_RATE	115200
#endif

//extern ARM_DRIVER_USART Driver_USART6;
//static ARM_DRIVER_USART *concole_usart_drv = &Driver_USART6;

//void console_usart_callback(uint32_t event)
//{
//    switch (event)
//    {
//    case ARM_USART_EVENT_RECEIVE_COMPLETE:    
//    case ARM_USART_EVENT_TRANSFER_COMPLETE:
//    case ARM_USART_EVENT_SEND_COMPLETE:
//    case ARM_USART_EVENT_TX_COMPLETE:
//        /* Success: Wakeup Thread */
////        osSignalSet(tid_scanner_usart_thread, 0x01);
//        break;
// 
//    case ARM_USART_EVENT_RX_TIMEOUT:
////         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
////		osSignalSet(tid_scanner_usart_thread, 0x01);
//        break;
// 
//    case ARM_USART_EVENT_RX_OVERFLOW:
//    case ARM_USART_EVENT_TX_UNDERFLOW:
////        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//        break;
//    }
//}

//__weak void console_uart_init(uint32_t baud){
//	
//	concole_usart_drv->Initialize(console_usart_callback);
//	
//	concole_usart_drv->PowerControl(ARM_POWER_FULL);
//    /*Configure the USART to 4800 Bits/sec */
//    concole_usart_drv->Control(ARM_USART_MODE_ASYNCHRONOUS |
//                      ARM_USART_DATA_BITS_8 |
//                      ARM_USART_PARITY_NONE |
//                      ARM_USART_STOP_BITS_1 |
//                      ARM_USART_FLOW_CONTROL_NONE, baud);
//     
//    /* Enable Receiver and Transmitter lines */
//    concole_usart_drv->Control (ARM_USART_CONTROL_TX, 1);
//    concole_usart_drv->Control (ARM_USART_CONTROL_RX, 1);
//}

//__weak void console_uart_send(uint8_t *buf,uint16_t len){
//	
//	ARM_USART_STATUS	usart_status;
//	
//	usart_status = concole_usart_drv->GetStatus();
//	if( usart_status.tx_busy ){
//	}
//	
//	concole_usart_drv->Send(buf,len);
//	
//	do{
//		vTaskDelay(1);
//		usart_status = concole_usart_drv->GetStatus();
//	}while(usart_status.tx_busy);
//}

//__weak uint8_t console_uart_get(uint8_t *buf){
//	if( concole_usart_drv->Receive(buf,1)==ARM_DRIVER_OK){
//		return 1;
//	}
//	else{
//		return 0;
//	}
//}


//#define console_uart_recv_time(buf,ti)		(buf,ti)//ringBufPop_block( 5, buf, 1, ti )
//#define console_uart_get(byte)				(byte)//ringBufPop( 5, byte, 1 )
//#define console_uart_num()					//ringBufNum( 5 )
//#define console_uart_clr()					//ringBufClr(5);memset(com_buf.slave_reserve_recv_buf,0,SLAVE_RECV_LEN)
//#define console_uart_recv_trigger()			//start_uart_rx( 5)

//extern void uart_console_start( uint16_t usStackSize, UBaseType_t uxPriority );


/*-----------------------------------------------------------*/

/*
 * The task that implements the command console processing.
 */
static void uart_console_task( void *pvParameters );

/*-----------------------------------------------------------*/

/* Const messages output by the command console. */
static const char * const pcWelcomeMessage = "LAXTON command server.\r\nType Help to view a list of registered commands.\r\n\r\n>";
static const char * const pcEndOfOutputMessage = "\r\n[Press ENTER to execute the previous command again]\r\n>";
static const char * const pcNewLine = "\r\n";

SemaphoreHandle_t xTxMutex = NULL;

/* Serial drivers that use task notifications may need the CLI task's handle. */
TaskHandle_t xCLITaskHandle = NULL;

/*-----------------------------------------------------------*/

FUN_STATUS_T uart_console_start( uint16_t usStackSize, UBaseType_t uxPriority )
{
	/* Create the semaphore used to access the UART Tx. */
	xTxMutex = xSemaphoreCreateMutex();
	configASSERT( xTxMutex );

	/* Create that task that handles the console itself. */
	if( xTaskCreate( 	uart_console_task,			/* The task that implements the command console. */
					"CLI",						/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
					usStackSize,				/* The size of the stack allocated to the task. */
					NULL,						/* The parameter is not used, so NULL is passed. */
					uxPriority,					/* The priority allocated to the task. */
					&xCLITaskHandle ) == pdPASS)			/* Serial drivers that use task notifications may need the CLI task's handle. */
		return FUN_OK;
	else
		return FUN_ERROR;
}
/*-----------------------------------------------------------*/

static void uart_console_task( void *pvParameters )
{
char cRxedChar;
uint8_t ucInputIndex = 0;
char *pcOutputString;
static char cInputString[ cmdMAX_INPUT_SIZE ], cLastInputString[ cmdMAX_INPUT_SIZE ];
BaseType_t xReturned;

	( void ) pvParameters;

	/* Obtain the address of the output buffer.  Note there is no mutual
	exclusion on this buffer as it is assumed only one command console interface
	will be used at any one time. */
	pcOutputString = FreeRTOS_CLIGetOutputBuffer();

	/* Initialise the UART. */
	console_uart_init(115200);

	console_uart_send((uint8_t*)"\r\n\r\n",4);
	console_uart_send((uint8_t*)"###          ####      ###    ##  ##########    ########     ###      ##\r\n",76);
	console_uart_send((uint8_t*)"###         ## ##       ###  ##       ##      ###      ##    #####    ##\r\n",76);
	console_uart_send((uint8_t*)"###         ##  ##       #####        ##      ##       ###   ##  ##   ##\r\n",76);
	console_uart_send((uint8_t*)"###        ##   ###       ###         ##     ###        ##   ##   ##  ##\r\n",76);
	console_uart_send((uint8_t*)"###       #########      ## ##        ##      ##       ###   ##    ## ##\r\n",76);
	console_uart_send((uint8_t*)"###       ##     ###    ##   ##       ##      ###      ##    ##     ####\r\n",76);
	console_uart_send((uint8_t*)"######## ###      ###  ##     ###     ##        #######      ##      ###\r\n",76);
	
	/* Send the welcome message. */
	console_uart_send( (uint8_t*) pcWelcomeMessage, strlen( pcWelcomeMessage ) );

	extern void vRegisterSampleCLICommands( void );
	vRegisterSampleCLICommands();
	console_user_init();
	
	for( ;; )
	{
		/* Wait for the next character.  The while loop is used in case
		INCLUDE_vTaskSuspend is not set to 1 - in which case portMAX_DELAY will
		be a genuine block time rather than an infinite block time. */
		while( console_uart_get( (uint8_t*)&cRxedChar ) ==0 ){
			vTaskDelay( 1 );
		}

		/* Ensure exclusive access to the UART Tx. */
		if( xSemaphoreTake( xTxMutex, cmdMAX_MUTEX_WAIT ) == pdPASS )
		{
			/* Echo the character back. */
			console_uart_send( (uint8_t*)&cRxedChar, 1 );

			/* Was it the end of the line? */
			if( ( cRxedChar == '\n' ) || ( cRxedChar == '\r' ) )
			{
				/* Just to space the output from the input. */
				console_uart_send( (uint8_t*) pcNewLine, strlen( pcNewLine ) );

				/* See if the command is empty, indicating that the last command
				is to be executed again. */
				if( ucInputIndex == 0 )
				{
					/* Copy the last command back into the input string. */
					strcpy( cInputString, cLastInputString );
				}

				/* Pass the received command to the command interpreter.  The
				command interpreter is called repeatedly until it returns
				pdFALSE	(indicating there is no more output) as it might
				generate more than one string. */
				do
				{
					/* Get the next output string from the command interpreter. */
					xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

					/* Write the generated string to the UART. */
					console_uart_send( (uint8_t*) pcOutputString, strlen( pcOutputString ) );

				} while( xReturned != pdFALSE );
				
				console_timer_reset();

				/* All the strings generated by the input command have been
				sent.  Clear the input string ready to receive the next command.
				Remember the command that was just processed first in case it is
				to be processed again. */
				strcpy( cLastInputString, cInputString );
				ucInputIndex = 0;
				memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );

				console_uart_send( (uint8_t*) pcEndOfOutputMessage, strlen( pcEndOfOutputMessage ) );
			}
			else
			{
				if( cRxedChar == '\r' )
				{
					/* Ignore the character. */
				}
				else if( ( cRxedChar == '\b' ) || ( cRxedChar == cmdASCII_DEL ) )
				{
					/* Backspace was pressed.  Erase the last character in the
					string - if any. */
					if( ucInputIndex > 0 )
					{
						ucInputIndex--;
						cInputString[ ucInputIndex ] = '\0';
					}
				}
				else
				{
					/* A character was entered.  Add it to the string entered so
					far.  When a \n is entered the complete	string will be
					passed to the command interpreter. */
					if( ( cRxedChar >= ' ' ) && ( cRxedChar <= '~' ) )
					{
						if( ucInputIndex < cmdMAX_INPUT_SIZE )
						{
							cInputString[ ucInputIndex ] = cRxedChar;
							ucInputIndex++;
						}
					}
				}
			}

			/* Must ensure to give the mutex back. */
			xSemaphoreGive( xTxMutex );
		}
	}
}
/*-----------------------------------------------------------*/

void vOutputString( const char * const pcMessage )
{
	if( xSemaphoreTake( xTxMutex, cmdMAX_MUTEX_WAIT ) == pdPASS )
	{
		console_uart_send( (uint8_t*) pcMessage, strlen( pcMessage ) );
		xSemaphoreGive( xTxMutex );
	}
}
/*-----------------------------------------------------------*/

void vOutputChar( const char cChar, const TickType_t xTicksToWait  )
{
	if( xSemaphoreTake( xTxMutex, xTicksToWait ) == pdPASS )
	{
		console_uart_send( (uint8_t*)&cChar, 1 );
		xSemaphoreGive( xTxMutex );
	}
}
/*-----------------------------------------------------------*/
