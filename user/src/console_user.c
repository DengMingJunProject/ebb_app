/** 
* @file         console.c 
* @brief        控制台用户命令. 
* @details  	用于调试显示程序运行状态. 
* @author       kun deng
* @date     2019-07-05 
* @version  A001 
* @par Copyright (c):  
*       LAXTON 
* @par History:          
*   version: kun, 2019-07-05, 建立\n 
*/  
#include "system.h"
#include "console_user.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"
#include <string.h>
#include "stdlib.h"
#include "xmodem.h"
#include "console_uart.h"
#include <time.h>
#include "main.h"
#include "crc32.h"
#include "config.h"
#include "tm_stm32_rtc.h"
#include "rl_fs.h"
#include "comm.h"
#include "fs.h"
#include "devfs.h"
#include "rf_module.h"
#include "console_run_time.h"
#include "display.h"
#include "lefs.h"
#include "election.h"

#define	console_user_pwd  "laxton"

static uint8_t login_flag=0;
static TimerHandle_t	console_timer;

//static BaseType_t console_user_test_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
//	( void ) pcCommandString;
//	( void ) xWriteBufferLen;
//	configASSERT( pcWriteBuffer );
//	
//	if( login_flag==0 ){
//		
//		strcpy(pcWriteBuffer,"请先登录\r\n");
//		return pdFALSE;
//	}
//	
//	const char *pcParameter;
//	BaseType_t xParameterStringLength, xReturn;
//	char para[2][20];
//	
//	for( uint8_t i=1; i<=1; i++ ){
//		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

//		if( pcParameter != NULL ){
//			
//			uint8_t len;
//			len = xParameterStringLength>20?20:xParameterStringLength;
//			strncpy( para[i-1], pcParameter, len );
//			para[i-1][len] = 0;
//		}
//	}
//	
//	uint8_t	type;
//	*pcWriteBuffer = 0;
//	
//	int res;
//	static FILE_HANDLER fd;
//			
//	res = midfs_fopen( &fd, para[0], "r" );
//	
//	if( res<0 ){
//		strcpy(pcWriteBuffer,"打开文件失败\r\n");
//	}
//	else{
//		strcpy(pcWriteBuffer,"打开文件成功\r\n");
//	}

//	return pdFALSE;
//}

//static const CLI_Command_Definition_t console_user_test ={
//	
//	"test",
//	"\r\ntest:		test ---- ",
//	console_user_test_execute,
//	1
//};											

