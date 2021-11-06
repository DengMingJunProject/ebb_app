/** 
* @file         lcdxx.c 
* @brief        液晶配置，初始化，使用的I2C总线读写. 
* @details  	由于液晶使用了2个I2C接口芯片驱动液晶屏及背光，在I2C驱动加入互斥信号量.
* @author       ken deng
* @date     	2019-07-19 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-07-18, 建立\n 
*/  
#include "system.h"
#include "lcdxx.h"
#include "tca9535pwr.h"
#include "mcp4018t.h"
#include "Driver_I2C.h"
#include "tm_stm32_gpio.h"
#include "tca9535pwr.h"
#include "dwt.h"

#undef LCDXX_DEBUG

#ifdef LCDXX_DEBUG
    #define	lcdxx_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[LCDXX](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define lcdxx_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[LCDXX] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define lcdxx_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[LCDXX] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define lcdxx_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define lcdxx_log(...)
    #define lcdxx_usr(...)
    #define lcdxx_err(...)
    #define lcdxx_dump(...)
#endif

static xSemaphoreHandle lcd_i2c_xSemaphore=NULL;

extern ARM_DRIVER_I2C            		Driver_I2C2;
static ARM_DRIVER_I2C *lcd_i2c_drv =	&Driver_I2C2;

static volatile uint32_t lcd_i2c_event;

/** 
* I2C事件回调函数. 
* 调用CMSIS驱动接口. 
* @param[in]   event：回调事件. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
void lcdxx_i2c_signalevent (uint32_t event) 
{
	/* Save received events */
	lcd_i2c_event |= event;

	/* Optionally, user can define specific actions for an event */

	if (event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
	/* Less data was transferred than requested */
		lcdxx_log("Less data was transferred than requested\r\n");
	}

	if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
	/* Transfer or receive is finished */
		lcdxx_log("Transfer or receive is finished\r\n");
	}

	if (event & ARM_I2C_EVENT_ADDRESS_NACK) {
	/* Slave address was not acknowledged */
		lcdxx_log("Slave address was not acknowledged\r\n");
	}

	if (event & ARM_I2C_EVENT_ARBITRATION_LOST) {
	/* Master lost bus arbitration */
		lcdxx_log("Master lost bus arbitration\r\n");
	}

	if (event & ARM_I2C_EVENT_BUS_ERROR) {
	/* Invalid start/stop position detected */
		lcdxx_log("Invalid start/stop position detected\r\n");
	}

	if (event & ARM_I2C_EVENT_BUS_CLEAR) {
	/* Bus clear operation completed */
		lcdxx_log("Bus clear operation completed\r\n");
	}

	if (event & ARM_I2C_EVENT_GENERAL_CALL) {
	/* Slave was addressed with a general call address */
		lcdxx_log("Slave was addressed with a general call address\r\n");
	}

	if (event & ARM_I2C_EVENT_SLAVE_RECEIVE) {
	/* Slave addressed as receiver but SlaveReceive operation is not started */
		lcdxx_log("Slave addressed as receiver but SlaveReceive operation is not started\r\n");
	}

	if (event & ARM_I2C_EVENT_SLAVE_TRANSMIT) {
	/* Slave addressed as transmitter but SlaveTransmit operation is not started */
		lcdxx_log("Slave addressed as transmitter but SlaveTransmit operation is not started\r\n");
	}
}

