#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "types.h"
#include "VideoGraphicsArray.h"

struct terminal_char_t
{
    char letter;
    u32 color;
};

class Terminal
{
private:
    // window_t _screen{};

public:
    // Single isntance. Cannot be used if not initialised.
    // explicit Terminal(const window_t* screen);
    Terminal();
    ~Terminal();
    static Terminal& get();

    // remove copy functionality
    Terminal(Terminal const& other) = delete;
    Terminal& operator=(Terminal const& other) = delete;

    void writeString(const char* data);
    void writeString(const char* data, u32 color);
    void writeChar(char c);
    void writeChar(char c, u32 color);
    void writeBuffer(const char* data, size_t len, u32 color);
    void writeBuffer(const char* data, size_t len);
    void newLine();
    void setScale(u32 new_scale);
    u32 getScale();
    void clear();

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
        //log.write_int(val);
        // log.new_line();
    }

    template <typename int_like1>
    void writeHex(int_like1 val)
    {
        // u16 n_bytes = log2(val);
        char out_str[255];
        const size_t len = hex_from_int(val, out_str, sizeof(val));
        char trimmed_str[len];
        for (size_t j = 0; j < len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
        writeString(trimmed_str);
    }

    void backspace() const;

private:
    void _scroll();
    void _render() const;
    void _putChar(terminal_char_t ch, u32 origin_x, u32 origin_y) const;
};


#endif //TERMINAL_H
