#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// #include "terminal.h"
#include "serial.h"
#include "string.h"
#include "types.h"
#include "multiboot2.h"
#include "vga.h"
#include "idt.h"
#include "pic.h"

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
u32 buffer[width * height];



// Is supposed to take a different set of arguments....
template<typename int_like>
void printf(const char* str, int_like key)
{
    vgap->writeString(str);

    int l = 0;
    for (; str[l] != 0; l++);
    vgap->writeHex(key);
    vgap->writeString("\n");
}

template<typename int_like>
void print_int(int_like val)
{
    vgap->writeInt(val);
}

void print_string(const char* str)
{
    vgap->writeString(str);
}
template<typename int_like>
void print_hex(const int_like val)
{
    vgap->writeHex(val);
}

void print_frame_buffer_info(const u32 stackPointer, const multiboot_header* multiboot_structure)
{
    printf("multiboot_structure    : 0x", (int)multiboot_structure);
    printf("stackPointer           : 0x", stackPointer);
    printf("screen buffer          : 0x", (int)buffer);
    printf("screen buffer[1024*768]: 0x", (int)&buffer[1024 * 768]);
    printf("FONT                   : 0x", (int)vgap->FONT);
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
    gdt_info gdt{};
    asm("sgdt %0" : "=m"(gdt));
    serial_write_string("GDT limit: ");
    serial_write_hex(gdt.limit);
    serial_write_string(" GDT base: ");
    serial_write_hex(gdt.base);
    serial_new_line();

    for (size_t i =0; i<  8; i++)
    {
        serial_write_string("GDT entry:");
        serial_write_int(i);
        serial_write_string(" data: ");
        uintptr_t gdt_ptr = static_cast<ptrdiff_t>(gdt.base +(8*i));
        serial_write_hex(gdt_ptr);
        serial_new_line();
    }


}

u16 get_cs()
{

    u16 i;
    asm("mov %%cs,%0" : "=r"(i));
    serial_write_string("CS: ");
    serial_write_hex(i);
    serial_new_line();
    return i;
}

u16 get_ds()
{
    u16 i;
    asm("mov %%ds,%0" : "=r"(i));
    serial_write_string("DS: ");
    serial_write_hex(i);
    serial_new_line();
    return i;
}

void test_writing_print_numbers()
{
    for (size_t i = 0; i < 50; i++)
    {
        // get_cs();
        print_int(i);
        print_string("\n");
    }
}

extern u32 DATA_CS;
extern u32 TEXT_CS;
extern int setGdt(u32 limit, u32 base);


extern "C"
void kernel_main(const u32 stackPointer, const multiboot_header* multiboot_structure, const u32 /*multiboot_magic*/)
{
    VideoGraphicsArray vga(multiboot_structure, buffer);
    vgap = &vga;
    vga.drawSplash();
    vga.bufferToScreen(false);
    serial_initialise();
    serial_write_string("LOADED OS.\n");
    pic_irq_remap();
    configure_pit(10000); // 10
    idt_install();
    serial_write_string("IDT installed\n");
    pic_enable_irq0();
    serial_write_string("PIC enabled\n");

    sleep(1000);
    vga.setScale(2);
    vga.clearWindow();
    vga.bufferToScreen(false);
    print_string("Loading Done.\n");

    for(;;) asm("hlt");


    // todo: inherit size of window and colour depth
    // todo: Create string handling to concatenate strings and print them more easily
    // todo: restructure code to have the graphics stuff handled in a different file with only printf handled in
    // kernel.cpp
    // todo: add data to the data section contianing the splash screen
    // Todo: implement user typing from keyboard inputs
    // Todo: automate the build process
}
