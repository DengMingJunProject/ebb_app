#ifndef __LCDXX_HH___
#define __LCDXX_HH___

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

/** 引脚编号对应扩展芯片的 */  
#define		LCD_CS		(1<<15)
#define		LCD_RS		(1<<14)
#define 	LCD_DATA	10
#define 	LCD_LED1	(1<<9)
#define 	LCD_LED2	(1<<8)
#define 	LCD_LED3	(1<<7)
#define 	LCD_LED4	(1<<6)

/** 背光引脚配置 */  
#ifndef 	LCD_BL_PIN
#define 	LCD_BL_PORT	GPIOE
#define	 	LCD_BL_PIN	GPIO_PIN_12
#endif

typedef enum
{
	LCDXX_ROW1 = 1,
	LCDXX_ROW2 = 2,
	LCDXX_ROW3 = 3,
	LCDXX_ROW4 = 4,
}LCDXX_ROW_TypeDef;


/** 液晶寄存器地址表 */  
#define LCDXX_CLEAR_DISPLAY                       0x01  /* Clears entire display and sets DDRAM address to 0        */
#define LCDXX_RETURN_HOME                         0x02  /* Sets DDRAM addr to 0, returns display to original positio*/
#define LCDXX_WRITE_ADD_DEC_1                     0x04  /* Increse the DDRAM address by 1 when write into DDRAM     */
#define LCDXX_WRITE_SHIFT                         0x05  /* Shift the entire display base on "0x04" or "0x05" command*/
#define LCDXX_WRITE_ADD_INC_1                     0x06  /* Decrese the DDRAM address by 1 when write into DDRAM     */
#define LCDXX_WRITE_SHIFT_ADD_INC                 0x07  /* Shift display and increse address after write            */
#define LCDXX_DISPLAY_OFF                         0x08  /* Turn off the display                                     */
#define LCDXX_DISPLAY_ON                          0x0C  /* Just turn on the display                                 */
#define LCDXX_BLINK_CURSOR                        0x0D  /* Blinking is displayed as switching between all blank dots*/
#define LCDXX_DISPLAY_CURSOR                      0x0E  /* Display the cursor                                       */
#define LCDXX_DISPLAY_BLINK_CURSOR                0x0F  /* Display and blinking the cursor                          */
#define LCDXX_AC_DEC_1                            0x10  /* Increase address counter by 1                            */
#define LCDXX_AC_INC_1                            0x14  /* Decrease address counter by 1                            */
#define LCDXX_DISPLAY_SHIFT_LEFT                  0x18  /* Shift left the whole display by 1                        */
#define LCDXX_DISPLAY_SHIFT_RIGHT                 0x1C  /* Shift right the whole display by 1                       */
#define LCDXX_4LINES_5X7_DISPALY_SINGLE           0x20  /* 4 data lines,single line display with character size 5x7 */
#define LCDXX_4LINES_5X10_DISPALY_SINGLE          0x24  /* 4 data lines,single line display with character size 5x10*/
#define LCDXX_4LINES_5X7_DISPALY_DOUBLE           0x28  /* 4 data lines,double line display with character size 5x7 */
#define LCDXX_4LINES_5X10_DISPALY_DOUBLE          0x2C  /* 4 data lines,double line display with character size 5x10*/
#define LCDXX_8LINES_5X7_DISPALY_SINGLE           0x30  /* 8 data lines,single line display with character size 5x7 */
#define LCDXX_8LINES_5X10_DISPALY_SINGLE          0x34  /* 8 data lines,single line display with character size 5x10*/
#define LCDXX_8LINES_5X7_DISPALY_DOUBLE           0x38  /* 8 data lines,double line display with character size 5x7 */
#define LCDXX_8LINES_5X10_DISPALY_DOUBLE          0x3C  /* 8 data lines,double line display with character size 5x10*/
#define LCDXX_CGRAM_START_ADDR                    0x40  /* Sets CGRAM address                                       */
#define LCDXX_SET_DDRAM_ADDRESS                   0x80  /* Sets DDRAM address                                       */

/*-----------------------------------------End of LCD1602 COMMAND Defination------------------------------------------*/

/** 使用4线数据线方式 */  
#define LCDXX_DATA_LINE_NUM                          4
#define LCDXX_CONTROL_LINE_NUM                       3

/** 每行显示起始地址 */  
#define LCDXX_COLUMN_NUMBER                       0x10
#define LCDXX_ROW_NUMBER						  0x04
#define LCDXX_SECOND_ROW_SATAR_ADD                0x40
#define LCDXX_THIRD_ROW_SATAR_ADD				  0x10
#define LCDXX_FOUR_ROW_SATAR_ADD				  0x50
//#define LCDXX_CHARACTER_MAX						  LCDXX_DATA_LINE_NUM*LCDXX_COLUMN_NUMBER

extern FUN_STATUS_T lcdxx_init(void);
extern FUN_STATUS_T lcdxx_wirte(uint8_t slave_addr, uint8_t * p_data, uint8_t len );
extern FUN_STATUS_T lcdxx_read(uint8_t slave_addr, uint8_t * p_data, uint8_t len, uint8_t w );
extern void lcdxx_clear_screen(void);
extern void lcdxx_clear_row(uint8_t rowNum);
extern void lcdxx_dispaly_data(uint8_t rowNum,uint8_t colNum,const uint8_t *p_buf, uint8_t dataLen);
extern void lcdxx_display_string(uint8_t x,uint8_t y,const char *p_buf,uint8_t clr);
extern void lcdxx_progress_bar(uint8_t row, uint8_t pos);
#endif
