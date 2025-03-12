#!/usr/bin/env sh
set -e
cmake -S ./ -B cmake-build-artos -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake -DENABLE_SERIAL_LOGGING:BOOL=ON -DENABLE_TERMINAL_LOGGING:BOOL=OFF
cmake --build cmake-build-artos -t ArtOS