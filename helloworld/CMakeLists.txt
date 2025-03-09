cmake_minimum_required(VERSION 3.22)

# Project name and language
project(helloworld LANGUAGES CXX)
ENABLE_LANGUAGE(ASM)

# Set the cross-compiler

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CXX_FLAGS_DEBUG} -O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/helloworld/hello_link.ld")
set(TARGET_BIN "hello.elf")
# Add sources
set(SOURCES
        helloworld.cpp
)


# Add the executable
add_executable(${TARGET_BIN} ${SOURCES})

# Link against the libraries
target_link_libraries(${TARGET_BIN} kernel_lib pdclib)