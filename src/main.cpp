#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/kernel.h>


#include "Serial.h"
#include "string.h"
#include "types.h"
#include "multiboot_header.h"
#include "VideoGraphicsArray.h"
#include "IDT.h"
#include "PIC.h"
#include "system.h"
#include "Terminal.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif


VideoGraphicsArray* vgap;
#define width 1024 // hard coded - not good please change
#define height 768 // hard coded - not good please change
u32 frame_buffer[width * height];


//
// // Is supposed to take a different set of arguments....
// template<typename int_like>
// void printf(const char* str, int_like key)
// {
//     auto terminal = Terminal::get();
//     terminal.write(str);
//
//     int l = 0;
//     for (; str[l] != 0; l++);
//     terminal.write(key);
//     terminal.newLine();
// }
//
// template<typename int_like>
// void print_int(int_like val)
// {
//     auto terminal = Terminal::get();
//     terminal.write(val);
// }
//
// void print_string(const char* str)
// {
//     auto terminal = Terminal::get();
//     terminal.write(str);
// }
//
// void print_char(const char c)
// {
//     auto terminal = Terminal::get();
//     terminal.write(c);
// }
//
// template<typename int_like>
// void print_hex(const int_like val)
// {
//     auto terminal = Terminal::get();
//     terminal.write(val, true);
// }
//
// void print_multiboot_header_info(const u32 stackPointer, const multiboot_header* multiboot_structure)
// {
//     printf("multiboot_structure    : 0x", (int)multiboot_structure);
//     printf("stackPointer           : 0x", stackPointer);
//     printf("screen buffer          : 0x", (int)frame_buffer);
//     printf("screen buffer[1024*768]: 0x", (int)&frame_buffer[1024 * 768]);
//     // printf("FONT                   : 0x", *FONT);
//     printf("buffer size            : 0x", 1024 * 768 * 4);
//     printf("vga                    : 0x", (int)vgap);
//
//     //    printf("multiboot_header
//     printf("u32 flags              : 0x", (int)multiboot_structure->flags);
//     printf("u32 mem_lower          : 0x", (int)multiboot_structure->mem_lower);
//     printf("u32 mem_upper          : 0x", (int)multiboot_structure->mem_upper);
//     printf("u32 boot_device        : 0x", (int)multiboot_structure->boot_device);
//     printf("u32 cmdline            : 0x", (int)multiboot_structure->cmdline);
//     printf("u32 mods_count         : 0x", (int)multiboot_structure->mods_count);
//     printf("u32 mods_addr          : 0x", (int)multiboot_structure->mods_addr);
//     printf("u32 syms1              : 0x", (int)multiboot_structure->syms1);
//     printf("u32 syms2              : 0x", (int)multiboot_structure->syms2);
//     printf("u32 syms3              : 0x", (int)multiboot_structure->syms3);
//     printf("u32 mmap_length        : 0x", (int)multiboot_structure->mmap_length);
//     printf("u32 mmap_addr          : 0x", (int)multiboot_structure->mmap_addr);
//     printf("u32 drives_length      : 0x", (int)multiboot_structure->drives_length);
//     printf("u32 drives_addr        : 0x", (int)multiboot_structure->drives_addr);
//     printf("u32 config_table       : 0x", (int)multiboot_structure->config_table);
//     printf("u32 boot_loader_name   : 0x", (int)multiboot_structure->boot_loader_name);
//     printf("u32 apm_table          : 0x", (int)multiboot_structure->apm_table);
//     printf("u32 vbe_control_info   : 0x", (int)multiboot_structure->vbe_control_info);
//     printf("u32 vbe_mode_info      : 0x", (int)multiboot_structure->vbe_mode_info);
//     printf("u16 vbe_mode           : 0x", (int)multiboot_structure->vbe_mode);
//     printf("u16 vbe_interface_seg  : 0x", (int)multiboot_structure->vbe_interface_seg);
//     printf("u32 vbe_interface_off  : 0x", (int)multiboot_structure->vbe_interface_off);
//     printf("u32 vbe_interface_len  : 0x", (int)multiboot_structure->vbe_interface_len);
//     printf("u64 framebuffer_addr   : 0x", (long)multiboot_structure->framebuffer_addr);
//     printf("u32 framebuffer_pitch  : 0x", (int)multiboot_structure->framebuffer_pitch);
//
//     printf("u32 framebuffer_width  : 0x", (int)multiboot_structure->framebuffer_width);
//     printf("u32 framebuffer_height : 0x", (int)multiboot_structure->framebuffer_height);
//     printf("u8 framebuffer_bpp     : 0x", (int)multiboot_structure->framebuffer_bpp);
//     printf("u8 framebuffer_type    : 0x", (int)multiboot_structure->framebuffer_type);
//     printf("u8 color_info[5]       : 0x", (int)multiboot_structure->color_info);
// }
//

