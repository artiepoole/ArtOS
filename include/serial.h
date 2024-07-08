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

    void newLine();
    void writeChar(unsigned char c);
    void writeString(const char* data);

    template<typename int_like>
    void writeInt(const int_like val)
    {
        char out_str[255];
        const int len = string_from_int(val, out_str);
        char trimmed_str[len];
        for (size_t j = 0; j < len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
       writeString(trimmed_str);
    }

    template<typename int_like1>
    void writeHex(const int_like1 val)
    {
        char out_str[255];
        const u32 len = hex_from_int(val, out_str, sizeof(val));
        char trimmed_str[len];
        for (size_t j = 0; j < len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
       writeString(trimmed_str);
    }
private:
    static char read();
    void _sendChar(unsigned char a);
    static int _received();
    static int _transmitEmpty();
    void _write(const  char* data, size_t size);
};



#endif //SERIAL_H
