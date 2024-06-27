//
// Created by artypoole on 26/06/24.
//

#include "string.h"


size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

size_t string_from_int(long val, char* out_str)
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

char digit_as_char(const int val)
{
    if (val > 10 or val < 0)
    {
        return ' ';
    }
    return static_cast<char>(val%10);
}