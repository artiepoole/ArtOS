cmake_minimum_required(VERSION 3.22)

project(doom LANGUAGES C CXX)

file(GLOB SRC
        "doomgeneric/*.h"
        "doomgeneric/*.c"
)

add_library(doom STATIC doomgeneric_artos.cpp ${SRC})
target_compile_definitions(doom
        PUBLIC
        FORLAPTOP=$<BOOL:${FORLAPTOP}>
)
target_include_directories(doom PRIVATE doomgeneric)
target_include_directories(doom PUBLIC .)
target_link_libraries(doom pdclib)
