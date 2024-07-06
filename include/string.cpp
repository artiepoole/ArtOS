//
// Created by artypoole on 26/06/24.
//

#include "string.h"

#include "serial.h"

template<typename int_like>
int log2(int_like val)
{
    if (val<0) return 0;
    int i = 0;
    while (val /= 2>0) i++;
    return i;
}

int strlen(const char* str)
{
    int len = 0;
    while (str[len])
        len++;
    return len;
}

template<typename int_like>
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

template<typename int_like>
char digit_as_char(const int_like val)
{
    if (val > 10 or val < 0)
    {
        return ' ';
    }
    return static_cast<char>(val % 10);
}

template<typename int_like>
int hex_from_int(int_like val, char* out_str, int_like n_bytes)
{
    const bool neg =  val < 0;
    int start;
    int len;
    if (neg)
    {
        out_str[0] = '-';
        start = 2 * n_bytes;
        len = n_bytes*2+2;
    }    else
    {

        start = 2 * n_bytes-1;
        len = n_bytes*2+1;
    }
    for (int i = start; i >=0; --i)
    {
        out_str[2 * n_bytes-i-1] = hex[(val >> 4*i) & 0xF];
    }

    out_str[2 * n_bytes] = '\0';
    return len;
}
