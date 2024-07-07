#include <gtest/gtest.h>

namespace artosstd {
    // This has clashes with the stdlib from the host machine. We want to include this from a namespace but
    // that doesn't work if you use the .h file. This is clearly grungy but I don't really have a better
    // option, without putting our whole stdlib in a namespace.
    #include "../include/string.cpp"
}

// Demonstrate some basic assertions.
TEST(StringTest, StrLen) {
    int len = artosstd::strlen("test");

    EXPECT_EQ(len, 4);
}