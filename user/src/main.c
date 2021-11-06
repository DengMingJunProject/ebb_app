#include "system.h"
#include "main.h"
#include "RTE_Components.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "tm_stm32_rcc.h"
#include "tm_stm32_rtc.h"
#include "crc32.h"
#include "config.h"
#include "console_uart.h"
#include "rf_module.h"
#include "comm.h"
#include "config.h"
#include "fs.h"
#include "beep.h"
#include "winusb_echo.h"
#include "usb_host.h"
#include "messagestorage.h"
#include "dwt.h"
#include "smartcard.h"
#include "lcdxx.h"
#include "mcp4018t.h"
#include "ioc.h"
#include "display.h"
#include "secure_storage.h"
#include "daemon.h"
#include "lefs.h"
#include "devfs.h"
#include "election.h"
#include "hash.h"
#include "crypt.h"
#include "console_run_time.h"
#include "event_log.h"
#include "urg.h"
#include "cm_backtrace.h"
#include "cjson.h"

//#undef MAIN_DEBUG

#define CON2(a, b) a##b##00

#ifdef MAIN_DEBUG
    #define	main_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[MAIN](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define main_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[MAIN] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define main_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[MAIN] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define main_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define main_log(...)
    #define main_usr(...)
    #define main_err(...)
    #define main_dump(...)
#endif

SYS_STATUS_T	sys_status;
const char *init_sta[]={"[OK]","[FAILED]"};

//void cjson_test(void)
//{
//	/*
//	{
//		"go_home": [
//			{ "addr":"01" , "switch_1":"on","switch_2":"on","switch_3":"on","switch_4":"on","switch_5":"on","switch_6":"on" },
//			{ "addr":"02" , "switch_1":"on","switch_2":"on","switch_3":"on","switch_4":"on","switch_5":"on","switch_6":"on" },
//			{ "addr":"03" , "switch_1":"on","switch_2":"on","switch_3":"on","switch_4":"on","switch_5":"on","switch_6":"on" }
//		],
//		"leave_home": [
//			{ "addr":"01" , "switch_1":"off","switch_2":"off","switch_3":"off","switch_4":"off","switch_5":"off","switch_6":"off" },
//			{ "addr":"02" , "switch_1":"off","switch_2":"off","switch_3":"off","switch_4":"off","switch_5":"off","switch_6":"off" },
//			{ "addr":"03" , "switch_1":"off","switch_2":"off","switch_3":"off","switch_4":"off","switch_5":"off","switch_6":"off" }
//		]
//	}
//	*/
//	
//	char* json_string =
////用char* 模拟一个JSON字符串
//	"{\
//		\"go_home\": [\
//			{ \"addr\":\"01\" , \"switch_1\":\"on\",\"switch_2\":\"on\",\"switch_3\":\"on\",\"switch_4\":\"on\",\"switch_5\":\"on\",\"switch_6\":\"on\" },\
//			{ \"addr\":\"02\" , \"switch_1\":\"on\",\"switch_2\":\"on\",\"switch_3\":\"on\",\"switch_4\":\"on\",\"switch_5\":\"on\",\"switch_6\":\"on\" },\
//			{ \"addr\":\"03\" , \"switch_1\":\"on\",\"switch_2\":\"on\",\"switch_3\":\"on\",\"switch_4\":\"on\",\"switch_5\":\"on\",\"switch_6\":\"on\" }\
//		],\
//		\"leave_home\": [\
//			{ \"addr\":\"01\" , \"switch_1\":\"off\",\"switch_2\":\"off\",\"switch_3\":\"off\",\"switch_4\":\"off\",\"switch_5\":\"off\",\"switch_6\":\"off\" },\
//			{ \"addr\":\"02\" , \"switch_1\":\"off\",\"switch_2\":\"off\",\"switch_3\":\"off\",\"switch_4\":\"off\",\"switch_5\":\"off\",\"switch_6\":\"off\" },\
//			{ \"addr\":\"03\" , \"switch_1\":\"off\",\"switch_2\":\"off\",\"switch_3\":\"off\",\"switch_4\":\"off\",\"switch_5\":\"off\",\"switch_6\":\"off\" }\
//		]\
//	}";
// 
//	//第一步打包JSON字符串
//	cJSON* cjson = cJSON_Parse(json_string);
// 
//	//判断是否打包成功
//	if(cjson == NULL){
//		main_log("cjson error…");
//	}
//	else{//打包成功调用cJSON_Print打印输出
//		main_log("%s\r\n",cJSON_Print(cjson));
//	}
// 
//	//获取数组对象
//	cJSON* test_arr = cJSON_GetObjectItem(cjson,"go_home");
// 
//	//获取数组对象个数便于循环
//	int arr_size = cJSON_GetArraySize(test_arr);//return arr_size 2
//	 
//	//获取test_arr数组对象孩子节点
//	cJSON* arr_item = test_arr->child;//子对象
//	 
//	//循环获取数组下每个字段的值并使用cJSON_Print打印
//	for(int i = 0;i <=(arr_size-1)/*0*/;++i){
//		main_log("%s\r\n",cJSON_Print(cJSON_GetObjectItem(arr_item,"addr")));
//		main_log("%s\r\n",cJSON_Print(cJSON_GetObjectItem(arr_item,"switch_1")));
//		main_log("%s\r\n",cJSON_Print(cJSON_GetObjectItem(arr_item,"switch_2")));
//		main_log("%s\r\n",cJSON_Print(cJSON_GetObjectItem(arr_item,"switch_3")));
//		main_log("%s\r\n",cJSON_Print(cJSON_GetObjectItem(arr_item,"switch_4")));
//		main_log("%s\r\n",cJSON_Print(cJSON_GetObjectItem(arr_item,"switch_5")));
//		main_log("%s\r\n",cJSON_Print(cJSON_GetObjectItem(arr_item,"switch_6")));
//		arr_item = arr_item->next;//下一个子对象
//	}
// 
//	//delete cjson 这里只需要释放cjson即可，因为其它的都指向它
//	cJSON_Delete(cjson);
//}

