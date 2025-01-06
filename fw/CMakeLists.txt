# File          : arm-none-eabi.cmake
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 28.12.2024

cmake_minimum_required(VERSION 3.20)

# Debug support
set(CMAKE_VERBOSE_MAKEFILE ON)

# Locate ARM GNU Toolchain configuration file
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/armv7em-hard-fpv4sp.cmake)

# Project configuration
project(fw C CXX ASM)

# Compiler configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)

# Enable compile command to ease indexing
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch sdk
include(cmake/sdk.cmake)

# Create project target
add_executable(${CMAKE_PROJECT_NAME})

# Add custom linker script
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
        -T "${CMAKE_CURRENT_SOURCE_DIR}/stm32f401xe.ld"
        -Wl,--orphan-handling=warn
)

# Add include directories
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${cmsis_core_standard_SOURCE_DIR}/CMSIS/Core/Include
        ${cmsis_core_device_SOURCE_DIR}/Include
        ${stm32_hal_SOURCE_DIR}/Inc
        core/inc
)

# Add project sources
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        ${cmsis_core_device_SOURCE_DIR}/Source/Templates/system_stm32f4xx.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_exti.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_gpio.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_pwr.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_rcc.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_rng.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_spi.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_tim.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_usart.c
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_utils.c
        core/src/startup_stm32f401xe.c
        core/src/main.c
)

# Add project macros
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        USE_FULL_LL_DRIVER
        HSE_VALUE=8000000
        HSE_STARTUP_TIMEOUT=100
        LSE_STARTUP_TIMEOUT=5000
        LSE_VALUE=32768
        EXTERNAL_CLOCK_VALUE=12288000
        HSI_VALUE=16000000
        LSI_VALUE=32000
        VDD_VALUE=3300
        PREFETCH_ENABLE=1
        INSTRUCTION_CACHE_ENABLE=1
        DATA_CACHE_ENABLE=1
        STM32F401xE
        $<$<CONFIG:Debug>:DEBUG>
)

# Add linked libraries/directories
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        ARM::V7EM-HARD-FPV4SP
        ARM::NoSys
        ARM::Nano
)
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE)