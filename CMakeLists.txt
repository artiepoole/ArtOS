cmake_minimum_required(VERSION 3.22)


ENABLE_LANGUAGE(ASM)
option(ENABLE_LOGGING "Enables serial logging using LOG, WRITE, NEWLINE macros for use with Qemu/com1." OFF)

project(ArtOS)

set(KERNEL_BIN "ArtOS.bin")
set(KERNEL_ISO ${CMAKE_PROJECT_NAME}.iso)

### Find all source files
file(GLOB INCLUDE
        "include/*.h"
        "include/*.cpp"
        "include/*.s"
        "include/timers/*.h"
        "include/timers/*.cpp"
        "include/communication/*.h"
        "include/communication/*.cpp"
        "include/constants/*.h"
        "include/constants/*.cpp"
)

file(GLOB KERNEL
        "src/sys/*.h"
        "src/sys/*.cpp"
)


add_subdirectory(pdclib)

## Building the sys binary
add_executable(${KERNEL_BIN} src/boot.s src/main.cpp
        ${INCLUDE} ${KERNEL}

)
target_include_directories(
        ${KERNEL_BIN} PUBLIC
        include
        include/timers
        include/communication/
        include/constants
        src
        src/sys
        )

set_target_properties(${KERNEL_BIN} PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -ffreestanding -O2 -nostdlib -Wl,-demangle")

add_custom_command(TARGET ${KERNEL_BIN} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${CMAKE_SOURCE_DIR}/bin/)


target_link_libraries(${KERNEL_BIN} pdclib)
target_compile_definitions(${KERNEL_BIN}
        PUBLIC
        ENABLE_LOGGING=$<BOOL:${ENABLE_LOGGING}>
)


### Generating the ISO file
add_custom_target(${KERNEL_ISO}
        COMMAND ../cmake-iso.sh
        COMMENT "Generating the kernel bootable iso file"
        BYPRODUCTS ${KERNEL_ISO})
add_dependencies(ArtOS.iso ArtOS.bin)
add_custom_command(TARGET ${KERNEL_ISO} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/)



