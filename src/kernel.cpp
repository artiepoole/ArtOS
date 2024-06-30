#include <float.h>
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
VideoGraphicsArray* vgap;
#define width 1024 // hard coded - not good please change
#define height 768 // hard coded - not good please change
u32 buffer[width * height];
static const char* hex = "0123456789ABCDEF";

inline i32 text_row;
inline i32 text_column;
inline u32 text_colour = 0xFFFFFF;
// inline uint16_t* terminal_buffer;


void printfHex(u8 key)
{
    char* foo = "00";

    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    vgap->writeString(foo);
}

void printfHex32(u32 key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex(key & 0xFF);
}

void printf(char* str, u32 key, i32 x, i32 y, u32 color)
{
    vgap->writeString(str);

    int l = 0;
    for (; str[l] != 0; l++);
    printfHex32(key);
}

void print_int(const int val)
{
    vgap->writeInt(val);
}

void print_string(const char* str)
{
    vgap->writeString(str);
}

void test_writing_print_numbers()
{
    for (size_t i = 0; i < 50; i++)
    {
        print_int(i);
        print_string("\n");
    }
}


extern "C" void kernel_main(u32 stackPointer, const multiboot_header* multiboot_structure, u32 /*multiboot_magic*/)
{
    VideoGraphicsArray vga(multiboot_structure, buffer);
    vgap = &vga;
    serial_initialise();
    serial_write_string("LOADED OS.\n");

    vga.drawSplash();
    vga.bufferToScreen(false);


    // todo: inherit size of window and colour depth
    // todo: Create string handling to concatenate strings and print them more easily
    // todo: restructure code to have the graphics stuff handled in a different file with only printf handled in
    // kernel.cpp
    // todo: add data to the data section contianing the splash screen
    // Todo: implement user typing from keyboard inputs
    // Todo: automate the build process
}
