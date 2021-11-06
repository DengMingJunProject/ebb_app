/** 
* @file         tca9535pwr.c 
* @brief        tca9535pwr i2c扩展芯片驱动文件. 
* @details  	调用了lcdxx.i2c驱动接口. 
* @author       ken deng
* @date     	2019-07-23 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-07-23, 建立\n 
*/  
#include "system.h"
#include "tca9535pwr.h"
#include "tm_stm32_exti.h"
#include "lcdxx.h"

TCA9535PWR_BUF_T	tca9535pwr_buf;

/** 
* 写芯片寄存器. 
* 调用了lcdxx.c的i2c驱动接口函数. 
* @param[in]   reg_addr:寄存器地址，data：写入的16位数据. 
* @param[out]  无.  
* @retval  1  成功 
* @retval  0   错误  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
uint8_t tca55pwr_write_reg ( uint8_t reg_addr, uint16_t data ) 
{	
	uint8_t buf[3];
	buf[0] = reg_addr;
	buf[1] = data&0xff;
	buf[2] = (data>>8)&0xff;
		
	return lcdxx_wirte(TCA9535PWR_ADDR,buf,3);
}

/** 
* 读芯片寄存器. 
* 调用了lcdxx.c的i2c驱动接口函数. 
* @param[in]   reg_addr:寄存器地址，p_data：读出的16位数据. 
* @param[out]  无.  
* @retval  1  成功 
* @retval  0   错误  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
FUN_STATUS_T tca55pwr_read_reg ( uint8_t reg_addr, uint16_t *p_data ) 
{
	
	uint8_t		buf[3];
	buf[0] = reg_addr;

	if( FUN_OK == lcdxx_read(TCA9535PWR_ADDR,buf,2,1) ){
		*p_data = (buf[0])|(buf[1]<<8);
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}

/** 
* 设置相应的输出端口为高. 
* 不会影响不设置的端口. 
* @param[in]   pin:引脚编号. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
void tca9535pwr_set_pin( uint16_t pin )
{
	tca9535pwr_buf.output |= pin&(~TCA9535PWR_IO_CONFIG);
	
	tca55pwr_write_reg(TCA9535PWR_OUTPUT0,tca9535pwr_buf.output);
}

/** 
* 设置相应的输出端口为低. 
* 不会影响不设置的端口. 
* @param[in]   pin:引脚编号. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
void tca9535pwr_clr_pin( uint16_t pin )
{
	tca9535pwr_buf.output &= ~(pin&(~TCA9535PWR_IO_CONFIG));
	
	tca55pwr_write_reg(TCA9535PWR_OUTPUT0,tca9535pwr_buf.output);
}

/** 
* 同时输出多位的数据，例如总线类型数据. 
* 不会影响不设置的端口. 
* @param[in]   data:要输出的数据，start：开始位， mask:输出数据的. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-23创建 
*/
void tca9535pwr_write_data( uint8_t data, uint8_t start, uint8_t mask )
{
	uint16_t mask_temp;
	mask_temp = mask;
	mask_temp = ((uint16_t)mask_temp)<<start;
	mask_temp = ~mask_temp;
	
	tca9535pwr_buf.output &= mask_temp&(~TCA9535PWR_IO_CONFIG);
	
	tca9535pwr_buf.output |= (data)<<start;
	
	tca55pwr_write_reg(TCA9535PWR_OUTPUT0,tca9535pwr_buf.output);
}

/** 
* 读取端口的输入状态. 
* 不会影响不设置的端口. 
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
uint16_t tca9535pwr_read_pin(void){
	
	tca55pwr_read_reg(TCA9535PWR_INPUT0,(uint16_t*)&tca9535pwr_buf.input);
	return tca9535pwr_buf.input;
}

/** 
* 芯片的中断回调函数. 
* 暂时没使用. 
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
void tca9535pwr_isr_handle(void)
{
	
}

/** 
* 设置端口的输入输出状态. 
* 在tca9535pwr.h配置相应的输入输出状态. 
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
void tca9535pwr_int(void)
{
//	TM_EXTI_Attach(TCA9535PWR_IRQ_PORT, TCA9535PWR_IRQ_PIN, TM_EXTI_Trigger_Falling);
	tca55pwr_write_reg(TCA9535PWR_CONFIG0,TCA9535PWR_IO_CONFIG);
}

