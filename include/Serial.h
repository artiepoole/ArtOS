//
// Created by artypoole on 25/06/24.
//
#ifndef SERIAL_H
#define SERIAL_H


#include "mystring.h"
#include "types.h" // TODO: Use stdlib "string.h" instead.

#define PORT 0x3f8          // COM1

class Serial
{
private:
    static char _read();
    static void _sendChar(unsigned char a);
    static int _received();
    static int _transmitEmpty();
    static void _write(const char* data, size_t size);

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

    static int com_read(char* dest, unsigned long count);
    static int com_write(const char* data, unsigned long count);

    void time_stamp();

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
        time_stamp();
        write(arg1);
        newLine();
    }

    template <typename... args_t>
    void log(args_t&&... args)
    {
        time_stamp();
        (write(args), ...);
        newLine();
    }


    template <typename int_like>
    requires is_int_like_v<int_like> && (!is_same_v<int_like, char>)
    void logHex(int_like val, const char* val_name="")
    {
        if (mystrlen(val_name)>0)
        {
            write(val_name);
            write(": ");
        }
        write(val, true);
        newLine();
    };



};


#endif //SERIAL_H
