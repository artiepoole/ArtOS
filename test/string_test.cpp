#include <gtest/gtest.h>

namespace artosstd {
    // Not sure what's going on but I can't seem to include this by its header file. This worked before this commit:
    // https://github.com/stupoole/ArtOS/commit/11cb5bc220b64ac5915a5bf9120a50c374a906b2
    #include "../include/string.cpp"
}

// Demonstrate some basic assertions.
TEST(StringTest, StrLen) {
    int len = artosstd::strlen("test");

    EXPECT_EQ(len, 4);
}