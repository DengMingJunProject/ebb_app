

#ifndef DBG_H__
#define DBG_H__
#include "SEGGER_RTT.h"
#include "time.h"
#include "stdio.h"

//#define NRF_LOG_USES_RTT      1
#define DRV_DEBUG               1
#define DBG_ERR_F               0x0001
#define DBG_USR_F               0x0002
#define DBG_LOG_F               0x0004
#define DBF_ALL                 0xFFFF

#define DBG_DISABLE             0
#define DBG_USE_RTT             1
#define DBG_USE_UART            2

#define DBG_ENABLE              DBG_USE_RTT

#define DBG_UART_COLOR          0

#if ( DBG_ENABLE == DBG_USE_RTT)
#define drv_flags               (DBG_ERR_F | DBG_USR_F | DBG_LOG_F)

#define DBG_OUT(flag, ... )                                             \
    do{                                                             \
        if(drv_flags & flag){                                           \
            SEGGER_RTT_printf(0,__VA_ARGS__);}                              \
    }while(0)

#define DBG_LOG(... )                                                   \
    do{                                                             \
        if(drv_flags & DBG_LOG_F){                                      \
            SEGGER_RTT_printf(0,RTT_CTRL_RESET RTT_CTRL_TEXT_WHITE __VA_ARGS__);} \
    }while(0)

#define DBG_ERR(... )                                                   \
    do{                                                             \
        if(drv_flags & DBG_ERR_F){                                      \
            SEGGER_RTT_printf(0,RTT_CTRL_RESET RTT_CTRL_TEXT_RED RTT_CTRL_BG_BRIGHT_YELLOW"ERR:"); \
            SEGGER_RTT_printf(0, __VA_ARGS__);}                             \
    }while(0)
	
#define DBG_FAULT(... )                                                   \
    do{                                                             \
        if(drv_flags & DBG_ERR_F){                                      \
            SEGGER_RTT_printf(0,RTT_CTRL_RESET RTT_CTRL_TEXT_RED RTT_CTRL_BG_BRIGHT_YELLOW"FAULT:"); \
            SEGGER_RTT_printf(0, __VA_ARGS__);while(1);}                             \
    }while(0)

#define DBG_USR(... )                                                   \
    do{                                                             \
        if(drv_flags & DBG_USR_F){                                      \
            SEGGER_RTT_printf(0,RTT_CTRL_RESET RTT_CTRL_TEXT_CYAN __VA_ARGS__);} \
    }while(0)


//static __inline void DBG_DUMP( uint8_t *buf, uint32_t size )
//{
//    int i;

//    for( i = 0; i < size; i++ )
//    {
//        SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_GREEN "%02X ", buf[i] );
//    }

//    SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_GREEN "\r\n" );
//}


//static __inline void DBG_TIME( time_t secTime, char *l_str )
//{
//    struct tm *tm;
//    SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_YELLOW "%d  ", secTime );
//    tm = localtime( &secTime );
//    SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_CYAN "%04d-%02d-%02d  %d:%02d:%02d  ", tm->tm_year + 1900,
//                       tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec );
//    SEGGER_RTT_printf( 0, RTT_CTRL_RESET RTT_CTRL_TEXT_CYAN "%s", l_str );
//}


#elif ( DBG_ENABLE == DBG_USE_UART )

#define drv_flags               (DBG_ERR_F | DBG_USR_F | DBG_LOG_F)

#define TERMIOS_BLACK           "\033[1;30m"
#define TERMIOS_RED             "\033[1;31mERR:"
#define TERMIOS_GREEN           "\033[1;32m"
#define TERMIOS_YELLOW          "\033[1;33m"
#define TERMIOS_BLUE            "\033[1;34m"
#define TERMIOS_MAGENTA         "\033[1;35m"
#define TERMIOS_CYAN            "\033[1;36m"
#define TERMIOS_WHITE           "\033[1;37m"
#define TERMIOS_DEFAULT         "\033[0m"


#define DBG_OUT(flag, ... )                                             \
    do{                                                             \
        if(drv_flags & flag){                                           \
            dDebug_Printf(TERMIOS_DEFAULT,__VA_ARGS__);}                                    \
    }while(0)

#define DBG_LOG(... )                                                   \
    do{                                                             \
        if(drv_flags & DBG_LOG_F){                                      \
            dDebug_Printf(TERMIOS_WHITE, __VA_ARGS__);}                                   \
    }while(0)

#define DBG_ERR(... )                                                   \
    do{                                                             \
        if(drv_flags & DBG_ERR_F){                                      \
            dDebug_Printf (TERMIOS_YELLOW,"[%s:%d]  ",  __FILE__,__LINE__); \
            dDebug_Printf( TERMIOS_RED,__VA_ARGS__);}                            \
    }while(0)

#define DBG_USR(... )                                                   \
    do{                                                             \
        if(drv_flags & DBG_USR_F){                                      \
            dDebug_Printf (TERMIOS_YELLOW,"[%s:%d]  ",  __FILE__,__LINE__); \
            dDebug_Printf(TERMIOS_CYAN, __VA_ARGS__);}                                   \
    }while(0)


static __inline void DBG_DUMP( uint8_t *buf, uint32_t size )
{
    int i;

    for( i = 0; i < size; i++ )
    {
        dDebug_Printf( TERMIOS_GREEN, "%02X ", buf[i] );
    }

    dDebug_Printf( TERMIOS_GREEN, "\r\n" );
}


static __inline void DBG_TIME( time_t secTime, char *l_str )
{
    struct tm *tm;
    dDebug_Printf( TERMIOS_MAGENTA, "%d\t", secTime );
    tm = localtime( &secTime );
    dDebug_Printf( TERMIOS_BLUE, "%04d-%02d-%02d  %d:%02d:%02d\t", tm->tm_year + 1900,
                   tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec );
    dDebug_Printf( TERMIOS_BLUE, "%s", l_str );
}



#else

#define DBG_OUT(... )
#define DBG_LOG(... )
#define DBG_ERR(... )
#define DBG_USR(... )
#define DBG_DUMP(... )
#define DBG_TIME(... )
#endif

extern void DBG_DUMP( uint8_t *buf, uint32_t size );
extern void DBG_TIME( time_t secTime, char *l_str );

#endif







