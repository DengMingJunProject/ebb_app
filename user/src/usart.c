/** 
* @file         usart.c 
* @brief        USART串口驱动. 
* @details  	6串口驱动程序，DMA方式，环形缓冲. 
* @author       ken deng
* @date     	2019-09-09 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-09-09, 建立\n 
*/  
#include "usart.h"
#include <stdarg.h>
#include <string.h>
#include "ring_buffer.h"
#include "stm32f4xx_hal.h"
#include "system.h"
#include "main.h"

#ifdef USART_DEBUG
    #define	usart_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[USART](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define usart_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[USART] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define usart_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[USART] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define usart_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define usart_log(...)
    #define usart_usr(...)
    #define usart_err(...)
    #define usart_dump(...)
#endif
	
static void usart_dma_init(uint8_t usart);

//#define uart_delay(v) HAL_Delay(v)
#define uart_delay(v )          vTaskDelay(v / portTICK_RATE_MS)

#if ( DBG_ENABLE == DBG_USE_UART )
    #if(UART_FOR_DEBUG == 0)
        #define UART1_TX_BUF_SIZE       1024
    #endif
    #if(UART_FOR_DEBUG == 1)
        #define UART2_TX_BUF_SIZE       1024
    #endif
    #if(UART_FOR_DEBUG == 2)
        #define UART3_TX_BUF_SIZE       1024
    #endif
    #if(UART_FOR_DEBUG == 3)
        #define UART4_TX_BUF_SIZE       1024
    #endif
    #if(UART_FOR_DEBUG == 4)
        #define UART5_TX_BUF_SIZE       1024
    #endif
    #if(UART_FOR_DEBUG == 5)
        #define UART6_TX_BUF_SIZE       1024
    #endif

#endif
#ifndef UART1_TX_BUF_SIZE
    #define UART1_TX_BUF_SIZE       1
#endif
#ifndef UART2_TX_BUF_SIZE
    #define UART2_TX_BUF_SIZE       1
#endif
#ifndef UART3_TX_BUF_SIZE
    #define UART3_TX_BUF_SIZE       1
#endif
#ifndef UART4_TX_BUF_SIZE
    #define UART4_TX_BUF_SIZE       1
#endif
#ifndef UART5_TX_BUF_SIZE
    #define UART5_TX_BUF_SIZE       1
#endif
#ifndef UART6_TX_BUF_SIZE
    #define UART6_TX_BUF_SIZE       1
#endif

#define UART1_RX_BUF_SIZE       1
#define UART2_RX_BUF_SIZE       1
#define UART3_RX_BUF_SIZE       1
#define UART4_RX_BUF_SIZE       1
#define UART5_RX_BUF_SIZE       1
#define UART6_RX_BUF_SIZE       1

#ifdef USART1_EN
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
uint8_t uart1_tx_buf[UART1_TX_BUF_SIZE];
uint8_t uart1_rx_buffer[UART1_RX_BUF_SIZE];
#endif

#ifdef USART2_EN
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;
uint8_t uart2_tx_buf[UART2_TX_BUF_SIZE];
uint8_t uart2_rx_buffer[UART2_RX_BUF_SIZE];
#endif

#ifdef USART3_EN
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;
uint8_t uart3_tx_buf[UART3_TX_BUF_SIZE];
uint8_t uart3_rx_buffer[UART3_RX_BUF_SIZE];
#endif

#ifdef USART4_EN
UART_HandleTypeDef huart4;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_uart4_tx;
uint8_t uart4_tx_buf[UART4_TX_BUF_SIZE];
uint8_t uart4_rx_buffer[UART4_RX_BUF_SIZE];
#endif

#ifdef USART5_EN
UART_HandleTypeDef huart5;
DMA_HandleTypeDef hdma_uart5_rx;
DMA_HandleTypeDef hdma_uart5_tx;
uint8_t uart5_tx_buf[UART5_TX_BUF_SIZE];
uint8_t uart5_rx_buffer[UART5_RX_BUF_SIZE];
#endif

#ifdef USART6_EN
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart6_rx;
DMA_HandleTypeDef hdma_usart6_tx;
uint8_t uart6_tx_buf[UART6_TX_BUF_SIZE];
uint8_t uart6_rx_buffer[UART6_RX_BUF_SIZE];
#endif

UART_HandleTypeDef *huart[MAX_UART_NUM] =
{
	#ifdef USART1_EN
    &huart1,
	#endif
	#ifdef USART2_EN
	&huart2, 
	#endif
	#ifdef USART3_EN
	&huart3,
	#endif
	#ifdef USART4_EN
	&huart4,
	#endif
	#ifdef USART5_EN
	&huart5,
	#endif
	#ifdef USART6_EN
	&huart6
	#endif
};

