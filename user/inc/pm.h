#ifndef __PM_H__
#define __PM_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

typedef struct{
	uint16_t	bat;
}PM_T;

#define pm_read_bat()	(pm.bat)

extern PM_T	pm;

#endif
