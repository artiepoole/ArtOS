cmake_minimum_required(VERSION 3.22)


ENABLE_LANGUAGE(ASM)
option(ENABLE_SERIAL_LOGGING "Enables serial logging using LOG, WRITE, NEWLINE macros for use with Qemu/com1." OFF)
option(ENABLE_TERMINAL_LOGGING "Overrides serial logging. Enables terminal logging using LOG, WRITE, NEWLINE macros. Only if logging is enabled." OFF)
option(FORLAPTOP "Enable building for real hardware, disable for QEMU." OFF)

project(ArtOS)

set(KERNEL_BIN "ArtOS.bin")
set(KERNEL_ISO ${CMAKE_PROJECT_NAME}.iso)

### Find all source files
file(GLOB INCLUDE
        "include/*.h"
        "include/*.cpp"
        "include/*.S"
        "include/timers/*.h"
        "include/timers/*.cpp"
        "include/device_info/*.h"
        "include/device_info/*.cpp"
        "include/constants/*.h"
        "include/constants/*.cpp"
)

file(GLOB KERNEL
        "src/sys/*.h"
        "src/sys/*.cpp"
)


add_subdirectory(pdclib)
add_subdirectory(doom)
## Building the sys binary
add_executable(${KERNEL_BIN} src/boot.S src/main.cpp ${INCLUDE} ${KERNEL}
        src/sys/logging.h)

target_include_directories(
        ${KERNEL_BIN} PUBLIC
        include
        include/timers
        include/device_info/
        include/constants
        src
        src/sys
        )

set_target_properties(${KERNEL_BIN} PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -ffreestanding -O2 -nostdlib -Wl,-demangle")

add_custom_command(
        TARGET ${KERNEL_BIN} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${CMAKE_SOURCE_DIR}/bin/
)


target_compile_definitions(${KERNEL_BIN}
        PUBLIC
        ENABLE_SERIAL_LOGGING=$<BOOL:${ENABLE_SERIAL_LOGGING}>
        ENABLE_TERMINAL_LOGGING=$<BOOL:${ENABLE_TERMINAL_LOGGING}>
        FORLAPTOP=$<BOOL:${FORLAPTOP}>
)

target_link_libraries(${KERNEL_BIN} pdclib doom)


### Generating the ISO file
add_custom_target(${KERNEL_ISO}
        COMMAND ../cmake-iso.sh
        COMMENT "Generating the kernel bootable iso file"
        BYPRODUCTS ${KERNEL_ISO})
add_dependencies(ArtOS.iso ArtOS.bin)
add_custom_command(TARGET ${KERNEL_ISO} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/)


#file(GLOB DOOM_SRC
#        "doom/doomgeneric/*.h"
#        "doom/doomgeneric/*.c"
#)
#
#add_executable(doomexec doom/doomgeneric_artos.cpp ${DOOM_SRC} ${KERNEL} ${INCLUDE})
#target_include_directories(doomexec PUBLIC doomgeneric)
#target_link_libraries(doomexec pdclib)
#set_target_properties(doomexec PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -ffreestanding -O2 -nostdlib -Wl,-demangle")
#
