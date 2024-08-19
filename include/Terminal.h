#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "colours.h"
#include "mystring.h"

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

    void newLine();
    void userLine();
    void setScale(u32 new_scale);
    u32 getScale();
    void clear();


    void write(const char* data, u32 colour = COLOR_BASE0); // buffer without known length also with colour
    void write(char c, u32 color = COLOR_BASE0); // single char
    void write(const char* data, size_t len, u32 colour=COLOR_BASE0); // buffer of fixed len

    template <typename int_like>
        requires is_int_like_v<int_like> && (!is_same_v<int_like, char>) // Any interger like number but not a char or char array.
    void write(int_like val, size_t hex_len = 0, const u32 color=COLOR_BASE0)
    {
        char out_str[255]; // long enough for any int type possible
        size_t len=0;
        if (hex_len > 0)
        {
            len = hex_from_int(val, out_str, hex_len);
        }
        else
        {
            len =string_from_int(val, out_str);
        }
        write(out_str, len, color);
    }


    void backspace();

private:
    static void _scroll();
    static void _render();
    static void _putChar(terminal_char_t ch, u32 origin_x, u32 origin_y);
};


template <typename int_like>
void printf(const char* str, int_like key)
{
    auto& terminal = Terminal::get();
    terminal.write(str);

    int l = 0;
    for (; str[l] != 0; l++);
    terminal.write(key, true);
    terminal.newLine();
}

template <typename int_like>
void print_int(int_like val)
{
    auto& terminal = Terminal::get();
    terminal.write(val);
}

inline void print_string(const char* str)
{
    auto& terminal = Terminal::get();
    terminal.write(str);
}

inline void print_char(const char c)
{
    auto& terminal = Terminal::get();
    terminal.write(c);
}

template <typename int_like>
void print_hex(const int_like val)
{
    auto& terminal = Terminal::get();
    terminal.write(val, true);
}

#endif //TERMINAL_H
