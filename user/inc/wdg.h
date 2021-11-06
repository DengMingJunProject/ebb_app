#ifndef __WDG_H__
#define __WDG_H__

#include "stm32f4xx_hal.h"
#include "macro_def.h"

extern FUN_STATUS_T wdg_init(void);
extern void wdg_iwdg_refresh(void);

#endif