// Is supposed to take a different set of arguments....
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

void print_string(const char* str)
{
    auto& terminal = Terminal::get();
    terminal.write(str);
}

void print_char(const char c)
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

void print_multiboot_header_info(const u32 stackPointer, const multiboot_header* multiboot_structure)
{
    printf("multiboot_structure    : 0x", (int)multiboot_structure);
    printf("stackPointer           : 0x", stackPointer);
    printf("screen buffer          : 0x", (int)frame_buffer);
    printf("screen buffer[1024*768]: 0x", (int)&frame_buffer[1024 * 768]);
    // printf("FONT                   : 0x", *FONT);
    printf("buffer size            : 0x", 1024 * 768 * 4);
    printf("vga                    : 0x", (int)vgap);

    //    printf("multiboot_header
    printf("u32 flags              : 0x", (int)multiboot_structure->flags);
    printf("u32 mem_lower          : 0x", (int)multiboot_structure->mem_lower);
    printf("u32 mem_upper          : 0x", (int)multiboot_structure->mem_upper);
    printf("u32 boot_device        : 0x", (int)multiboot_structure->boot_device);
    printf("u32 cmdline            : 0x", (int)multiboot_structure->cmdline);
    printf("u32 mods_count         : 0x", (int)multiboot_structure->mods_count);
    printf("u32 mods_addr          : 0x", (int)multiboot_structure->mods_addr);
    printf("u32 syms1              : 0x", (int)multiboot_structure->syms1);
    printf("u32 syms2              : 0x", (int)multiboot_structure->syms2);
    printf("u32 syms3              : 0x", (int)multiboot_structure->syms3);
    printf("u32 mmap_length        : 0x", (int)multiboot_structure->mmap_length);
    printf("u32 mmap_addr          : 0x", (int)multiboot_structure->mmap_addr);
    printf("u32 drives_length      : 0x", (int)multiboot_structure->drives_length);
    printf("u32 drives_addr        : 0x", (int)multiboot_structure->drives_addr);
    printf("u32 config_table       : 0x", (int)multiboot_structure->config_table);
    printf("u32 boot_loader_name   : 0x", (int)multiboot_structure->boot_loader_name);
    printf("u32 apm_table          : 0x", (int)multiboot_structure->apm_table);
    printf("u32 vbe_control_info   : 0x", (int)multiboot_structure->vbe_control_info);
    printf("u32 vbe_mode_info      : 0x", (int)multiboot_structure->vbe_mode_info);
    printf("u16 vbe_mode           : 0x", (int)multiboot_structure->vbe_mode);
    printf("u16 vbe_interface_seg  : 0x", (int)multiboot_structure->vbe_interface_seg);
    printf("u32 vbe_interface_off  : 0x", (int)multiboot_structure->vbe_interface_off);
    printf("u32 vbe_interface_len  : 0x", (int)multiboot_structure->vbe_interface_len);
    printf("u64 framebuffer_addr   : 0x", (long)multiboot_structure->framebuffer_addr);
    printf("u32 framebuffer_pitch  : 0x", (int)multiboot_structure->framebuffer_pitch);

    printf("u32 framebuffer_width  : 0x", (int)multiboot_structure->framebuffer_width);
    printf("u32 framebuffer_height : 0x", (int)multiboot_structure->framebuffer_height);
    printf("u8 framebuffer_bpp     : 0x", (int)multiboot_structure->framebuffer_bpp);
    printf("u8 framebuffer_type    : 0x", (int)multiboot_structure->framebuffer_type);
    printf("u8 color_info[5]       : 0x", (int)multiboot_structure->color_info);
}


