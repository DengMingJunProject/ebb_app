/** 
* @file         xmodem.c 
* @brief        ����XMODEMЭ���ڴ����ϴ��������ļ�. 
* @details  	����SecureCRT����������нӿڴ����ļ�. 
* @author       ken deng 
* @date     	2019-09-09 
* @version  	A001 
* @par Copyright (c):  
*       LAXTON company 
* @par History:          
*   version: ken degn, 2001-90-09, ����\n 
*/  
#include "system.h"
#include "crc16.h"
#include "console_uart.h"
#include "xmodem.h"
#include "rl_fs.h"
#include "fs.h"
#include "wdg.h"
#include <string.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 1000
#define MAXRETRANS 25

/** 
* ���ô��ں������һ���ֽ�. 
* ��. 
* @param[in]   c:�����ֽ�. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
static void _outbyte(int c)
{
	console_uart_send((uint8_t*)&c,1);
}

/** 
* ��ʽ�����һ���ֽ�. 
* ��. 
* @param[in]   c:�����ֽ�. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
static void outbyte(int c)
{
        static char prev = 0;
        if (c < ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\b')
                return;
        if (c == '\n' && prev != '\r') _outbyte('\r');
        _outbyte(c);
        prev = c;
}

/** 
* �ȴ�ָ��ʱ��Ӵ��ڶ���һ���ֽ�. 
* ��. 
* @param[in]   timeout:��ʱʱ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
static int _inbyte(unsigned short timeout) // msec timeout
{
        unsigned char c;
		uint16_t i;
		for( i=0;i<timeout; i++ ){
			
			if( console_uart_get(&c) != 0 ){
				break;
			}
			vTaskDelay( 1 );
		}
		if( i==timeout )
			return -2;
		
        return c;
}

/** 
* ����CRCУ��ֵ�����رȽϽ��. 
* ��. 
* @param[in]   crc:�����CRCֵ��buf:��������sz:���ݳ���. 
* @param[out]  ��.  
* @retval  1:�Ƚ���ȷ��0:�Ƚ�ʧ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
static int check(int crc, const unsigned char *buf, int sz)
{
	if (crc) {
		unsigned short crc = crc16_ccitt(buf, sz);
		unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
		if (crc == tcrc)
			return 1;
	}
	else {
		int i;
		unsigned char cks = 0;
		for (i = 0; i < sz; ++i) {
			cks += buf[i];
		}
		if (cks == buf[sz])
		return 1;
	}

	return 0;
}

/** 
* ��ս��ջ�����. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
static void flushinput(void)
{
	while (_inbyte(((DLY_1S)*3)>>1) >= 0)
		;
}

/** 
* ��xmodem�������ݷŵ��ڴ�ռ�. 
* ��. 
* @param[in]   dest:�������ռ䣬destsz:���յĳ���. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
int xmodem_receive_memory(unsigned char *dest, int destsz)
{
	unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	unsigned char *p;
	int bufsz, crc = 0;
	unsigned char trychar = 'C';
	unsigned char packetno = 1;
	int i, c, len = 0;
	int retry, retrans = MAXRETRANS;

	for(;;) {
		for( retry = 0; retry < 16; ++retry) {
			if (trychar) _outbyte(trychar);
			if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
				switch (c) {
				case SOH:
					bufsz = 128;
					goto start_recv;
				case STX:
					bufsz = 1024;
					goto start_recv;
				case EOT:
					flushinput();
					_outbyte(ACK);
					return len; /* normal end */
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						flushinput();
						_outbyte(ACK);
						return -1; /* canceled by remote */
					}
					break;
				default:
					break;
				}
			}
		}
		if (trychar == 'C') { trychar = NAK; continue; }
		flushinput();
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		return -2; /* sync error */

	start_recv:
		if (trychar == 'C') crc = 1;
		trychar = 0;
		p = xbuff;
		*p++ = c;
		for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i) {
			if ((c = _inbyte(DLY_1S)) < 0) goto reject;
			*p++ = c;
		}

		if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
			(xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
			check(crc, &xbuff[3], bufsz)) {
			if (xbuff[1] == packetno)	{
				register int count = destsz - len;
				if (count > bufsz) count = bufsz;
				if (count > 0) {
					memcpy (&dest[len], &xbuff[3], count);
					len += count;
				}
				++packetno;
				retrans = MAXRETRANS+1;
			}
			if (--retrans <= 0) {
				flushinput();
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				return -3; /* too many retry error */
			}
			_outbyte(ACK);
			continue;
		}
	reject:
		flushinput();
		_outbyte(NAK);
	}
}

