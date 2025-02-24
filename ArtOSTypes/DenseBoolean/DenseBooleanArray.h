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

#ifndef DENSEBOOLEANARRAY_H
#define DENSEBOOLEANARRAY_H
// #include "DenseBoolean.h"

inline size_t DBA_ERR_IDX = -1;

template <typename int_like>
class __attribute__ ((__packed__)) DenseBooleanArray
{
public:
    DenseBooleanArray() = default;


    explicit DenseBooleanArray(const size_t total_bits)
    {
        capacity = total_bits;
        n_bits = sizeof(int_like) * 8;
        array_len = (total_bits + (n_bits - 1)) / n_bits;

        set_all(false);
    }

    DenseBooleanArray(const size_t total_bits, const bool def_bool)
    {
        capacity = total_bits;
        n_bits = sizeof(int_like) * 8;
        array_len = (total_bits + (n_bits - 1)) / n_bits;
        array = new int_like[array_len];
        set_all(def_bool);
    }

    ~DenseBooleanArray()
    {
        delete[] array;
    }


    // TODO: not a super safe init.
    /* late init */
    void init(void* initial_array, const size_t total_bits, bool b)
    {
        capacity = total_bits;
        n_bits = sizeof(int_like) * 8;
        array_len = (total_bits + (n_bits - 1)) / n_bits;
        array = reinterpret_cast<int_like*>(initial_array);
        int_like v = get_mask(b);
        for (size_t i = 0; i < array_len; i++)
        {
            array[i] = v;
        }
    }

    bool operator [](const size_t bit_idx)
    {
        if (bit_idx >= capacity) return false;
        constexpr int_like v = 1;
        return array[bit_idx / n_bits] & (v << (bit_idx % n_bits));
    }


    void set_bit(const size_t idx, bool b)
    {
        size_t array_idx = idx / n_bits;
        size_t bit_idx = idx % n_bits;
        if (bit_idx > this->n_bits) return;
        constexpr int_like v = 1;
        if (b)
            array[array_idx] |= (v << (bit_idx));
        else
        {
            array[array_idx] &= ~(v << (bit_idx));
        }
    }


    int get_next_false()
    {
        return get_next_false(0);
    }

    // Exact same logic as get_next_true but all the data is inverted.
    size_t get_next_false(const size_t offset)
    {
        size_t array_idx = offset / n_bits;
        size_t item_idx = offset % n_bits;
        constexpr int_like v = -1;
        int_like mask = v << item_idx; // returns, e.g. 11111000 if item_idx == 3
        int_like data = ~array[array_idx] & mask; // ignores lowest bits
        item_idx = 0;

        while (data == 0 && array_idx < array_len) // while data is all ones (no falses in int_like)
        {
            array_idx++;
            data = ~array[array_idx];
        }

        for (constexpr int_like vv = 1; !(data & vv << item_idx) && item_idx <= n_bits; item_idx++)
        {
        } // bit wise

        item_idx = array_idx * n_bits + item_idx;
        if (item_idx >= capacity) return DBA_ERR_IDX; // possible if last entry is not full
        return item_idx;
    }


    size_t get_next_falses(const size_t offset, size_t n)
    {
        size_t start_idx = get_next_false(offset);
        size_t next_idx = get_next_true(start_idx);
        while (next_idx - start_idx < n && start_idx < DBA_ERR_IDX && next_idx < DBA_ERR_IDX)
        {
            start_idx = get_next_false(next_idx);
            next_idx = get_next_true(start_idx);
        }
        if (start_idx == DBA_ERR_IDX) return DBA_ERR_IDX; // no trues after offset
        if (next_idx == DBA_ERR_IDX && start_idx + n > capacity) return DBA_ERR_IDX; // doesn't fit
        return start_idx; // should fit
    }

    size_t get_next_true()
    {
        return get_next_true(0);
    }


    size_t get_next_true(const size_t offset)
    {
        size_t array_idx = offset / n_bits;
        size_t item_idx = offset % n_bits;
        constexpr int_like v = -1;
        int_like mask = v << item_idx;
        int_like data = array[array_idx] & mask; // ignores lowest bits
        item_idx = 0;

        while (data == 0 && array_idx < array_len) // while data is all zeros (no trues in int_like)
        {
            array_idx++;
            data = array[array_idx];
        }

        for (int_like vv = 1; !(data & vv << item_idx) && item_idx <= n_bits; item_idx++)
        {
        } // bitwise search within.

        item_idx = array_idx * n_bits + item_idx;
        if (item_idx >= capacity) return DBA_ERR_IDX; // possible if last entry is not full
        return item_idx;
    }

    // gets start idx of a contiguous chunk of trues with length at least as large as n.
    size_t get_next_trues(const size_t offset, size_t n)
    {
        size_t start_idx = get_next_true(offset);
        size_t next_idx = get_next_false(start_idx);
        while (next_idx - start_idx < n && start_idx < DBA_ERR_IDX && next_idx < DBA_ERR_IDX)
        {
            start_idx = get_next_true(next_idx);
            next_idx = get_next_false(start_idx);
        }
        if (start_idx == DBA_ERR_IDX) return DBA_ERR_IDX; // no trues after offset
        if (next_idx == DBA_ERR_IDX && start_idx + n > capacity) return DBA_ERR_IDX; // doesn't fit
        return start_idx; // should fit
    }

    size_t get_array_len() { return array_len; }

    // return error or n copied. 
    size_t set_range(const size_t start, const size_t end, const bool b)
    {
        if (end > capacity || end < start || end == 0 || end >= DBA_ERR_IDX) return DBA_ERR_IDX;
        const int_like mask = get_mask(b);
        size_t bit = start;
        for (; bit % n_bits > 0 && bit < end; bit++) // set bits up to next whole chunk
        {
            set_bit(bit, b);
        }
        for (; bit + n_bits < end; bit += n_bits) // set all whole chunks
        {
            array[bit / n_bits] = mask;
        }
        for (; bit < end; bit++) // set remaining bits
        {
            set_bit(bit, b);
        }
        return end - start;
    }

    int_like get_mask(bool b)
    {
        int_like v = 0;
        if (b) v = ~0;
        return v;
    }

    void set_all(bool b)
    {
        const int_like v = get_mask(b);
        for (int i = 0; i < array_len; i++)
        {
            array[i] = v;
        }
    }

private:
    int_like* array;
    size_t array_len = 0; // n_items in array
    size_t capacity = 0; // bits
    size_t n_bits = 0;
};


#endif //DENSEBOOLEANARRAY_H
