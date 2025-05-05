cmake_minimum_required(VERSION 3.22)
option(ENABLE_SERIAL_LOGGING "Enables serial logging using LOG, WRITE, NEWLINE macros for use with Qemu/com1." OFF)
option(ENABLE_TERMINAL_LOGGING "Overrides serial logging. Enables terminal logging using LOG, WRITE, NEWLINE macros. Only if logging is enabled." OFF)
option(SIMD "Replace memcpy with SIMD version?" ON)
option(FORLAPTOP "Enable building for real hardware, disable for QEMU." OFF)

project(ArtOS)
ENABLE_LANGUAGE(ASM)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)


set(KERNEL_LIB "ArtOS_lib")
set(KERNEL_NAME ${CMAKE_PROJECT_NAME})
set(KERNEL_BIN ${KERNEL_NAME}.bin)
set(KERNEL_ISO ${KERNEL_NAME}.iso)

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


add_subdirectory(ArtOS_lib)
add_subdirectory(executable_src/bart)
add_subdirectory(pdclib)
add_subdirectory(executable_src/doom)
add_subdirectory(ArtOSTypes)
#add_subdirectory(helloworld)
## Building the sys binary
add_executable(${KERNEL_BIN}
        Specific/x86/boot.S
        Generic/main.cpp
        ${Generic}
        ${Specific}
        ${ArchitectureSpecific}
)


target_include_directories(
        ${KERNEL_BIN} PUBLIC#if SIMD
        #include "SIMD.h"
        #endif
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
        Generic/Utility
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
        Specific/x86/Memory
        Specific/x86/PCI
        Specific/x86/ProcessInterruptControllers
        Specific/x86/Timers
)

target_include_directories(
        ${KERNEL_BIN} PUBLIC
        ArtOS_lib/
        ArtOS_lib/keymaps
        ArtOS_lib/SIMD
)

set_target_properties(
        ${KERNEL_BIN} PROPERTIES
        LINKER_LANGUAGE CXX
        LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/art_kernel.ld -ffreestanding -O2 -nostdlib -Wl,-demangle"
)

add_custom_command(
        TARGET ${KERNEL_BIN} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${CMAKE_SOURCE_DIR}/bin/
)


target_compile_definitions(${KERNEL_BIN}
        PUBLIC
        ENABLE_SERIAL_LOGGING=$<BOOL:${ENABLE_SERIAL_LOGGING}>
        ENABLE_TERMINAL_LOGGING=$<BOOL:${ENABLE_TERMINAL_LOGGING}>
        SIMD=$<BOOL:${SIMD}>
        FORLAPTOP=$<BOOL:${FORLAPTOP}>
)


target_link_libraries(${KERNEL_BIN} PUBLIC pdclib doom ArtOSTypes)


add_executable(b.art executable_src/bart/bart.cpp)
target_link_libraries(b.art ArtOS_lib pdclib)
set_target_properties(b.art PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/art_executable.ld -ffreestanding -O2 -nostdlib -Wl,-demangle")

add_executable(hello.art executable_src/helloworld/helloworld.cpp)
target_link_libraries(hello.art ArtOS_lib)
set_target_properties(hello.art PROPERTIES LINKER_LANGUAGE CXX LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/art_executable.ld -ffreestanding -O2 -nostdlib -Wl,-demangle")


target_compile_definitions(pdclib PUBLIC
        SIMD=$<BOOL:${SIMD}>
)

### Generating the ISO file
# TODO: perhaps there is a better way to handle this in cmake
add_custom_target(${KERNEL_NAME}
        COMMAND ../cmake-iso.sh
        COMMENT "Generating the kernel bootable iso file"
        BYPRODUCTS ${KERNEL_ISO}
)
add_dependencies(${KERNEL_NAME} ${KERNEL_BIN} ${KERNEL_LIB} b.art hello.art)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/
)

#target_compile_options(hello.elf PRIVATE -O3 -DNDEBUG)
