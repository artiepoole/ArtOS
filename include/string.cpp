//
// Created by artypoole on 26/06/24.
//

#include "string.h"

#include "serial.h"


int log2(u64 val)
{
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

int string_from_int(u64 val, char* out_str)
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
    return static_cast<char>(val % 10);
}

int hex_from_int(u64 val, char* out_str, u32 n_bytes)
{
    // int n_bytes = log2(val)/4;
    // serial_write_string("Val: ");
    // serial_write_int(val);
    // serial_new_line();

    for (i32 i = 2 * n_bytes-1; i >=0; i--)
    {
        out_str[2 * n_bytes-i-1] = hex[(val >> 4*i) & 0xF];
        // serial_write_string("val shifted by: ");
        // serial_write_int(4*i);
        // serial_write_string(" = ");
        // serial_write_int((val >> 4*i) & 0xF);
        // serial_new_line();
    }
    out_str[2 * n_bytes] = '\0';
    return n_bytes*2+1;
}
