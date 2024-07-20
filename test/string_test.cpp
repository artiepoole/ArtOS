#include <gtest/gtest.h>

#include "../include/mystring.h"


// Demonstrate some basic assertions.
TEST(StringTest, StrLen)
{
    const size_t len = mystrlen("test");

    EXPECT_EQ(len, 5);
}

TEST(StringTest, string_from_int_pos)
{
    char outstr[32];
    string_from_int(1234, outstr);
    const int len = mystrlen(outstr);
    char trimmed[len+1];
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
    string_from_int(-1234, outstr);
    const int len =  mystrlen(outstr);
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
    string_from_int(0, outstr);
    const int len = mystrlen(outstr);
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
    hex_from_int(in, outstr, 1);
    const int len = mystrlen(outstr);
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
    hex_from_int(in, outstr, 1);
    const int len = mystrlen(outstr);
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
    hex_from_int(in, outstr, 4);
    const int len = mystrlen(outstr);
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
    hex_from_int(in, outstr, 1);
    const int len = mystrlen(outstr);
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
    const auto out = digit_as_char(in);
    ASSERT_EQ(out, '1');
}


TEST(StringTest, log2_pos)
{
    auto out = log2(0);
    ASSERT_EQ(out, 0);
    out = log2(1);
    ASSERT_EQ(out, 0);
    out = log2(2);
    ASSERT_EQ(out, 1);
    out = log2(4);
    ASSERT_EQ(out, 2);
    out = log2(128);
    ASSERT_EQ(out, 7);
    out = log2(145);
    ASSERT_EQ(out, 7);
}

TEST(StringTest, log2_neg)
{
    const auto out = log2(-5);
    ASSERT_EQ(out, 2);
}
