#include "ltc1955.h"
#include "system.h"
#include "tm_stm32_gpio.h"
#include "dwt.h"
#include "tm_stm32_exti.h"

#define set_din(value)		TM_GPIO_SetPinValue(LTC1955_DIN_PORT, LTC1955_DIN_PIN, value)
#define set_sclk(value)		TM_GPIO_SetPinValue(LTC1955_SCLK_PORT, LTC1955_SCLK_PIN, value)
#define read_dout()			TM_GPIO_GetInputPinValue(LTC1955_DOUT_PORT, LTC1955_DOUT_PIN)
#define set_ld(value)		TM_GPIO_SetPinValue(LTC1955_LD_PORT, LTC1955_LD_PIN, value)

LTC1955_OPERATION_T			ltc1955_operation;
bool						ltc1955_fault;

/** 
* ltc1955配置读写. 
* 写入配置信息同时并读出ltc1955状态. 
* @param[in]   wrtie_byte:写入配置字. 
* @param[out]  read_byte:读出配置字.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-06-27创建 
*/
void ltc1955_spi_read_write(uint16_t *read_byte, uint16_t write_byte)
{
	uint16_t read_temp=0;
	
	set_ld(1);
	dwt_delay_cycle(1);
	
	for( uint8_t i=0; i<16; i++ ){
		
		read_temp <<= 1;
		
		if( write_byte & 0x8000){
			set_din(1);
		}
		else{
			set_din(0);
		}
		dwt_delay_cycle(1);
		
		set_sclk(1);
		dwt_delay_cycle(1);
		
		set_sclk(0);
		dwt_delay_cycle(1);
		
		if( read_dout() ){
			read_temp |= 0x0001;
		}
		
		write_byte <<= 1;
	}
	*read_byte = read_temp;	
	
	dwt_delay_cycle(1);
	set_ld(0);
	dwt_delay_cycle(1);
	set_sclk(1);
	dwt_delay_cycle(1);
	set_ld(1);
}

/** 
* 轮询ltc1955状态及配置参数. 
* 由上层应用定时调用.
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-06-27创建 
*/
void ltc1955_poll_operation(void)
{
	ltc1955_spi_read_write(&ltc1955_operation.status.word,ltc1955_operation.config.word);
	
	ltc1955_fault = (TM_GPIO_GetInputPinValue(LTC1955_FAULT_PORT, LTC1955_FAULT_PIN)==0);
}

/** 
* 选择卡接口. 
* 无.
* @param[in]   card:CARD_A,CARD_B. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-06-27创建 
*/
void ltc1955_select(uint8_t card)
{
	if( card==CARD_A ){
		LTC1955_CARD_A_SEL(1);
		LTC1955_CARD_B_SEL(0);
	}
	else if( card==CARD_B ){
		LTC1955_CARD_A_SEL(0);
		LTC1955_CARD_B_SEL(1);
	}
	else{
		LTC1955_CARD_A_SEL(0);
		LTC1955_CARD_B_SEL(0);
	}
	
	ltc1955_spi_read_write(&ltc1955_operation.status.word,ltc1955_operation.config.word);
}

/** 
* 复位卡接口. 
* 无.
* @param[in]   sta:复位输出高低1或0. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-06-27创建 
*/
void ltc1955_reset(uint8_t card_num, uint8_t sta)
{
	ltc1955_select(card_num);
	TM_GPIO_SetPinValue(LTC1955_RIN_PORT, LTC1955_RIN_PIN, sta);
}

/** 
* 智能卡电源控制接口. 
* 无.
* @param[in]   sta:1使能电源输出，0电源不输出. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-03创建 
*/
void ltc1955_power(uint8_t card_num, uint8_t sta)
{
	if( card_num == CARD_A ){
		LTC1955_CARD_A_VCC(sta);
	}
	else if( card_num == CARD_B ){
		LTC1955_CARD_B_VCC(sta);
	}
	
	ltc1955_spi_read_write(&ltc1955_operation.status.word,ltc1955_operation.config.word);
}

/** 
* ltc1955芯片相关初始化. 
* 需用到端口始化.
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-06-27创建 
*/
void ltc1955_init(void){
	
	TM_GPIO_Init(LTC1955_DOUT_PORT, LTC1955_DOUT_PIN, 	TM_GPIO_Mode_IN, 	TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, 	TM_GPIO_Speed_Fast);
	TM_GPIO_Init(LTC1955_SCLK_PORT, LTC1955_SCLK_PIN, 	TM_GPIO_Mode_OUT, 	TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, 	TM_GPIO_Speed_Fast);
	TM_GPIO_Init(LTC1955_DIN_PORT, 	LTC1955_DIN_PIN, 	TM_GPIO_Mode_OUT, 	TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, 	TM_GPIO_Speed_Fast);
	TM_GPIO_Init(LTC1955_LD_PORT, 	LTC1955_LD_PIN, 	TM_GPIO_Mode_OUT, 	TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, 	TM_GPIO_Speed_Fast);
	TM_GPIO_Init(LTC1955_FAULT_PORT,LTC1955_FAULT_PIN, 	TM_GPIO_Mode_IN, 	TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, 		TM_GPIO_Speed_Fast);
	TM_GPIO_Init(LTC1955_RIN_PORT,	LTC1955_RIN_PIN, 	TM_GPIO_Mode_OUT, 	TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, 	TM_GPIO_Speed_Fast);
	
	memset(&ltc1955_operation,0,sizeof(LTC1955_OPERATION_T));
	
	LTC1955_CARD_A_CLOCK(SYNC_MODE);
	LTC1955_CARD_B_CLOCK(SYNC_MODE);
	LTC1955_CARD_COMM(IOA_DATA);
	LTC1955_CARD_A_VCC(VCC_0V);
	LTC1955_CARD_B_VCC(VCC_0V);
	LTC1955_PULL_UP(0);
	LTC1955_CARD_A_SEL(0);
	LTC1955_CARD_B_SEL(0);
	ltc1955_spi_read_write(&ltc1955_operation.status.word,ltc1955_operation.config.word);
	
	ltc1955_reset(1,0);
	ltc1955_reset(2,0);
}
