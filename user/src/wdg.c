/** 
* @file         wdg.c 
* @brief        初始化看门狗. 
* @details  	溢出时间，喂狗定义. 
* @author       kun 
* @date     	2018-05-08
* @version  	A001 
* @par Copyright (c):  
*        
* @par History:          
*   version: kun, 2018-05-08, 建立\n 
*/ 

#include "wdg.h"

WWDG_HandleTypeDef 	wwdg_handler;
IWDG_HandleTypeDef	iwdg_handler;

/** 
* 看门狗初始化. 
* 独立狗与窗口看门狗初始化. 
* @param[in]   无.
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-08创建 
*/
FUN_STATUS_T wdg_init(void){
	
	///<窗口看门狗12ms左右溢出
	///<t（溢出时间ms）=(42M/4096)/Prescaler*(7f-3f)
    wwdg_handler.Instance=WWDG;
    wwdg_handler.Init.Prescaler=WWDG_PRESCALER_1; 
    wwdg_handler.Init.Window=0x7f;        
    wwdg_handler.Init.Counter=0x5f;       
    wwdg_handler.Init.EWIMode=WWDG_EWI_ENABLE;
    HAL_WWDG_Init(&wwdg_handler);   
	
	///<14ms独立看门狗超时
	///<t（溢出时间ms）=rlr*T=重装值*分频值/32.
	iwdg_handler.Instance = IWDG;
	iwdg_handler.Init.Prescaler = IWDG_PRESCALER_256;
	iwdg_handler.Init.Reload = 1250;
	HAL_IWDG_Init(&iwdg_handler);
	
	FUN_OK;
}

/** 
* 独立看门狗喂狗. 
* 由外部线程调用. 
* @param[in]   无.
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-08创建 
*/
void wdg_iwdg_refresh(void){
	HAL_IWDG_Refresh(&iwdg_handler);
}

/** 
* 窗口看门狗中断初始化. 
* 初始化中断向量，使能提前唤醒中断. 
* @param[in]   无.
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-08创建 
*/
void HAL_WWDG_MspInit(WWDG_HandleTypeDef *hwwdg){
	
    __HAL_RCC_WWDG_CLK_ENABLE();
    HAL_NVIC_SetPriority(WWDG_IRQn,2,3);     
    HAL_NVIC_EnableIRQ(WWDG_IRQn);          
}

/** 
* 窗口看门狗喂狗中断. 
* 窗口看门狗提前唤醒中断处理函数. 
* @param[in]   无.
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-08创建 
*/
void WWDG_IRQHandler(void){
	
    HAL_WWDG_IRQHandler(&wwdg_handler);
}

/** 
* 窗口看门狗喂狗. 
* 由中断回调. 
* @param[in]   无.
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      kun于2018-05-08创建 
*/
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef* hwwdg){
	
    HAL_WWDG_Refresh(&wwdg_handler);
}
