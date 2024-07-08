#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

class Terminal
{
public:
    Terminal();
    ~Terminal();
    static Terminal & get();

    void writeString(const char* data) const;
    void writeChar(char c) const;

    template <typename int_like>
    void writeInt(int_like val)
    {
        char out_str[255]; // long enough for any int type possible
        const size_t len = string_from_int(val, out_str);
        char trimmed_str[len];
        for (size_t j = 0; j <= len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
        writeString(trimmed_str);
    }

    template <typename int_like1>
    void writeHex(int_like1 val)
    {
        char out_str[255];
        const size_t len = hex_from_int(val, out_str, sizeof(val));
        char trimmed_str[len];
        for (size_t j = 0; j < len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
        writeString(trimmed_str);
    }


private:
    void _scrollTerminal() const;
    void _renderTerminal() const;
};




#endif //TERMINAL_H
