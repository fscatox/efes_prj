# File          : sdk.cmake
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 02.01.2025
# Description   : Fetch software development kit

include(FetchContent)

FetchContent_Declare(
        cmsis_core_standard
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/CMSIS/core
        GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
        GIT_TAG 5.9.0
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        cmsis_core_device
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/CMSIS/device
        GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis-device-f4.git
        GIT_TAG v2.6.10
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        stm32_hal
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdk/HAL
        GIT_REPOSITORY https://github.com/STMicroelectronics/stm32f4xx-hal-driver.git
        GIT_TAG v1.8.3
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(cmsis_core_standard cmsis_core_device)
FetchContent_MakeAvailable(stm32_hal)