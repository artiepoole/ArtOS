#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Hardware text mode color constants. */
//
// enum vga_color
// {
//     VGA_COLOR_BLACK = 0,
//     VGA_COLOR_BLUE = 1,
//     VGA_COLOR_GREEN = 2,
//     VGA_COLOR_CYAN = 3,
//     VGA_COLOR_RED = 4,
//     VGA_COLOR_MAGENTA = 5,
//     VGA_COLOR_BROWN = 6,
//     VGA_COLOR_LIGHT_GREY = 7,
//     VGA_COLOR_DARK_GREY = 8,
//     VGA_COLOR_LIGHT_BLUE = 9,
//     VGA_COLOR_LIGHT_GREEN = 10,
//     VGA_COLOR_LIGHT_CYAN = 11,
//     VGA_COLOR_LIGHT_RED = 12,
//     VGA_COLOR_LIGHT_MAGENTA = 13,
//     VGA_COLOR_LIGHT_BROWN = 14,
//     VGA_COLOR_WHITE = 15,
//     _VGA_COLOUR_COUNT = 16,
// };

static constexpr size_t SCREEN_WIDTH_TEXT = 1024/8;
static constexpr size_t SCREEN_HEIGHT_TEXT = 768/8;

//
// uint8_t vga_entry_color(vga_color fg, vga_color bg);

// void terminal_initialize();
//
// void terminal_new_line();
//
// void terminal_draw_colour_ascii(const char* text, const uint8_t* colour, size_t width, size_t height);
//
// void terminal_writechar(char c);
//
// void terminal_writestring(const char* data);

// void print_string(const char* str);




#endif //TERMINAL_H
