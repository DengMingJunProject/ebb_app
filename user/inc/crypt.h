#ifndef __CRYPT_H__
#define __CRYPT_H__

#include "stdint.h"
#include "smartcard.h"
#include "macro_def.h"

#define	CRYPT_BLOCK_SIZE		16

#define AT_SC				NULL
#define SC_IND				&g_smartCardManage[0]
#define SC_BACKUP			&g_smartCardManage[1]

extern void crypt_test(void);
extern FUN_STATUS_T crypt_init(void);
extern void crypt_encryption_block(uint8_t *key, uint8_t *in, uint8_t *out, uint16_t size);
extern void crypt_encryption(uint8_t *key, uint8_t *in, uint8_t *out, uint16_t size);
extern void crypt_decryption(uint8_t *key, uint8_t *in, uint8_t *out, uint16_t size);
extern uint16_t crypt_write(CARD_MANAGE_T *card_manage,uint16_t addr, uint16_t len, uint8_t *ptr);
extern uint16_t crypt_read(CARD_MANAGE_T *card_manage,uint16_t addr, uint16_t len, uint8_t *ptr);
#endif
