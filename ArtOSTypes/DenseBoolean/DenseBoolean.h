//
// Created by artypoole on 02/12/24.
//

#ifndef DENSEBOOLEAN_H
#define DENSEBOOLEAN_H

#include "types.h"

// TODO: this could be replaced with using operators on int_like within the DBA class to maximise density. This currently requires extra bytes as overhead.
template <typename int_like>
class __attribute__ ((__packed__)) DenseBoolean
{
public:
    explicit DenseBoolean(int_like data)
    {
        size_in_bits = sizeof(int_like) * 8;
        this->raw = data;
    }

    explicit DenseBoolean()
    {
        size_in_bits = sizeof(int_like) * 8;
        this->raw = 0;
    }

    void init(int_like data)
    {
        size_in_bits = sizeof(int_like) * 8;
        this->raw = data;
    }

    int_like data()
    {
        return this->raw;
    }

    [[nodiscard]] size_t n_bits() const { return this->size_in_bits; }

    bool operator [](const size_t idx)
    {
        return raw & (1 << (idx % size_in_bits));
    }

    bool operator [](const int idx)
    {
        if (idx < 0 || idx >= size_in_bits) return false;
        return raw & (1 << (idx % size_in_bits));
    }

    void set_bit(size_t idx, const bool b)
    {
        if (idx > this->size_in_bits) return;
        constexpr int_like v = 1;
        if (b)
            raw |= (v << (idx));
        else
        {
            raw &= ~(v << (idx));
        }
    }

    void set_data(int_like data)
    {
        this->raw = data;
    }

    friend bool operator==(DenseBoolean const& lhs, DenseBoolean const rhs)
    {
        return (lhs.raw & rhs.data()) != 0;
    }

    friend bool operator!=(DenseBoolean const& lhs, DenseBoolean const rhs)
    {
        return !(lhs.raw == rhs.data());
    }

    friend bool operator==(DenseBoolean const& lhs, int_like const rhs)
    {
        return (lhs.raw & rhs) != 0;
    }

    friend bool operator!=(DenseBoolean const& lhs, int_like const rhs)
    {
        return !(lhs.raw == rhs);
    }

    // TODO: maybe implement int_like comparisons?

private:
    size_t size_in_bits = 0;
    int_like raw = 0;
};


#endif //DENSEBOOLEAN_H