/** 
* ���ڴ�ռ������ͨ��xmodem����. 
* ��. 
* @param[in]   src:�������ռ䣬srcsz:���յĳ���. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
int xmodem_transmit_memory(unsigned char *src, int srcsz)
{
	unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	int bufsz, crc = -1;
	unsigned char packetno = 1;
	int i, c, len = 0;
	int retry;

	for(;;) {
		for( retry = 0; retry < 16; ++retry) {
			if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
				switch (c) {
				case 'C':
					crc = 1;
					goto start_trans;
				case NAK:
					crc = 0;
					goto start_trans;
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						_outbyte(ACK);
						flushinput();
						return -1; /* canceled by remote */
					}
					break;
				default:
					break;
				}
			}
		}
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		flushinput();
		return -2; /* no sync */

		for(;;) {
		start_trans:
			xbuff[0] = SOH; bufsz = 128;
			xbuff[1] = packetno;
			xbuff[2] = ~packetno;
			c = srcsz - len;
			if (c > bufsz) c = bufsz;
			if (c >= 0) {
				memset (&xbuff[3], 0, bufsz);
				if (c == 0) {
					xbuff[3] = CTRLZ;
				}
				else {
					memcpy (&xbuff[3], &src[len], c);
					if (c < bufsz) xbuff[3+c] = CTRLZ;
				}
				if (crc) {
					unsigned short ccrc = crc16_ccitt(&xbuff[3], bufsz);
					xbuff[bufsz+3] = (ccrc>>8) & 0xFF;
					xbuff[bufsz+4] = ccrc & 0xFF;
				}
				else {
					unsigned char ccks = 0;
					for (i = 3; i < bufsz+3; ++i) {
						ccks += xbuff[i];
					}
					xbuff[bufsz+3] = ccks;
				}
				for (retry = 0; retry < MAXRETRANS; ++retry) {
					for (i = 0; i < bufsz+4+(crc?1:0); ++i) {
						_outbyte(xbuff[i]);
					}
					if ((c = _inbyte(DLY_1S)) >= 0 ) {
						switch (c) {
						case ACK:
							++packetno;
							len += bufsz;
							goto start_trans;
						case CAN:
							if ((c = _inbyte(DLY_1S)) == CAN) {
								_outbyte(ACK);
								flushinput();
								return -1; /* canceled by remote */
							}
							break;
						case NAK:
						default:
							break;
						}
					}
				}
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				flushinput();
				return -4; /* xmit error */
			}
			else {
				for (retry = 0; retry < 10; ++retry) {
					_outbyte(EOT);
					if ((c = _inbyte((DLY_1S)<<1)) == ACK) break;
				}
				flushinput();
				return (c == ACK)?len:-5;
			}
		}
	}
}

/** 
* ��xmodem�������ݴ��ڵ��ļ���. 
* ��. 
* @param[in]   file:Ŀ¼·�����ļ����ƣ�size���յĳ���. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
int xmodem_receive_file(char *file,uint32_t size)
{
	unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	unsigned char *p;
	int bufsz, crc = 0;
	unsigned char trychar = 'C';
	unsigned char packetno = 1;
	int i, c, len = 0;
	int retry, retrans = MAXRETRANS;

	int destsz=size;
	
	FILE *fd;
    int res;
		
	fd = fopen(file, "w" );
	
	for(;;) {
		for( retry = 0; retry < 16; ++retry) {
			if (trychar) _outbyte(trychar);
			if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
				switch (c) {
				case SOH:
					bufsz = 128;
					goto start_recv;
				case STX:
					bufsz = 1024;
					goto start_recv;
				case EOT:
					flushinput();
					_outbyte(ACK);
					fclose(fd);
					return len; /* normal end */
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						flushinput();
						_outbyte(ACK);
						fclose(fd);
						return -1; /* canceled by remote */
					}
					break;
				default:
					break;
				}
			}
		}
		if (trychar == 'C') { trychar = NAK; continue; }
		flushinput();
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		fclose(fd);
		return -2; /* sync error */

	start_recv:
		if (trychar == 'C') crc = 1;
		trychar = 0;
		p = xbuff;
		*p++ = c;
		for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i) {
			if ((c = _inbyte(DLY_1S)) < 0) goto reject;
			*p++ = c;
		}

		if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
			(xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
			check(crc, &xbuff[3], bufsz)) {
			if (xbuff[1] == packetno)	{
				register int count = destsz - len;
				if (count > bufsz) count = bufsz;
				if (count > 0) {
//					memcpy (&dest[len], &xbuff[3], count);
					vTaskDelay(10);
					fseek( fd, len, SEEK_SET );
					if( fwrite( &xbuff[3],sizeof (uint8_t), count, fd ) != len ){
						
						res = -1;
					}
//					wdg_iwdg_refresh();
					vTaskDelay(10);
					
					len += count;
				}
				++packetno;
				retrans = MAXRETRANS+1;
			}
			if (--retrans <= 0) {
				flushinput();
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				fclose(fd);
				return -3; /* too many retry error */
			}
			_outbyte(ACK);
			continue;
		}
	reject:
		flushinput();
		_outbyte(NAK);
	}
}

