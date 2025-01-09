cmake_minimum_required(VERSION 3.22)

# Project name and language
project(HelloWorld_ArtOS LANGUAGES CXX)
ENABLE_LANGUAGE(ASM)

# Set the cross-compiler
set(CMAKE_CXX_COMPILER i686-elf-g++)
set(CMAKE_CXX_FLAGS "-ffreestanding -nostdlib")
set(CMAKE_EXE_LINKER_FLAGS "-T hello_link.ld")

# Add sources
set(SOURCES
        HelloWorld_ArtOS.cpp
)


# Add the executable
add_executable(HelloWorld_ArtOS ${SOURCES})

# Link against the libraries
target_link_libraries(HelloWorld_ArtOS kernel_lib pdclib)