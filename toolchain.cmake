set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i686)


set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(TOOLS $ENV{HOME}/opt/cross-compiler/)
set(CMAKE_ASM_COMPILER ${TOOLS}/bin/i686-elf-as)
set(CMAKE_C_COMPILER ${TOOLS}/bin/i686-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLS}/bin/i686-elf-g++)
set(CMAKE_VERBOSE_MAKEFILE 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "-ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti") ###
set(CMAKE_C_FLAGS "-ffreestanding -O2 -Wall -Wextra")



set(CRT_BEGIN /home/artypoole/opt/cross-compiler/lib/gcc/i686-elf/14.1.0/crtbegin.o)
set(CRT_END /home/artypoole/opt/cross-compiler/lib/gcc/i686-elf/14.1.0/crtend.o)

set(CMAKE_C_LINK_EXECUTABLE
        "<CMAKE_C_COMPILER>  <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> ${CRT_BEGIN} <OBJECTS> ${CRT_END} -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CRT_BEGIN} <OBJECTS> ${CRT_END} -o <TARGET> <LINK_LIBRARIES>")