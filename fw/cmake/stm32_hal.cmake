# File          : stm32_hal.cmake
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 05.02.2025

set(stm32_hal_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/HAL)

if (NOT (TARGET STM32::LL))
    add_library(STM32::LL INTERFACE IMPORTED)
    target_compile_options(STM32::LL INTERFACE -Wno-unused-parameter)
    target_include_directories(STM32::LL INTERFACE
            ${stm32_hal_SOURCE_DIR}/Inc
    )
    target_sources(STM32::LL INTERFACE
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_adc.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_crc.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_dac.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_dma.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_exti.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_gpio.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_i2c.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_lptim.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_pwr.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_rcc.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_rng.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_rtc.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_spi.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_tim.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_usart.c
            ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_utils.c
    )
endif ()
