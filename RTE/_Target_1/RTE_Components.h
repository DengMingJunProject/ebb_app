
/*
 * Auto generated Run-Time-Environment Configuration File
 *      *** Do not modify ! ***
 *
 * Project: 'ebb_app' 
 * Target:  'Target 1' 
 */

#ifndef RTE_COMPONENTS_H
#define RTE_COMPONENTS_H


/*
 * Define the Device Header File: 
 */
#define CMSIS_device_header "stm32f4xx.h"

/*  ARM.FreeRTOS::RTOS:Config:CMSIS RTOS2:10.2.0 */
#define RTE_RTOS_FreeRTOS_CONFIG_RTOS2  /* RTOS FreeRTOS Config for CMSIS RTOS2 API */
/*  ARM.FreeRTOS::RTOS:Core:Cortex-M:10.2.0 */
#define RTE_RTOS_FreeRTOS_CORE          /* RTOS FreeRTOS Core */
/*  ARM.FreeRTOS::RTOS:Event Groups:10.2.0 */
#define RTE_RTOS_FreeRTOS_EVENTGROUPS   /* RTOS FreeRTOS Event Groups */
/*  ARM.FreeRTOS::RTOS:Heap:Heap_4:10.2.0 */
#define RTE_RTOS_FreeRTOS_HEAP_4        /* RTOS FreeRTOS Heap 4 */
/*  ARM.FreeRTOS::RTOS:Timers:10.2.0 */
#define RTE_RTOS_FreeRTOS_TIMERS        /* RTOS FreeRTOS Timers */
/*  ARM::CMSIS:RTOS2:FreeRTOS:10.2.0 */
#define RTE_CMSIS_RTOS2                 /* CMSIS-RTOS2 */
        #define RTE_CMSIS_RTOS2_FreeRTOS        /* CMSIS-RTOS2 FreeRTOS */
/*  ARM::CMSIS:RTOS:FreeRTOS:10.2.0 */
#define RTE_CMSIS_RTOS                  /* CMSIS-RTOS */
        #define RTE_CMSIS_RTOS_FreeRTOS         /* CMSIS-RTOS FreeRTOS */
/*  Keil.ARM Compiler::Compiler:I/O:File:File System:1.2.0 */
#define RTE_Compiler_IO_File            /* Compiler I/O: File */
          #define RTE_Compiler_IO_File_FS         /* Compiler I/O: File (File System) */
/*  Keil.MDK-Pro::File System:CORE:LFN:6.11.0 */
#define RTE_FileSystem_Core             /* File System Core */
          #define RTE_FileSystem_LFN              /* File System with Long Filename support */
          #define RTE_FileSystem_Release          /* File System Release Version */
/*  Keil.MDK-Pro::File System:Drive:Memory Card:6.11.0 */
#define RTE_FileSystem_Drive_MC_0       /* File System Memory Card Drive 0 */

/*  Keil.MDK-Pro::File System:Drive:NAND:6.11.0 */
#define RTE_FileSystem_Drive_NAND_0     /* File System NAND Flash Drive 0 */

/*  Keil.MDK-Pro::USB:CORE:Release:6.13.0 */
#define RTE_USB_Core                    /* USB Core */
          #define RTE_USB_Core_Release            /* USB Core Release Version */
/*  Keil.MDK-Pro::USB:Device:6.13.0 */
#define RTE_USB_Device_0                /* USB Device 0 */

/*  Keil.MDK-Pro::USB:Device:Custom Class:6.13.0 */
#define RTE_USB_Device_CustomClass_0    /* USB Device Custom Class instance 0 */

/*  Keil.MDK-Pro::USB:Host:6.13.0 */
#define RTE_USB_Host_0                  /* USB Host 0 */

/*  Keil::CMSIS Driver:MCI:2.8 */
#define RTE_Drivers_MCI0                /* Driver MCI0 */
/*  Keil::CMSIS Driver:USART:2.10 */
#define RTE_Drivers_USART1              /* Driver USART1 */
        #define RTE_Drivers_USART2              /* Driver USART2 */
        #define RTE_Drivers_USART3              /* Driver USART3 */
        #define RTE_Drivers_USART4              /* Driver USART4 */
        #define RTE_Drivers_USART5              /* Driver USART5 */
        #define RTE_Drivers_USART6              /* Driver USART6 */
        #define RTE_Drivers_USART7              /* Driver USART7 */
        #define RTE_Drivers_USART8              /* Driver USART8 */
        #define RTE_Drivers_USART9              /* Driver USART9 */
        #define RTE_Drivers_USART10             /* Driver USART10 */
/*  Keil::CMSIS Driver:USB Device:High-speed:2.19 */
#define RTE_Drivers_USBD1               /* Driver USBD1 */
/*  Keil::CMSIS Driver:USB Host:High-speed:2.22 */
#define RTE_Drivers_USBH1               /* Driver USBH1 */
/*  Keil::Device:STM32Cube Framework:Classic:1.7.4 */
#define RTE_DEVICE_FRAMEWORK_CLASSIC
/*  Keil::Device:STM32Cube HAL:Common:1.7.4 */
#define RTE_DEVICE_HAL_COMMON
/*  Keil::Device:STM32Cube HAL:Cortex:1.7.4 */
#define RTE_DEVICE_HAL_CORTEX
/*  Keil::Device:STM32Cube HAL:DMA:1.7.4 */
#define RTE_DEVICE_HAL_DMA
/*  Keil::Device:STM32Cube HAL:GPIO:1.7.4 */
#define RTE_DEVICE_HAL_GPIO
/*  Keil::Device:STM32Cube HAL:I2C:1.7.4 */
#define RTE_DEVICE_HAL_I2C
/*  Keil::Device:STM32Cube HAL:PWR:1.7.4 */
#define RTE_DEVICE_HAL_PWR
/*  Keil::Device:STM32Cube HAL:RCC:1.7.4 */
#define RTE_DEVICE_HAL_RCC
/*  Keil::Device:STM32Cube HAL:SPI:1.7.4 */
#define RTE_DEVICE_HAL_SPI
/*  Keil::Device:STM32Cube HAL:USART:1.7.4 */
#define RTE_DEVICE_HAL_USART
/*  Keil::Device:Startup:2.6.2 */
#define RTE_DEVICE_STARTUP_STM32F4XX    /* Device Startup for STM32F4 */


#endif /* RTE_COMPONENTS_H */
