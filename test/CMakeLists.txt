cmake_minimum_required(VERSION 3.22)

set(TOOLS /usr)
set(CMAKE_C_COMPILER ${TOOLS}/bin/gcc)
set(CMAKE_CXX_COMPILER ${TOOLS}/bin/g++)
set(CMAKE_VERBOSE_MAKEFILE 1)

project(ArtOS_Test)

# GoogleTest requires at least C++14, Concept requires 20+
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()



add_executable(
        string_test
        string_test.cpp
        ../include/mystring.cpp
        ../include/mystring.h

)

target_link_libraries(
        string_test
        GTest::gtest_main
)
target_include_directories(string_test PRIVATE ../include ../include/constants)

include(GoogleTest)
gtest_discover_tests(string_test)