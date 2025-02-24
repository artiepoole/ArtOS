// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artypoole on 02/12/24.
//

#include <gtest/gtest.h>

#include "DenseBoolean.h"
#include "types.h"

// TODO: test attemots to write out of bounds
// TODO: test different types


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

TEST(DenseBooleanTest, late_init)
{
    constexpr size_t n_bits = sizeof(u32) * 8;
    const auto array_p = new DenseBoolean<u32>;
    auto my_array = *array_p;
    constexpr u32 data = -1; // all true
    my_array.init(data);
    for (size_t i = 0; i < n_bits; i++)
    {
        ASSERT_TRUE(my_array[i]) << "first occurred when i = " << i << '\n';;
    }
}