/** 
* ���ļ�ͨ��xmodemЭ�鷢�ͳ�ȥ. 
* ��. 
* @param[in]   file:Ŀ¼·�����ļ�����. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-09���� 
*/
int xmodem_transmit_file(char *file)
{
	unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	int bufsz, crc = -1;
	unsigned char packetno = 1;
	int i, c, len = 0;
	int retry;
	unsigned char *src;
	int srcsz;
	
	FILE *fd;
    int res;
	
	fd = fopen( file, "r" );
	
	if( fd==NULL )
		return -6;
	
	srcsz = fs_fsize(file);

	for(;;) {
		for( retry = 0; retry < 16; ++retry) {
			if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
				switch (c) {
				case 'C':
					crc = 1;
					goto start_trans;
				case NAK:
					crc = 0;
					goto start_trans;
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						_outbyte(ACK);
						flushinput();
						fclose(fd);
						return -1; /* canceled by remote */
					}
					break;
				default:
					break;
				}
			}
		}
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		flushinput();
		fclose(fd);
		return -2; /* no sync */

		for(;;) {
		start_trans:
			xbuff[0] = SOH; bufsz = 128;
			xbuff[1] = packetno;
			xbuff[2] = ~packetno;
			c = srcsz - len;
			if (c > bufsz) c = bufsz;
			if (c >= 0) {
				memset (&xbuff[3], 0, bufsz);
				if (c == 0) {
					xbuff[3] = CTRLZ;
				}
				else {
					vTaskDelay(1);
					fseek( fd, len, SEEK_SET );
					if( fread( &xbuff[3], sizeof(uint8_t), c, fd ) != c ){
						
						res = -1;
					}
					vTaskDelay(1);
					if (c < bufsz) xbuff[3+c] = CTRLZ;
				}
				if (crc) {
					unsigned short ccrc = crc16_ccitt(&xbuff[3], bufsz);
					xbuff[bufsz+3] = (ccrc>>8) & 0xFF;
					xbuff[bufsz+4] = ccrc & 0xFF;
				}
				else {
					unsigned char ccks = 0;
					for (i = 3; i < bufsz+3; ++i) {
						ccks += xbuff[i];
					}
					xbuff[bufsz+3] = ccks;
				}
				for (retry = 0; retry < MAXRETRANS; ++retry) {
					for (i = 0; i < bufsz+4+(crc?1:0); ++i) {
						_outbyte(xbuff[i]);
					}
					if ((c = _inbyte(DLY_1S)) >= 0 ) {
						switch (c) {
						case ACK:
							++packetno;
							len += bufsz;
							goto start_trans;
						case CAN:
							if ((c = _inbyte(DLY_1S)) == CAN) {
								_outbyte(ACK);
								flushinput();
								fclose(fd);
								return -1; /* canceled by remote */
							}
							break;
						case NAK:
						default:
							break;
						}
					}
				}
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				flushinput();
				fclose(fd);
				return -4; /* xmit error */
			}
			else {
				for (retry = 0; retry < 10; ++retry) {
					_outbyte(EOT);
					if ((c = _inbyte((DLY_1S)<<1)) == ACK) break;
				}
				flushinput();
				fclose(fd);
				return (c == ACK)?len:-5;
			}
		}
	}
}