USART_TypeDef *uart_type[MAX_UART_NUM] =
{
	#ifdef USART1_EN
    USART1,
	#endif
	#ifdef USART2_EN
	USART2,
	#endif
	#ifdef USART3_EN
	USART3,
	#endif
	#ifdef USART4_EN
	UART4, 
	#endif
	#ifdef USART5_EN
	UART5,
	#endif
	#ifdef USART6_EN
	USART6
	#endif
};

const DMA_HandleTypeDef *hdma_usart_tx[MAX_UART_NUM] =
{
	#ifdef USART1_EN
    &hdma_usart1_tx,
	#endif
	#ifdef USART2_EN
    &hdma_usart2_tx,
	#endif
	#ifdef USART3_EN
    &hdma_usart3_tx,
	#endif
	#ifdef USART4_EN
    &hdma_uart4_tx,
	#endif
	#ifdef USART5_EN
    &hdma_uart5_tx,
	#endif
	#ifdef USART6_EN
    &hdma_usart6_tx
	#endif
};

const DMA_HandleTypeDef *hdma_usart_rx[MAX_UART_NUM] =
{
	#ifdef USART1_EN
    &hdma_usart1_rx,
	#endif
	#ifdef USART2_EN
    &hdma_usart2_rx,
	#endif
	#ifdef USART3_EN
    &hdma_usart3_rx,
	#endif
	#ifdef USART4_EN
    &hdma_uart4_rx,
	#endif
	#ifdef USART5_EN
    &hdma_uart5_rx,
	#endif
	#ifdef USART6_EN
    &hdma_usart6_rx
	#endif
};

typedef struct
{
    uint8_t 	*pBuf;
    uint32_t 	bufSize;
    uint32_t 	rxlen;
    uint32_t 	dataPopPtr;
} ringBufHandlerType;

typedef struct
{
    volatile uint32_t ready;
    uint8_t        *buf;
    uint32_t        buf_len;
} uart_tx_type;

uart_tx_type uart_tx[MAX_UART_NUM] =
{
	#ifdef USART1_EN
    1, uart1_tx_buf, UART1_TX_BUF_SIZE,
	#endif
	#ifdef USART2_EN
    1, uart2_tx_buf, UART2_TX_BUF_SIZE,
	#endif
	#ifdef USART3_EN
    1, uart3_tx_buf, UART3_TX_BUF_SIZE,
	#endif
	#ifdef USART4_EN
    1, uart4_tx_buf, UART4_TX_BUF_SIZE,
	#endif
	#ifdef USART5_EN
    1, uart5_tx_buf, UART5_TX_BUF_SIZE,
	#endif
	#ifdef USART6_EN
    1, uart6_tx_buf, UART6_TX_BUF_SIZE
	#endif
};

ringBufHandlerType ringBuf[MAX_UART_NUM] =
{
	#ifdef USART1_EN
    uart1_rx_buffer, UART1_RX_BUF_SIZE, 0, 0,
	#endif
	#ifdef USART2_EN
    uart2_rx_buffer, UART2_RX_BUF_SIZE, 0, 0,
	#endif
	#ifdef USART3_EN
    uart3_rx_buffer, UART3_RX_BUF_SIZE, 0, 0,
	#endif
	#ifdef USART4_EN
    uart4_rx_buffer, UART4_RX_BUF_SIZE, 0, 0,
	#endif
	#ifdef USART5_EN
    uart5_rx_buffer, UART5_RX_BUF_SIZE, 0, 0,
	#endif
	#ifdef USART6_EN
    uart6_rx_buffer, UART6_RX_BUF_SIZE, 0, 0
	#endif
};

