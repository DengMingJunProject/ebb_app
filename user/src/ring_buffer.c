/** 
* @file     ring_buffer.c
* @brief    环形缓冲操作. 
* @details  环形缓冲区读写. 
* @author   ken deng
* @date     2019-09-27 
* @version  A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-09-27, 创建\n 
*/  
#include "ring_buffer.h"
#include <string.h>

/** 
* 环形缓冲句柄初始化. 
* 无. 
* @param[in]   ring_buffer：缓冲区的地句柄,buffer:缓冲指针，buffer_size:缓冲大小. 
* @param[out]  无.  
* @retval  操作结果  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T ring_buffer_init( /*@out@*/ ring_buffer_t *ring_buffer, /*@keep@*/ uint8_t *buffer, uint32_t buffer_size )
{
    ring_buffer->buffer = ( uint8_t * )buffer;
    ring_buffer->size   = buffer_size;
    ring_buffer->head   = 0;
    ring_buffer->tail   = 0;
    return FUN_OK;
}

FUN_STATUS_T ring_buffer_deinit( ring_buffer_t *ring_buffer )
{
    return FUN_OK;
}

/** 
* 从环形缓冲中写入数据. 
* 无. 
* @param[in]   ring_buffer：缓冲区的地句柄,data:返回数据指针，data_length:写入长度. 
* @param[out]  无.  
* @retval  成功写入长度  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
uint32_t ring_buffer_write( ring_buffer_t *ring_buffer, const uint8_t *data, uint32_t data_length )
{
    uint32_t tail_to_end = ring_buffer->size  - ring_buffer->tail;
    uint32_t amount_to_copy = MIN( data_length, ( tail_to_end + ring_buffer->head ) % (ring_buffer->size + 1) );
    memcpy( &ring_buffer->buffer[ring_buffer->tail], data, MIN( amount_to_copy, tail_to_end ) );
    if( tail_to_end < amount_to_copy )
    {
        memcpy( &ring_buffer->buffer[ 0 ], data + tail_to_end, amount_to_copy - tail_to_end );
    }
    ring_buffer->tail = ( ring_buffer->tail + amount_to_copy ) % ring_buffer->size;
    return amount_to_copy;
}

/** 
* 从缓冲中读取一个字节数据不删除缓冲中的数据. 
* 无. 
* @param[in]   ring_buffer：缓冲区的地句柄，contiguous_bytes:缓冲大小. 
* @param[out]  data:返回数据指针.  
* @retval  操作结果  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T ring_buffer_get_data( ring_buffer_t *ring_buffer, uint8_t **data, uint32_t *contiguous_bytes )
{
    uint32_t head_to_end = ring_buffer->size - ring_buffer->head;
    *data = &ring_buffer->buffer[ring_buffer->head];
    *contiguous_bytes = MIN( head_to_end, ( head_to_end + ring_buffer->tail ) % ring_buffer->size );
    return FUN_OK;
}

FUN_STATUS_T ring_buffer_consume( ring_buffer_t *ring_buffer, uint32_t bytes_consumed )
{
    ring_buffer->head = ( ring_buffer->head + bytes_consumed ) % ring_buffer->size;
    return FUN_OK;
}

/** 
* 从环形缓冲中读取数据. 
* 无. 
* @param[in]   ring_buffer：缓冲区的地句柄,data_length:读取长度,number_of_bytes_read：成功读取长度. 
* @param[out]  data:返回数据指针.  
* @retval  读取结果  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
FUN_STATUS_T ring_buffer_read( ring_buffer_t *ring_buffer, uint8_t *data, uint32_t data_length, uint32_t *number_of_bytes_read )
{
    uint32_t max_bytes_to_read;
    uint32_t i;
    uint32_t head = ring_buffer->head;
    max_bytes_to_read = MIN( data_length, ring_buffer_used_space( ring_buffer ) );
    if( max_bytes_to_read != 0 )
    {
        for( i = 0; i != max_bytes_to_read; i++, ( head = ( head + 1 ) % ring_buffer->size ) )
        {
            data[ i ] = ring_buffer->buffer[ head ];
        }
        ring_buffer_consume( ring_buffer, max_bytes_to_read );
    }
    *number_of_bytes_read = max_bytes_to_read;
    return FUN_OK;
}

/** 
* 计算缓冲区剩余的空间. 
* 无. 
* @param[in]   ring_buffer：缓冲区的地句柄. 
* @param[out]  无.  
* @retval  返回的缓冲区剩余大小  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
uint32_t ring_buffer_free_space( ring_buffer_t *ring_buffer )
{
    uint32_t tail_to_end = ring_buffer->size-1 - ring_buffer->tail;
    return ( ( tail_to_end + ring_buffer->head ) % ring_buffer->size );
}

/** 
* 计算缓冲区使用的空间. 
* 无. 
* @param[in]   ring_buffer：缓冲区的地句柄. 
* @param[out]  无.  
* @retval  返回的缓冲区使用的大小  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      ken deng于2019-09-27创建 
*/
uint32_t ring_buffer_used_space( ring_buffer_t *ring_buffer )
{
    uint32_t head_to_end = ring_buffer->size - ring_buffer->head;
    return ( ( head_to_end + ring_buffer->tail ) % ring_buffer->size );
}
