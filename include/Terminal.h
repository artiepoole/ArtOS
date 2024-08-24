#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "colours.h"
#include "mystring.h"



struct terminal_char_t
{
    char letter;
    PALETTE_t colour;
};


inline PALETTE_t colour_bkgd = COLOR_BASE02;
inline PALETTE_t colour_frgd = COLOR_BASE0;
inline PALETTE_t colour_accent = COLOR_CYAN;
inline PALETTE_t colour_value = COLOR_MAGENTA;
inline PALETTE_t colour_error = COLOR_RED;


class Terminal
{


public:
    // Single instance. Cannot be used if not initialised.
    // explicit Terminal(const window_t* screen);
    Terminal(u32 width, u32 height);
    ~Terminal();
    static Terminal& get();

    // remove copy functionality
    Terminal(Terminal const& other) = delete;
    Terminal& operator=(Terminal const& other) = delete;

    static void newLine();
    void userLine();
    void setScale(u32 new_scale);
    u32 getScale();
    static void clear();


    static u32 user_write(const char* data, u32 count);
    static u32 user_err(const char* data, u32 count);


    void write(bool b);
    static void write(const char* data, size_t len, PALETTE_t colour = COLOR_BASE0); // buffer of fixed len

    static void write(const char* data, PALETTE_t colour = COLOR_BASE0); // buffer without known length also with colour
    static void write(char c, PALETTE_t colour = COLOR_BASE0); // single char

    static void setChar(size_t x,size_t y, char c, PALETTE_t colour);
    static void time_stamp();

    template <typename int_like>
        requires is_int_like_v<int_like> && (!is_same_v<int_like, char>) // Any interger like number but not a char or char array.
    void write(const int_like val, const bool hex = false, PALETTE_t colour = colour_value)
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
        write(out_str, colour);
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
    void logHex(int_like val, const char* val_name = "")
    {
        if (mystrlen(val_name) > 0)
        {
            write(val_name, from_hex(val));
            write(": ");
        }
        write(val, true);
        newLine();
    };
    static void backspace();
    void refresh();

private:
    static void _scroll();
    static void _render();
    static void _putChar(terminal_char_t ch, u32 origin_x, u32 origin_y);
    static void _write(const char* data, u32 count, PALETTE_t colour);
    static void _append(const char* data, u32 count, PALETTE_t colour);
    static void _render_queue(const terminal_char_t* data, size_t len); // used to display data from before.

};



#endif //TERMINAL_H
