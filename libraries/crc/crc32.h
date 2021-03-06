/*********************************************************************************
File info	:				crc32.h
Mark		:				
*********************************************************************************/



#ifndef __CRC32_H__
#define __CRC32_H__

#include "stm32fxxx_hal.h"
#include "stdint.h"
/*********************************************************************************
Function	:	CRC32
Description	:
Para		:	const unsigned char *buf, 
				U32 size
Input		:	None
Output		:	None
Return		:	U32 
Mark		:	
*********************************************************************************/
extern uint32_t crc32(const unsigned char *buf, uint32_t size);


#endif


/*********************************************************************************
						     End of file
*********************************************************************************/



