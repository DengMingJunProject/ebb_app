#ifndef __MCP4018T_H__
#define __MCP4018T_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

/** MCP4018 I2Cµÿ÷∑ */  
#define 	MCP4018T_ADDR		0x2f

extern FUN_STATUS_T mcp4018t_init(void);
extern void mcp4018t_set( uint8_t value );
extern void mcp4018t_led_alarm(void);

#endif
