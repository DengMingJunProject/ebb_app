/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::USB:Device
 * Copyright (c) 2004-2018 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    WinUSB_Echo.c
 * Purpose: USB Device WinUSB Device Echo Bulk Out to Bulk In
 *----------------------------------------------------------------------------*/
#include "system.h"
#include "main.h"
#include "WinUSB_Echo.h"
#include "rl_usb.h"

#include "RTE_Components.h"
#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif

// Main stack size must be multiple of 8 Bytes
//#define APP_MAIN_STK_SZ (1024U)
//uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
//const osThreadAttr_t app_main_attr = {
//.stack_mem  = &app_main_stk[0],
//.stack_size = sizeof(app_main_stk)
//};

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
//__NO_RETURN void winusb_task (void *argument) {
//  (void)argument;

//#ifdef RTE_Compiler_EventRecorder
//  EventRecorderInitialize(0, 1);
//  EventRecorderEnable (EventRecordError, 0xA0U, 0xA6U);  /* USBD Error Events */
//  EventRecorderEnable (EventRecordAll  , 0xA0U, 0xA0U);  /* USBD Core Events */
//  EventRecorderEnable (EventRecordAll  , 0xA2U, 0xA2U);  /* USBD Custom Class Events */
//#endif

//  USBD_Initialize(0U);                  // USB Device 0 Initialization
//  USBD_Connect   (0U);                  // USB Device 0 Connect

//  while (1) {
////    osThreadFlagsWait (0, osFlagsWaitAny, osWaitForever);
//	  vTaskDelay( 1000 );
//  }
//}

FUN_STATUS_T winusb_init(void){
//	xTaskCreate( winusb_task, "WINUSB", WINUSB_STACK_SIZE, NULL, WINUSB_TASK_PRIORITY, NULL	);
	#ifdef RTE_Compiler_EventRecorder
	EventRecorderInitialize(0, 1);
	EventRecorderEnable (EventRecordError, 0xA0U, 0xA6U);  /* USBD Error Events */
	EventRecorderEnable (EventRecordAll  , 0xA0U, 0xA0U);  /* USBD Core Events */
	EventRecorderEnable (EventRecordAll  , 0xA2U, 0xA2U);  /* USBD Custom Class Events */
	#endif

	USBD_Initialize(0U);                  // USB Device 0 Initialization
	USBD_Connect   (0U);                  // USB Device 0 Connect
	
	return FUN_OK;
}
