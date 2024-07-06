#include <gtest/gtest.h>

namespace artosstd {
    #include "../include/string.h"
}

// Demonstrate some basic assertions.
TEST(StringTest, StrLen) {
    int len = artosstd::strlen("test");

    EXPECT_EQ(len, 4);
}