cmake_minimum_required(VERSION 3.22)

project(doom LANGUAGES C CXX)

file(GLOB SRC
        "doomgeneric/*.h"
        "doomgeneric/*.c"
)

file(GLOB PDCLIB_H
        "../pdclib/include/*.h"
        "../pdclib/platform/example/include/pdclib/*.H"
)

add_subdirectory(../pdclib ../pdclib/cmake-build-release/libpdclib.a)
add_library(doom STATIC doomgeneric_artos.c ${SRC} ${PDCLIB_H})
target_include_directories(doom PRIVATE doomgeneric ../pdclib/include ../pdclib/platform/example/include/pdclib/)
target_link_libraries(pdclib)