//
// Created by artypoole on 25/06/24.
//
#ifndef SERIAL_H
#define SERIAL_H

#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "ports.h"

#define PORT 0x3f8          // COM1

class Serial
{
public:
    Serial();
    ~Serial();
    static Serial & get();
    bool connected;

    void new_line();
    void write_char(unsigned char c);
    void write_string(const char* data);

    template<typename int_like>
    void write_int(const int_like val)
    {
        char out_str[255];
        const int len = string_from_int(val, out_str);
        char trimmed_str[len];
        for (size_t j = 0; j < len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
       write_string(trimmed_str);
    }

    template<typename int_like1>
    void write_hex(const int_like1 val)
    {
        char out_str[255];
        const u32 len = hex_from_int(val, out_str, sizeof(val));
        char trimmed_str[len];
        for (size_t j = 0; j < len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
       write_string(trimmed_str);
    }
private:
    static char read();
    void send_char(unsigned char a);
    static int received();
    static int transmit_empty();
    void write(const  char* data, size_t size);
};



#endif //SERIAL_H