/** 
* 读取I2C驱动产生事件. 
* 调用CMSIS驱动接口. 
* @param[in]   pooling:器件地址. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
FUN_STATUS_T lcdxx_i2c_read_event (uint8_t slave_addr, uint8_t addr, uint8_t *buf, uint32_t len) 
{
	uint8_t a[2];

	a[0] = (uint8_t)(addr >> 8);
	a[1] = (uint8_t)(addr & 0xFF);
	/* Clear event flags before new transfer */
	lcd_i2c_event = 0U;

	lcd_i2c_drv->MasterTransmit (slave_addr, a, 2, true);

	/* Wait until transfer completed */
	while ((lcd_i2c_event & ARM_I2C_EVENT_TRANSFER_DONE) == 0U);
	/* Check if all data transferred */
	if ((lcd_i2c_event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U) return FUN_ERROR;

	/* Clear event flags before new transfer */
	lcd_i2c_event = 0U;

	lcd_i2c_drv->MasterReceive (slave_addr, buf, len, false);

	/* Wait until transfer completed */
	while ((lcd_i2c_event & ARM_I2C_EVENT_TRANSFER_DONE) == 0U);
	/* Check if all data transferred */
	if ((lcd_i2c_event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U) return FUN_ERROR;

	return FUN_OK;
}

/** 
* 初始化I2C驱动接口. 
* 调用CMSIS驱动接口. 
* @param[in]   pooling:器件地址. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
FUN_STATUS_T lcdxx_i2c_init(bool pooling) 
{
	int32_t status;
	uint8_t val;
 
	if (pooling == true) {
		lcd_i2c_drv->Initialize (NULL);
	} else {
		lcd_i2c_drv->Initialize (lcdxx_i2c_signalevent);
	}
	lcd_i2c_drv->PowerControl (ARM_POWER_FULL);
	lcd_i2c_drv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
	lcd_i2c_drv->Control      (ARM_I2C_BUS_CLEAR, 0);
	
	lcd_i2c_xSemaphore = xSemaphoreCreateMutex();
 
	return FUN_OK;
}

/** 
* 从I2C总线写入数据. 
* 调用CMSIS驱动接口. 
* @param[in]   slave_addr:器件地址，p_data:数据，len:数据长度. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
FUN_STATUS_T lcdxx_wirte(uint8_t slave_addr, uint8_t * p_data, uint8_t len )
{
	uint8_t time=0;
	
	if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
		xSemaphoreTake(lcd_i2c_xSemaphore,portMAX_DELAY);
		
	lcd_i2c_drv->MasterTransmit (slave_addr, p_data, len, false);

	while (lcd_i2c_drv->GetStatus().busy && time<10 ){
			dwt_delay_us(100);
			time++;
	}
	if( time==10 ){
		if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
			xSemaphoreGive(lcd_i2c_xSemaphore);
		return FUN_ERROR;
	}

	if (lcd_i2c_drv->GetDataCount() != len){
		
		if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
			xSemaphoreGive(lcd_i2c_xSemaphore);
		return FUN_ERROR;
	}

	if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
		xSemaphoreGive(lcd_i2c_xSemaphore);
	return FUN_OK;
}

/** 
* 从I2C总线读取数据. 
* 调用CMSIS驱动接口. 
* @param[in]   slave_addr:器件地址，p_data:数据，len:数据长度. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
FUN_STATUS_T lcdxx_read(uint8_t slave_addr, uint8_t * p_data, uint8_t len, uint8_t w )
{
	uint8_t	time=0;
	
	if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
		xSemaphoreTake(lcd_i2c_xSemaphore,portMAX_DELAY);
	
	if( w>0 ){
		lcd_i2c_drv->MasterTransmit (slave_addr, p_data, w, true);

		/* Wait until transfer completed */
		while (lcd_i2c_drv->GetStatus().busy && time<10 ){
			dwt_delay_us(100);
			time++;
		}
		if( time==10 ){
			if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
				xSemaphoreGive(lcd_i2c_xSemaphore);
			return FUN_ERROR;
		}
		/* Check if all data transferred */
		if (lcd_i2c_drv->GetDataCount () != 1){
			if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
				xSemaphoreGive(lcd_i2c_xSemaphore);
			return FUN_ERROR;
		}
	}
	
	lcd_i2c_drv->MasterReceive (slave_addr, p_data, len, false);

	/* Wait until transfer completed */
	while (lcd_i2c_drv->GetStatus().busy && time<10 ){
			dwt_delay_us(100);
			time++;
	}
	if( time==10 ){
		if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
			xSemaphoreGive(lcd_i2c_xSemaphore);
		return FUN_ERROR;
	}
	/* Check if all data transferred */
	if (lcd_i2c_drv->GetDataCount () != len){
		if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
			xSemaphoreGive(lcd_i2c_xSemaphore);
		return FUN_ERROR;
	}
	
	if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
		xSemaphoreGive(lcd_i2c_xSemaphore);
	return FUN_OK;
}

#define LCDXX_CS_HIGH()                           tca9535pwr_set_pin( LCD_CS )
#define LCDXX_CS_LOW()                            tca9535pwr_clr_pin( LCD_CS )
#define LCDXX_RS_DATA()                           tca9535pwr_set_pin( LCD_RS )
#define LCDXX_RS_COMMAND()                        tca9535pwr_clr_pin( LCD_RS )