/** 
* 出厂默认状态初始化. 
* 出厂默认状态初始化. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：factoryset <NO.>
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-09-06创建 
*/
static BaseType_t console_user_factory_set_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please loging first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[1][10];
	
	for( uint8_t i=1; i<=1; i++ ){
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>10?10:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
			para[i-1][len] = 0;
		}
	}
	
	uint8_t	no;
	*pcWriteBuffer = 0;

	no = atoi(para[0]);
	
	if( no==0 ){

		sprintf(pcWriteBuffer,"The device has been restored to factory Settings\r\n");
	}
	else{
		
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_factory_set ={
	
	"factoryset",
	"\r\nfactoryset:	factoryset <NO.> ---- restore factory settings ",
	console_user_factory_set_execute,
	1
};
									
/** 
* 上传文件到PC端. 
* 上传文件. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：upload <file>
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-10创建 
*/
static BaseType_t console_user_upload_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char buf[20];
	
	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,1,&xParameterStringLength);

	if( pcParameter != NULL ){
		uint8_t len;
		len = xParameterStringLength>20?20:xParameterStringLength;
		strncpy( buf, pcParameter, len );
		buf[len] = 0;
		int32_t size;
		
		size = xmodem_transmit_file(buf);
		
		if( size>0 )
			sprintf(pcWriteBuffer,"Upload file successful，length:%d\r\n",size);
		else
			sprintf(pcWriteBuffer,"Upload file failed\r\n");
	}
	else{
		strcpy(pcWriteBuffer,"Parameter error");
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_upload ={
	
	"upload",
	"\r\nupload:		upload <filename> ---- transmit the file to PC",
	console_user_upload_execute,
	1
};

/** 
* 下载文件到spi flash,. 
* 将会删除同名文件. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：download <file>
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-09创建 
*/
static BaseType_t console_user_download_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[2][20];
	uint32_t size;
	for( uint8_t i=1; i<=2; i++ ){
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>20?20:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
			para[i-1][len] = 0;
			if( i==2 )
				size = atoi(para[1]);
		}
	}

	if( pcParameter != NULL ){
		int32_t recv_len;
		
		recv_len = xmodem_receive_file(para[0],size);
//		static char buf[20];
//		recv_len = xmodem_receive_memory(buf, size);
		
		*pcWriteBuffer = 0;
		
		sprintf(pcWriteBuffer,"Download file successful，length:%d\r\n",recv_len);
	}
	else{
		strcpy(pcWriteBuffer,"Parameter error");
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_download ={
	
	"download",
	"\r\ndownload:	download <filename> <size> ---- receive the file form the PC,file name ,receive size",
	console_user_download_execute,
	2
};

/** 
* 退出登录命令行操作. 
* 退出登录. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：login <密码>
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_exit_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	strcpy(pcWriteBuffer,"logout\r\n");
	login_flag = 0;
	xTimerStop(console_timer,0);
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_exit ={
	
	"exit",
	"\r\nexit:		exit ---- logout",
	console_user_exit_execute,
	0
};

/** 
* 登录到命令行操作. 
* 验证密码登录操作. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：login <密码>
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_login_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char buf[20];
	
	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,1,&xParameterStringLength);

	if( pcParameter != NULL ){
			
		uint8_t len;
		len = xParameterStringLength>20?20:xParameterStringLength;
		strncpy( buf, pcParameter, len );
		buf[len] = 0;
		
		if( strlen(buf)==strlen(console_user_pwd) ){
			if( strcmp(buf,console_user_pwd)==0 ){
				login_flag = 1;
				sprintf(pcWriteBuffer,"login successful,no input, exit after one hour\r\n");
				xTimerStart(console_timer,0);
			}
			else{
				sprintf(pcWriteBuffer,"password error\r\n");
			}
		}
		else{
			sprintf(pcWriteBuffer,"password error\r\n");
		}
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_login ={
	
	"login",
	"\r\nlogin:		login <pwd> ---- login to CLI",
	console_user_login_execute,
	1
};

/** 
* 设置RTC时间. 
* 从命令行输入设置时间. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：settime 2019-07-05 09:43:12
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_set_time_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[2][20];
	*pcWriteBuffer = 0;
	
	for( uint8_t i=1; i<=2; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>10?10:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
		}
	}
	TM_RTC_t rtc;
	uint16_t year;
	uint8_t  month,day,hour,minute,second;
	sscanf(para[0],"%4d-%2d-%2d",(int*)&year,(int*)&month,(int*)&day);
	sscanf(para[1],"%2d:%2d:%2d",(int*)&hour,(int*)&minute,(int*)&second);
	
	rtc.Year = year%100;
	rtc.Month = month;
	rtc.Day =  day;
	rtc.Hours = hour;
	rtc.Minutes = minute;
	rtc.Seconds = second;
	rtc.WeekDay = 1;
	
	TM_RTC_SetDateTime(&rtc, TM_RTC_Format_BIN);
	
	sprintf(pcWriteBuffer,"set time %04d-%02d-%02d %02d:%02d:%02d successful\r\n",rtc.Year+2000,rtc.Month,rtc.Day,rtc.Hours,rtc.Minutes,rtc.Seconds);
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_set_time ={
	
	"settime",
	"\r\nsettime:	settime <Y-M-D> <H:M:S> ---- set RTC time",
	console_user_set_time_execute,
	2
};

/** 
* 获取RTC时间. 
* 从命令行获取RTC时间. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：gettime 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_get_time_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}
	
	TM_RTC_t rtc;
	TM_RTC_GetDateTime(&rtc, TM_RTC_Format_BIN);
	
	sprintf(pcWriteBuffer,"system time：%04d-%02d-%02d %02d:%02d:%02d\r\n",rtc.Year+2000,rtc.Month,rtc.Day,rtc.Hours,rtc.Minutes,rtc.Seconds);
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_get_time ={
	
	"gettime",
	"\r\ngettime:	gettime ---- read RTC time",
	console_user_get_time_execute,
	0
};

/** 
* 打印线程信息. 
* 从命令行输出线程信息. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：threadinfo
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-10创建 
*/
static BaseType_t console_user_thread_info_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}
	
	*pcWriteBuffer = 0;
	
	sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)],"Task\t\tstatus\tPRIO\tstack\tTaskNum\r\n" );
	vTaskList( &pcWriteBuffer[strlen(pcWriteBuffer)] );
	sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)],"\r\nFreeHeapSize:%u\r\n", xPortGetFreeHeapSize() );
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_thread_info ={
	
	"threadinfo",
	"\r\nthreadinfo:	threadinfo ---- thread infomation",
	console_user_thread_info_execute,
	0
};

#ifdef CONSOLE_ENABLE_RUN_TIME
/** 
* 打印线程CPU占用率. 
* 无. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：runtime
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-10-23创建 
*/
static BaseType_t console_user_run_time_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}
	
	*pcWriteBuffer = 0;
	
	sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)],"Task\t\ttick cnt\tusage rate\r\n" );
	vTaskGetRunTimeStats( &pcWriteBuffer[strlen(pcWriteBuffer)] );
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_run_time ={
	
	"runtime",
	"\r\nruntime:	runtime ---- Run time status",
	console_user_run_time_execute,
	0
};

#endif

/** 
* 打印线程最大堆栈深度. 
* 无. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：runtime
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-10-23创建 
*/
static BaseType_t console_user_wcsd_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}
	
	*pcWriteBuffer = 0;
	
	sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)],"Task\t\t\tMax stack remaining\r\n" );
	
	uint32_t TotalRunTime;
	UBaseType_t task_max,x;
	TaskStatus_t *status_array;
	task_max=uxTaskGetNumberOfTasks();
	status_array=malloc(task_max*sizeof(TaskStatus_t));
	uint32_t rema_stack;
	
	if(status_array!=NULL){
		task_max = uxTaskGetSystemState((TaskStatus_t* )status_array,
		(UBaseType_t)task_max,
		(uint32_t*)&TotalRunTime);
		for( x=0; x<task_max; x++){
			
			rema_stack = uxTaskGetStackHighWaterMark(status_array[x].xHandle);
			
			sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)],"%s\t\t\t%d\r\n",status_array[x].pcTaskName,rema_stack );
		}
	}
	free(status_array);  //释放内存
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_wcsd ={
	
	"wcsd",
	"\r\nwcsd:		wcsd ---- Minimum remaining stack space",
	console_user_wcsd_execute,
	0
};

