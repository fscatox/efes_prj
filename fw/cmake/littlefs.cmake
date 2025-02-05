# File          : littlefs.cmake
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 05.02.2025

include(ExternalProject)

ExternalProject_Add(LittleFs_
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/littlefs
        CONFIGURE_COMMAND ""
        BUILD_COMMAND cd <SOURCE_DIR> && make BUILDDIR=<BINARY_DIR> CC=${CMAKE_C_COMPILER} AR=${CMAKE_AR}
        INSTALL_COMMAND ""
)

if (NOT (TARGET LittleFs))
    add_library(LittleFs INTERFACE IMPORTED)
    ExternalProject_Get_Property(LittleFs_ SOURCE_DIR BINARY_DIR)
    target_include_directories(LittleFs INTERFACE ${SOURCE_DIR})
    target_link_directories(LittleFs INTERFACE ${BINARY_DIR})
    target_link_libraries(LittleFs INTERFACE lfs)
endif ()
