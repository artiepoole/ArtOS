//
// Created by artypoole on 02/12/24.
//

#ifndef DENSEBOOLEANARRAY_H
#define DENSEBOOLEANARRAY_H
#include "DenseBoolean.h"


template <typename int_like>
class DenseBooleanArray
{
public:
    explicit DenseBooleanArray(const size_t total_bits)
    {
        capacity = total_bits;
        n_bits = sizeof(int_like) * 8;
        array_len = (total_bits + (n_bits - 1)) / n_bits;
        array = new DenseBoolean<int_like>[array_len];
    }

    explicit DenseBooleanArray(const size_t total_bits, const bool def_bool)
    {
        capacity = total_bits;
        n_bits = sizeof(int_like) * 8;
        array_len = (total_bits + (n_bits - 1)) / n_bits;
        array = new DenseBoolean<int_like>[array_len];
        int_like v = 0;
        if (def_bool) v = ~0;
        for (int i = 0; i < array_len; i++)
        {
            array[i].set_data(v);
        }
    }

    ~DenseBooleanArray()
    {
        delete[] array;
    }


    bool operator [](const size_t bit_idx)
    {
        return array[bit_idx / n_bits].data() & (1 << (bit_idx % n_bits));
    }

    void set_bit(const size_t idx, bool b)
    {
        array[idx / n_bits].set_bit(idx % n_bits, b);
    }

    int get_next_false()
    {
        return get_next_false(0);
    }

    template <typename sint_like>
    int get_next_false(const sint_like offset)
    {
        if (offset < 0) return -1;
        if (offset >= capacity) return -1;
        return get_next_false(static_cast<size_t>(abs(offset))); // safe cast
    }

    // Exact same logic as get_next_true but all the data is inverted.
    int get_next_false(const size_t offset)
    {
        int array_idx = offset / n_bits;
        int item_idx = offset % n_bits;
        int_like mask = (-1) << item_idx; // returns, e.g. 11111000 if item_idx == 3
        int_like data = ~array[array_idx].data() & mask; // ignores lowest bits
        item_idx = 0;

        while (data == 0 && array_idx < array_len) // while data is all ones (no falses in int_like)
        {
            array_idx++;
            data = ~array[array_idx].data();
        }

        while (!(data & 0x1 << item_idx) && item_idx < n_bits) // bit wise
        {
            item_idx++;
        }

        item_idx = array_idx * n_bits + item_idx;
        if (item_idx >= capacity) return -1; // possible if last entry is not full
        return item_idx;
    }

    template <typename sint_like>
    int get_next_falses(const sint_like offset, size_t n)
    {
        if (offset < 0) return -1;
        if (offset >= capacity) return -1;
        return get_next_falses(static_cast<size_t>(abs(offset)), n); // safe cast
    }


    int get_next_falses(const size_t offset, size_t n)
    {
        int start_idx = get_next_false(offset);
        int next_idx = get_next_true(start_idx + 1);
        while (next_idx - start_idx < n && start_idx > 0 && next_idx > 0)
        {
            start_idx = get_next_false(next_idx);
            next_idx = get_next_true(start_idx + 1);
        }
        if (start_idx < 0) return -1; // no trues after offset
        if (next_idx < 0 && start_idx + n > capacity) return -1; // doesn't fit
        return start_idx; // should fit
    }

    int get_next_true()
    {
        return get_next_true(0);
    }

    template <typename sint_like>
    int get_next_true(const sint_like offset)
    {
        if (offset < 0) return -1;
        if (offset >= capacity) return -1;
        return get_next_true(static_cast<size_t>(abs(offset))); // safe cast
    }

    int get_next_true(const size_t offset)
    {
        int array_idx = offset / n_bits;
        int item_idx = offset % n_bits;
        int_like mask = (-1) << item_idx;
        int_like data = array[array_idx].data() & mask; // ignores lowest bits
        item_idx = 0;
        while (data == 0 && array_idx < array_len) // while data is all zeros (no trues in int_like)
        {
            array_idx++;
            data = array[array_idx].data();
        }

        while (!(data & 0x1 << item_idx) && item_idx < n_bits) // bitwise search within.
        {
            item_idx++;
        }

        item_idx = array_idx * n_bits + item_idx;
        if (item_idx >= capacity) return -1; // possible if last entry is not full
        return item_idx;
    }

    template <typename sint_like>
    int get_next_trues(const sint_like offset, size_t n)
    {
        if (offset < 0) return -1;
        if (offset >= capacity) return -1;
        return get_next_trues(static_cast<size_t>(abs(offset)), n); // safe cast
    }

    // gets start idx of a contiguous chunk of trues with length at least as large as n.
    int get_next_trues(const int offset, size_t n)
    {
        int start_idx = get_next_true(offset);
        int next_idx = get_next_false(start_idx + 1);
        while (next_idx - start_idx < n && start_idx > 0 && next_idx > 0)
        {
            start_idx = get_next_true(next_idx);
            next_idx = get_next_false(start_idx + 1);
        }
        if (start_idx < 0) return -1; // no trues after offset
        if (next_idx < 0 && start_idx + n > capacity) return -1; // doesn't fit
        return start_idx; // should fit
    }

    size_t get_array_len() { return array_len; }

    int set_range(int start, int n, bool b) { return -1; }

private:
    DenseBoolean<int_like>* array;
    int array_len; // n_items in array
    int capacity; // bits
    int n_bits;
};


#endif //DENSEBOOLEANARRAY_H