/** 
* 根据串口句柄找出串口驱动序号. 
* 无. 
* @param[in]   shuart:串口句柄. 
* @param[out]  无.  
* @retval  串口序号
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
uint32_t usart_get_index( UART_HandleTypeDef *shuart )
{
    uint32_t uartx;

    for( uartx = 0; uartx < MAX_UART_NUM; uartx++ ){
		
        if( shuart->Instance == huart[uartx]->Instance )
            break;
    }

    return uartx;
}

/** 
* 串口接收中断回调函数. 
* 清空标志位. 
* @param[in]   huart:串口句柄. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
void usart_receive_idle( UART_HandleTypeDef *huart )
{
    //    uint32_t uartx, rx_len;
    CLEAR_BIT( huart->Instance->CR1, USART_CR1_PEIE );
    CLEAR_BIT( huart->Instance->CR3, USART_CR3_EIE );

    if( ( __HAL_UART_GET_FLAG( huart, UART_FLAG_IDLE ) != RESET ) )
    {
        __HAL_UART_CLEAR_IDLEFLAG( huart );
        /*
        uartx = get_uart_index( huart );
        rx_len = ringBuf[uartx].bufSize - __HAL_DMA_GET_COUNTER( hdma_usart_rx[uartx] );
        if( rx_len < ringBuf[uartx].dataPopPtr )
        {
            rx_len = rx_len + ringBuf[uartx].bufSize - ringBuf[uartx].dataPopPtr;
        }
        else
        {
            rx_len -= ringBuf[uartx].dataPopPtr;
        }
        ringBuf[uartx].rxlen = rx_len;
        */
    }
}

