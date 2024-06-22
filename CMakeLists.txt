cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(ArtOS)

add_subdirectory(./src)
add_subdirectory(./doc)
add_subdirectory(./tests)

add_library(kernel STATIC kernel.c)
target_link_libraries(kernel ArtOS)