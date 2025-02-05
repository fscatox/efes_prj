# File          : cmsis.cmake
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 05.02.2025

set(cmsis_core_standard_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-Core/Standard)
set(cmsis_core_device_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-Core/Device)

if (NOT (TARGET CMSIS::Core))
    add_library(CMSIS::Core INTERFACE IMPORTED)
    target_include_directories(CMSIS::Core INTERFACE
            ${cmsis_core_standard_SOURCE_DIR}/CMSIS/Core/Include
            ${cmsis_core_device_SOURCE_DIR}/Include
    )
    target_sources(CMSIS::Core INTERFACE
            ${cmsis_core_device_SOURCE_DIR}/Source/Templates/system_stm32f4xx.c
    )
endif ()