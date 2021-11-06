#ifndef __XMODEM_H__
#define __XMODEM_H__

#include <stdint.h>
#include <stdbool.h>

extern int xmodem_transmit_memory(unsigned char *src, int srcsz);
extern int xmodem_receive_memory(unsigned char *dest, int destsz);
extern int xmodem_transmit_file(char *file);
extern int xmodem_receive_file(char *file,uint32_t size);
#endif
