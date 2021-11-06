/** 
* @file     ring_buffer.c
* @brief    ���λ������. 
* @details  ���λ�������д. 
* @author   ken deng
* @date     2019-09-27 
* @version  A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 2019-09-27, ����\n 
*/  
#include "ring_buffer.h"
#include <string.h>

/** 
* ���λ�������ʼ��. 
* ��. 
* @param[in]   ring_buffer���������ĵؾ��,buffer:����ָ�룬buffer_size:�����С. 
* @param[out]  ��.  
* @retval  �������  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-27���� 
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
* �ӻ��λ�����д������. 
* ��. 
* @param[in]   ring_buffer���������ĵؾ��,data:��������ָ�룬data_length:д�볤��. 
* @param[out]  ��.  
* @retval  �ɹ�д�볤��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-27���� 
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
* �ӻ����ж�ȡһ���ֽ����ݲ�ɾ�������е�����. 
* ��. 
* @param[in]   ring_buffer���������ĵؾ����contiguous_bytes:�����С. 
* @param[out]  data:��������ָ��.  
* @retval  �������  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-27���� 
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
* �ӻ��λ����ж�ȡ����. 
* ��. 
* @param[in]   ring_buffer���������ĵؾ��,data_length:��ȡ����,number_of_bytes_read���ɹ���ȡ����. 
* @param[out]  data:��������ָ��.  
* @retval  ��ȡ���  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-27���� 
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
* ���㻺����ʣ��Ŀռ�. 
* ��. 
* @param[in]   ring_buffer���������ĵؾ��. 
* @param[out]  ��.  
* @retval  ���صĻ�����ʣ���С  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-27���� 
*/
uint32_t ring_buffer_free_space( ring_buffer_t *ring_buffer )
{
    uint32_t tail_to_end = ring_buffer->size-1 - ring_buffer->tail;
    return ( ( tail_to_end + ring_buffer->head ) % ring_buffer->size );
}

/** 
* ���㻺����ʹ�õĿռ�. 
* ��. 
* @param[in]   ring_buffer���������ĵؾ��. 
* @param[out]  ��.  
* @retval  ���صĻ�����ʹ�õĴ�С  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-09-27���� 
*/
uint32_t ring_buffer_used_space( ring_buffer_t *ring_buffer )
{
    uint32_t head_to_end = ring_buffer->size - ring_buffer->head;
    return ( ( head_to_end + ring_buffer->tail ) % ring_buffer->size );
}
