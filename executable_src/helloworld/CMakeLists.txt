cmake_minimum_required(VERSION 3.22)

project(hello LANGUAGES CXX)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack")
set(TARGET_BIN "hello.art")