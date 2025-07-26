# File          : armv7em-hard-fpv4sp.cmake
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 28.12.2024
# Description   : toolchain configuration file for ARMv7E-M with
#                 FP and DSP extensions

# Add "cmake/" to search path of include() and find_package()
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Load generic toolchain configuration for arm-none-eabi target
include(arm-none-eabi)

# Toolchain configuration for armv7em-hard-fpv4sp
if (NOT (TARGET ARM::V7EM-HARD-FPV4SP))
    add_library(ARM::V7EM-HARD-FPV4SP INTERFACE IMPORTED)
    target_link_libraries(ARM::V7EM-HARD-FPV4SP INTERFACE ARM)

    set(ARCH_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
    target_compile_options(ARM::V7EM-HARD-FPV4SP INTERFACE "${ARCH_FLAGS}")
    target_link_options(ARM::V7EM-HARD-FPV4SP INTERFACE "${ARCH_FLAGS}")
endif ()