void vApplicationStackOverflowHook( xTaskHandle xTask, signed char *pcTaskName )
{	
    /* Run time stack overflow checking is performed if
        configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
        called if a stack overflow is detected. */
    main_err( "Task:%s stack overflow!\r\n", pcTaskName );
	char buf[DISP_BUF_SIZE];
	snprintf(buf,DISP_BUF_SIZE,"Task:%s stack overflow!",pcTaskName);
	display_text(buf,1);
	while(1);
}

void vApplicationMallocFailedHook (void){
	
	main_err( "Can not malloc more memory for APP\r\n" );
	display_text("Can not malloc more memory for APP",0);
	while(1);
}
	
uint32_t HAL_GetTick (void) 
{	
	static uint32_t ticks = 0U;
	uint32_t i;

	if (osKernelGetState () == osKernelRunning) {
		return ((uint32_t)osKernelGetTickCount ());
	}

	/* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
	for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
	}
	return ++ticks;
}

void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_RCC_USART6_CLK_ENABLE();

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler( char *file, int line )
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while( 1 )
    {
    }

    /* USER CODE END Error_Handler_Debug */
}

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /**Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
//    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
  }
}

#ifdef __DEBUG_VERSION__
#define STM32_UUID ((uint32_t *)0x1FFF7A10)

void main_get_device_id( void )
{
	memset(&sys_status,0,sizeof(SYS_STATUS_T));
    uint32_t CpuID[3];
    //获取CPU唯一ID
    CpuID[0] = STM32_UUID[0];
    CpuID[1] = STM32_UUID[1];
    CpuID[2] = STM32_UUID[2];
    //加密算法,很简单的加密算法
    sys_status.sys_id.id = crc32( ( uint8_t * )CpuID, 3 * 4 );
    memcpy( sys_status.sys_id.uuid, ( char * )CpuID, 12 );

    for( int i = 0; i < 12; i ++ )
        sprintf( sys_status.sys_id.uuid_str + i * 2, "%02X", sys_status.sys_id.uuid[i] );

	DBG_LOG("\r\n\r\n");
    DBG_LOG( "================ application info ================\r\n\r\n" );
    DBG_LOG( "  DeviceID:\t\t %08X \r\n", sys_status.sys_id.id );
    DBG_LOG( "  UID:\t\t\t %s \r\n", sys_status.sys_id.uuid_str );
    DBG_LOG( "  Hardware version:\t %05u\r\n", HARDWARE_VERSION );
    DBG_LOG( "  Firmware version:\t %u.%u.%02u\r\n", SOFTWARE_VERSION/1000, SOFTWARE_VERSION%1000/100, SOFTWARE_VERSION%100 );
    DBG_LOG( "  Build time:\t\t %s %s\r\n\r\n", __DATE__, __TIME__ );
    DBG_LOG( "==================================================\r\n\r\n" );
}
#endif

void main_cmpany_logo(void)
{
//	DBG_LOG("\r\n\r\n");
//	DBG_LOG("###          ###      ###     ### ##########      ####       ###      ## \r\n");
//	DBG_LOG("###         #####     ####   #### ###########  ##########   #####     ###\r\n");
//	DBG_LOG("###         ######      ### ####      ###     ####    ####   #####    ###\r\n");
//	DBG_LOG("###        ###  ##       #####        ###    ####      ###   ## ###   ###\r\n");
//	DBG_LOG("###        ##   ###      #####        ###    ###       ###   ##  ###  ###\r\n");
//	DBG_LOG("###       ##########    #######       ###    ####      ###   ##   #######\r\n");
//	DBG_LOG("###      ###########   #### ####      ###     ####    ####   ###   ######\r\n");
//	DBG_LOG("############      ### ####   ####     ###      #########    ####    #####\r\n");
//	DBG_LOG("                                                   ##                    \r\n");
	
	DBG_LOG("\r\n\r\n");
	DBG_LOG("###          ####      ###    ##  ##########    ########     ###      ##\r\n");
	DBG_LOG("###         ## ##       ###  ##       ##      ###      ##    #####    ##\r\n");
	DBG_LOG("###         ##  ##       #####        ##      ##       ###   ##  ##   ##\r\n");
	DBG_LOG("###        ##   ###       ###         ##     ###        ##   ##   ##  ##\r\n");
	DBG_LOG("###       #########      ## ##        ##      ##       ###   ##    ## ##\r\n");
	DBG_LOG("###       ##     ###    ##   ##       ##      ###      ##    ##     ####\r\n");
	DBG_LOG("######## ###      ###  ##     ###     ##        #######      ##      ###\r\n");
	
//	DBG_LOG("\r\n\r\n");                                                                                                                                                     
//	DBG_LOG("   ####     ########   #########       ##### \r\n");
//	DBG_LOG("#######    ####  ####  ###   ####     #####  \r\n");
//	DBG_LOG("##  ###    ###    ###        ###    ###  ##  \r\n");
//	DBG_LOG("    ###    ###    ###      ####    ###   ##  \r\n");
//	DBG_LOG("    ###    ####   ###    ####     ###########\r\n");
//	DBG_LOG("    ###     ###  ####  ###### ### ###########\r\n");
//	DBG_LOG("    ###      ######    ##########        ##  \r\n");
}

void main_disp_init_str(char *str1, const char *str2)
{
	static uint8_t step=0;
	char buf[100];
	snprintf(buf,100,"%-50s%s\r\n",str1,str2);
	DBG_LOG("%s",buf);
	HAL_Delay( 200 );
	
	if( step == 0 ){
		lcdxx_display_string(0,1,"   EBB SYSTEM",1);
		lcdxx_display_string(0,2,"Initializing...",1);
	}
	
	lcdxx_display_string(0,3,str1,1);
	step ++;
	float rate=0;
	rate = (float)step/20.0*16;
	
	if( str2==init_sta[0] )
		display_process_bar(rate);
	else{
		lcdxx_display_string(0,4,init_sta[1],1);
		while(1);
	}
}

void main_task( void *pvParameters )
{   
	main_disp_init_str("Filesystem",init_sta[fs_init()]);
	main_disp_init_str("Config file",init_sta[config_init()]);
	main_disp_init_str("Communication",init_sta[comm_init()]);
	main_disp_init_str("Encrypting file system",init_sta[lefs_init()]);
	main_disp_init_str("Device File System",init_sta[devfs_init()]);
	main_disp_init_str("Election",init_sta[election_init()]);
	main_disp_init_str("Event log",init_sta[event_log_init()]);
	
	main_cmpany_logo();
	
	beep_set_alarm(BP_START_UP);
	switch_election(EBB_STUS_START_UP);
	event_log_write(CRITICAL_LEVEL, SYSTEM_TYPE, "Device Turned on");
	ioc_keypad_led_off();
	
	while(1){
		
		TM_RTC_GetDateTime(&sys_status.rtc, TM_RTC_Format_BIN);
		config_timming_save();
		sys_status.sys_run_para.heap_size =  xPortGetFreeHeapSize();
		vTaskDelay( 1000 );
		
//		static uint8_t cnt=1;
//		char buf[20];
//		
//		if( cnt <= EVENT_LOG_MAX ){
//			snprintf(buf,20,"hello world! %d",cnt++);
//			event_log_write(CRITICAL_LEVEL, MEMORY_TYPE, buf);
//			snprintf(buf,20,"hello world! %d",cnt++);
//			event_log_write(CRITICAL_LEVEL, MEMORY_TYPE, buf);
//			snprintf(buf,20,"hello world! %d",cnt++);
//			event_log_write(CRITICAL_LEVEL, MEMORY_TYPE, buf);
//			snprintf(buf,20,"hello world! %d",cnt++);
//			event_log_write(CRITICAL_LEVEL, MEMORY_TYPE, buf);
//			snprintf(buf,20,"hello world! %d",cnt++);
//			event_log_write(CRITICAL_LEVEL, MEMORY_TYPE, buf);
//		}
	}
}

uint8_t main_init(void){
	
	if( xTaskCreate( main_task, "MAIN", MAIN_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL	) == pdPASS ){
		return 0;
	}
	else{
		return 1;
	}
}

//static uint32_t tick1,tick2=0;  
//void test1_task( void *pvParameters )
//{
//	
//	while(1){--
//		DBG_LOG("test1 running %d\r\n",dwt_read()/168/1000);
//		tick1 = dwt_read()/168/1000;
//		vTaskDelay( 10 );
//	}
//}

//void test2_task( void *pvParameters )
//{
//	while(1){
//		DBG_LOG("test2 running %d\r\n",dwt_read()/168/1000);
//		tick1 = dwt_read()/168/1000;
////		vTaskDelay( 1 );
//	}
//}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 
void fault_test_by_div0(void) {
    volatile int * SCB_CCR = (volatile int *) 0xE000ED14; // SCB->CCR
    int x, y, z;

    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */

    x = 10;
    y = 0;
    z = x / y;
    printf("z:%d\n", z);
}
/**
  * @}
  */ 
