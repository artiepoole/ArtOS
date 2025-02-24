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

#include "DenseBooleanArray.h"
#include "types.h"

// TODO: make more unit like
// TODO: test attemots to write out of bounds
// TODO: test different types and catching of DB/DBA type mismatch

using IntLikeTypes = ::testing::Types<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t>;

template <typename T>
class DenseBooleanArrayTest : public ::testing::Test
{
};

TYPED_TEST_SUITE(DenseBooleanArrayTest, IntLikeTypes);

TYPED_TEST(DenseBooleanArrayTest, DefaultFalseBasics)
{
    constexpr size_t n_bits = 64;
    const size_t bits_per_element = 8 * sizeof(TypeParam);
    DenseBooleanArray<TypeParam> dba(n_bits, false);
    dba.set_bit(0, true);
    dba.set_bit(33, true);
    dba.set_bit(34, true);
    ASSERT_TRUE(dba[0]);
    ASSERT_FALSE(dba[1]);
    ASSERT_TRUE(dba[33]);
    ASSERT_FALSE(dba[32]);
    ASSERT_EQ(dba.get_array_len(), (n_bits +bits_per_element-1) /(bits_per_element));
    ASSERT_EQ(dba.get_next_false(), 1);
    const size_t next_true = dba.get_next_true(2);
    ASSERT_EQ(next_true, 33);
    ASSERT_TRUE(dba[next_true]);
    ASSERT_EQ(dba.get_next_true(next_true+1), 34);
    ASSERT_EQ(dba.get_next_true(next_true+2), -1);
}

// TODO: make more unit like
// TODO: should test all bits in an item not just a subset?
TYPED_TEST(DenseBooleanArrayTest, default_true_basics)
{
    constexpr size_t n_bits = 125;
    const size_t bits_per_element = 8 * sizeof(TypeParam);
    DenseBooleanArray<TypeParam> dba(n_bits, true);
    dba.set_bit(0, false);
    dba.set_bit(1, false);
    dba.set_bit(64, false);
    dba.set_bit(65, false);
    ASSERT_TRUE(dba[2]);
    ASSERT_FALSE(dba[1]);
    ASSERT_EQ(dba.get_array_len(), (n_bits +bits_per_element-1) /(bits_per_element));
    ASSERT_EQ(dba.get_next_true(), 2);
    ASSERT_TRUE(dba[32]);
    size_t next_false = dba.get_next_false(2);
    ASSERT_EQ(next_false, 64);
    ASSERT_EQ(dba.get_next_false(next_false+1), 65);
}

// TODO: this should test finding a good range and handling of: no suitable ranges, start finds an area but doesn't have space for the whole range etc.
TYPED_TEST(DenseBooleanArrayTest, get_chunk_true)
{
    DenseBooleanArray<TypeParam> dba(125, true);
    dba.set_bit(0, false);
    dba.set_bit(1, false);
    dba.set_bit(3, false);
    dba.set_bit(4, false);
    size_t start = dba.get_next_trues(0, 5);
    ASSERT_EQ(start, 5);
    ASSERT_EQ(dba.get_next_trues(6, 128), -1);
}

TYPED_TEST(DenseBooleanArrayTest, get_chunk_false)
{
    DenseBooleanArray<TypeParam> dba(125, false);
    dba.set_bit(0, true);
    dba.set_bit(1, true);
    dba.set_bit(3, true);
    dba.set_bit(4, true);
    size_t start = dba.get_next_falses(0, 5);
    ASSERT_EQ(start, 5);
    ASSERT_EQ(dba.get_next_falses(6, 128), -1);
}


TYPED_TEST(DenseBooleanArrayTest, late_inits)
{
    constexpr size_t n_DBs = 5;
    constexpr size_t n_bits = sizeof(TypeParam) * 8 * n_DBs;
    TypeParam array_true[n_DBs];
    TypeParam array_false[n_DBs];
    const auto dba_true = new DenseBooleanArray<TypeParam>;
    const auto dba_false = new DenseBooleanArray<TypeParam>;


    dba_true->init(array_true, n_bits, true);
    dba_false->init(array_false, n_bits, false);

    for (size_t i = 0; i < n_bits; i++)
    {
        ASSERT_TRUE((*dba_true)[i]) << "first occurred when i = " << i << '\n';
        ASSERT_FALSE((*dba_false)[i]) << "first occurred when i = " << i << '\n';
    }
}


TYPED_TEST(DenseBooleanArrayTest, set_data)
{
    [[maybe_unused]] const size_t bits_per_element = 8 * sizeof(TypeParam);
    constexpr size_t n_DBs = 4;
    constexpr size_t total_bits = sizeof(TypeParam) * 8 * n_DBs;
    const auto my_dba_p = new DenseBooleanArray<TypeParam>(total_bits, true);
    auto my_dba = *my_dba_p;
    my_dba.set_all(false);

    for (size_t i = 0; i < total_bits; i++)
    {
        ASSERT_FALSE(my_dba[i]) << "first occurred when i = " << i << '\n';
    }

    // Covers starting inside an element, spanning two full elements and ending inside an element.
    size_t start = bits_per_element / 2;
    const size_t end = bits_per_element / 2 + 2 * bits_per_element;

    my_dba.set_range(start, end, true);

    for (size_t i = 0; i < total_bits; i++)
    {
        ASSERT_EQ(my_dba[i], i >= start && i < end) << "first occurred when i = " << i << '\n';
    }
}
