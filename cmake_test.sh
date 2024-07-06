#!/usr/bin/env sh

cmake -S test -B cmake-build-test && cmake --build cmake-build-test && (cd cmake-build-test && ctest)