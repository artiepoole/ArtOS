#!/usr/bin/env sh
# This is used by github to run the tests for Continuous Integration support.
cmake -S test -B cmake-build-test && cmake --build cmake-build-test && (cd cmake-build-test && ctest)