static void lcdxx_write_en(void)
{
	LCDXX_CS_HIGH();
	HAL_Delay( 2 );
	LCDXX_CS_LOW();
}

/** 
* 写入一个命令. 
* 无. 
* @param[in]   command:命令. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
static void lcdxx_write_command(uint8_t command)
{
	LCDXX_CS_LOW();
	LCDXX_RS_COMMAND();
	tca9535pwr_write_data((command >> LCDXX_DATA_LINE_NUM) & 0x0F,LCD_DATA,0x0f);
	lcdxx_write_en();
	tca9535pwr_write_data(command & 0x0F,LCD_DATA,0x0f);
	lcdxx_write_en();
}

/** 
* 向显示缓冲写入一个显示数据. 
* 无. 
* @param[in]   data:数据. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
static void lcdxx_write_data(uint8_t data)
{
	LCDXX_CS_LOW();
	LCDXX_RS_DATA();
	tca9535pwr_write_data((data >> LCDXX_DATA_LINE_NUM) & 0x0F,LCD_DATA,0x0f);
	lcdxx_write_en();
	tca9535pwr_write_data(data & 0x0F,LCD_DATA,0x0f);
	lcdxx_write_en();
}

/** 
* 向显示缓冲写入显示数据. 
* 无. 
* @param[in]   p_buf:数字指针，dataLen:写入数据长度. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
static void lcdxx_write_display_data(const uint8_t *p_buf, uint8_t dataLen)
{
	uint8_t i = 0;
	dataLen = dataLen>LCDXX_COLUMN_NUMBER?LCDXX_COLUMN_NUMBER:dataLen;
	for (i = 0u;i < dataLen;i++){
		
		lcdxx_write_data(p_buf[i]);
	}
}

/** 
* 向CGRAM写入点阵数据. 
* 无. 
* @param[in]   num:个数，lattice:点阵数据指针. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
void lcdxx_write_lattice(uint8_t num, uint8_t *lattice){
	
	lcdxx_write_command(LCDXX_CGRAM_START_ADDR+8*num);
	for( uint8_t i=0; i<8; i++ ){
		lcdxx_write_data(*lattice++);
	}
}

/** 
* 设置操作显示缓冲坐标. 
* 无. 
* @param[in]   row_num:列号，col_num:行号. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
static void lcdxx_set_display_locate(uint8_t row_num,uint8_t col_num)
{
	if (row_num == LCDXX_ROW1){
		
		lcdxx_write_command(col_num | LCDXX_SET_DDRAM_ADDRESS);
	}
	else if (row_num == LCDXX_ROW2){
		
		lcdxx_write_command(col_num | LCDXX_SET_DDRAM_ADDRESS | LCDXX_SECOND_ROW_SATAR_ADD);
	}
	else if (row_num == LCDXX_ROW3){
		
		lcdxx_write_command(col_num | LCDXX_SET_DDRAM_ADDRESS | LCDXX_THIRD_ROW_SATAR_ADD);
	}
	else if (row_num == LCDXX_ROW4){
		
		lcdxx_write_command(col_num | LCDXX_SET_DDRAM_ADDRESS | LCDXX_FOUR_ROW_SATAR_ADD);
	}
	else{
		DBG_FAULT("set lcd x&y fault, file %s line %d\r\n",__FILE__, __LINE__);
	}
}

/** 
* 指定坐标显示指定长度字符. 
* 无. 
* @param[in]   row_num:列号，col_num:行号，p_buf:显示字符，dataLen:长度. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
void lcdxx_dispaly_data(uint8_t row_num,uint8_t col_num,const uint8_t *p_buf, uint8_t dataLen)
{
	lcdxx_set_display_locate(row_num,col_num);
	lcdxx_write_display_data(p_buf ,dataLen);

}

/** 
* 指定坐标显示一行字符. 
* 无. 
* @param[in]   x:列号，y:行号，p_buf:显示字符，clr:一行剩余位置是否清空. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
void lcdxx_display_string(uint8_t x,uint8_t y,const char *p_buf,uint8_t clr)
{
	lcdxx_set_display_locate(y,x);
	lcdxx_write_display_data((uint8_t*)p_buf ,strlen(p_buf));
	if( clr ){
		for( uint8_t i=x+strlen(p_buf); i<LCDXX_COLUMN_NUMBER; i++){
			
			lcdxx_write_data(' ');
		}
	}
}

/** 
* 清空一行. 
* 无. 
* @param[in]   row_num:行号. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
void lcdxx_clear_row(uint8_t row_num)
{
	uint8_t i = 0;
	lcdxx_set_display_locate(row_num,0);

	/* Write ' ' to the GRAM, so the LCD will display nothing                                                           */
	for (i = 0u;i < LCDXX_COLUMN_NUMBER;i++){
		lcdxx_write_data(' ');
	}
}

