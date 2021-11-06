/** 
* @file         dwt.c 
* @brief        Ӳ����ʱ. 
* @details  	������ģ��ʱ����ʱ. 
* @author       ken deng 
* @date     	2019-07-25 
* @version  	A001 
* @par Copyright (c):  
*       
* @par History:          
*   version: ken deng, 019-07-25, ����\n 
*/  
#include "dwt.h"

/* --------------------------------------------REGISTERS-------------------------------------------------*/
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)

/** 
* Ӳ����ʱ��ʼ��. 
* ��. 
* @param[in]   ��. 
* @param[out]  ��.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-25���� 
*/
void dwt_init(void)
{
	DEM_CR         |= (unsigned int)DEM_CR_TRCENA;   /* Enable Cortex-M4's DWT CYCCNT reg.  */
	DWT_CYCCNT      = (unsigned int)0u;
	DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}

/** 
* ms�������ʱ. 
* ��. 
* @param[in]   _ulDelayTime����ʱmsʱ��. 
* @param[out]  ����us����ʱ����.  
* @retval  ��
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      ken deng��2019-07-25���� 
*/
void dwt_delay_ms(uint32_t _ulDelayTime)
{
	dwt_delay_us(1000*_ulDelayTime);
}


/**********************************************************************************************************************
 * FunctionName : bsp_DelayUS
 * Brief        : The delay here is implemented by the internal count of the CPU, 32-bit counter
 * Author       : Eric2013
 * Param        : _ulDelayTime : Delay length in 1 us
 * RetVal       : None
 * Version      : V1.0
 * Note         :1. In the case of a main frequency of 168MHz, the 32-bit counter is full 2^32/168000000 = 25.565 seconds. 
 *                  It is recommended to use this function for delay, and the delay is less than 1 second.  
 *               2. Actually through the oscilloscope test, the subtle delay function actually runs more than 0.25us more than the actual setting.��
 *               The test condition data is as follows:
 *             -------------------------------------------
 *                Test             Actual execution
 *             bsp_DelayUS(1)          1.2360us
 *             bsp_DelayUS(2)          2.256us
 *             bsp_DelayUS(3)          3.256us
 *             bsp_DelayUS(4)          4.256us
 *             bsp_DelayUS(5)          5.276us
 *             bsp_DelayUS(6)          6.276us
 *             bsp_DelayUS(7)          7.276us
 *             bsp_DelayUS(8)          8.276us
 *             bsp_DelayUS(9)          9.276us
 *             bsp_DelayUS(10)         10.28us
***********************************************************************************************************************/
/** 
* us����ʱ����. 
* ��. 
* @param[in]   _ulDelayTime:us��ʱ����. 
* @param[out]  ��.  
* @retval  ��.
* @par ��ʶ�� 
*      ���� 
* @par 
* Note         :1. In the case of a main frequency of 168MHz, the 32-bit counter is full 2^32/168000000 = 25.565 seconds. 
 *                  It is recommended to use this function for delay, and the delay is less than 1 second.  
 *               2. Actually through the oscilloscope test, the subtle delay function actually runs more than 0.25us more than the actual setting.��
 *               The test condition data is as follows:
 *             -------------------------------------------
 *                Test             Actual execution
 *             bsp_DelayUS(1)          1.2360us
 *             bsp_DelayUS(2)          2.256us
 *             bsp_DelayUS(3)          3.256us
 *             bsp_DelayUS(4)          4.256us
 *             bsp_DelayUS(5)          5.276us
 *             bsp_DelayUS(6)          6.276us
 *             bsp_DelayUS(7)          7.276us
 *             bsp_DelayUS(8)          8.276us
 *             bsp_DelayUS(9)          9.276us
 *             bsp_DelayUS(10)         10.28us
* @par �޸���־ 
*      Ken Deng��2019-07-25���� 
*/
void dwt_delay_us(uint32_t _ulDelayTime)
{
    uint32_t tCnt, tDelayCnt;
	uint32_t tStart;
		
	tStart = DWT_CYCCNT;                                     /* Counter value just entered */
	tCnt = 0;
	tDelayCnt = _ulDelayTime * (SystemCoreClock / 1000000);	 /* Number of beats required */ 		      

	while(tCnt < tDelayCnt)
	{
		tCnt = DWT_CYCCNT - tStart; /* During the process of subtraction, if the first 32-bit counter is recounted, it can still be calculated correctly. */	
	}
}
/** 
* �Ի���������ʱ������stm32f4 168M 1����Ϊ6ns. 
* ��Ҫ��Ӧ�ó�����л���,������stm32f4 168M��С��ʱ��λ120ns. 
* @param[in]   ��ʱ����. 
* @param[out]  none.  
* @retval  OK  �ɹ� 
* @retval  ERROR   ����  
* @par ��ʶ�� 
*      ���� 
* @par ���� 
*      �� 
* @par �޸���־ 
*      Ken Deng��2019-07-02���� 
*/
__inline void dwt_delay_cycle(uint32_t _ulDelayTime)
{
	uint32_t tStart;
	tStart = DWT_CYCCNT;                                     /* Counter value just entered */
	while((DWT_CYCCNT - tStart) < _ulDelayTime);
}

uint32_t dwt_read(void){
	return DWT_CYCCNT;
}