/** 
* 打印主控硬件及软件信息. 
* 从命令行输出主控硬件及软件信息. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：version
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_version_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}

    for( int i = 0; i < 12; i ++ )
        sprintf( sys_status.sys_id.uuid_str + i * 2, "%02X", sys_status.sys_id.uuid[i] );

	*pcWriteBuffer = 0;
	sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)], "\r\n\r\n================ application info ================\r\n\r\n" );
    sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)], "  DeviceID:\t\t %08X \r\n", sys_status.sys_id.id );
    sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)], "  UID:\t\t\t %s \r\n", sys_status.sys_id.uuid_str );
    sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)], "  Hardware version:\t %05u\r\n", HARDWARE_VERSION );
    sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)], "  Firmware version:\t %u.%u.%u\r\n", SOFTWARE_VERSION / 1000, SOFTWARE_VERSION / 10 % 100,
             SOFTWARE_VERSION % 10 );
    sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)], "  Build time:\t\t %s %s\r\n\r\n", __DATE__, __TIME__ );
    sprintf( &pcWriteBuffer[strlen(pcWriteBuffer)], "==================== LAXTON ====================\r\n\r\n" );
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_version ={
	
	"version",
	"\r\nversion:	version ---- Version infomation",
	console_user_version_execute,
	0
};

/** 
* 重启主控. 
* 从命令行重启主控. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：reboot
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_reboot_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}
	
	static uint8_t reset_flag=1;
	if( reset_flag ){
		
		strcpy(pcWriteBuffer,"rebooting......\r\n");
		reset_flag = 0;
		return pdTRUE;
	}
	else{
		vTaskDelay(1000 );
		HAL_NVIC_SystemReset();
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_reboot ={
	
	"reboot",
	"\r\nreboot:		reboot ---- reboot the board",
	console_user_reboot_execute,
	0
};

/** 
* 格式化SPI flash. 
* 从命令行格式化SPI flash. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：flash
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_format_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	extern const struct lfs_config lfs_cfg;
	
	if( funmount( "N0:" ) == fsOK ){
		if( fs_format ("N0:")== fsOK){
			
			strcpy(pcWriteBuffer,"Format successful\r\n");
			if( fmount( "N0:" ) == fsOK ){
				strcpy(&pcWriteBuffer[strlen(pcWriteBuffer)],"Mount disk successful\r\n");
				return pdFALSE;
			}
			else{
				strcpy(&pcWriteBuffer[strlen(pcWriteBuffer)],"Mount disk failed\r\n");
				return pdFALSE;
			}
		}
		else{
			strcpy(pcWriteBuffer,"format driver failed\r\n");
			return pdFALSE;
		}
	}
	else{
		strcpy(pcWriteBuffer,"Unmount driver failed\r\n");
		return pdFALSE;
	}
}

static const CLI_Command_Definition_t console_user_format ={
	
	"format",
	"\r\nformat:		format <driver> ---- format disk flash",
	console_user_format_execute,
	1
};

/** 
* 删除文件. 
* 删除相应的文件操作. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：rm <file>
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-10创建 
*/
static BaseType_t console_user_rm_file_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){
	
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char buf[20];
	
	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,1,&xParameterStringLength);

	if( pcParameter != NULL ){
		uint8_t len;
		len = xParameterStringLength>20?20:xParameterStringLength;
		strncpy( buf, pcParameter, len );
		buf[len] = 0;
		int res;
		
		*pcWriteBuffer = 0;
		
		res = fdelete(buf,NULL);
		if( res==fsOK )
			sprintf(pcWriteBuffer,"delete file %s Ok\r\n",buf);
		else
			sprintf(pcWriteBuffer,"delete file %s Error\r\n",buf);
	}
	else{
		strcpy(pcWriteBuffer,"parameter error");
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_rm_file ={
	
	"rm",
	"\r\nrm:		rm <file> ---- delete file",
	console_user_rm_file_execute,
	1
};


/** 
* 打印文件列表. 
* 从命令行列出存在所有文件信息. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：ls
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_list_file_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

//	( void ) pcCommandString;
//	( void ) xWriteBufferLen;
//	configASSERT( pcWriteBuffer );
//	
//	if( login_flag==0 ){
//		
//		strcpy(pcWriteBuffer,"请先登录\r\n");
//		return pdFALSE;
//	}

//	lfs_dir_t dir;
//    int err = lfs_dir_open( &mid_fs, &dir, "/" );
//    if( err )
//    {
//        return err;
//    }
//    struct lfs_info info;
//	*pcWriteBuffer = 0;
//    while( true )
//    {
//        int res = lfs_dir_read( &mid_fs, &dir, &info );
//        if( res < 0 )
//        {
//            return res;
//        }
//        if( res == 0 )
//        {
//            break;
//        }
//        switch( info.type )
//        {
//            case LFS_TYPE_REG:
//				if( strlen(info.name)>8 )
//					sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%s\t%u byte\r\n",info.name,info.size);
//				else
//					sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%s\t\t%u byte\r\n",info.name,info.size);
//				break;
//			case LFS_TYPE_DIR:
//				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%s\t\tdir\r\n",info.name);
//				break;
//			default:
//				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"?\r\n");
//				break;
//        }
//    }
//    err = lfs_dir_close( &mid_fs, &dir );
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_list_file ={
	
	"ls",
	"\r\nls:		ls ---- list file list",
	console_user_list_file_execute,
	0
};

/** 
* 以十六制打印文件内容. 
* 从命令行以十六进制打印文件内容. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
static BaseType_t console_user_cat_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char file_name[50];
	static uint32_t	offset=0;
	static FILE *fd;
	
	if( offset==0 ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,1,&xParameterStringLength);

		if( pcParameter != NULL ){
				
			uint8_t len;
			len = xParameterStringLength>50?50:xParameterStringLength;
			strncpy( file_name, pcParameter, len );
			file_name[len] = 0;
						
//			fd = fopen( file_ptr, "rb" );
			fd = fopen( CONECT_STR(DEVFS_DRIVER,"DeviceInformation"), "rb" );
			
			if( fd==NULL ){
				strcpy(pcWriteBuffer,"Open file failed\r\n");
				return pdFALSE;
			}
			else{
				strcpy(pcWriteBuffer,"Open file successful\r\n");
				offset = 1;
				return pdTRUE;
			}
		}
	}
	else{
		uint8_t	buf[16],len;
		if( offset==1 )
			offset = 0;
		fseek( fd, offset, SEEK_SET );
		len = fread( buf, sizeof(uint8_t), 16, fd );
		
		*pcWriteBuffer = 0;
//		if( len>0 )
//			sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"<%04d>",(offset/16)+1);
		for( uint8_t i=0; i<len; i++ ){
			sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%02X ",buf[i]);
		}
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"\r\n");
		
		if(  len != 16 ){
			
			fclose(fd);
			offset = 0;
			sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Read file complete\r\n");
			return pdFALSE;
		}
		else{
			offset += 16;
			return pdTRUE;
		}
		
	}
	
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_cat ={
	
	"cat",
	"\r\ncat:		cat <file> ---- list a file as hex",
	console_user_cat_execute,
	1
};

/** 
* 列出已经向EBB注册成功的EVT设备地址及UID号. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-09-04创建 
*/
static BaseType_t console_user_ls_evt_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
		
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"List all registered EVT devices information\r\n");
	sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Addr\tUID\r\n");
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( comm_state.evt_state[i].addr != EVT_ADDR_NULL ){
			sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%02x\t%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",\
			comm_state.evt_state[i].comm_evt_data.rf_id,
			comm_state.evt_state[i].comm_evt_data.evt_id[0],comm_state.evt_state[i].comm_evt_data.evt_id[1],comm_state.evt_state[i].comm_evt_data.evt_id[2],\
			comm_state.evt_state[i].comm_evt_data.evt_id[3],comm_state.evt_state[i].comm_evt_data.evt_id[4],comm_state.evt_state[i].comm_evt_data.evt_id[5],\
			comm_state.evt_state[i].comm_evt_data.evt_id[6],comm_state.evt_state[i].comm_evt_data.evt_id[7],comm_state.evt_state[i].comm_evt_data.evt_id[8],\
			comm_state.evt_state[i].comm_evt_data.evt_id[9],comm_state.evt_state[i].comm_evt_data.evt_id[10],comm_state.evt_state[i].comm_evt_data.evt_id[11]);
		}
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_ls_evt ={
	
	"lsevt",
	"\r\nlsevt:		lsevt ---- List all registered EVT devices",
	console_user_ls_evt_execute,
	0
};

