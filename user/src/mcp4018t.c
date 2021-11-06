/** 
* @file         mcp4018t.c 
* @brief        mcp4018t数字电位器驱动文件. 
* @details  	调用了lcdxx.i2c驱动接口. 
* @author       ken deng
* @date     	2019-07-23 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-07-23, 建立\n 
*/  
#include "mcp4018t.h"
#include "lcdxx.h"

/** 
* 数字电位器的阻值设置. 
* 无. 
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
void mcp4018t_set( uint8_t value )
{
	lcdxx_wirte(MCP4018T_ADDR, &value, 1 );
}

/** 
* 液晶背光灯报警. 
* 利用数字电位器的阻值变化，产生背光灯的的变化. 
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
* 数字电位器初始化. 
* 数字电位器初值设置. 
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
FUN_STATUS_T mcp4018t_init(void)
{
	mcp4018t_set(1);
	
	return FUN_OK;
}