/** 
* 液晶清屏. 
* 清空液晶显示缓冲区. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
void lcdxx_clear_screen(void)
{
	lcdxx_write_command(LCDXX_CLEAR_DISPLAY);              /* Clear the whole display                                */
}

/** 
* 液晶初始化. 
* 初始化液晶使用的I2C接口，扩展芯片，数字电位器芯片. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
void lcdxx_progress_bar(uint8_t row, uint8_t pos)
{	
	lcdxx_set_display_locate(row,pos);
	lcdxx_write_data(0);
}

/** 
* 液晶初始化. 
* 初始化液晶使用的I2C接口，扩展芯片，数字电位器芯片. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  无
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-07-18创建 
*/
FUN_STATUS_T lcdxx_init(void)
{
	TM_GPIO_Init(LCD_BL_PORT, LCD_BL_PIN, 	TM_GPIO_Mode_IN, 	TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, 	TM_GPIO_Speed_Low);
	TM_GPIO_SetPinLow(LCD_BL_PORT, LCD_BL_PIN);
	lcdxx_i2c_init(false);
	tca9535pwr_int();
	mcp4018t_init();
	
	static uint16_t temp;
	temp = tca9535pwr_read_pin();
	
	tca9535pwr_set_pin(LCD_LED1|LCD_LED2|LCD_LED3|LCD_LED4);
	
	tca9535pwr_clr_pin(LCD_LED1|LCD_LED2|LCD_LED3|LCD_LED4);
	
//	tca9535pwr_set_pin( LCD_LED1 );
//	tca9535pwr_set_pin( LCD_LED2 );
//	tca9535pwr_set_pin( LCD_LED3 );
//	tca9535pwr_set_pin( LCD_LED4 );
//	tca9535pwr_clr_pin( LCD_LED1 );
//	tca9535pwr_clr_pin( LCD_LED2 );
//	tca9535pwr_clr_pin( LCD_LED3 );
//	tca9535pwr_clr_pin( LCD_LED4 );
	
//	tca55pwr_write_reg(TCA9535PWR_OUTPUT0,0xffff);
//	tca55pwr_write_reg(TCA9535PWR_OUTPUT0,0x0000);
//	tca55pwr_read_reg (TCA9535PWR_OUTPUT0, &temp);
	
//	LCDXX_CS_HIGH();
//	LCDXX_CS_LOW();
//	LCDXX_RS_DATA();
//	LCDXX_RS_COMMAND();
//	tca9535pwr_write_data(0xA5,LCD_DATA,0x0f);
//	tca9535pwr_write_data(0x5A,LCD_DATA,0x0f);
//	
//	tca55pwr_read_reg(TCA9535PWR_OUTPUT0,(uint16_t*)&temp);
	
	lcdxx_write_command(LCDXX_8LINES_5X7_DISPALY_DOUBLE);
	lcdxx_write_command(LCDXX_4LINES_5X10_DISPALY_DOUBLE); /* Enable 4 data lines display mode                       */
	lcdxx_write_en();
	lcdxx_write_command(LCDXX_WRITE_ADD_INC_1);            /* Each write operation add the AC pointer by 1           */
	lcdxx_write_command(LCDXX_DISPLAY_ON);                 /* Turn on the LCD display                                */
	lcdxx_write_command(LCDXX_CLEAR_DISPLAY);              /* Clear the whole display                                */
	
	const uint8_t block_shape[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	lcdxx_write_lattice(0,(uint8_t*)block_shape);
	
//	lcdxx_display_string(0,LCDXX_ROW1,"hello world1",1);
//	lcdxx_display_string(0,LCDXX_ROW2,"hello world2",1);
//	lcdxx_display_string(0,LCDXX_ROW3,"hello world3",1);
//	lcdxx_display_string(0,LCDXX_ROW4,"hello world4",1);
	
	return FUN_OK;
}