/** 
* 列出已经向EBB注册成功的EVT设备地址及UID号. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-09-04创建 
*/
static BaseType_t console_user_ls_comm_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
		
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"List EVT communication states\r\n");
	sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Addr\tSent\tReceived\tTimeout\tReply\tMaxTime\tIdError\r\n");
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( comm_state.evt_state[i].addr != EVT_ADDR_NULL ){
			sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%d\t%d\t%d\t\t%d\t%d\t%d\t%d\r\n",\
			comm_state.evt_state[i].comm_evt_data.rf_id,comm_state.evt_state[i].comm_status.poll_send,\
			comm_state.evt_state[i].comm_status.poll_recv,comm_state.evt_state[i].comm_status.poll_timeout,\
			comm_state.evt_state[i].comm_wait_msg_sta.msg_time,comm_state.evt_state[i].comm_status.poll_max_time,\
			comm_state.evt_state[i].comm_status.poll_id_err);
		}
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_ls_comm ={
	
	"lscomm",
	"\r\nlscomm:		lscomm ---- List EVT communication states",
	console_user_ls_comm_execute,
	0
};

/** 
* 列出EBB接收EVT通信的状态. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-12-18创建 
*/
static BaseType_t console_user_ls_rf_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
		
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"List EVT RF receive states\r\n");
	sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Addr\tSEG err\tCRC err\tSIZE err\r\n");
	for( uint8_t i=0; i<EVT_NUM; i++ ){
		if( rf_module.recv_sta[i].sur_addr != 0 ){
			sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%d\t%d\t%d\t%d\r\n",\
			rf_module.recv_sta[i].sur_addr,rf_module.recv_sta[i].err_sta.seg_err,\
			rf_module.recv_sta[i].err_sta.crc_err,rf_module.recv_sta[i].err_sta.size_err);
		}
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_ls_rf ={
	
	"lsrf",
	"\r\nlsrf:		lsrf ---- List EVT RF receive states",
	console_user_ls_rf_execute,
	0
};


/** 
* 列出已经向EBB注册成功的EVT设备地址及UID号. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-09-04创建 
*/
static BaseType_t console_user_ls_vote_total_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[2][20];
	*pcWriteBuffer = 0;
	
	for( uint8_t i=1; i<=1; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>10?10:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
		}
	}
	
	uint8_t addr;
	addr = atoi(para[0]);
	
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"List EVT voting %d memory total states\r\n",addr);
	sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Candidate\tVote\r\n");
	uint8_t i;
	for( i=0; i<EVT_NUM; i++ ){
		if( comm_state.evt_state[i].addr == addr && addr != EVT_ADDR_NULL ){
			for( uint8_t j=0; j<CANDIDATE_MAX; j++ ){
				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%d\t\t%d\r\n",j+1,comm_state.evt_state[i].vote_tally.vote_candidate[j].total);
			}
			break;
		}
	}
	if( i==EVT_NUM )
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Can not found the address of the EVT\r\n");
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_ls_vote_total ={
	
	"lsvotetotal",
	"\r\nlsvotetotal:	lsvotetotal <addr> ---- List EVT voting memory total states",
	console_user_ls_vote_total_execute,
	1
};

