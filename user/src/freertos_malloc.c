/** 
* @file         freertos_malloc.c 
* @brief        freertos的内存分配及释放函数. 
* @details  	将freetos的内存管理函数封装成标准内存管理函数. 
* @author       Ken Deng 
* @date     	2019-05-04 
* @version  A001 
* @par Copyright (c):  
*       LAXTON 
* @par History:          
*   2019-05-04: Ken Deng, 2019-05-04, 建立\n 
*/  
#include "system.h"
#include "main.h"
#include "stm32f4xx_hal.h"

#include <string.h>

/** 
* 判断当前运行代码是否在中断中. 
* 无. 
* @param[in]   无. 
* @param[out]  无.  
* @retval  0：在中断中 
* @retval  !0:不在中断中  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-05-04创建 
*/
inline uint8_t inISR( void )
{
#if   defined ( __CC_ARM )
    register uint32_t __regIPSR          __ASM( "ipsr" );
    return( __regIPSR );
#else
    return ( __get_IPSR() != 0 );
#endif
}

void *pvPortCalloc( size_t nelem, size_t elsize );

/** 
* 内存申请函数. 
* 无. 
* @param[in]   size:申请大小. 
* @param[out]  无.  
* @retval  NULL:没有内存可用 
* @retval  返回内存地址  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-05-04创建 
*/
void *malloc( size_t size )
{
    void *p = NULL;
	
    assert_param( !inISR() );
    p = ( pvPortMalloc( size ) );

    return p;
}

/** 
* 修改动态内存大小. 
* 无. 
* @param[in]   ptr:内存地址,size:修改大小. 
* @param[out]  无.  
* @retval  返回内存地址  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-05-04创建 
*/
void *realloc( void *ptr, size_t size )
{
//    assert_param( !inISR() );
//    return ( pvPortRealloc( ptr, size ) );
	return 0;
}

/** 
* 内存申请函数. 
* 无. 
* @param[in]   size:申请大小. 
* @param[out]  无.  
* @retval  NULL:没有内存可用 
* @retval  返回内存地址  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-05-04创建 
*/
void *calloc( size_t nelem, size_t elsize )
{
    assert_param( !inISR() );
    return ( pvPortCalloc( nelem, elsize ) );
}

/** 
* 内存释放函数. 
* 无. 
* @param[in]   ptr:内存地址. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-05-04创建 
*/
void free( void *ptr )
{
    if( ptr == NULL )
        return;
    assert_param( !inISR() );

    vPortFree( ptr );
}

/** 
* 分配nelem个长度为elsize的连续空间函数. 
* 无. 
* @param[in]   nelem：区域个数，lesize:内存块大小. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-05-04创建 
*/
void *pvPortCalloc( size_t nelem, size_t elsize )
{
    assert_param( !inISR() );
    void *pvReturn = NULL;
    size_t len;
    len = nelem * elsize;
    pvReturn = pvPortMalloc( len );
    if( pvReturn != NULL )
        memset( pvReturn, 0x00, len );
    return ( pvReturn );
}

/** 
* 内存拷贝安全函数. 
* 无. 
* @param[in]   det:目标地址，sour:源地址，len:长度. 
* @param[out]  无.  
* @retval  无  
* @par 标识符 
*      保留 
* @par 其它 
*      无 
* @par 修改日志 
*      Ken Deng于2019-05-04创建 
*/
void *memcpy_s(void *dest,const void *sour,size_t len )
{
	static xSemaphoreHandle mem_xSemaphore=NULL;
	if( mem_xSemaphore==NULL ){
		mem_xSemaphore = xSemaphoreCreateMutex();
	}
	
	xSemaphoreTake(mem_xSemaphore,portMAX_DELAY);
	
	for( uint32_t i=0; i<len; i++ ){
		*(uint8_t*)dest++ = *(uint8_t*)sour++;
	}
	
	xSemaphoreGive(mem_xSemaphore);
}
