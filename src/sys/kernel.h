//
// Created by artypoole on 09/07/24.
//

#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"
#include "Terminal.h"
#include "RTC.h"


void write_standard(const char* buffer, unsigned long len);

void write_error(const char* buffer, unsigned long len);


template <typename type_t>
void print(type_t const& arg1)
{
    auto& term = Terminal::get();
    term.write(arg1);
    term.newLine();
}

template <typename... args_t>
void print(args_t&&... args)
{
    auto& term = Terminal::get();
    (term.write(args), ...);
    term.newLine();
}

template <typename... args_t>
void print_colour(u32 color, args_t&&... args)
{
    auto& term = Terminal::get();
    (term.write(args, color), ...);
    term.newLine();
}

template <typename int_like>
requires is_int_like_v<int_like> && (!is_same_v<int_like, char>)
void print_hex(int_like val, size_t hex_len, u32 color = COLOR_BASE00)
{
    auto& term = Terminal::get();
    term.write(val, hex_len, color);
    term.newLine();
}

extern "C"
void _exit(int status);

tm *get_time();
time_t get_epoch_time();

u32 get_clock_rate();
u64 get_current_clock();

// void draw_screen_region(unsigned long * frame_buffer);


#endif //KERNEL_H