/** 
* 列出已经向EBB注册成功的EVT设备地址及UID号. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-09-04创建 
*/
static BaseType_t console_user_ls_vote_site_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[2][20];
	*pcWriteBuffer = 0;
	
	for( uint8_t i=1; i<=1; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>10?10:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
		}
	}
	
	uint8_t addr;
	addr = atoi(para[0]);
	
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"List EVT voting site %d states\r\n",addr);
	sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Candidate\tVote\r\n");
	uint8_t i;
	for( i=0; i<EVT_NUM; i++ ){
		if( comm_state.evt_state[i].addr == addr && addr != EVT_ADDR_NULL ){
			
			uint16_t	tally[CANDIDATE_MAX],novote;
			memset(&tally,0,CANDIDATE_MAX*2);
			
			lefs_read(comm_state.evt_state[i].site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,novote),\
					sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->novote), (uint8_t*)&novote);
			sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"novote\t\t%d\r\n",novote);
			
			lefs_read(comm_state.evt_state[i].site_config_file_name, offsetof(DEVFS_SITE_CONFIGURATION_T,tally),\
					sizeof(((DEVFS_SITE_CONFIGURATION_T*)0)->tally), (uint8_t*)&tally);

			for( uint8_t j=0; j<CANDIDATE_MAX; j++ ){
				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%d\t\t%d\r\n",j+1,tally[j]);
			}
			break;
		}
	}
	if( i==EVT_NUM )
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Can not found the address of the EVT\r\n");
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_ls_vote_site ={
	
	"lsvotesite",
	"\r\nlsvotesite:	lsvotesite <addr> ---- List EVT voting site states",
	console_user_ls_vote_site_execute,
	1
};

/** 
* 列出已经向EBB注册成功的EVT设备地址及UID号. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-09-04创建 
*/
static BaseType_t console_user_ls_vote_type_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[2][20];
	*pcWriteBuffer = 0;
	
	for( uint8_t i=1; i<=1; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>10?10:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
		}
	}
	
	uint8_t vote_type;
	vote_type = atoi(para[0]);
	
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"List EVT voting type %d states\r\n",vote_type);
	sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Candidate\tVote\r\n");
	uint8_t i;
	if( vote_type>=VOTE_TYPE1 && vote_type<=VOTE_TYPE4 ){
		
		DEVFS_CANDIDATE_TALLIES_T candidate_tallies;
		
		memset(&candidate_tallies,0,sizeof(DEVFS_CANDIDATE_TALLIES_T));
			
		lefs_read(CONECT_STR(DEVFS_DRIVER,"VoteTallies"), offsetof(DEVFS_VOTE_TALLIES_T,vote1_tally) + \
			sizeof(DEVFS_CANDIDATE_TALLIES_T)*(vote_type-1),\
			sizeof(((DEVFS_CANDIDATE_TALLIES_T*)0)->candidate_tally), (uint8_t*)&candidate_tallies);
		
		for( uint8_t j=0; j<CANDIDATE_MAX; j++ ){
			sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%d\t\t%d\r\n",j+1,candidate_tallies.candidate_tally[j]);
		}
	}
	else
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Can not found the voting type\r\n");
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_ls_vote_type ={
	
	"lsvotetype",
	"\r\nlsvotetype:	lsvotetype <type> ---- List EVT voting type states",
	console_user_ls_vote_type_execute,
	1
};

/** 
* 列出候选人名单. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-09-04创建 
*/
static BaseType_t console_user_ls_candidate_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	static uint8_t num=0;
	static uint8_t vote_type;
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[2][20];
	*pcWriteBuffer = 0;
	char candidate_buf[CANDIDATE_NAME_LEN];
	
	if( num!=0 )
		goto	CANDIDATE_NAME_PRINT;
	
	for( uint8_t i=1; i<=1; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>10?10:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
		}
	}
	
	vote_type = atoi(para[0]);
	
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"List EVT voting type %d name\r\n",vote_type);
	sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"No.\tName\r\n");
	if( vote_type>=VOTE_TYPE1 && vote_type<=VOTE_TYPE4 ){

		while( num<election_candidate_num(vote_type) ){
CANDIDATE_NAME_PRINT:
			if( FUN_OK == devfs_read_candidates_name(vote_type, num, candidate_buf)){
				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%d\t%s\r\n",num+1,&candidate_buf[4]);
				num++;
				if( strlen(pcWriteBuffer)>100 )
					return pdTRUE;
			}
		}
	}
	else
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Can not found the voting type\r\n");
	
	num = 0;
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_ls_candidate ={
	
	"lscandidate",
	"\r\nlscandidate:	lscandidate <type> ---- List candidate of voting type",
	console_user_ls_candidate_execute,
	1
};

