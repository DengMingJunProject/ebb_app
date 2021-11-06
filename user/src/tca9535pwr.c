/** 
* @file         tca9535pwr.c 
* @brief        tca9535pwr i2c��չоƬ�����ļ�. 
* @details  	������lcdxx.i2c�����ӿ�. 
* @author       ken deng
* @date     	2019-07-23 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-07-23, ����\n 
*/  
#include "system.h"
#include "tca9535pwr.h"
#include "tm_stm32_exti.h"
#include "lcdxx.h"

TCA9535PWR_BUF_T	tca9535pwr_buf;

/** 
* доƬ�Ĵ���. 
* ������lcdxx.c��i2c�����ӿں���. 
* @param[in]   reg_addr:�Ĵ�����ַ��data��д���16λ����. 
* @param[out]  ��.  
* @retval  1  �ɹ� 
* @retval  0   ����  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-23���� 
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
* ��оƬ�Ĵ���. 
* ������lcdxx.c��i2c�����ӿں���. 
* @param[in]   reg_addr:�Ĵ�����ַ��p_data��������16λ����. 
* @param[out]  ��.  
* @retval  1  �ɹ� 
* @retval  0   ����  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-23���� 
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
* ������Ӧ������˿�Ϊ��. 
* ����Ӱ�첻���õĶ˿�. 
* @param[in]   pin:���ű��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-23���� 
*/
void tca9535pwr_set_pin( uint16_t pin )
{
	tca9535pwr_buf.output |= pin&(~TCA9535PWR_IO_CONFIG);
	
	tca55pwr_write_reg(TCA9535PWR_OUTPUT0,tca9535pwr_buf.output);
}

/** 
* ������Ӧ������˿�Ϊ��. 
* ����Ӱ�첻���õĶ˿�. 
* @param[in]   pin:���ű��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-23���� 
*/
void tca9535pwr_clr_pin( uint16_t pin )
{
	tca9535pwr_buf.output &= ~(pin&(~TCA9535PWR_IO_CONFIG));
	
	tca55pwr_write_reg(TCA9535PWR_OUTPUT0,tca9535pwr_buf.output);
}

/** 
* ͬʱ�����λ�����ݣ�����������������. 
* ����Ӱ�첻���õĶ˿�. 
* @param[in]   data:Ҫ��������ݣ�start����ʼλ�� mask:������ݵ�. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-23���� 
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
* ��ȡ�˿ڵ�����״̬. 
* ����Ӱ�첻���õĶ˿�. 
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
uint16_t tca9535pwr_read_pin(void){
	
	tca55pwr_read_reg(TCA9535PWR_INPUT0,(uint16_t*)&tca9535pwr_buf.input);
	return tca9535pwr_buf.input;
}

/** 
* оƬ���жϻص�����. 
* ��ʱûʹ��. 
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
void tca9535pwr_isr_handle(void)
{
	
}

/** 
* ���ö˿ڵ��������״̬. 
* ��tca9535pwr.h������Ӧ���������״̬. 
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
void tca9535pwr_int(void)
{
//	TM_EXTI_Attach(TCA9535PWR_IRQ_PORT, TCA9535PWR_IRQ_PIN, TM_EXTI_Trigger_Falling);
	tca55pwr_write_reg(TCA9535PWR_CONFIG0,TCA9535PWR_IO_CONFIG);
}

