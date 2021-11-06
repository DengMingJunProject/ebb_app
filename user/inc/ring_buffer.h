#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "stdint.h"
#include "macro_def.h"

#ifndef   MIN
#define MIN(a, b)         (((a) < (b)) ? (a) : (b))
#endif

#ifndef   MAX
#define MAX(a, b)         (((a) > (b)) ? (a) : (b))
#endif

typedef  struct
{
    uint8_t *buffer;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
} ring_buffer_t;

/* Ring Buffer API */
FUN_STATUS_T ring_buffer_init( ring_buffer_t *ring_buffer, uint8_t *buffer, uint32_t buffer_size );
FUN_STATUS_T ring_buffer_deinit( ring_buffer_t *ring_buffer );
uint32_t ring_buffer_write( ring_buffer_t *ring_buffer, const uint8_t *data, uint32_t data_length );
uint32_t ring_buffer_used_space( ring_buffer_t *ring_buffer );
uint32_t ring_buffer_free_space( ring_buffer_t *ring_buffer );
FUN_STATUS_T ring_buffer_get_data( ring_buffer_t *ring_buffer, uint8_t * *data, uint32_t *contiguous_bytes );
FUN_STATUS_T ring_buffer_consume( ring_buffer_t *ring_buffer, uint32_t bytes_consumed );
FUN_STATUS_T ring_buffer_read( ring_buffer_t *ring_buffer, uint8_t *data, uint32_t data_length,uint32_t *number_of_bytes_read );

#endif
