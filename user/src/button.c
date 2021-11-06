/** 
* @file         button.c 
* @brief        按键扫描，识别按键动作. 
* @details  	向按键消息队列发送按键消息. 
* @author       ken deng 
* @date     	2019-07-23 
* @version  	A001 
* @par Copyright (c):  
*       laxton company 
* @par History:          
*   version: ken deng, 2019-07-23, 建立\n 
*/  
#include "button.h"
#include "tca9535pwr.h"
#include "display.h"

#undef BUTTON_DEBUG
#ifdef BUTTON_DEBUG
    #define	button_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[BUTTON](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define button_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[BUTTON] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define button_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[BUTTON] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define button_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define button_log(...)
    #define button_usr(...)
    #define button_err(...)
    #define button_dump(...)
#endif

BUTTON_T g_button[] = /* Initialization structure                                                               */
{
	{CON_BUTTON ,KEY_K1 ,0,0,BUTTON_STATE0,BUTTON_STATE0,BUTTON_NO_EVENT},
	{CAN_BUTTON ,KEY_K2 ,0,0,BUTTON_STATE0,BUTTON_STATE0,BUTTON_NO_EVENT},
	{UP_BUTTON  ,KEY_K4 ,0,0,BUTTON_STATE0,BUTTON_STATE0,BUTTON_NO_EVENT},
	{DOWN_BUTTON,KEY_K3 ,0,0,BUTTON_STATE0,BUTTON_STATE0,BUTTON_NO_EVENT},
};

const uint8_t g_buttonNum = (uint8_t)(sizeof(g_button) / sizeof(BUTTON_T));

/** 
* 读取按键状态. 
* 无. 
* @param[in]   button_name:按键序号. 
* @param[out]  无.  
* @retval  按键状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
static GPIO_PinState button_read_status(BUTTON_NAME_T button_name)
{
	GPIO_PinState button_status;
	if( tca9535pwr_read_pin()&(g_button[button_name].button_pin) )
		button_status = GPIO_PIN_SET;
	else
		button_status = GPIO_PIN_RESET;
	
	return button_status;
}

/** 
* 按键状态检测. 
* 标记按键检测序号. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  按键状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
static void button_detect(void)
{
	uint8_t i = 0u;
	GPIO_PinState buttonStatus;
  
	uint8_t scan_num;
	
	scan_num = g_buttonNum;
	
	for (i = 0u;i < scan_num;i++){
		
		buttonStatus = button_read_status((BUTTON_NAME_T)i);
		switch (g_button[i].button_detect_step){
			
			case BUTTON_STATE0 : 
				if (buttonStatus == BUTTON_VAIL_LEVEL){
					
					g_button[i].button_detect_step = BUTTON_STATE1;
				}
			break;
				
			case BUTTON_STATE1 : 
				if (buttonStatus == BUTTON_VAIL_LEVEL){
			   
					g_button[i].button_detect_step    = BUTTON_STATE2;
					g_button[i].button_detect_cnt = 0u;
				}
				else{
				
					g_button[i].button_detect_step  = BUTTON_STATE0;
				}
			break;
			
			case BUTTON_STATE2 : 
				if (buttonStatus != BUTTON_VAIL_LEVEL){ /* Button released, then a single click event occur                 */
					g_button[i].button_event         = BUTTON_SINGLE_CLICK;             
					g_button[i].button_detect_step    = BUTTON_STATE0;
					g_button[i].button_detect_cnt = 0u;
				}
				else if (++g_button[i].button_detect_cnt > BUTTON_LONG_CLICK_TIME) {
				
					/* Longer than the define time, a long click event occur                                                 */ 
					g_button[i].button_event         = BUTTON_LONG_CLICK;
					g_button[i].button_detect_step    = BUTTON_STATE3;
					g_button[i].button_detect_cnt = 0u;
				}
			break;
			
			case BUTTON_STATE3 : 
				g_button[i].button_detect_step = BUTTON_STATE0;                      
			break;
		
			default:
				g_button[i].button_detect_step = BUTTON_STATE0;
			break;  
		}
	}
}

/** 
* 按键扫描. 
* 识别按键的单按，双按，长按，释放动作. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  按键状态  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
void button_scan(void)
{
	uint8_t i = 0u;
	BUTTON_MSG_T button_msg;
	
	button_detect();
	  
	for (i=0u; i<g_buttonNum; i++){
		
		switch (g_button[i].button_scan_step){
			
			case BUTTON_STATE0 :  /* Single click, need to wait to check if there are double click event                    */
				if (g_button[i].button_event == BUTTON_SINGLE_CLICK){
					
					g_button[i].button_event       = BUTTON_NO_EVENT;
					g_button[i].button_scan_step    = BUTTON_STATE1;
					g_button[i].button_scan_cnt = 0u;
				}
				else if (g_button[i].button_event == BUTTON_LONG_CLICK){ /* Long click event, put it to the buffer right away*/
					
					button_msg.value = g_button[i].button_name;
					button_msg.event = BUTTON_LONG_CLICK;
					display_button_in(&button_msg);
					button_log("button long click %d\r\n",i);
			   
					g_button[i].button_event       = BUTTON_NO_EVENT;
					g_button[i].button_scan_cnt = 0u;
				}
			break;
	    
			case BUTTON_STATE1 : 
				if (g_button[i].button_event == BUTTON_SINGLE_CLICK){ /* Another single click occur, then double click event */
					
					button_msg.value = g_button[i].button_name;;
					button_msg.event = BUTTON_DOUBLE_EVENT;
					display_button_in(&button_msg);
					button_log("button double click %d\r\n",i);
					g_button[i].button_event       = BUTTON_NO_EVENT;
					g_button[i].button_scan_step    = BUTTON_STATE0;
					g_button[i].button_scan_cnt = 0u;
				} 
				else{		/* No another click occur, then just count it as single click                                            */
					if (++g_button[i].button_scan_cnt > BUTTON_DOUBLE_CLICK_TIME){
						button_msg.value = g_button[i].button_name;;
						button_msg.event = BUTTON_SINGLE_CLICK;
						display_button_in(&button_msg);
						button_log("button click %d\r\n",i);
						g_button[i].button_event       = BUTTON_NO_EVENT;
						g_button[i].button_scan_step    = BUTTON_STATE0;
						g_button[i].button_scan_cnt = 0u;
					}
				}
			break;
	    
			default : g_button[i].button_scan_step = BUTTON_STATE0; 
				break;
		}
	}
}

/** 
* 按键扫描初始化. 
* 空函数留空. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
FUN_STATUS_T button_init(void)
{
	return FUN_OK;
}