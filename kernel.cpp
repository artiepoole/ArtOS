#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "src/terminal.h"
#include "src/serial.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif


extern "C"
void kernel_main(void)
{
    /* Initialize terminal interface */
    terminal_initialize();
    serial_initialise();
    // Todo: implement user typing from keyboard inputs
    // Todo: automate the build process

    terminal_writestring("Welcome to ArtOS!\n");

    for (long i = 1; i <= VGA_WIDTH * VGA_HEIGHT; i++)
    {
        char out_str[255];
        const size_t len = string_from_int(i, out_str);
        char trimmed_str[len];
        for (int j = 0; j <= len; j++)
        {
            trimmed_str[j] = out_str[j];
        }
        terminal_writestring(trimmed_str);
        serial_write_string(trimmed_str);
        serial_new_line();



        // terminal_new_line();
    }
    // int i=0;
    // while (true)
    // {
    //     terminal_writechar(static_cast<char>(i % 10 + 48));
    //     terminal_new_line();
    //     i++;
    // }
}
