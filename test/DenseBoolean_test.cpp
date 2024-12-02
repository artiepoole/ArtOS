//
// Created by artypoole on 02/12/24.
//

#include <gtest/gtest.h>

#include "DenseBoolean.h"
#include "types.h"


TEST(DenseBooleanTest, get)
{
    constexpr u16 val = 0b1010101010101010;
    DenseBoolean denseBoolean(val);
    ASSERT_EQ(denseBoolean.data(), val);
}

TEST(DenseBooleanTest, set_true)
{
    constexpr u16 val = 0; // all bits false
    DenseBoolean denseBoolean(val);
    denseBoolean.set_bit(3, true);
    ASSERT_TRUE(denseBoolean[3]);
    ASSERT_FALSE(denseBoolean[2]);
}

TEST(DenseBooleanTest, set_false)
{
    constexpr u16 val = ~0; // all bits true
    DenseBoolean denseBoolean(val);
    denseBoolean.set_bit(3, false);
    ASSERT_FALSE(denseBoolean[3]);
    ASSERT_TRUE(denseBoolean[2]);
}

