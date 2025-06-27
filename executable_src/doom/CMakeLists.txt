cmake_minimum_required(VERSION 3.22)

project(doom.art LANGUAGES C CXX)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack")
set(TARGET_BIN "doom.art")