/** 
* 列出已经向EBB注册成功的EVT设备地址及UID号. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-09-04创建 
*/
static BaseType_t console_user_ls_vote_mem_inc_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[2][20];
	*pcWriteBuffer = 0;
	
	for( uint8_t i=1; i<=1; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>10?10:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
		}
	}
	
	uint8_t addr;
	addr = atoi(para[0]);
	
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"List EVT voting %d memory increase states\r\n",addr);
	sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Candidate\tVote\r\n");
	uint8_t i;
	for( i=0; i<EVT_NUM; i++ ){
		if( comm_state.evt_state[i].addr == addr && addr != EVT_ADDR_NULL ){
			for( uint8_t j=0; j<CANDIDATE_MAX; j++ ){
				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%d\t\t%d\r\n",j+1,comm_state.evt_state[i].vote_tally.vote_candidate[j].increased);
			}
			break;
		}
	}
	if( i==EVT_NUM )
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Can not found the address of the EVT\r\n");
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_ls_vote_inc ={
	
	"lsvoteinc",
	"\r\nlsvoteinc:	lsvoteinc <addr> ---- List EVT voting memory increase states",
	console_user_ls_vote_mem_inc_execute,
	1
};

/** 
* 列出已经向EBB注册成功的EVT设备地址及UID号. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：cat filename
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-09-04创建 
*/
static BaseType_t console_user_create_record_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[2][20];
	*pcWriteBuffer = 0;
	
	for( uint8_t i=1; i<=0; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>10?10:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
		}
	}
	
	uint8_t vote_type;
	vote_type = atoi(para[0]);
	
	*pcWriteBuffer = 0;
	sprintf(pcWriteBuffer,"Create the voting records\r\n");
	
	if( devfs_vote_records_create() == FUN_OK ){
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"successfully\r\n");
	}
	else{
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"failed\r\n");
	}
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_create_record ={
	
	"createrecord",
	"\r\ncreaterecord:	createrecord ---- Create voting record",
	console_user_create_record_execute,
	0
};


/** 
* 设置VoteSettings文件的开始时间及结束时间. 
* . 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：election s/e 2019-10-14 13:30:00
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-10-14创建 
*/
static BaseType_t console_user_election_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[3][20];
	
	for( uint8_t i=1; i<=3; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>20?20:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
		}
	}
	
	*pcWriteBuffer = 0;
	uint16_t year;
	DEVFS_ECECTION_DATE_TIME_T date_time;
	sscanf(para[1],"%4d-%2d-%2d",(int*)&year,(int*)&date_time.month,(int*)&date_time.day);
	date_time.year = year%100;
	sscanf(para[2],"%2d:%2d:%2d",(int*)&date_time.hour,(int*)&date_time.minute,(int*)&date_time.second);
	if( strcmp(para[0],"s")==0 ){
		if( FUN_OK == devfs_write_election_time( 1, &date_time ) ){
			sprintf(pcWriteBuffer,"Write election start time %d-%d-%d %d:%d:%d successful\r\n",\
			date_time.year,date_time.month,date_time.day,date_time.hour,date_time.minute,date_time.second);
			
			TM_RTC_AlarmTime_t 			set_time;
			set_time.Type = TM_RTC_AlarmType_DayInMonth;
			set_time.Day = date_time.day;
			set_time.Hours = date_time.hour;
			set_time.Minutes = date_time.minute;
			set_time.Seconds = date_time.second;
			TM_RTC_EnableAlarm(TM_RTC_Alarm_A, &set_time, TM_RTC_Format_BIN);
		}
		else
			sprintf(pcWriteBuffer,"Write election start time failed\r\n");
	}
	else if( strcmp(para[0],"e")==0 ){
		if( FUN_OK == devfs_write_election_time( 2, &date_time ) ){
			sprintf(pcWriteBuffer,"Write election end time %d-%d-%d %d:%d:%d successful\r\n",\
			date_time.year,date_time.month,date_time.day,date_time.hour,date_time.minute,date_time.second);
			
			TM_RTC_AlarmTime_t 			set_time;
			set_time.Type = TM_RTC_AlarmType_DayInMonth;
			set_time.Day = date_time.day;
			set_time.Hours = date_time.hour;
			set_time.Minutes = date_time.minute;
			set_time.Seconds = date_time.second;
			TM_RTC_EnableAlarm(TM_RTC_Alarm_B, &set_time, TM_RTC_Format_BIN);
		}
		else
			sprintf(pcWriteBuffer,"Write election end time failed\r\n");
	}
	else{
		sprintf(pcWriteBuffer,"Parameter error\r\n");
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_election ={
	
	"election",
	"\r\nelection:	election <s/e> <Y-M-D> <H:M:S>---- Set election date&time",
	console_user_election_execute,
	3
};

/** 
* 设置选举三种状态. 
* 未开始，投票中，结束. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：state n/d/c
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-10-14创建 
*/
static BaseType_t console_user_state_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[1][20];
	
	for( uint8_t i=1; i<=1; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>20?20:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
			para[i-1][len] = 0;
		}
	}
	
	*pcWriteBuffer = 0;
	if( strcmp(para[0],"n")==0 ){
		memset(&config.evt_save,0,sizeof(EVT_SAVE_T));
		config.evt_save.rf_id = 0x14;
		
		config.programmed = 0;
		
		election_setting();
		
		DEVFS_ECECTION_DATE_TIME_T election_time;
		memset(&election_time,0,sizeof(DEVFS_ECECTION_DATE_TIME_T));
		devfs_write_election_time( 1, &election_time );
		devfs_write_election_time( 2, &election_time );
		
		sprintf(pcWriteBuffer,"Set election state nostated\r\n");
	}
	else if( strcmp(para[0],"d")==0 ){
		
		election_start();
		
		TM_RTC_DisableAlarm(TM_RTC_Alarm_A);
		
		sprintf(pcWriteBuffer,"Set election state during\r\n");
	}
	else if( strcmp(para[0],"c")==0 ){
		
		election_stop();
		
		TM_RTC_DisableAlarm(TM_RTC_Alarm_A);
		TM_RTC_DisableAlarm(TM_RTC_Alarm_B);
		
		sprintf(pcWriteBuffer,"Set election state closed\r\n");
	}
	else{
		sprintf(pcWriteBuffer,"Parameter error\r\n");
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_state ={
	
	"state",
	"\r\nstate:		state <n/d/c> ---- Set election state nostarted,during,closed",
	console_user_state_execute,
	1
};

/** 
* 设置选举三种状态. 
* 未开始，投票中，结束. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：state n/d/c
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-10-14创建 
*/
static BaseType_t console_user_programmed_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[1][20];
	
	for( uint8_t i=1; i<=0; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>20?20:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
			para[i-1][len] = 0;
		}
	}
	
	*pcWriteBuffer = 0;
	config.programmed = 1;
	sprintf(pcWriteBuffer,"Set EBB programmed successfully\r\n");
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_programmed ={
	
	"programmed",
	"\r\nprogrammed:	programmed ---- Set EBB programmed",
	console_user_programmed_execute,
	0
};


