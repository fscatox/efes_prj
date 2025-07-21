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

# Create project target
add_executable(${CMAKE_PROJECT_NAME})

# Generate program image
set(BIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${BIN_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating ${BIN_FILE}")

# Additional options
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -save-temps
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

# External libraries
include(cmsis)
include(stm32_hal)
add_subdirectory(lib/compile-time-regular-expressions)

# Add core include directories
file(GLOB core_inc_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "core/inc/*")
list(FILTER core_inc_subdirs EXCLUDE REGEX "[.]")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        core/inc
        ${core_inc_subdirs}
)

# Add core project sources
file(GLOB_RECURSE core_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "core/src/*.c*")
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
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
        PRINT_ENABLE=1
        $<$<CONFIG:Debug>:DEBUG>
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        ARM::V7EM-HARD-FPV4SP
        ARM::NoSys
        ARM::Nano
        CMSIS::Core
        STM32::LL
        ctre
)