/** 
* 启动串口DMA接收. 
* 无. 
* @param[in]   uartx:串口号码. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
void usart_start_rx( uint32_t uartx )
{
    //__HAL_UART_ENABLE_IT( huart[uartx], UART_IT_IDLE );
    HAL_UART_Receive_DMA( huart[uartx], ringBuf[uartx].pBuf, ringBuf[uartx].bufSize );
}

/** 
* 初始化串口. 
* 无. 
* @param[in]   uartx:串口号码，bps:波特率，word_len:位数，even:奇偶位，stop:停止位,rbuf:接收缓冲区，rbuf_size:接收缓冲大小
				tbuf:发送缓冲区，tbuf_size:发送缓冲区大小. 
* @param[out]  无.  
* @retval  1初始化成功，0：初始化成功.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
int usart_open( uint32_t uartx, uint32_t bps, uint32_t word_len, int even, int stop,
                uint8_t *rbuf, uint32_t rbuf_size, uint8_t *tbuf, uint32_t tbuf_size )
{
    int rc = 1;
	
	HAL_UART_DMAStop( huart[uartx] );
	
//	//UART3 MOUDBUS使用，不使用这段驱动
//	if( uartx==2 )
//		return 0;
	
    huart[uartx]->Instance = uart_type[uartx];
    huart[uartx]->Init.BaudRate = bps;

    if( word_len == 9 )
        huart[uartx]->Init.WordLength = UART_WORDLENGTH_9B;
    else
        huart[uartx]->Init.WordLength = UART_WORDLENGTH_8B;

    if( stop == 2 )
        huart[uartx]->Init.StopBits = UART_STOPBITS_2;
    else
        huart[uartx]->Init.StopBits = UART_STOPBITS_1;

    if( even == 1 )
        huart[uartx]->Init.Parity = UART_PARITY_EVEN;
    else if( even == 2 )
        huart[uartx]->Init.Parity = UART_PARITY_ODD;
    else
        huart[uartx]->Init.Parity = UART_PARITY_NONE;

    huart[uartx]->Init.Mode = UART_MODE_TX_RX;
    huart[uartx]->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart[uartx]->Init.OverSampling = UART_OVERSAMPLING_16;

    if( HAL_UART_Init( huart[uartx] ) != HAL_OK )
    {
        _Error_Handler( __FILE__, __LINE__ );
        rc = 0;
    }
    else
    {
        ringBuf[uartx].pBuf = rbuf;
        ringBuf[uartx].bufSize = rbuf_size;
        uart_tx[uartx].buf = tbuf;
        uart_tx[uartx].buf_len = tbuf_size;
        HAL_UART_Receive_DMA( huart[uartx], ringBuf[uartx].pBuf, ringBuf[uartx].bufSize );
    }

	usart_dma_init(uartx);
	
    return rc;
}

/** 
* 从缓冲区获取指定长度的数据，如不足有多少返回多少. 
* 无. 
* @param[in]   uartx:串口号码,len:获取长度. 
* @param[out]  buf:接收的缓冲区.  
* @retval  返回的实际获取数据长度.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
uint32_t usart_pop( uint32_t uartx, uint8_t *buf, uint32_t len )
{
    uint32_t retval;
    //计算数据量
    retval = usart_num( uartx );
    len = len > retval ? retval : len;
    retval = len;
    ringBuf[uartx].rxlen -= retval;

    while( len > 0 )
    {
        *buf++ = ringBuf[uartx].pBuf[ringBuf[uartx].dataPopPtr];
        ringBuf[uartx].dataPopPtr = ( ringBuf[uartx].dataPopPtr + 1 ) % ringBuf[uartx].bufSize;
        len--;
    }

    return retval;
}

/** 
* 从缓冲区获取指定长度的数据，如不足等待指定时间. 
* 无. 
* @param[in]   uartx:串口号码,len:获取长度,timeout:超时时间. 
* @param[out]  buf:接收的缓冲区.  
* @retval  返回的实际获取数据长度.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
uint32_t usart_pop_time( uint32_t uartx, uint8_t *buf, uint32_t len, uint32_t timeout )
{
    uint32_t retval, tmo;
    //计算数据量
    tmo = HAL_GetTick() + timeout;

    while( usart_num( uartx ) < len )
    {
        if( tmo < HAL_GetTick() )
            break;
    }

    retval = usart_num( uartx );
    len = len > retval ? retval : len;
    retval = len;
    ringBuf[uartx].rxlen -= retval;

    while( len > 0 )
    {
        *buf++ = ringBuf[uartx].pBuf[ringBuf[uartx].dataPopPtr];
        ringBuf[uartx].dataPopPtr = ( ringBuf[uartx].dataPopPtr + 1 ) % ringBuf[uartx].bufSize;
        len--;
    }

    return retval;
}

/** 
* 从缓冲区获取1个字节的数据. 
* 无. 
* @param[in]   uartx:串口号码. 
* @param[out]  无.  
* @retval  返回实际数据，没有返回0.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
uint8_t usart_pop_byte( uint32_t uartx )
{
    uint8_t retval = 0;

    if( usart_num( uartx ) )
    {
        retval = ringBuf[uartx].pBuf[ringBuf[uartx].dataPopPtr];
        ringBuf[uartx].dataPopPtr = ( ringBuf[uartx].dataPopPtr + 1 ) % ringBuf[uartx].bufSize;
    }

    return retval;
}

/** 
* 获取缓冲区数据长度. 
* 无. 
* @param[in]   uartx:串口号码. 
* @param[out]  无.  
* @retval  返回实际长度.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
uint32_t usart_num( uint32_t uartx )
{
    uint32_t retval = 0;

    //计算数据量
    //retval = ringBuf[uartx].rxlen;
    if( !retval )
    {
        retval = ringBuf[uartx].bufSize - __HAL_DMA_GET_COUNTER( hdma_usart_rx[uartx] );

        if( retval < ringBuf[uartx].dataPopPtr )
        {
            retval = retval + ringBuf[uartx].bufSize - ringBuf[uartx].dataPopPtr;
        }
        else
        {
            retval -= ringBuf[uartx].dataPopPtr;
        }
    }

    return retval;
}

/** 
* 清空接收缓冲区. 
* 无. 
* @param[in]   uartx:串口号码. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
void usart_reset( uint32_t uartx )
{	
	ringBuf[uartx].rxlen = 0;
	ringBuf[uartx].dataPopPtr = 0;
}

/** 
* 打印接收缓冲区的内容. 
* 无. 
* @param[in]   uartx:串口号码. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
void usart_dump_rxbuf( uint32_t uartx )
{
    usart_log( "\r\nUART%d--DMA:%d\r\n", uartx + 1, __HAL_DMA_GET_COUNTER( hdma_usart_rx[uartx] ) );
    usart_dump( ringBuf[uartx].pBuf, ringBuf[uartx].bufSize );
}

void dbg_uart_start_send( void );

/** 
* DMA发送的中断回调函数. 
* 设置发送完成中断标志. 
* @param[in]   uartx:串口号码. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
void HAL_UART_TxCpltCallback( UART_HandleTypeDef *huart )  //锟斤拷锟斤拷锟斤拷锟?
{
    uint32_t uartx;
    uartx = usart_get_index( huart );
    __HAL_DMA_DISABLE( hdma_usart_tx[uartx] );
    uart_tx[uartx].ready = 1;
#if ( DBG_ENABLE == DBG_USE_UART )

    if( uartx == UART_FOR_DEBUG )
        dbg_uart_start_send();

#endif
    //start_uart_rx( uartx );
}

/** 
* usart发送. 
* 如果正在发送，等待指定时间，还没有发送完成，强制中断并开始新的数据发送. 
* @param[in]   uartx:串口号码,ptr:发送数据指针,len:发送长度. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
void usart_send( uint32_t uartx, uint8_t *ptr, uint32_t len )
{
    uint32_t i = 10;

    if( len > uart_tx[uartx].buf_len )
        return;

    while( ( !uart_tx[uartx].ready ) && ( i-- ) )
        uart_delay( 2 );

    if( !i )
        __HAL_DMA_DISABLE( hdma_usart_tx[uartx] );

    uart_tx[uartx].ready = 0;
    memcpy( uart_tx[uartx].buf, ptr, len );
    HAL_UART_Transmit_DMA( huart[uartx], uart_tx[uartx].buf, len );
}

/** 
* usart发送. 
* 如果正在发送，等待指定时间，还没有发送完成，强制中断并开始新的数据发送. 
* @param[in]   uartx:串口号码,ptr:发送数据指针,len:发送长度,timeout：超时时间. 
* @param[out]  无.  
* @retval  无.
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-09创建 
*/
int usart_send_timeout( uint32_t uartx, uint8_t *ptr, uint32_t len, uint32_t timeout )
{
    uint32_t i;
    int rc = 0;

    if( len > uart_tx[uartx].buf_len )
        goto exit;

    i = timeout;

    while( ( !uart_tx[uartx].ready ) && ( i-- ) )
        uart_delay( 1 );

    if( !i )
    {
        __HAL_DMA_DISABLE( hdma_usart_tx[uartx] );
    }

    uart_tx[uartx].ready = 0;
    memcpy( uart_tx[uartx].buf, ptr, len );
    HAL_UART_Transmit_DMA( huart[uartx], uart_tx[uartx].buf, len );
    rc = len;
exit:
    return rc;
}

