cmake_minimum_required(VERSION 3.1.3...3.27.7 FATAL_ERROR)

project(ArtOS_lib LANGUAGES C CXX)


file(GLOB_RECURSE LibFiles
        "*.cpp"
)

add_library(ArtOS_lib STATIC kernel.cpp ${LibFiles})

target_include_directories(ArtOS_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

