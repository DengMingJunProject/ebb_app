#include "system.h"
#include "cm_freertos.h"
#include "dbg.h"
#include "stdio.h"

void cm_printf( const char * sFormat, ... )
{
	DBG_LOG(__VA_ARGS__);
}
