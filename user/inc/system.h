#ifndef __FREERTOS_HEADER_H__
#define __FREERTOS_HEADER_H__

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "semphr.h" 
#include "timers.h"
#include "event_groups.h"
#include "cmsis_os.h"
#include "dbg.h"
#include "freertos_malloc.h"

#define DEBUG(x)    (x)
#define DEBUG_ENABLE	1					///debug main switch

#ifdef __DEBUG_VERSION__
#define MAIN_DEBUG
#define RTC_DEBUG
#define DAEMON_DEBUG
#define RF_MODULE_DEBUG
#define MID_FS_DEBUG
#define EVENTLOG_DEBUG
#define AT88SC_DEBUG
#define SMARTCARD_DEBUG
#define DISPLAY_DEBUG
#define COMM_DEBUG
#define MENU_FUNC_DEBUG
#define CONFIG_DEBUG
#define OTA_DEBUG
#define CRYPT_DEBUG
#define IOBOARD_DEBUG
#define BUTTON_DEBUG
#define PRINER_DEBUG
#define FS_DEBUG
#define LCDXX_DEBUG
#define USART_DEBUG
#define LEFS_DEBUG
#define ELECTION_DEBUG
#define DEVFS_DEBUG
#endif

///< RF module发送处理线程参数.  
#define RF_MODULE_SEND_TASK_PRIORITY		configMAX_PRIORITIES - 1
#define RF_MODULE_SEND_STACK_SIZE			512
///< RF module接收处理线程参数.  
#define RF_MODULE_RECV_TASK_PRIORITY		configMAX_PRIORITIES - 1
#define RF_MODULE_RECV_STACK_SIZE			512
///< MAIN线程处理线程参数.  
#define MAIN_TASK_PRIORITY					configMAX_PRIORITIES - 20
#define MAIN_STACK_SIZE						1024
///< 通信轮询处理线程参数.  
#define COMM_POLL_TASK_PRIORITY				configMAX_PRIORITIES - 2
#define COMM_POLL_STACK_SIZE				1024
///< 通信轮询处理线程参数.  
#define COMM_SEND_TASK_PRIORITY				configMAX_PRIORITIES - 2
#define COMM_SEND_STACK_SIZE				1024
///< 智能卡处理线程参数.  
#define SMARTCARD_TASK_PRIORITY				configMAX_PRIORITIES - 1
#define SMARTCARD_STACK_SIZE				2048
///< 显示处理线程参数.  
#define DISPLAY_TASK_PRIORITY				configMAX_PRIORITIES - 1
#define DISPLAY_STACK_SIZE					512
///< 选举状态处理线程参数.  
#define ELECTION_TASK_PRIORITY				configMAX_PRIORITIES - 5
#define ELECTION_STACK_SIZE					1024
///< IO，按键处理线程参数.  
#define IOC_TASK_PRIORITY					configMAX_PRIORITIES - 4
#define IOC_STACK_SIZE						512
///< 紧急情况处理线程参数.  
#define URG_TASK_PRIORITY					configMAX_PRIORITIES - 10
#define URG_STACK_SIZE						512
///< 事件记录处理线程参数.  
#define EVENT_LOG_TASK_PRIORITY				configMAX_PRIORITIES - 20
#define EVENT_LOG_STACK_SIZE				1024
///< OTA处理线程参数.  
#define OTA_TASK_PRIORITY					configMAX_PRIORITIES - 7
#define OTA_STACK_SIZE						1024
///< 命令行调试线程参数.  
#define CONSOLE_TASK_PRIORITY				configMAX_PRIORITIES - 15
#define CONSOLE_STACK_SIZE					1024
/////< WinUSB.  
//#define WINUSB_TASK_PRIORITY				configMAX_PRIORITIES - 6
//#define WINUSB_STACK_SIZE					512
///< USB Host.  
#define USB_HOST_TASK_PRIORITY				configMAX_PRIORITIES - 6
#define USB_HOST_STACK_SIZE					1024
///< message storage.  
#define MESSAGE_STORAGE_TASK_PRIORITY		configMAX_PRIORITIES - 6
#define MESSAGE_STORAGE_STACK_SIZE			512
#endif
