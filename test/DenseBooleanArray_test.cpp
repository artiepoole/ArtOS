//
// Created by artypoole on 02/12/24.
//
#include <gtest/gtest.h>

#include "DenseBooleanArray.h"
#include "types.h"

// todo: test set_data

// // TODO: make more unit like
TEST(DenseBooleanArrayTest, default_false_basics) {
    DenseBooleanArray<u32> dba(64, false);
    dba.set_bit(0, true);
    dba.set_bit(33, true);
    dba.set_bit(34, true);
    ASSERT_TRUE(dba[0]);
    ASSERT_FALSE(dba[1]);
    ASSERT_TRUE(dba[33]);
    ASSERT_FALSE(dba[32]);
    ASSERT_EQ(dba.get_array_len(), 2);
    ASSERT_EQ(dba.get_next_false(), 1);
    const size_t next_true = dba.get_next_true(2);
    ASSERT_EQ(next_true, 33);
    ASSERT_TRUE(dba[next_true]);
    ASSERT_EQ(dba.get_next_true(next_true+1), 34);
    ASSERT_EQ(dba.get_next_true(next_true+2), -1);
}

// TODO: make more unit like
// TODO: should test all bits in an item not just a subset?
TEST(DenseBooleanArrayTest, default_true_basics) {
    DenseBooleanArray<u32> dba(125, true);
    dba.set_bit(0, false);
    dba.set_bit(1, false);
    dba.set_bit(64, false);
    dba.set_bit(65, false);
    ASSERT_TRUE(dba[2]);
    ASSERT_FALSE(dba[1]);
    ASSERT_EQ(dba.get_array_len(), 4);
    ASSERT_EQ(dba.get_next_true(), 2);
    ASSERT_TRUE(dba[32]);
    size_t next_false = dba.get_next_false(2);
    ASSERT_EQ(next_false, 64);
    ASSERT_EQ(dba.get_next_false(next_false+1), 65);
}

// TODO: this should test finding a good range and handling of: no suitable ranges, start finds an area but doesn't have space for the whole range etc.
TEST(DenseBooleanArrayTest, get_chunk_true) {
    DenseBooleanArray<u32> dba(125, true);
    dba.set_bit(0, false);
    dba.set_bit(1, false);
    dba.set_bit(3, false);
    dba.set_bit(4, false);
    size_t start = dba.get_next_trues(0, 5);
    ASSERT_EQ(start, 5);
    ASSERT_EQ(dba.get_next_trues(6, 128), -1);
}

TEST(DenseBooleanArrayTest, get_chunk_false) {
    DenseBooleanArray<u32> dba(125, false);
    dba.set_bit(0, true);
    dba.set_bit(1, true);
    dba.set_bit(3, true);
    dba.set_bit(4, true);
    size_t start = dba.get_next_falses(0, 5);
    ASSERT_EQ(start, 5);
    ASSERT_EQ(dba.get_next_falses(6, 128), -1);
}