struct gdt_info
{
    u16 limit;
    u32 base;
}__attribute__((packed));

struct gdt_entry
{
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 access;
    u8 granularity;
    u8 base_high;
} __attribute__((packed));


void get_GDT()
{
    auto& log = Serial::get();
    gdt_info gdt{};
    asm("sgdt %0" : "=m"(gdt));
    log.write("GDT limit: ");
    log.write(gdt.limit, true);
    log.write(" GDT base: ");
    log.write(gdt.base, true);
    log.newLine();

    for (size_t i = 0; i < 8; i++)
    {
        log.write("GDT entry:");
        log.write(i);
        log.write(" data: ");
        uintptr_t gdt_ptr = static_cast<ptrdiff_t>(gdt.base + (8 * i));
        log.write(gdt_ptr, true);
        log.newLine();
    }
}

u16 get_cs()
{
    auto& log = Serial::get();
    u16 i;
    asm("mov %%cs,%0" : "=r"(i));
    log.write("CS: ");
    log.write(i,true);
    log.newLine();
    return i;
}

u16 get_ds()
{
    auto& log = Serial::get();
    u16 i;
    asm("mov %%ds,%0" : "=r"(i));
    log.write("DS: ");
    log.write(i, true);
    log.newLine();
    return i;
}

// void test_writing_print_numbers()
// {
//     for (size_t i = 0; i < 50; i++)
//     {
//         // get_cs();
//         print_int(i);
//         print_string("\n");
//     }
// }

extern u32 DATA_CS;
extern u32 TEXT_CS;
extern int setGdt(u32 limit, u32 base);
/**
 *
 */
extern char kernel_end;
void* kernel_brk = &kernel_end;

u8 modifiers = 0; // caps, ctrl, alt, shift  -> C ! ^ *