/** 
* 设置选举三种状态. 
* 未开始，投票中，结束. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：state n/d/c
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-10-14创建 
*/
static BaseType_t console_user_dcong_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[1][20];
	
	for( uint8_t i=1; i<=0; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>20?20:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
			para[i-1][len] = 0;
		}
	}
	
	*pcWriteBuffer = 0;
	
	if (fdelete ("N0:\\config", NULL) == fsOK){
		sprintf(pcWriteBuffer,"Delete config successful\r\n");
	}
	else{
		sprintf(pcWriteBuffer,"Delete config failed\r\n");
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_dcong_config ={
	
	"dcong",
	"\r\ndcong:		dcong ---- delete config file and please reboot",
	console_user_dcong_execute,
	0
};

/** 
* 设置选举三种状态. 
* 未开始，投票中，结束. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：state n/d/c
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-10-14创建 
*/
static BaseType_t console_user_retrieve_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[1][20];
	
	for( uint8_t i=1; i<=1; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>20?20:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
			para[i-1][len] = 0;
		}
	}
	
	*pcWriteBuffer = 0;
	
	uint8_t addr;
	addr = atoi(para[0]);
	if( addr>= EVT_ADDR_MIN && addr <EVT_ADDR_MAX ){
		if( FUN_OK == comm_pack(addr, config.ebb_addr, RF_EVT_CHN_POLL, (char*)CMD_REQ_CUR_VOTE_TALLY, NULL, 0) ){
			sprintf(pcWriteBuffer,"Retrieve command send to the %02X EVT successful\r\n",addr);
		}
		else{
			sprintf(pcWriteBuffer,"Retrieve command send to the %02X EVT failed\r\n",addr);
		}
	}
	else{
		sprintf(pcWriteBuffer,"Retrieve address is not in the rang 0x14-0xFE\r\n");
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_retrieve ={
	
	"retrieve",
	"\r\nretrieve:	retrieve <addr> ---- Retrieve votes",
	console_user_retrieve_execute,
	1
};

/** 
* 设置选举三种状态. 
* 未开始，投票中，结束. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：state n/d/c
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-10-14创建 
*/
static BaseType_t console_user_tally_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[1][20];
	
	for( uint8_t i=1; i<=1; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>20?20:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
			para[i-1][len] = 0;
		}
	}
	
	*pcWriteBuffer = 0;
	
	uint8_t addr;
	addr = atoi(para[0]);
	if( addr>= EVT_ADDR_MIN && addr <EVT_ADDR_MAX ){
	
		COMM_EVT_STATE_T *comm_evt_state_ptr;
		comm_evt_state_ptr = comm_state.evt_state;
		
		for( uint8_t i=0; i<EVT_NUM; i++ ){
			if( comm_evt_state_ptr->addr == addr ){
				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Total count %d\r\n",comm_evt_state_ptr->vote_tally.total_cnt);
				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Total votes %d\r\n",comm_evt_state_ptr->vote_tally.total_voters);
				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Total no votes %d\r\n",comm_evt_state_ptr->vote_tally.no_voters);
				sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"EVT No.\tSTA\tINC\tTOL\r\n");
				for( uint8_t j=0; j<CANDIDATE_MAX; j++ ){
					sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"%d\t%c\t%d\t%d\r\n",\
					j+1,\
					comm_evt_state_ptr->vote_tally.vote_candidate[j].sta==0?'-':comm_evt_state_ptr->vote_tally.vote_candidate[j].sta,\
					comm_evt_state_ptr->vote_tally.vote_candidate[j].increased,\
					comm_evt_state_ptr->vote_tally.vote_candidate[j].total);
				}
				break;
			}
		}
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_tally ={
	
	"tally",
	"\r\ntally:		tally <addr> ---- Retrieve tally",
	console_user_tally_execute,
	1
};

