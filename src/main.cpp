#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#include "Serial.h"
#include "types.h"
#include "multiboot_header.h"
#include "VideoGraphicsArray.h"
#include "IDT.h"
#include "PIC.h"
#include "Terminal.h"
// #include "stdlib.h"
// #include "malloc.c"

#include "LocalAPIC.h"
#include "IOAPIC.h"

#include "ACPI.h"
#include "PCI.h"
#include "multiboot2.h"
#include "stdio.h"
#include "CPUID.h"
#include "RTC.h"
#include "PIT.h"
#include "TSC.h"
#include "SMBIOS.h"
#include "EventQueue.h"
#include "string.h"
extern "C" {
#include "doomgeneric/doomgeneric.h"
}


/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

// VideoGraphicsArray* vgap;
u8 keyboard_modifiers = 0; // caps, ctrl, alt, shift  -> C ! ^ *


extern "C"
void kernel_main(unsigned long magic, unsigned long boot_info_addr)
{
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) // Should be 0x36d76289
    {
        // Invalid magic
        return;
    }

    if (boot_info_addr & 7)
    {
        // misaligned MBI
        return;
    }

    // Get logging and timestamping running asap
    auto log = Serial();
    RTC rtc;

    // Then load all the boot information into a usable format.
    LOG("Populating boot info.");
    [[maybe_unused]] artos_boot_header* boot_info = multiboot2_populate(boot_info_addr);
    multiboot2_tag_framebuffer_common* frame_info = multiboot2_get_framebuffer();
    full_madt_t* full_madt = populate_madt(multiboot2_get_MADT_table_address());
    [[maybe_unused]] LocalAPIC local_apic(get_local_apic_base_addr());
    [[maybe_unused]] IOAPIC io_apic(full_madt->io_apic.physical_address);

    // then load the rest of the singleton classes.
    WRITE("Mon Jan 01 00:00:00 1970\tLoading singletons...\n");
    EventQueue events;
    VideoGraphicsArray vga(frame_info);
    Terminal terminal;
    PIC::disable_entirely();

    // remap IRQs in APIC
    io_apic.remapIRQ(2, 32); // PIT moved to pin2 on APIC. 0 is taken for something else
    io_apic.remapIRQ(1, 33); // Keyboard
    io_apic.remapIRQ(8, 40); // RTC

    configurePit(2000);

    // local_apic.configure_timer(1024);
    // Configure interrupt tables and enable interrupts.
    IDT idt;

    LOG("Singletons loaded.");


    vga.drawSplash();
    vga.draw();


    terminal.setScale(2);
    vga.draw();
    terminal.write(" Loading Done.\n");

    // FILE* com = fopen("/dev/com1", "w");
    // fprintf(com, "%s\n", "This should print to com0");

    PCI_list();
    [[maybe_unused]] auto IDE_controller = PCIDevice(0, 1, 1);

    LOG("LOADED OS.");

    run_doom();

    // Event handler loop.
    LOG("Entering event loop.");
    while (true)
    {
        if (events.pendingEvents())
        {
            auto [type, data] = events.getEvent();
            // LOG("Found event. Type: ", static_cast<int>(type), " lower: ", data.lower_data, " upper: ",data.upper_data);
            switch (type)
            {
            case NULL_EVENT:
                {
                    WRITE("NULL EVENT\n");
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
                                keyboard_modifiers &= 0b1110; // not 0100
                                break;
                            }

                        case '^': // ctrl bit 1
                            {
                                keyboard_modifiers &= 0b1101; // not 0010
                                break;
                            }
                        case '!': // alt bit 2
                            {
                                keyboard_modifiers &= 0b1011; // not 0001
                                break;
                            }
                        default:
                            {
                                break;
                            }
                        }
                    }

                    // todo: Add a line buffer and parsing to inputs on enter.
                    // todo: Add an key handler which deals with modifier keys
                    // todo: handle backspace
                    // todo: write an actual terminal class.

                    // WRITE("Key up event in main loop.\n");
                    break;
                }
            case KEY_DOWN:
                {
                    // WRITE("Key down event in main loop: ");
                    size_t cin = data.lower_data;
                    char key = key_map[cin];
                    // WRITE(key);
                    // NEWLINE();
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
                                keyboard_modifiers |= 0b0010;
                                break;
                            }
                        case '*': // shift bit 0
                            {
                                keyboard_modifiers |= 0b0001;
                                // print("Shift pressed", keyboard_modifiers);
                                break;
                            }
                        case '!': // alt bit 2
                            {
                                keyboard_modifiers |= 0b0100;
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
                                keyboard_modifiers ^= 0b1000;
                                break;
                            }
                        default:
                            {
                                bool is_alpha = (key >= 97 && key <= 122);
                                if (keyboard_modifiers & 0b1000) // caps lock enabled
                                    if (is_alpha) // alphanumeric keys get shifted to caps
                                    {
                                        terminal.write(shift_map[cin]);
                                        break;
                                    }
                                if ((keyboard_modifiers & 0b0001)) // shift is down or capslock is on
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
                    WRITE("Unhandled event.\n");
                    WRITE("Type: ");
                    WRITE(static_cast<int>(type));
                    WRITE(" lower: ");
                    WRITE(data.lower_data, true);
                    WRITE(" upper: ");
                    WRITE(data.upper_data, true);
                    NEWLINE();
                    break;
                }
            }
        }
    }
    WRITE("ERROR: Left main loop.");
    asm("hlt");


    // todo: inherit size of _window and colour depth
    // todo: Create string handling to concatenate strings and print them more easily
    // todo: restructure code to have the graphics stuff handled in a different file with only printf handled in
    // main.cpp
    // todo: add data to the data section containing the splash screen
    // Todo: implement user typing from keyboard inputs
    // Todo: automate the build process
}
