#include "dbg.h"

void DBG_DUMP( uint8_t *buf, uint32_t size )
{
    int i;

    for( i = 0; i < size; i++ )
    {
        SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_GREEN "%02X ", buf[i] );
    }

    SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_GREEN "\r\n" );
}


void DBG_TIME( time_t secTime, char *l_str )
{
    struct tm *tm;
    SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_YELLOW "%d  ", secTime );
    tm = localtime( &secTime );
    SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_CYAN "%04d-%02d-%02d  %d:%02d:%02d  ", tm->tm_year + 1900,
                       tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec );
    SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_CYAN "%s", l_str );
}
