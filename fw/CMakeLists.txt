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
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)

# Enable compile command to ease indexing
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Locate external sources
set(cmsis_core_standard_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-Core/Standard)
set(cmsis_core_device_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-Core/Device)
set(stm32_hal_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/HAL)

# Create project target
add_executable(${CMAKE_PROJECT_NAME})

# Additional options
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -Wno-missing-field-initializers
)

# Sane debugging paths
cmake_path(NORMAL_PATH CMAKE_CURRENT_SOURCE_DIR OUTPUT_VARIABLE OLD_PREFIX)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -ffile-prefix-map=${OLD_PREFIX}=.
)

# Add custom linker script
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
        -T "${CMAKE_CURRENT_SOURCE_DIR}/stm32f401xe.ld"
        #-Wl,--orphan-handling=warn
)

# Add include directories
file(GLOB core_inc_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "core/inc/*")
list(FILTER core_inc_subdirs EXCLUDE REGEX "[.]")

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${cmsis_core_standard_SOURCE_DIR}/CMSIS/Core/Include
        ${cmsis_core_device_SOURCE_DIR}/Include
        ${stm32_hal_SOURCE_DIR}/Inc
        core/inc
        ${core_inc_subdirs}
)

# Add project sources
file(GLOB_RECURSE core_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "core/src/*.c*")

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
        ${stm32_hal_SOURCE_DIR}/Src/stm32f4xx_ll_dma.c
        ${core_src}
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
        PRINTD_ENABLE=1
        $<$<CONFIG:Debug>:DEBUG>
)

# Add linked libraries/directories
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        ARM::V7EM-HARD-FPV4SP
        ARM::NoSys
        ARM::Nano
)