#define HARDWARE_VERSION               "V1.0.0"
#define SOFTWARE_VERSION               "V0.1.0"
int main(void)
{	
	HAL_Init();
	
	#ifdef __RELEASE_VERSION__
    SCB->VTOR = FLASH_BASE | 0x10000;
	#endif
	/* Configure the system clock to 168 MHz */
	SystemClock_Config();
//	TM_RCC_InitSystem();
	SystemCoreClockUpdate();
	
	main_get_device_id();
	DBG_LOG("System clock set to %d Hz\r\n",HAL_RCC_GetSysClockFreq());
	TM_RTC_Init(TM_RTC_ClockSource_External);
	TM_RTC_GetDateTime(&sys_status.rtc, TM_RTC_Format_BIN);
	dwt_init();
	
//	cjson_test();
	
//	cm_backtrace_init("ebb_app", HARDWARE_VERSION, SOFTWARE_VERSION);

	main_disp_init_str("Display",init_sta[display_init()]);
	main_disp_init_str("Hash",init_sta[hash_int()]);
	main_disp_init_str("Cryption",init_sta[crypt_init()]);
	main_disp_init_str("Daemon",init_sta[daemon_init()]);	
	main_disp_init_str("Console",init_sta[uart_console_start(CONSOLE_STACK_SIZE,CONSOLE_TASK_PRIORITY)]);
	main_disp_init_str("Rf module",init_sta[rf_module_init()]);
	main_disp_init_str("Secure storage",init_sta[secure_storage_init()]);
	main_disp_init_str("Io controler",init_sta[ioc_init()]);
	main_disp_init_str("Urgency",init_sta[urg_init()]);
	main_disp_init_str("Beeper",init_sta[beep_init()]);
	main_disp_init_str("USB Host",init_sta[usb_host_init()]);
	main_disp_init_str("Smartcard",init_sta[sc_init()]);
	main_disp_init_str("Main",init_sta[main_init()]);
//	main_disp_init_str("WinUSB initialize %s\r\n",init_sta[winusb_init()]);

//	message_storage_init();
	
	vTaskStartScheduler();
	
	while(1);
}
