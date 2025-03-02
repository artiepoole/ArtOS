set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i686)


set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_VERBOSE_MAKEFILE 1)

set(CMAKE_ASM_FLAGS "-m32 -no-pie")
set(CMAKE_LINK_FLAGS "-m32 -no-pie")
set(CMAKE_CXX_FLAGS "-ffreestanding -Wall -Wextra  -fno-exceptions -fno-rtti -m32 -no-pie") # -Werror
set(CMAKE_C_FLAGS "-ffreestanding -Wall -Wextra -m32 -no-pie")
set(CMAKE_POSITION_INDEPENDENT_CODE 1)
#
set(CRT_BEGIN ${CMAKE_SOURCE_DIR}/external_resources/compilerfiles/crtbegin.o)
set(CRT_END ${CMAKE_SOURCE_DIR}/external_resources/compilerfiles/crtend.o)


set(CMAKE_C_LINK_EXECUTABLE
        "<CMAKE_C_COMPILER>  <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> ${CRT_BEGIN} <OBJECTS> ${CRT_END} -o <TARGET> <LINK_LIBRARIES> -no-pie ")

set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CRT_BEGIN} <OBJECTS> ${CRT_END} -o <TARGET> <LINK_LIBRARIES> -lgcc -no-pie")

set(CMAKE_C_FLAGS_RELEASE "-O3 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")