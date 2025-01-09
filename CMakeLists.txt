cmake_minimum_required(VERSION 3.22)

option(ENABLE_SERIAL_LOGGING "Enables serial logging using LOG, WRITE, NEWLINE macros for use with Qemu/com1." OFF)
option(ENABLE_TERMINAL_LOGGING "Overrides serial logging. Enables terminal logging using LOG, WRITE, NEWLINE macros. Only if logging is enabled." OFF)
option(FORLAPTOP "Enable building for real hardware, disable for QEMU." OFF)

project(ArtOS)
ENABLE_LANGUAGE(ASM)

set(KERNEL_BIN "ArtOS.bin")
set(KERNEL_ISO ${CMAKE_PROJECT_NAME}.iso)


### Find all source files
file(GLOB_RECURSE Specific
        "Specific/Devices/*.cpp"
        "Specific/Devices/*.h"
        "Specific/multiboot2/*.cpp"
        "Specific/multiboot2/*.h"
)

file(GLOB_RECURSE ArchitectureSpecific
        "Specific/*.cpp"
        "Specific/x86/*.cpp"
        "Specific/x86/*.h"
        "Specific/x86/*.c"
        "Specific/x86/*.S"
)

file(GLOB_RECURSE Generic
        "Generic/*.h"
        "Generic/*.cpp"
        "Generic/*.S"
)


add_subdirectory(pdclib)
add_subdirectory(doom)
add_subdirectory(ArtOSTypes)
## Building the sys binary
add_executable(${KERNEL_BIN} Specific/x86/boot.S Generic/main.cpp ${Generic} ${Specific} ${ArchitectureSpecific})

target_include_directories(
        ${KERNEL_BIN} PUBLIC
        Generic/
        Generic/sys
        Generic/sys/Communication
        Generic/sys/Constants
        Generic/sys/DataTypes
        Generic/sys/Events
        Generic/sys/FileSystem
        Generic/sys/ComponentTypes
        Generic/sys/Logging
        Generic/sys/Memory
        Generic/sys/Storage
        Generic/sys/Terminal
        Generic/sys/Scheduling
)


target_include_directories(
        ${KERNEL_BIN} PUBLIC

        Specific/multiboot2
        Specific/Devices/PCI
        Specific/Devices/Storage
        Specific/Devices/Storage/IDEDrive

        Specific/x86
        Specific/x86/BIOS
        Specific/x86/CPU
        Specific/x86/Display
        Specific/x86/Drivers/Storage
        Specific/x86/IO
        Specific/x86/IO/keymaps
        Specific/x86/PCI
        Specific/x86/ProcessInterruptControllers
        Specific/x86/Timers
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

target_link_libraries(${KERNEL_BIN} pdclib doom ArtOSTypes)


### Generating the ISO file
add_custom_target(${KERNEL_ISO}
        COMMAND ../cmake-iso.sh
        COMMENT "Generating the kernel bootable iso file"
        BYPRODUCTS ${KERNEL_ISO})
add_dependencies(ArtOS.iso ArtOS.bin)
add_custom_command(TARGET ${KERNEL_ISO} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/
)


