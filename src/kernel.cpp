#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../include/terminal.h"
#include "../include/serial.h"
#include "../include/string.h"
#include "../include/types.h"
#include "../include/multiboot2.h"
#include "../include/vga.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

// void test_writing_print_numbers()
// {
//     for (size_t i = 1; i <= VGA_WIDTH * VGA_HEIGHT; i++)
//     {
//         char out_str[255];
//         const size_t len = string_from_int(static_cast<long>(i), out_str);
//         char trimmed_str[len];
//         for (size_t j = 0; j <= len; j++)
//         {
//             trimmed_str[j] = out_str[j];
//         }
//         terminal_writestring(trimmed_str);
//         serial_write_string(trimmed_str);
//         serial_new_line();
//     }
// }

// extern "C"
// void test_colour()
// {
//     size_t im_width = VGA_WIDTH;
//     size_t im_height = VGA_HEIGHT;
//     char text_data[im_width * im_height] = {' '};
//     uint8_t colour_data[im_width * im_height] = {0};
//     for (size_t i = 0; i < (im_width * im_height); i++)
//     {
//         text_data[i] = 'A';
//         colour_data[i] = vga_entry_color(static_cast<vga_color>((i+1)%_VGA_COLOUR_COUNT), static_cast<vga_color>(i%(_VGA_COLOUR_COUNT/2)));
//
//
//     }
//     const auto *colour_ptr = colour_data;
//     terminal_draw_colour_ascii(text_data, colour_ptr, im_width, im_height);
// }
VideoGraphicsArray *vgap;
#define width 1024 // hard coded - not good please change
#define height 768 // hard coded - not good please change
u32 buffer[width * height];
static const char* hex = "0123456789ABCDEF";

inline i32 text_row;
inline i32 text_column;
inline u32 text_colour = 0xFFFFFF;
// inline uint16_t* terminal_buffer;


void printfHex(u8 key, i32 x, i32 y, u32 color) {
    char* foo = "00";

    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    vgap->PutStr(foo, x, y, color);
}

void printfHex32(u32 key, i32 x, i32 y, u32 color) {
    printfHex((key >> 24) & 0xFF, x, y, color);
    printfHex((key >> 16) & 0xFF, x+16, y, color);
    printfHex((key >> 8) & 0xFF, x+32, y, color);
    printfHex( key & 0xFF, x+48, y, color);
}

void printf(char* str, u32 key, i32 x, i32 y, u32 color) {
    vgap->PutStr(str, x, y, color);

    int l = 0;
    for (; str[l] != 0; l++);
    printfHex32(key, x + (l*8), y, color);
}



void print_string(const char* str)
{
    vgap->PutStr(str, width/2, height/2, text_colour);

}

extern "C" void kernel_main(u32 stackPointer , const multiboot_header* multiboot_structure, u32 /*multiboot_magic*/)
{
    VideoGraphicsArray vga(multiboot_structure, buffer);
    vgap = &vga;
    vga.bufferToScreen();
    serial_initialise();
    serial_write_string("LOADED OS.\n");
    print_string("Welcome to ArtOS!");
    vga.bufferToScreen();
    // todo: add integer font scaling
    // todo: draw a window
    // todo: define default colours - solarised dark theme.



    // terminal_writestring("Welcome to ArtOS!\n");
    // test_colour();

    // Todo: implement user typing from keyboard inputs
    // Todo: automate the build process
}
