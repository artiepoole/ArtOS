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
private:
    char _read();
    void _sendChar(unsigned char a);
    int _received();
    int _transmitEmpty();
    void _write(const char* data, size_t size);

public:
    Serial();
    ~Serial();
    static Serial& get();

    // remove copy functionality
    Serial(Serial const& other) = delete;
    Serial& operator=(Serial const& other) = delete;

    bool connected;

    void newLine();
    void write(unsigned char c);
    void write(const char* data);
    void write(const char* data, size_t len);

    template <typename int_like>
        requires is_int_like_v<int_like> && (!is_same_v<int_like, char>) // Any interger like number but not a char or char array.
    void write(const int_like val, const bool hex = false)
    {
        char out_str[255];
        if (hex)
        {
            hex_from_int(val, out_str, sizeof(val));
        }
        else
        {
            string_from_int(val, out_str);
        }
        write(out_str);
    }

    template <typename type_t>
    void log(type_t const& arg1)
    {
        write(arg1);
        newLine();
    }

    template <typename... args_t>
    void log(args_t&&... args)
    {
        (write(args), ...);
        newLine();
    }


    template <typename int_like>
    requires is_int_like_v<int_like> && (!is_same_v<int_like, char>)
    void logHex(int_like val, const char* val_name="")
    {
        if (strlen(val_name)>0)
        {
            write(val_name);
            write(": ");
        }
        write(val, true);
        newLine();
    };



};


#endif //SERIAL_H
