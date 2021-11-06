/** 
* @file         wdg.c 
* @brief        ��ʼ�����Ź�. 
* @details  	���ʱ�䣬ι������. 
* @author       kun 
* @date     	2018-05-08
* @version  	A001 
* @par Copyright (c):  
*        
* @par History:          
*   version: kun, 2018-05-08, ����\n 
*/ 

#include "wdg.h"

WWDG_HandleTypeDef 	wwdg_handler;
IWDG_HandleTypeDef	iwdg_handler;

/** 
* ���Ź���ʼ��. 
* �������봰�ڿ��Ź���ʼ��. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-08���� 
*/
FUN_STATUS_T wdg_init(void){
	
	///<���ڿ��Ź�12ms�������
	///<t�����ʱ��ms��=(42M/4096)/Prescaler*(7f-3f)
    wwdg_handler.Instance=WWDG;
    wwdg_handler.Init.Prescaler=WWDG_PRESCALER_1; 
    wwdg_handler.Init.Window=0x7f;        
    wwdg_handler.Init.Counter=0x5f;       
    wwdg_handler.Init.EWIMode=WWDG_EWI_ENABLE;
    HAL_WWDG_Init(&wwdg_handler);   
	
	///<14ms�������Ź���ʱ
	///<t�����ʱ��ms��=rlr*T=��װֵ*��Ƶֵ/32.
	iwdg_handler.Instance = IWDG;
	iwdg_handler.Init.Prescaler = IWDG_PRESCALER_256;
	iwdg_handler.Init.Reload = 1250;
	HAL_IWDG_Init(&iwdg_handler);
	
	FUN_OK;
}

/** 
* �������Ź�ι��. 
* ���ⲿ�̵߳���. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-08���� 
*/
void wdg_iwdg_refresh(void){
	HAL_IWDG_Refresh(&iwdg_handler);
}

/** 
* ���ڿ��Ź��жϳ�ʼ��. 
* ��ʼ���ж�������ʹ����ǰ�����ж�. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-08���� 
*/
void HAL_WWDG_MspInit(WWDG_HandleTypeDef *hwwdg){
	
    __HAL_RCC_WWDG_CLK_ENABLE();
    HAL_NVIC_SetPriority(WWDG_IRQn,2,3);     
    HAL_NVIC_EnableIRQ(WWDG_IRQn);          
}

/** 
* ���ڿ��Ź�ι���ж�. 
* ���ڿ��Ź���ǰ�����жϴ�����. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-08���� 
*/
void WWDG_IRQHandler(void){
	
    HAL_WWDG_IRQHandler(&wwdg_handler);
}

/** 
* ���ڿ��Ź�ι��. 
* ���жϻص�. 
* @param[in]   ��.
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      kun��2018-05-08���� 
*/
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef* hwwdg){
	
    HAL_WWDG_Refresh(&wwdg_handler);
}
