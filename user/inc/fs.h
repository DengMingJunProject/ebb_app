#ifndef __FS_H__
#define __FS_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

extern uint8_t fs_init(void);
extern uint32_t fs_fsize(char *par);
extern uint8_t fs_format(char *par);
#endif
