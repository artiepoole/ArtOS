cmake_minimum_required(VERSION 3.28)



ENABLE_LANGUAGE(ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "-ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti") ###
set(CMAKE_C_FLAGS "-ffreestanding -O2 -Wall -Wextra")

project(ArtOS)

set(KERNEL_BIN "ArtOS.bin")
set(KERNEL_ISO ${CMAKE_PROJECT_NAME}.iso)
### A simple lib to test C compilation and linking
#add_library(mylib mylib.c)
file(GLOB INCLUDE
        "include/*.h"
        "include/*.cpp"
        "include/*.s"
)
## Building the kernel binary
#add_executable(${KERNEL_BIN} boot.s kernel.cpp mylib)




add_executable(${KERNEL_BIN} src/boot.s src/kernel.cpp
        ${INCLUDE}
)
set_target_properties(${KERNEL_BIN} PROPERTIES LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -ffreestanding -O2 -nostdlib -lgcc")
add_custom_command(TARGET ${KERNEL_BIN} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${CMAKE_SOURCE_DIR}/bin/)
target_include_directories(${KERNEL_BIN} PUBLIC include)

### Generating the ISO file
add_custom_target(${KERNEL_ISO}
        COMMAND ../cmake-iso.sh
        COMMENT "Generating the kernel bootable iso file"
        BYPRODUCTS ${KERNEL_ISO})
add_dependencies(ArtOS.iso ArtOS.bin)
add_custom_command(TARGET ${KERNEL_ISO} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_ISO} ${CMAKE_SOURCE_DIR}/bin/)

### Todo: Require the making of the iso to not fail.