/** 
* 设置选举三种状态. 
* 未开始，投票中，结束. 
* @param[in]   xWriteBufferLen:可最大打印长度，pcCommandString：命令行参数. 
* @param[out]  pcWriteBuffer：需打印字符输出缓冲.  
* @retval  pdTRUE  		执行函数退出后返回再次执行
* @retval  pdFALSE   	执行函数退出后不再返回
* @par 用例：state n/d/c
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun deng于2019-10-14创建 
*/
static BaseType_t console_user_new_tally_execute( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ){

	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	if( login_flag==0 ){
		
		strcpy(pcWriteBuffer,"Please login first\r\n");
		return pdFALSE;
	}
	
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	char para[1][20];
	
	for( uint8_t i=1; i<=0; i++ ){
		
		pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,i,&xParameterStringLength);

		if( pcParameter != NULL ){
			
			uint8_t len;
			len = xParameterStringLength>20?20:xParameterStringLength;
			strncpy( para[i-1], pcParameter, len );
			para[i-1][len] = 0;
		}
	}
	
	*pcWriteBuffer = 0;
	
	if( devfs_vote_tallies_init() == FUN_OK ){
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Create new tally file successfully\r\n");
	}
	else{
		sprintf(&pcWriteBuffer[strlen(pcWriteBuffer)],"Create new tally file failed\r\n");
	}
	
	return pdFALSE;
}

static const CLI_Command_Definition_t console_user_new_tally ={
	
	"newtally",
	"\r\nnewtally:	newtally ---- create new tally file",
	console_user_new_tally_execute,
	0
};

/** 
* 控制台定时器回调. 
* 没有任何操作，定时器超时退出登陆. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-07-30创建 
*/
void console_timer_callback(TimerHandle_t xTimer ){
	
	login_flag=0;
}

/** 
* 控制台定时器复位. 
* 有命令输入复位控制台定时器. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-30创建 
*/
void console_timer_reset(void){
	
	xTimerReset(console_timer,3600000);
}


/** 
* 添加命令处理. 
* 添加命令处理，添加顺序决定命令的打印顺序. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无.
* @par 标识符
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2019-07-06创建 
*/
void console_user_init(void)
{
	FreeRTOS_CLIRegisterCommand( &console_user_list_file );
	FreeRTOS_CLIRegisterCommand( &console_user_rm_file );
	FreeRTOS_CLIRegisterCommand( &console_user_cat );
	FreeRTOS_CLIRegisterCommand( &console_user_format );
	FreeRTOS_CLIRegisterCommand( &console_user_reboot );
	FreeRTOS_CLIRegisterCommand( &console_user_version );
	FreeRTOS_CLIRegisterCommand( &console_user_thread_info );
	#ifdef CONSOLE_ENABLE_RUN_TIME
	FreeRTOS_CLIRegisterCommand( &console_user_run_time );
	run_time_init();
	#endif
	FreeRTOS_CLIRegisterCommand( &console_user_wcsd );
	FreeRTOS_CLIRegisterCommand( &console_user_login );
	FreeRTOS_CLIRegisterCommand( &console_user_exit );
	FreeRTOS_CLIRegisterCommand( &console_user_download );
	FreeRTOS_CLIRegisterCommand( &console_user_upload );
	FreeRTOS_CLIRegisterCommand( &console_user_factory_set );	
	FreeRTOS_CLIRegisterCommand( &console_user_get_time );
	FreeRTOS_CLIRegisterCommand( &console_user_set_time );
	FreeRTOS_CLIRegisterCommand( &console_user_ls_evt );
	FreeRTOS_CLIRegisterCommand( &console_user_ls_comm );
	FreeRTOS_CLIRegisterCommand( &console_user_ls_rf );
	FreeRTOS_CLIRegisterCommand( &console_user_ls_vote_total );
	FreeRTOS_CLIRegisterCommand( &console_user_ls_vote_inc );
	FreeRTOS_CLIRegisterCommand( &console_user_ls_vote_site );
	FreeRTOS_CLIRegisterCommand( &console_user_ls_vote_type );
	FreeRTOS_CLIRegisterCommand( &console_user_ls_candidate );
	FreeRTOS_CLIRegisterCommand( &console_user_create_record );
	FreeRTOS_CLIRegisterCommand( &console_user_election );
	FreeRTOS_CLIRegisterCommand( &console_user_state );
	FreeRTOS_CLIRegisterCommand( &console_user_programmed );	
	FreeRTOS_CLIRegisterCommand( &console_user_dcong_config );
	FreeRTOS_CLIRegisterCommand( &console_user_retrieve );
	FreeRTOS_CLIRegisterCommand( &console_user_tally );
	FreeRTOS_CLIRegisterCommand( &console_user_new_tally );
//	FreeRTOS_CLIRegisterCommand( &console_user_test );
	
	console_timer = xTimerCreate( "console",3600000,pdTRUE,(void *)0,console_timer_callback);
	
//	static TM_RTC_t rtc;
//	static int year, month, day;
//	sscanf("2019-08-14","%d-%d-%d",&year,&month,&day);
}
