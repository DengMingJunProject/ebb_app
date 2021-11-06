#ifndef __LEFS_H__
#define __LEFS_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

extern FUN_STATUS_T lefs_init(void);
extern FUN_STATUS_T lefs_new( char *file_name, uint32_t len );
extern FUN_STATUS_T lefs_read(char *file_name, uint32_t pos, uint32_t len, uint8_t *ptr);
extern FUN_STATUS_T lefs_write(char *file_name, uint32_t pos, uint32_t len, uint8_t *ptr);
#endif
