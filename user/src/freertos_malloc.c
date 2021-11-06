/** 
* @file         freertos_malloc.c 
* @brief        freertos���ڴ���估�ͷź���. 
* @details  	��freetos���ڴ��������װ�ɱ�׼�ڴ������. 
* @author       Ken Deng 
* @date     	2019-05-04 
* @version  A001 
* @par Copyright (c):  
*       LAXTON 
* @par History:          
*   2019-05-04: Ken Deng, 2019-05-04, ����\n 
*/  
#include "system.h"
#include "main.h"
#include "stm32f4xx_hal.h"

#include <string.h>

/** 
* �жϵ�ǰ���д����Ƿ����ж���. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  0�����ж��� 
* @retval  !0:�����ж���  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-05-04���� 
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
* �ڴ����뺯��. 
* ��. 
* @param[in]   size:�����С. 
* @param[out]  ��.  
* @retval  NULL:û���ڴ���� 
* @retval  �����ڴ��ַ  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-05-04���� 
*/
void *malloc( size_t size )
{
    void *p = NULL;
	
    assert_param( !inISR() );
    p = ( pvPortMalloc( size ) );

    return p;
}

/** 
* �޸Ķ�̬�ڴ��С. 
* ��. 
* @param[in]   ptr:�ڴ��ַ,size:�޸Ĵ�С. 
* @param[out]  ��.  
* @retval  �����ڴ��ַ  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-05-04���� 
*/
void *realloc( void *ptr, size_t size )
{
//    assert_param( !inISR() );
//    return ( pvPortRealloc( ptr, size ) );
	return 0;
}

/** 
* �ڴ����뺯��. 
* ��. 
* @param[in]   size:�����С. 
* @param[out]  ��.  
* @retval  NULL:û���ڴ���� 
* @retval  �����ڴ��ַ  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-05-04���� 
*/
void *calloc( size_t nelem, size_t elsize )
{
    assert_param( !inISR() );
    return ( pvPortCalloc( nelem, elsize ) );
}

/** 
* �ڴ��ͷź���. 
* ��. 
* @param[in]   ptr:�ڴ��ַ. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-05-04���� 
*/
void free( void *ptr )
{
    if( ptr == NULL )
        return;
    assert_param( !inISR() );

    vPortFree( ptr );
}

/** 
* ����nelem������Ϊelsize�������ռ亯��. 
* ��. 
* @param[in]   nelem�����������lesize:�ڴ���С. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-05-04���� 
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
* �ڴ濽����ȫ����. 
* ��. 
* @param[in]   det:Ŀ���ַ��sour:Դ��ַ��len:����. 
* @param[out]  ��.  
* @retval  ��  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-05-04���� 
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
