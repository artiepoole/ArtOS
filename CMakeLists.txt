cmake_minimum_required(VERSION 3.22)


ENABLE_LANGUAGE(ASM)


project(ArtOS)

set(KERNEL_BIN "ArtOS.bin")
set(KERNEL_ISO ${CMAKE_PROJECT_NAME}.iso)

### Find all source files
file(GLOB INCLUDE
        "include/*.h"
        "include/*.cpp"
        "include/*.s"
)

file(GLOB KERNEL
        "src/kernel/*.cpp"
        "src/kernel/*.h"
)


add_subdirectory(pdclib)

## Building the kernel binary
add_executable(${KERNEL_BIN} src/boot.s src/main.cpp
        ${INCLUDE} ${KERNEL}
)
set_target_properties(${KERNEL_BIN} PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -ffreestanding -O2 -nostdlib -lgcc -Wl,-demangle")
add_custom_command(TARGET ${KERNEL_BIN} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${CMAKE_SOURCE_DIR}/bin/)
target_include_directories(${KERNEL_BIN} PUBLIC include src src/kernel pdclib/
        pdclib/include
        pdclib/include/pdclib

        pdclib/platform/example/include

        pdclib/platform/example/functions

        pdclib/platform/example/include

        pdclib/platform/example/include/pdclib

        pdclib/platform/example/functions/_PDCLIB/
        pdclib/platform/example/functions/signal
        pdclib/platform/example/functions/stdio
        pdclib/platform/example/functions/stdlib
        pdclib/platform/example/functions/threads
        pdclib/platform/example/functions/time

        pdclib/functions/_dlmalloc/
        pdclib/functions/_dtoa
        pdclib/functions/_PDCLIB
        pdclib/functions/_tzcode
        pdclib/functions/ctype
        pdclib/functions/inttypes
        pdclib/functions/locale
        pdclib/functions/stdio
        pdclib/functions/stdlib
        pdclib/functions/string
        pdclib/functions/time

        ../src
        ../src/kernel
        ../include

)
target_link_libraries(pdclib)


### Generating the ISO file
add_custom_target(${KERNEL_ISO}
        COMMAND ../cmake-iso.sh
        COMMENT "Generating the kernel bootable iso file"
        BYPRODUCTS ${KERNEL_ISO})
add_dependencies(ArtOS.iso ArtOS.bin)
add_custom_command(TARGET ${KERNEL_ISO} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/)



