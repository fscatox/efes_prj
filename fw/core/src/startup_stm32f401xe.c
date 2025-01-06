/**
 * @file     startup_stm32f401xe.c
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     05.01.2025
 * @brief    CMSIS-Core(M) Device Startup File
 */

#include <stddef.h>
#include "stm32f4xx.h"

/*
 *  External references
 */

/* Symbols defined in linker script */
extern uintptr_t __stack_base[];

extern uintptr_t __data_vstart[];
extern uintptr_t __data_lstart[];
extern uintptr_t __data_size[];

extern uintptr_t __bss_start[];
extern uintptr_t __bss_size[];

/* C/C++ runtime entry point */
extern void _start(void) __NO_RETURN;

/*
 *  Interrupt handlers
 *  (most of them weakly linked to the Default_Handler)
 */

void Default_Handler(void) {
    while (1);
}

/* System and Fault handlers */

__NO_RETURN void Reset_Handler(void) {

  /* Initialize subsystems (FPU) */
  SystemInit();

  /* Initialize data and bss sections into SRAM */
  __builtin_memcpy(__data_vstart, __data_lstart, (size_t) __data_size);
  __builtin_memset(__bss_start, 0, (size_t) __bss_size);

  /* C/C++ entry point */
  _start();

}
void NMI_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)
    __attribute__((weak));
void MemManage_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)
    __attribute__((weak, alias("Default_Handler")));

/* IRQs handlers */
void WWDG_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TAMP_STAMP_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream0_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream1_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream2_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream3_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream4_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream5_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream6_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void ADC_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_TIM9_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_TIM10_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_TIM11_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void OTG_FS_WKUP_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream7_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void SDIO_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM5_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream0_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream1_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream2_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream3_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream4_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void OTG_FS_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream5_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream6_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream7_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void USART6_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void SPI4_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));

/*
 *  Vector table
 */

typedef void (*VECTOR_TABLE_Type)(void);

const VECTOR_TABLE_Type VectorTable[101]
    __attribute__((used, section(".isr_vector"))) = {
        (VECTOR_TABLE_Type)__stack_base,
        Reset_Handler,
        NMI_Handler,
        HardFault_Handler,
        MemManage_Handler,
        BusFault_Handler,
        UsageFault_Handler,
        0,
        0,
        0,
        0,
        SVC_Handler,
        DebugMon_Handler,
        0,
        PendSV_Handler,
        SysTick_Handler,

        WWDG_IRQHandler,
        PVD_IRQHandler,
        TAMP_STAMP_IRQHandler,
        RTC_WKUP_IRQHandler,
        FLASH_IRQHandler,
        RCC_IRQHandler,
        EXTI0_IRQHandler,
        EXTI1_IRQHandler,
        EXTI2_IRQHandler,
        EXTI3_IRQHandler,
        EXTI4_IRQHandler,
        DMA1_Stream0_IRQHandler,
        DMA1_Stream1_IRQHandler,
        DMA1_Stream2_IRQHandler,
        DMA1_Stream3_IRQHandler,
        DMA1_Stream4_IRQHandler,
        DMA1_Stream5_IRQHandler,
        DMA1_Stream6_IRQHandler,
        ADC_IRQHandler,
        0,
        0,
        0,
        0,
        EXTI9_5_IRQHandler,
        TIM1_BRK_TIM9_IRQHandler,
        TIM1_UP_TIM10_IRQHandler,
        TIM1_TRG_COM_TIM11_IRQHandler,
        TIM1_CC_IRQHandler,
        TIM2_IRQHandler,
        TIM3_IRQHandler,
        TIM4_IRQHandler,
        I2C1_EV_IRQHandler,
        I2C1_ER_IRQHandler,
        I2C2_EV_IRQHandler,
        I2C2_ER_IRQHandler,
        SPI1_IRQHandler,
        SPI2_IRQHandler,
        USART1_IRQHandler,
        USART2_IRQHandler,
        0,
        EXTI15_10_IRQHandler,
        RTC_Alarm_IRQHandler,
        OTG_FS_WKUP_IRQHandler,
        0,
        0,
        0,
        0,
        DMA1_Stream7_IRQHandler,
        0,
        SDIO_IRQHandler,
        TIM5_IRQHandler,
        SPI3_IRQHandler,
        0,
        0,
        0,
        0,
        DMA2_Stream0_IRQHandler,
        DMA2_Stream1_IRQHandler,
        DMA2_Stream2_IRQHandler,
        DMA2_Stream3_IRQHandler,
        DMA2_Stream4_IRQHandler,
        0,
        0,
        0,
        0,
        0,
        0,
        OTG_FS_IRQHandler,
        DMA2_Stream5_IRQHandler,
        DMA2_Stream6_IRQHandler,
        DMA2_Stream7_IRQHandler,
        USART6_IRQHandler,
        I2C3_EV_IRQHandler,
        I2C3_ER_IRQHandler,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        FPU_IRQHandler,
        0,
        0,
        SPI4_IRQHandler
};