# File          : arm-none-eabi.cmake
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 28.12.2024
# Description   : toolchain configuration file for aarch32 bare-metal (arm-none-eabi)
#                 (see: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Locate the ARM GNU Toolchain on the host
set(TOOLCHAIN_TRIPLE arm-none-eabi)

if (NOT DEFINED TOOLCHAIN_PATH)
    find_program(TOOLCHAIN_GCC_PATH "${TOOLCHAIN_TRIPLE}-gcc" REQUIRED)
    cmake_path(GET TOOLCHAIN_GCC_PATH PARENT_PATH TOOLCHAIN_BIN_PATH)
    cmake_path(GET TOOLCHAIN_BIN_PATH PARENT_PATH TOOLCHAIN_PATH)
    message(STATUS "TOOLCHAIN_PATH not set, found in PATH: " ${TOOLCHAIN_PATH})
endif ()

# Logical root of target environment, where to find headers and libraries
set(CMAKE_SYSROOT "${TOOLCHAIN_PATH}/${TOOLCHAIN_TRIPLE}")

# Define cross-compilation tools and utilities
set(TOOLCHAIN_PATH_PREFIX "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_TRIPLE}")

set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH_PREFIX}-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH_PREFIX}-g++")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

set(CMAKE_AR "${TOOLCHAIN_PATH_PREFIX}-ar")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PATH_PREFIX}-objcopy")
set(CMAKE_SIZE "${TOOLCHAIN_PATH_PREFIX}-size")

# Cannot link without custom flags and linker script
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Configure find_* commands
# While cross-compiling, includes, libraries, and packages are found in the
# target system prefixes; executables are found only on the host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Suffixes for cross-compiled executables
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")

# Common optimization flags
set(CMAKE_C_FLAGS_DEBUG "-Og -g3" CACHE STRING "")
set(CMAKE_ASM_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} CACHE STRING "")

set(CMAKE_C_FLAGS_RELEASE "-O3 -g0" CACHE STRING "")
set(CMAKE_ASM_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} CACHE STRING "")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g3" CACHE STRING "")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} CACHE STRING "")

set(CMAKE_C_FLAGS_MINSIZEREL "-Os -g0" CACHE STRING "")
set(CMAKE_ASM_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL} CACHE STRING "")
set(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL} CACHE STRING "")

# Cross compilation configuration
# Library targets to collect compiler/linker options for dependents

if (NOT (TARGET ARM))
    add_library(ARM INTERFACE IMPORTED)

    # Compilation stage

    # Do not use the PLT for GNU ifunc
    target_compile_options(ARM INTERFACE -fno-plt)
    # Place data and functions in their own sections
    target_compile_options(ARM INTERFACE -ffunction-sections -fdata-sections)
    # Warnings, debug support
    target_compile_options(ARM INTERFACE -Wall -Wextra -fstack-usage)

    # C++ dialect
    # Disable generation of information for use by the C++ runtime type identification features
    target_compile_options(ARM INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
    # Disable exception handling
    target_compile_options(ARM INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
    # Reduce code size in code that doesn't need to be thread-safe
    target_compile_options(ARM INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>)

    # Linking stage

    # Prevents linking  with shared libraries
    target_link_options(ARM INTERFACE -static)
    # Generate link map
    target_link_options(ARM INTERFACE -Wl,-Map=${CMAKE_PROJECT_NAME}.map)
    # Print size of all memory regions (as defined in the linker script)
    target_link_options(ARM INTERFACE -Wl,--print-memory-usage)
    # Garbage  collection of unused input sections
    target_link_options(ARM INTERFACE -Wl,--gc-sections)
endif ()

# C/C++ standard library alternatives
# (GCC specs: https://gcc.gnu.org/onlinedocs/gcc/Spec-Files.html)
# (ARM GNU toolchain specs in ${CMAKE_SYSROOT}/lib)

if(NOT (TARGET ARM::FreeStanding))
    add_library(ARM::FreeStanding INTERFACE IMPORTED)
    target_link_libraries(ARM::FreeStanding INTERFACE ARM)
    target_compile_options(ARM::FreeStanding INTERFACE -ffreestanding)
    target_link_options(ARM::FreeStanding INTERFACE -nostdlib)
endif()

if(NOT (TARGET ARM::NoSys))
    add_library(ARM::NoSys INTERFACE IMPORTED)
    target_link_libraries(ARM::NoSys INTERFACE ARM)
    target_link_options(ARM::NoSys INTERFACE --specs=nosys.specs)
endif()

if (NOT (TARGET ARM::Newlib))
    add_library(ARM::Newlib INTERFACE IMPORTED)
    target_link_libraries(ARM::Newlib INTERFACE ARM)

    # Link with C/C++ standard libraries
    set(STDLIB_C_LINK -Wl,--start-group -lc -lm -Wl,--end-group)
    set(STDLIB_CXX_LINK -Wl,--start-group -lstdc++ -lsupc++ -Wl,--end-group)
    target_link_options(ARM::Newlib INTERFACE
            "$<$<LINK_LANGUAGE:C>:${STDLIB_C_LINK}>"
            "$<$<LINK_LANGUAGE:CXX>:${STDLIB_CXX_LINK}>"
    )
endif ()

if(NOT (TARGET ARM::Nano))
    add_library(ARM::Nano INTERFACE IMPORTED)
    target_link_libraries(ARM::Nano INTERFACE ARM::Newlib)

    # Replace C/C++ standard libraries
    target_compile_options(ARM::Nano INTERFACE --specs=nano.specs)
    target_link_options(ARM::Nano INTERFACE --specs=nano.specs)
endif()

if(NOT (TARGET ARM::Nano::FloatPrint))
    add_library(ARM::Nano::FloatPrint INTERFACE IMPORTED)
    target_link_libraries(ARM::Nano::FloatPrint INTERFACE ARM::Nano)
    target_link_options(ARM::Nano::FloatPrint INTERFACE -Wl,--undefined,_printf_float)
endif()

if(NOT (TARGET ARM::Nano::FloatScan))
    add_library(ARM::Nano::FloatScan INTERFACE IMPORTED)
    target_link_libraries(ARM::Nano::FloatScan INTERFACE ARM::Nano)
    target_link_options(ARM::Nano::FloatScan INTERFACE -Wl,--undefined,_scanf_float)
endif()
