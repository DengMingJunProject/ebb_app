/** 
* @file         mcp4018t.c 
* @brief        mcp4018t���ֵ�λ�������ļ�. 
* @details  	������lcdxx.i2c�����ӿ�. 
* @author       ken deng
* @date     	2019-07-23 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-07-23, ����\n 
*/  
#include "mcp4018t.h"
#include "lcdxx.h"

/** 
* ���ֵ�λ������ֵ����. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-23���� 
*/
void mcp4018t_set( uint8_t value )
{
	lcdxx_wirte(MCP4018T_ADDR, &value, 1 );
}

/** 
* Һ������Ʊ���. 
* �������ֵ�λ������ֵ�仯����������Ƶĵı仯. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-23���� 
*/
void mcp4018t_led_alarm(void)
{
	static uint8_t	step=0;
	static uint8_t	dir=0;
	if( dir==0 ){
		mcp4018t_set(step++);
		if( step >= 25 )
			dir = 1;
	}
	else{
		mcp4018t_set(step--);
		if( step == 0 )
			dir = 0;
	}
}

/** 
* ���ֵ�λ����ʼ��. 
* ���ֵ�λ����ֵ����. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-23���� 
*/
FUN_STATUS_T mcp4018t_init(void)
{
	mcp4018t_set(1);
	
	return FUN_OK;
}