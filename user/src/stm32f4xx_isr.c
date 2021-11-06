/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "system.h"
#include "stm32f4xx.h"
#include "stm32f4xx_isr.h"
#include "stm32f4xx_hal.h"
#include "rf_module.h"
#include "dbg.h"
#include "tca9535pwr.h"
#include "lcdxx.h"
#include "console_uart.h"


void TM_EXTI_Handler(uint16_t GPIO_Pin) {
	
	/* Check for proper interrupt pin */
	if (GPIO_Pin == RF_IRQ_PIN) {
		rf_module_isr_handle();
	}
	else if (GPIO_Pin == TCA9535PWR_IRQ_PIN) {
		tca9535pwr_isr_handle();
	}
}


// hard fault handler in C,  
// with stack frame location as input parameter  
void hard_fault_handler_c(unsigned int * hardfault_args)  
{  
	unsigned int stacked_r0;  
	unsigned int stacked_r1;  
	unsigned int stacked_r2;  
	unsigned int stacked_r3;  
	unsigned int stacked_r12;  
	static unsigned int stacked_lr;  
	static unsigned int stacked_pc;  
	unsigned int stacked_psr;  
	unsigned int CFSR;
    unsigned int HFSR;
	 
	stacked_r0 = ((unsigned long) hardfault_args[0]);  
	stacked_r1 = ((unsigned long) hardfault_args[1]);  
	stacked_r2 = ((unsigned long) hardfault_args[2]);  
	stacked_r3 = ((unsigned long) hardfault_args[3]);  
	 
	stacked_r12 = ((unsigned long) hardfault_args[4]);  
	stacked_lr = ((unsigned long) hardfault_args[5]);  
	stacked_pc = ((unsigned long) hardfault_args[6]);  
	stacked_psr = ((unsigned long) hardfault_args[7]);  
	 
	DBG_LOG ("[Hard fault handler]\n");  
	DBG_LOG ("R0 = 0x%x\n", stacked_r0);  
	DBG_LOG ("R1 = 0x%x\n", stacked_r1);  
	DBG_LOG ("R2 = 0x%x\n", stacked_r2);  
	DBG_LOG ("R3 = 0x%x\n", stacked_r3);  
	DBG_LOG ("R12 = 0x%x\n", stacked_r12);  
	DBG_LOG ("LR = 0x%x\n", stacked_lr);  
	DBG_LOG ("PC = 0x%x\n", stacked_pc);  
	DBG_LOG ("PSR = 0x%x\n", stacked_psr);  
	DBG_LOG ("BFAR = 0x%x\n", (*((volatile unsigned long *)(0xE000ED38))));  
	DBG_LOG ("CFSR = 0x%x\n", (*((volatile unsigned long *)(0xE000ED28))));  
	DBG_LOG ("HFSR = 0x%x\n", (*((volatile unsigned long *)(0xE000ED2C))));  
	DBG_LOG ("DFSR = 0x%x\n", (*((volatile unsigned long *)(0xE000ED30))));  
	DBG_LOG ("AFSR = 0x%x\n", (*((volatile unsigned long *)(0xE000ED3C))));  
	
	CFSR = *((volatile unsigned long *)(0xE000ED28));
    DBG_LOG ("CFSR = 0x%08x\r\n", CFSR); 
    DBG_LOG (" BFARVALID   :%d\r\n",CFSR&(1<<15)?1:0);
    DBG_LOG (" STKERR      :%d\r\n",CFSR&(1<<12)?1:0);
    DBG_LOG (" UNSTKERR    :%d\r\n",CFSR&(1<<11)?1:0);
    DBG_LOG (" IMPRECISERR :%d\r\n",CFSR&(1<<10)?1:0);
    DBG_LOG (" PRECISERR   :%d\r\n",CFSR&(1<<9)?1:0);
    DBG_LOG (" IBUSERR     :%d\r\n",CFSR&(1<<8)?1:0);
    DBG_LOG (" MMARVALID   :%d\r\n",CFSR&(1<<7)?1:0);
    DBG_LOG (" MSTKERR     :%d\r\n",CFSR&(1<<4)?1:0);
    DBG_LOG (" MUNSTKERR   :%d\r\n",CFSR&(1<<3)?1:0);
    DBG_LOG (" DACCVIOL    :%d\r\n",CFSR&(1<<1)?1:0);
    DBG_LOG (" IACCVIOL    :%d\r\n",CFSR&(1<<0)?1:0);
    DBG_LOG (" DIVBYZERO   :%d\r\n",CFSR&(1<<25)?1:0);
    DBG_LOG (" UNALIGNED   :%d\r\n",CFSR&(1<<24)?1:0);
    DBG_LOG (" NOCP        :%d\r\n",CFSR&(1<<19)?1:0);
    DBG_LOG (" INVPC       :%d\r\n",CFSR&(1<<18)?1:0);
    DBG_LOG (" INVSTATE    :%d\r\n",CFSR&(1<<17)?1:0);
    DBG_LOG (" UNDEFINSTR  :%d\r\n",CFSR&(1<<16)?1:0);
    HFSR = *((volatile unsigned long *)(0xE000ED2C));
    DBG_LOG ("HFSR = 0x%08x\r\n", HFSR); 
    DBG_LOG (" DEBUGEVT    :%d\r\n",HFSR&((uint32_t)1<<31)?1:0);
    DBG_LOG (" FORCED      :%d\r\n",HFSR&(1<<30)?1:0);
    DBG_LOG (" VECTBL      :%d\r\n",HFSR&(1<<1)?1:0);
	
//	fault_trace_disp();
//	heap_size_disp();
//	char buf[17];
//	snprintf(buf,17,"%08X%08X",stacked_lr,CFSR);
//	lcdxx_clear_screen();
//	lcdxx_dispaly_data(LCDXX_ROW1,0,(uint8_t*)"Hard fault      ",16);
//	lcdxx_dispaly_data(LCDXX_ROW2,0,(uint8_t*)buf,16);
	   
 	while(1)
	{
			;;
	} 
}  

void HardFault_Handler(void){
	
	while(1){
	}
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


