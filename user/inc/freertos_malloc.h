#ifndef __NEW_MALLOC_H__
#define __NEW_MALLOC_H__

extern void *malloc( size_t size );
extern void free( void *ptr );
extern void *memcpy_s(void *dest,const void *sour,size_t len );
#endif
