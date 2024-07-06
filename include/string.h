//
// Created by artypoole on 26/06/24.
//

#ifndef STRING_H
#define STRING_H

#include "serial.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "types.h"

static constexpr char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


// extern "C"
int strlen(const char* str);

//extern "C"
template <typename int_like>
int string_from_int(int_like val, char* out_str)
{
    bool const is_negative = val < 0;
    int i = 0;
    int n_digits = 0;
    if (is_negative)
    {
        out_str[i] = '-';
        i++;
        n_digits++;
    }
    auto tmp = val;
    while ((tmp /= 10) != 0) ++n_digits;

    while (i <= n_digits)
    {
        out_str[n_digits - i] = static_cast<char>(val % 10 + 48);
        val /= 10;
        i++;
    }
    out_str[i] = '\0';

    return i;
}

template <typename int_like>
char digit_as_char(const int_like val)
{
    if (val > 10 or val < 0)
    {
        return ' ';
    }
    return static_cast<char>(val % 10);
}

//extern "C"
template <typename int_like1, typename int_like2>
int hex_from_int(int_like1 val, char* out_str, int_like2 n_bytes)
{
    const bool neg = val < 0;
    int offset;
    int len;
    if (neg)
    {
        out_str[0] = '-';
        len = (n_bytes * 2) + 2;
        val = -val;
        offset = 0;
    }
    else
    {
        len = (n_bytes * 2) + 1;
        offset=1;
    }
    for (int i = 0; i < 2*n_bytes; i++)
    {
        out_str[len-2-i] = hex[(val >> 4 * i) & 0xF];
    }


    out_str[len-1] = '\0';
    return len;
}


template <typename int_like>
int log2(int_like val)
{
    if (val < 0) val = -val;
    int i = 0;
    while ((val /= 2) > 0)
    {
        i++;
    }
    return i;
}

#endif //STRING_H