#if ( DBG_ENABLE != DBG_USE_UART )
void dbg_uart_init( void )
{
}

#else

#define DBG_UART_BUF_LEN    2048
#define DBG_UART_TMP_BUF_LEN    512
uint8_t dbg_buf_tmp[DBG_UART_TMP_BUF_LEN];
uint8_t dbg_uart_buf[DBG_UART_BUF_LEN];
ring_buffer_t dbg_uart_ringbuf;

uint8_t hal_dbg_uart_tbuf[DBG_UART_TMP_BUF_LEN];
uint8_t hal_dbg_uart_rbuf[16];

void dbg_uart_init( void )
{
    static int uart_is_init = 0;

    if( !uart_is_init )
    {
        usart_open( UART_FOR_DEBUG, 57600, 8, 0, 1, hal_dbg_uart_rbuf, 16, hal_dbg_uart_tbuf, DBG_UART_TMP_BUF_LEN );
        ring_buffer_init( &dbg_uart_ringbuf, dbg_uart_buf, DBG_UART_BUF_LEN );
        uart_is_init = 1;
    }
}


void dbg_uart_start_send( void )
{
    int s_len;

    if( uart_tx[UART_FOR_DEBUG].ready )
    {
        ring_buffer_read( &dbg_uart_ringbuf, uart_tx[UART_FOR_DEBUG].buf, uart_tx[UART_FOR_DEBUG].buf_len,
                          ( uint32_t * )&s_len );

        if( s_len )
        {
            uart_led_switch( UART_FOR_DEBUG, 0 );
            HAL_UART_Transmit_DMA( huart[UART_FOR_DEBUG], uart_tx[UART_FOR_DEBUG].buf, s_len );
            uart_tx[UART_FOR_DEBUG].ready = 0;
        }
    }
}

void dDebug_Printf( const char *color, const char *format, ... )
{
    uint32_t length, col_len = 0;
    va_list args;
    dbg_uart_init();
    dbg_buf_tmp[0] = 0;
#if(DBG_UART_COLOR)

    if( color != NULL )
    {
        strcat( ( char * )dbg_buf_tmp, color );
        col_len = strlen( color );
    }

#else
    col_len = 0;
#endif
    va_start( args, format );
    length = vsnprintf( ( char * ) dbg_buf_tmp + col_len, DBG_UART_TMP_BUF_LEN - col_len, ( char * ) format, args );
    va_end( args );
    ring_buffer_write( &dbg_uart_ringbuf, ( uint8_t * )dbg_buf_tmp, length  + col_len );
    dbg_uart_start_send();
}
#endif

