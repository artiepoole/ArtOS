cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(ArtOS)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(CMAKE_STATIC_LIBRARY_PREFIX "")


add_library(kernel STATIC kernel.c)
target_link_libraries(kernel ArtOS)