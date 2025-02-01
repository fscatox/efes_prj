/**
 * @file     startup_stm32f401xe.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     05.01.2025
 * @brief    CMSIS-Core(M) Device Startup File
 */

#include <cstddef>
#include <cstdlib>
#include <debug.h>
#include <stm32f4xx.h>

/* Symbols from linker script */
extern unsigned char __stack_base[];

extern unsigned char __data_vstart[];
extern unsigned char __data_lstart[];
extern unsigned char __data_size[];

/* Prevent name mangling for these symbols */
extern "C" {

/* C/C++ runtime entry point */
[[noreturn]] void _start(void);

[[noreturn]] void Reset_Handler() {

  /* CMSIS subsystems initialization (FPU, ...) */
  SystemInit();

  /* Initialize data section into SRAM
   * (bss section is initialized by the runtime) */
  __builtin_memcpy(__data_vstart, __data_lstart,
                   reinterpret_cast<size_t>(__data_size));

  /* C/C++ entry point */
  _start();
}

}

/* Default handler behavior
 * Forward exception number to exit(), which attempts soft reset */
extern "C" void Default_Handler() {
  auto isr_number = __get_IPSR();
  PRINTE("Uncaught IRQ: Exception number = %d", isr_number);
  exit(-2);
}

/* System and Fault Handlers */
[[gnu::alias("Default_Handler"), gnu::weak]] void NMI_Handler();
[[gnu::alias("Default_Handler"), gnu::weak]] void HardFault_Handler();
[[gnu::alias("Default_Handler"), gnu::weak]] void MemManage_Handler();
[[gnu::alias("Default_Handler"), gnu::weak]] void BusFault_Handler();
[[gnu::alias("Default_Handler"), gnu::weak]] void UsageFault_Handler();
[[gnu::alias("Default_Handler"), gnu::weak]] void SVC_Handler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DebugMon_Handler();
[[gnu::alias("Default_Handler"), gnu::weak]] void PendSV_Handler();
[[gnu::alias("Default_Handler"), gnu::weak]] void SysTick_Handler();

/* IRQs handlers */
[[gnu::alias("Default_Handler"), gnu::weak]] void WWDG_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void PVD_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TAMP_STAMP_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void RTC_WKUP_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void FLASH_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void RCC_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void EXTI0_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void EXTI1_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void EXTI2_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void EXTI3_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void EXTI4_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA1_Stream0_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA1_Stream1_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA1_Stream2_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA1_Stream3_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA1_Stream4_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA1_Stream5_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA1_Stream6_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void ADC_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void EXTI9_5_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TIM1_BRK_TIM9_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TIM1_UP_TIM10_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TIM1_TRG_COM_TIM11_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TIM1_CC_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TIM2_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TIM3_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TIM4_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void I2C1_EV_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void I2C1_ER_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void I2C2_EV_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void I2C2_ER_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void SPI1_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void SPI2_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void USART1_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void USART2_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void EXTI15_10_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void RTC_Alarm_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void OTG_FS_WKUP_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA1_Stream7_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void SDIO_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void TIM5_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void SPI3_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA2_Stream0_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA2_Stream1_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA2_Stream2_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA2_Stream3_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA2_Stream4_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void OTG_FS_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA2_Stream5_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA2_Stream6_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void DMA2_Stream7_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void USART6_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void I2C3_EV_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void I2C3_ER_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void FPU_IRQHandler();
[[gnu::alias("Default_Handler"), gnu::weak]] void SPI4_IRQHandler();

using Vector = void (*)();

static const Vector vtable[] [[gnu::used, gnu::section(".isr_vector")]] {
    reinterpret_cast<Vector>(__stack_base),
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    SVC_Handler,
    DebugMon_Handler,
    nullptr,
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
    nullptr,
    nullptr,
    nullptr,
    nullptr,
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
    nullptr,
    EXTI15_10_IRQHandler,
    RTC_Alarm_IRQHandler,
    OTG_FS_WKUP_IRQHandler,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    DMA1_Stream7_IRQHandler,
    nullptr,
    SDIO_IRQHandler,
    TIM5_IRQHandler,
    SPI3_IRQHandler,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    DMA2_Stream0_IRQHandler,
    DMA2_Stream1_IRQHandler,
    DMA2_Stream2_IRQHandler,
    DMA2_Stream3_IRQHandler,
    DMA2_Stream4_IRQHandler,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    OTG_FS_IRQHandler,
    DMA2_Stream5_IRQHandler,
    DMA2_Stream6_IRQHandler,
    DMA2_Stream7_IRQHandler,
    USART6_IRQHandler,
    I2C3_EV_IRQHandler,
    I2C3_ER_IRQHandler,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    FPU_IRQHandler,
    nullptr,
    nullptr,
    SPI4_IRQHandler
};
