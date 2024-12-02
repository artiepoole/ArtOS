cmake_minimum_required(VERSION 3.22)

project(ArtOSTypes LANGUAGES C CXX)

file(GLOB SOURCES
        "types.h"
        "Comparisons/*.cpp"
        "Comparisons/Devices/*.h"
        "DenseBoolean/*.cpp"
        "DenseBoolean/*.h"
        "Lists/*.cpp"
        "Lists/*.h"
)

add_library(ArtOSTypes STATIC ${SOURCES})

target_include_directories(ArtOSTypes PUBLIC
        ./
        Comparisons/
        DenseBoolean/
        Lists/
)
