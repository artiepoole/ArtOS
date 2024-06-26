cmake_minimum_required(VERSION 3.28)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i686)


set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(TOOLS $ENV{HOME}/opt/cross-compiler/)
set(CMAKE_ASM_COMPILER ${TOOLS}/bin/i686-elf-as)
set(CMAKE_C_COMPILER ${TOOLS}/bin/i686-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLS}/bin/i686-elf-g++)
#set(CMAKE_CXX_LINK_EXECUTABLE ${TOOLS}/bin/i686-elf-gcc)

set(CMAKE_VERBOSE_MAKEFILE 1)

ENABLE_LANGUAGE(ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "-ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti")
set(CMAKE_C_FLAGS "-ffreestanding -O2 -Wall -Wextra")

project(ArtOS)

set(KERNEL_BIN "ArtOS.bin")
set(KERNEL_ISO ${CMAKE_PROJECT_NAME}.iso)

### A simple lib to test C compilation and linking
#add_library(mylib mylib.c)

## Building the kernel binary
#add_executable(${KERNEL_BIN} boot.s kernel.cpp mylib)
add_executable(${KERNEL_BIN} boot.s kernel.cpp
        src/terminal.cpp
        src/serial.cpp
        src/string.cpp)
set_target_properties(${KERNEL_BIN} PROPERTIES LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -ffreestanding -O2 -nostdlib -lgcc")
add_custom_command(TARGET ${KERNEL_BIN} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${CMAKE_SOURCE_DIR}/bin/)

### Generating the ISO file
add_custom_target(${KERNEL_ISO}
        COMMAND ../cmake-iso.sh
        COMMENT "Generating the kernel bootable iso file"
        BYPRODUCTS ${KERNEL_ISO})
add_dependencies(ArtOS.iso ArtOS.bin)
add_custom_command(TARGET ${KERNEL_ISO} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/)