#ifdef USART1_EN
/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void usart1_msp_init(UART_HandleTypeDef* uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
/* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
}

/**
* @brief This function handles USART1 global interrupt.
*/
void USART1_IRQHandler( void )
{
    /* USER CODE BEGIN USART1_IRQn 0 */
    /* USER CODE END USART1_IRQn 0 */
    HAL_UART_IRQHandler( &huart1 );
    /* USER CODE BEGIN USART1_IRQn 1 */
    usart_receive_idle( &huart1 );
    /* USER CODE END USART1_IRQn 1 */
}

/**
* @brief This function handles DMA2 stream7 global interrupt.
*/
void DMA2_Stream7_IRQHandler( void )
{
    /* USER CODE BEGIN DMA2_Stream7_IRQn 0 */
    /* USER CODE END DMA2_Stream7_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_usart1_tx );
    /* USER CODE BEGIN DMA2_Stream7_IRQn 1 */
    /* USER CODE END DMA2_Stream7_IRQn 1 */
}

/**
* @brief This function handles DMA2 stream2 global interrupt.
*/
void DMA2_Stream2_IRQHandler( void )
{
    /* USER CODE BEGIN DMA2_Stream2_IRQn 0 */
    /* USER CODE END DMA2_Stream2_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_usart1_rx );
    /* USER CODE BEGIN DMA2_Stream2_IRQn 1 */
    /* USER CODE END DMA2_Stream2_IRQn 1 */
}
#endif

#ifdef USART2_EN
/* USART2 init function */
void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void usart2_msp_init(UART_HandleTypeDef* uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Stream6;
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
}
/**
* @brief This function handles USART2 global interrupt.
*/
void USART2_IRQHandler( void )
{
    /* USER CODE BEGIN USART2_IRQn 0 */
    /* USER CODE END USART2_IRQn 0 */
    HAL_UART_IRQHandler( &huart2 );
    /* USER CODE BEGIN USART2_IRQn 1 */
    usart_receive_idle( &huart2 );
    /* USER CODE END USART2_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream5 global interrupt.
*/
void DMA1_Stream5_IRQHandler( void )
{
    /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */
    /* USER CODE END DMA1_Stream5_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_usart2_rx );
    /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */
    /* USER CODE END DMA1_Stream5_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream6 global interrupt.
*/
void DMA1_Stream6_IRQHandler( void )
{
    /* USER CODE BEGIN DMA1_Stream6_IRQn 0 */
    /* USER CODE END DMA1_Stream6_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_usart2_tx );
    /* USER CODE BEGIN DMA1_Stream6_IRQn 1 */
    /* USER CODE END DMA1_Stream6_IRQn 1 */
}
#endif

#ifdef USART3_EN
/* USART3 init function */
void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void usart3_msp_init(UART_HandleTypeDef* uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();
  
    /**USART3 GPIO Configuration    
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART3 DMA Init */
    /* USART3_RX Init */
    hdma_usart3_rx.Instance = DMA1_Stream1;
    hdma_usart3_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart3_rx);

    /* USART3_TX Init */
    hdma_usart3_tx.Instance = DMA1_Stream3;
    hdma_usart3_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart3_tx);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
}

/**
* @brief This function handles USART3 global interrupt.
*/
void USART3_IRQHandler( void )
{
	/* USER CODE BEGIN USART1_IRQn 0 */
    /* USER CODE END USART1_IRQn 0 */
    HAL_UART_IRQHandler( &huart3 );
    /* USER CODE BEGIN USART1_IRQn 1 */
    usart_receive_idle( &huart3 );
    /* USER CODE END USART1_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream1 global interrupt.
*/
void DMA1_Stream1_IRQHandler( void )
{
    /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */
    /* USER CODE END DMA1_Stream1_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_usart3_rx );
    /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */
    /* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream3 global interrupt.
*/
void DMA1_Stream3_IRQHandler( void )
{
    /* USER CODE BEGIN DMA1_Stream3_IRQn 0 */
    /* USER CODE END DMA1_Stream3_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_usart3_tx );
    /* USER CODE BEGIN DMA1_Stream3_IRQn 1 */
    /* USER CODE END DMA1_Stream3_IRQn 1 */
}

#endif

#ifdef USART4_EN
/* UART4 init function */
void MX_UART4_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void usart4_msp_init(UART_HandleTypeDef* uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* USER CODE BEGIN UART4_MspInit 0 */

  /* USER CODE END UART4_MspInit 0 */
    /* UART4 clock enable */
	  
    __HAL_RCC_UART4_CLK_ENABLE();
  
    /**UART4 GPIO Configuration    
    PA0-WKUP     ------> UART4_TX
    PA1     ------> UART4_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* UART4 DMA Init */
    /* UART4_RX Init */
    hdma_uart4_rx.Instance = DMA1_Stream2;
    hdma_uart4_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart4_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart4_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart4_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart4_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart4_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart4_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart4_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart4_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart4_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_uart4_rx);

    /* UART4_TX Init */
    hdma_uart4_tx.Instance = DMA1_Stream4;
    hdma_uart4_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart4_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart4_tx.Init.Mode = DMA_NORMAL;
    hdma_uart4_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart4_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart4_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_uart4_tx);

    /* UART4 interrupt Init */
    HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspInit 1 */

  /* USER CODE END UART4_MspInit 1 */
}

/**
* @brief This function handles UART4 global interrupt.
*/
void UART4_IRQHandler( void )
{
    /* USER CODE BEGIN UART4_IRQn 0 */
    /* USER CODE END UART4_IRQn 0 */
    HAL_UART_IRQHandler( &huart4 );
    /* USER CODE BEGIN UART4_IRQn 1 */
    usart_receive_idle( &huart4 );
    /* USER CODE END UART4_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream2 global interrupt.
*/
void DMA1_Stream2_IRQHandler( void )
{
    /* USER CODE BEGIN DMA1_Stream2_IRQn 0 */
    /* USER CODE END DMA1_Stream2_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_uart4_rx );
    /* USER CODE BEGIN DMA1_Stream2_IRQn 1 */
    /* USER CODE END DMA1_Stream2_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream4 global interrupt.
*/
void DMA1_Stream4_IRQHandler( void )
{
    /* USER CODE BEGIN DMA1_Stream4_IRQn 0 */
    /* USER CODE END DMA1_Stream4_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_uart4_tx );
    /* USER CODE BEGIN DMA1_Stream4_IRQn 1 */
    /* USER CODE END DMA1_Stream4_IRQn 1 */
}

#endif

#ifdef USART5_EN
/* UART5 init function */
void MX_UART5_Init(void)
{

  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void usart5_msp_init(UART_HandleTypeDef* uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  /* USER CODE BEGIN UART5_MspInit 0 */

  /* USER CODE END UART5_MspInit 0 */
    /* UART5 clock enable */
    __HAL_RCC_UART5_CLK_ENABLE();
  
    /**UART5 GPIO Configuration    
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* UART5 DMA Init */
    /* UART5_RX Init */
    hdma_uart5_rx.Instance = DMA1_Stream0;
    hdma_uart5_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart5_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart5_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart5_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart5_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart5_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart5_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart5_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart5_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart5_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_uart5_rx);

    /* UART5_TX Init */
    hdma_uart5_tx.Instance = DMA1_Stream7;
    hdma_uart5_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart5_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart5_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart5_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart5_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart5_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart5_tx.Init.Mode = DMA_NORMAL;
    hdma_uart5_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart5_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart5_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_uart5_tx);

    /* UART5 interrupt Init */
    HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspInit 1 */

  /* USER CODE END UART5_MspInit 1 */
}

/**
* @brief This function handles UART5 global interrupt.
*/
void UART5_IRQHandler( void )
{
    /* USER CODE BEGIN UART5_IRQn 0 */
    /* USER CODE END UART5_IRQn 0 */
    HAL_UART_IRQHandler( &huart5 );
    /* USER CODE BEGIN UART5_IRQn 1 */
    usart_receive_idle( &huart5 );
    /* USER CODE END UART5_IRQn 1 */
}

void DMA1_Stream0_IRQHandler( void )
{
    /* USER CODE BEGIN DMA1_Stream0_IRQn 0 */
    /* USER CODE END DMA1_Stream0_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_uart5_rx );
    /* USER CODE BEGIN DMA1_Stream0_IRQn 1 */
    /* USER CODE END DMA1_Stream0_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream7 global interrupt.
*/
void DMA1_Stream7_IRQHandler( void )
{
    /* USER CODE BEGIN DMA1_Stream7_IRQn 0 */
    /* USER CODE END DMA1_Stream7_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_uart5_tx );
    /* USER CODE BEGIN DMA1_Stream7_IRQn 1 */
    /* USER CODE END DMA1_Stream7_IRQn 1 */
}

#endif

#ifdef USART6_EN
/* USART6 init function */
void MX_USART6_UART_Init(void)
{

  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void usart6_msp_init(UART_HandleTypeDef* uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  /* USER CODE BEGIN USART6_MspInit 0 */

  /* USER CODE END USART6_MspInit 0 */
    /* USART6 clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();
  
    /**USART6 GPIO Configuration    
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART6 DMA Init */
    /* USART6_RX Init */
    hdma_usart6_rx.Instance = DMA2_Stream1;
    hdma_usart6_rx.Init.Channel = DMA_CHANNEL_5;
    hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart6_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart6_rx);

    /* USART6_TX Init */
    hdma_usart6_tx.Instance = DMA2_Stream7;
    hdma_usart6_tx.Init.Channel = DMA_CHANNEL_5;
    hdma_usart6_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart6_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_tx.Init.Mode = DMA_NORMAL;
    hdma_usart6_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart6_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart6_tx);

    /* USART6 interrupt Init */
    HAL_NVIC_SetPriority(USART6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);
  /* USER CODE BEGIN USART6_MspInit 1 */

  /* USER CODE END USART6_MspInit 1 */
}

/**
* @brief This function handles USART6 global interrupt.
*/
void USART6_IRQHandler( void )
{
    /* USER CODE BEGIN USART6_IRQn 0 */
    /* USER CODE END USART6_IRQn 0 */
    HAL_UART_IRQHandler( &huart6 );
    /* USER CODE BEGIN USART6_IRQn 1 */
    usart_receive_idle( &huart6 );
    /* USER CODE END USART6_IRQn 1 */
}

/**
* @brief This function handles DMA2 stream1 global interrupt.
*/
void DMA2_Stream1_IRQHandler( void )
{
    /* USER CODE BEGIN DMA2_Stream1_IRQn 0 */
    /* USER CODE END DMA2_Stream1_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_usart6_rx );
    /* USER CODE BEGIN DMA2_Stream1_IRQn 1 */
    /* USER CODE END DMA2_Stream1_IRQn 1 */
}

/**
* @brief This function handles DMA2 stream6 global interrupt.
*/
void DMA2_Stream7_IRQHandler( void )
{
    /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */
    /* USER CODE END DMA2_Stream6_IRQn 0 */
    HAL_DMA_IRQHandler( &hdma_usart6_tx );
    /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */
    /* USER CODE END DMA2_Stream6_IRQn 1 */
}
#endif

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
	#ifdef USART1_EN
	if(uartHandle->Instance==USART1)
	{
		usart1_msp_init(uartHandle);
	}
	#endif
	#ifdef USART2_EN
	if(uartHandle->Instance==USART2)
	{
		usart2_msp_init(uartHandle);
	}
	#endif
	#ifdef USART3_EN
	if(uartHandle->Instance==USART3)
	{
		usart3_msp_init(uartHandle);
	}
	#endif
	#ifdef USART4_EN
	if(uartHandle->Instance==UART4)
	{
		usart4_msp_init(uartHandle);
	}
	#endif
	#ifdef USART5_EN
	if(uartHandle->Instance==UART5)
	{
		usart5_msp_init(uartHandle);
	}
	#endif
	#ifdef USART6_EN
	if(uartHandle->Instance==USART6)
	{
		usart6_msp_init(uartHandle);
	}
	#endif
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspDeInit 0 */

  /* USER CODE END UART4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART4_CLK_DISABLE();
  
    /**UART4 GPIO Configuration    
    PA0-WKUP     ------> UART4_TX
    PA1     ------> UART4_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1);

    /* UART4 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* UART4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspDeInit 1 */

  /* USER CODE END UART4_MspDeInit 1 */
  }
  else if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspDeInit 0 */

  /* USER CODE END UART5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART5_CLK_DISABLE();
  
    /**UART5 GPIO Configuration    
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    /* UART5 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* UART5 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspDeInit 1 */

  /* USER CODE END UART5_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();
  
    /**USART3 GPIO Configuration    
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspDeInit 0 */

  /* USER CODE END USART6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART6_CLK_DISABLE();
  
    /**USART6 GPIO Configuration    
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);

    /* USART6 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART6 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART6_IRQn);
  /* USER CODE BEGIN USART6_MspDeInit 1 */

  /* USER CODE END USART6_MspDeInit 1 */
  }
} 

/** 
  * Enable DMA controller clock
  */
void usart_dma_init(uint8_t usart) 
{
	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	
	#ifdef USART1_EN
	if( usart == USART1_NUM ){
		/* DMA2_Stream2_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
	}
	#endif
	#ifdef USART2_EN
	if( usart == USART2_NUM ){
		/* DMA1_Stream5_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
		/* DMA1_Stream6_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
	}
	#endif
	#ifdef USART3_EN
	if( usart == USART3_NUM ){
		/* DMA1_Stream3_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	}
	#endif
	#ifdef USART4_EN
	if( usart == USART4_NUM ){
		/* DMA1_Stream2_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
		/* DMA1_Stream4_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
	}
	#endif
	#ifdef USART5_EN
	if( usart == USART5_NUM ){
		/* DMA1_Stream0_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
		/* DMA1_Stream7_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
	}
	#endif
	#ifdef USART6_EN
	if( usart == USART6_NUM ){
		/* DMA1_Stream1_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
		/* DMA2_Stream7_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
	}
	#endif
}
