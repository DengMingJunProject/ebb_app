#ifndef __HASH_H__
#define __HASH_H__

#include "stdint.h"
#include "stdbool.h"
#include "macro_def.h"

extern FUN_STATUS_T hash_int(void);
extern void hash_count(uint8_t *in_buf, uint16_t size, uint8_t *out_buf);

#endif