extern "C"
void kernel_main(const u32 /*stackPointer*/, const multiboot_header* multiboot_structure, const u32 /*multiboot_magic*/)
{
    auto log = Serial();
    log.write("LOADED OS.\n");
    log.newLine();
    EventQueue events;
    VideoGraphicsArray vga(multiboot_structure, frame_buffer);

    // vgap = &vga;
    Terminal terminal{};

    PIC pic;
    vga.drawSplash();
    vga.draw();
    log.write("LOADED OS.\n");
    configurePit(10000); // 10
    IDT idt;


    pic.enableIRQ(0);
    pic.enableIRQ(1);

    // u32 step = load_bar_region[2]/100;
    // size_t n_steps = 50;
    // for (size_t i =0; i <= n_steps; i++)
    // {
    //    vga.fillRectangle(load_bar_region[0], load_bar_region[1], load_bar_region[2]* i/n_steps , load_bar_region[3], COLOR_BASE0);
    //     vga.draw();
    //     sleep(50);
    //
    // }
    // sleep(1000);
    terminal.setScale(2);
    vga.draw();
    // terminal.write(" Loading Done.\n");
    int i = -32;
    size_t x = 133;
    terminal.write(i);
    terminal.newLine();
    log.logHex(x, "x");
    print_hex(x, 4, COLOR_RED);


    print("i: ", i, " x: ", x);
    terminal.userLine();


    // Event handler loop.
    while (true)
    {
        if (events.pendingEvents())
        {
            auto [type, data] = events.getEvent();
            // log.write("Found event\n");
            // log.write("type: ");
            // log.write(static_cast<int>(type));
            // log.write(" lower: ");
            // log.write(data.lower_data, true);
            // log.write(" upper: ");
            // log.write(data.upper_data, true);
            // log.newLine();
            switch (type)
            {
            case NULL_EVENT:
                {
                    log.write("NULL EVENT\n");
                    break;
                }
            case KEY_UP:
                {
                    size_t cin = data.lower_data;
                    char key = key_map[cin];

                    if (key_map[cin] != 0)
                    {
                        switch (key)
                        {
                        case '*': // shift bit 0
                            {
                                modifiers &= 0b1110; // not 0100
                                break;
                            }

                        case '^': // ctrl bit 1
                            {
                                modifiers &= 0b1101; // not 0010
                                break;
                            }
                        case '!': // alt bit 2
                            {
                                modifiers &= 0b1011; // not 0001
                                break;
                            }
                        default:
                            {
                                break;
                            }
                        }
                    }

                    // todo: Add a line byffer and parsing to inputs on enter.
                    // todo: Add an key handler which deals with modifier keys
                    // todo: handle backspace
                    // todo: write an actual terminal class.

                    // log.write("Key up event in main loop.\n");
                    break;
                }
            case KEY_DOWN:
                {
                    // log.write("Key down event in main loop: ");
                    size_t cin = data.lower_data;
                    char key = key_map[cin];
                    // log.write(key);
                    // log.newLine();
                    if (key_map[cin] != 0)
                    {
                        switch (key)
                        {
                        case '\b': // backspace
                            {
                                terminal.backspace();
                                break;
                            }
                        case '\t': // tab
                            {
                                terminal.write("    ");
                                break;
                            }
                        case '^': // ctrl bit 1
                            {
                                modifiers |= 0b0010;
                                break;
                            }
                        case '*': // shift bit 0
                            {
                                modifiers |= 0b0001;
                                // print("Shift pressed", modifiers);
                                break;
                            }
                        case '!': // alt bit 2
                            {
                                modifiers |= 0b0100;
                                break;
                            }
                        case 'H': // Home
                            {
                                // todo: handle home
                                break;
                            }
                        case 'E': // end
                            {
                                // go to end of line
                                break;
                            }
                        case 'U': // up
                            {
                                // probably won't handle up
                                break;
                            }
                        case 'D': // down
                            {
                                // probably won't handle this
                                break;
                            }
                        case '<': // left
                            {
                                // move left
                                break;
                            }
                        case '>': // right
                            {
                                // move right
                                break;
                            }
                        case 'C': // capital C meaning caps lock
                            {
                                modifiers ^= 0b1000;
                                break;
                            }
                        default:
                            {
                                bool is_alpha = (key >= 97 && key <= 122);
                                if (modifiers & 0b1000) // caps lock enabled
                                    if (is_alpha) // alphanumeric keys get shifted to caps
                                    {
                                        terminal.write(shift_map[cin]);
                                        break;
                                    }
                                if ((modifiers & 0b0001)) // shift is down or capslock is on
                                {
                                    terminal.write(shift_map[cin]);
                                    break;
                                }
                                else
                                {
                                    terminal.write(key);
                                }

                                break;
                            }
                        }
                    }
                    break;
                }
            default:
                {
                    log.write("Unhandled event.\n");
                    log.write("Type: ");
                    log.write(static_cast<int>(type));
                    log.write(" lower: ");
                    log.write(data.lower_data, true);
                    log.write(" upper: ");
                    log.write(data.upper_data, true);
                    log.newLine();
                    break;
                }
            }
        }
    }
    log.write("ERROR: Left main loop.");
    asm("hlt");


    // todo: inherit size of _window and colour depth
    // todo: Create string handling to concatenate strings and print them more easily
    // todo: restructure code to have the graphics stuff handled in a different file with only printf handled in
    // main.cpp
    // todo: add data to the data section contianing the splash screen
    // Todo: implement user typing from keyboard inputs
    // Todo: automate the build process
}
