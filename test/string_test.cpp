#include <gtest/gtest.h>

namespace artosstd
{
    // This has clashes with the stdlib from the host machine. We want to include this from a namespace but
    // that doesn't work if you use the .h file. This is clearly grungy but I don't really have a better
    // option, without putting our whole stdlib in a namespace.
#include "../include/string.cpp"
}

// Demonstrate some basic assertions.
TEST(StringTest, StrLen)
{
    const size_t len = artosstd::strlen("test");

    EXPECT_EQ(len, 4);
}

TEST(StringTest, string_from_int_pos)
{
    char outstr[32];
    const int len = artosstd::string_from_int(1234, outstr);
    char trimmed[len];
    for (size_t i = 0; i < len; i++)
    {
        trimmed[i] = outstr[i];
    }

    EXPECT_EQ(len, 5);
    constexpr char expected[5] = "1234";
    EXPECT_STREQ(trimmed, expected);
}

TEST(StringTest, string_from_int_neg)
{
    char outstr[32];
    const int len = artosstd::string_from_int(-1234, outstr);
    char trimmed[len];
    for (size_t i = 0; i < len; i++)
    {
        trimmed[i] = outstr[i];
    }

    EXPECT_EQ(len, 6);
    constexpr char expected[6] = "-1234";
    ASSERT_STREQ(trimmed, expected);
}

TEST(StringTest, string_from_int_zero)
{
    char outstr[32];
    const int len = artosstd::string_from_int(0, outstr);
    char trimmed[len];
    for (size_t i = 0; i < len; i++)
    {
        trimmed[i] = outstr[i];
    }

    EXPECT_EQ(len, 2);
    constexpr char expected[6] = "0";
    ASSERT_STREQ(trimmed, expected);
}

TEST(StringTest, string_from_hex_pos)
{
    char outstr[32];
    unsigned char in = 0xAE;
    const int len = artosstd::hex_from_int(in, outstr, 1);
    char trimmed[len];
    for (size_t i = 0; i < len; i++)
    {
        trimmed[i] = outstr[i];
    }

    EXPECT_EQ(len, 3);
    constexpr char expected[3] = "AE";
    EXPECT_STREQ(trimmed, expected);
}

TEST(StringTest, string_from_hex_neg)
{
    char outstr[32];
    signed char in = -33;
    const int len = artosstd::hex_from_int(in, outstr, 1);
    char trimmed[len];
    for (size_t i = 0; i < len; i++)
    {
        trimmed[i] = outstr[i];
    }

    EXPECT_EQ(len, 4);
    constexpr char expected[4] = "-21";
    ASSERT_STREQ(trimmed, expected);
}

TEST(StringTest, string_from_hex_neg_hexin)
{
    char outstr[32];
    const int in = -0xFE;
    const int len = artosstd::hex_from_int(in, outstr, 4);
    char trimmed[len];
    for (size_t i = 0; i < len; i++)
    {
        trimmed[i] = outstr[i];
    }

    EXPECT_EQ(len, 10);
    constexpr char expected[10] = "-000000FE";
    ASSERT_STREQ(trimmed, expected);
}

TEST(StringTest, string_from_hex_zero)
{
    char outstr[32];
    unsigned char in = 0x00;
    const int len = artosstd::hex_from_int(in, outstr, 1);
    char trimmed[len];
    for (size_t i = 0; i < len; i++)
    {
        trimmed[i] = outstr[i];
    }

    EXPECT_EQ(len, 3);
    constexpr char expected[3] = "00";
    ASSERT_STREQ(trimmed, expected);
}


TEST(StringTest, digit_as_char)
{
    const unsigned char in = 1;
    const auto out = artosstd::digit_as_char(in);
    ASSERT_EQ(out, '1');
}


TEST(StringTest, log2)
{
    auto out = artosstd::log2(0);
    ASSERT_EQ(out, 0);
    out = artosstd::log2(1);
    ASSERT_EQ(out, 0);
    out = artosstd::log2(2);
    ASSERT_EQ(out, 1);
    out = artosstd::log2(4);
    ASSERT_EQ(out, 2);
    out = artosstd::log2(128);
    ASSERT_EQ(out, 7);
    out = artosstd::log2(145);
    ASSERT_EQ(out, 7);

}