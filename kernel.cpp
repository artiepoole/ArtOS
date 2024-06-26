#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "src/terminal.h"
#include "src/serial.h"
#include "src/string.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void test_writing_print_numbers()
{
    for (size_t i = 1; i <= VGA_WIDTH * VGA_HEIGHT; i++)
    {
        char out_str[255];
        const size_t len = string_from_int(static_cast<long>(i), out_str);
        char trimmed_str[len];
        for (size_t j = 0; j <= len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
        terminal_writestring(trimmed_str);
        serial_write_string(trimmed_str);
        serial_new_line();
    }
}

extern "C"
void test_colour()
{
    size_t im_width = VGA_WIDTH;
    size_t im_height = VGA_HEIGHT;
    char text_data[im_width * im_height] = {' '};
    uint8_t colour_data[im_width * im_height] = {0};
    for (size_t i = 1; i < (im_width * im_height); i++)
    {
        text_data[i] = 'A';
        colour_data[i] = vga_entry_color(static_cast<vga_color>((i+1)%_VGA_COLOUR_COUNT), static_cast<vga_color>(i%(_VGA_COLOUR_COUNT/2)));
        // colour_data[i] = vga_entry_color(static_cast<vga_color>((i+1)%_VGA_COLOUR_COUNT), VGA_COLOR_BLACK);
        // colour_data[i] = vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK);


    }
    const auto *colour_ptr = colour_data;
    terminal_draw_colour_ascii(text_data, colour_ptr, im_width, im_height);
}

extern "C"
void kernel_main(void)
{
    terminal_initialize();
    serial_initialise();
    terminal_writestring("Welcome to ArtOS!\n");
    test_colour();

    // Todo: implement user typing from keyboard inputs
    // Todo: automate the build process
}
