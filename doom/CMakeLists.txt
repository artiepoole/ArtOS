cmake_minimum_required(VERSION 3.22)

project(doom LANGUAGES C CXX)

file(GLOB SRC
        "doomgeneric/*.h"
        "doomgeneric/*.c"
)

add_library(doom STATIC doomgeneric_artos.c ${SRC}
)
target_include_directories(doom PRIVATE doomgeneric/)
