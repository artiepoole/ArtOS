//
// Created by artypoole on 26/06/24.
//

#ifndef STRING_H
#define STRING_H


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "types.h"

static constexpr char dec[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
static constexpr char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


// extern "C"
size_t strlen(const char* str);

template <typename int_like>
char digit_as_char(const int_like val)
{
    if (val > 10 or val < 0)
    {
        return ' ';
    }
    return dec[val];
}

//extern "C"
template <typename int_like>
void string_from_int(int_like val, char* out_str)
{
    bool const is_negative = val < 0;

    int n_digits = 1; // +1 because the while loop adds after dividing by one. can't print 0 otherwise
    int len = 0;


    if (is_negative)
    {
        val = -val;
    }

    auto tmp = val;
    while ((tmp /= 10) != 0) ++n_digits;

    if (is_negative)
    {
        out_str[0] = '-';
        len = n_digits + 2;
    }
    else
    {
        len = n_digits + 1;
    }

    for (int i = 0; i < n_digits; i++)
    {

        out_str[len - i - 2] = digit_as_char(val % 10);
        val /= 10;
    }
    out_str[len - 1] = '\0';
}

//extern "C"
template <typename int_like1, typename int_like2>
void hex_from_int(int_like1 val, char* out_str, int_like2 n_bytes)
{
    const bool neg = val < 0;
    int_like2 len;
    if (neg)
    {
        out_str[0] = '-';
        len = (n_bytes * 2) + 2;
        val = -val;
    }
    else
    {
        len = (n_bytes * 2) + 1;
    }
    for (int_like2 i = 0; i < 2 * n_bytes; i++)
    {
        out_str[len - 2 - i] = hex[(val >> 4 * i) & 0xF];
    }


    out_str[len - 1] = '\0';
}


template <typename int_like>
int log2(int_like val)
{
    // todo: decide if rounding down or up is more useful.
    if (val < 0) val = -val;
    int i = 0;
    while ((val /= 2) > 0)
    {
        i++;
    }
    return i;
}

#endif //STRING_H
