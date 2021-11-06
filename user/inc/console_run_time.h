#ifndef __CONSOLE_RUN_TIME_H__
#define __CONSOLE_RUN_TIME_H__

#include "stdint.h"
#include "stdbool.h"

extern uint32_t runtime_counter;

#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configGENERATE_RUN_TIME_STATS 1

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() 	(runtime_counter=0)
#define portGET_RUN_TIME_COUNTER_VALUE() 			(runtime_counter)

extern void run_time_init(void);

#endif
