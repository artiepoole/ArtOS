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

#include "CPPMemory.h"
#include "icxxabi.h"

#include "LocalAPIC.h"
#include "IOAPIC.h"

#include "ACPI.h"
#include "PCIDevice.h"
#include "multiboot2.h"
#include "stdio.h"
#include "Files.h"
#include "ArtDirectory.h"
#include "ArtFile.h"
#include "CPUID.h"
#include "RTC.h"
#include "PIT.h"
#include "TSC.h"
#include "SMBIOS.h"
#include "EventQueue.h"
#include "ports.h"
#include "string.h"
#include "IDEStorageContainer.h"
#include "IDE_DMA_PRDT.h"
#include "ATA.h"
#include "ATAPIDrive.h"
#include "PCIDevice.h"
#include "BusMasterController.h"
#include "iso_fs.h"

extern "C" {
#include "doomgeneric/doomgeneric.h"
}

#include "logging.h"


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
ATAPIDrive* atapi_drives = nullptr;
uintptr_t BM_controller_base_port;


void process_cmd(char* buf, size_t len)
{
    if (len == 0) return;
    if (strncasecmp(buf, "play doom", 9) == 0)
    {
        run_doom();
    }
    else
    {
        auto& term = Terminal::get();
        term.write("Unknown command: ", COLOR_RED);
        term.write(buf, len, COLOR_MAGENTA);
        term.newLine();
    }
}


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
    // Load serial immediately for logging.
#if ENABLE_SERIAL_LOGGING
    auto serial = Serial();
#endif

    // We want timestamping to work asap.
    WRITE("DAY MON DD HH:MM:SS YYYY\tLoading singletons...\n");
    RTC rtc;

    // Then load all the boot information into a usable format.
    LOG("Populating boot info.");
    [[maybe_unused]] artos_boot_header* boot_info = multiboot2_populate(boot_info_addr);
    multiboot2_tag_framebuffer_common* frame_info = multiboot2_get_framebuffer();

#if FORLAPTOP
    cpuid_manufacturer_info_t* manu_info= cpuid_get_manufacturer_info();
    LOG("CPUID Max parameter in decimal: ", manu_info->max_param);

    cpuid_core_frequency_info_t* freq_info= cpuid_get_frequency_info();
    LOG("CPUID freq info: core clock freq: ", freq_info->core_clock_freq_hz, " base freq: ", freq_info->cpu_base_freq_MHz, " cpu_max_freq: ", freq_info->cpu_max_freq_MHz);

    LOG("Current speed from SMBIOS: ", SMBIOS_get_CPU_clock_rate_hz());

    LOG("CR0 CACHE DISABLED?: ", static_cast<bool>(get_cr0().CD));
#endif
    // And then we want graphics.
    VideoGraphicsArray vga(frame_info);
    vga.draw();

    // Then we disable the old PIC.
    PIC::disable_entirely();
    // and load boot info
    uintptr_t madt_addr = multiboot2_get_MADT_table_address();
    TIMESTAMP();
    WRITE("MADT address: ");
    WRITE(madt_addr, true);
    NEWLINE();
    full_madt_t* full_madt = populate_madt(madt_addr);
    LOG("LAPIC count: ", full_madt->LAPIC_count);
    [[maybe_unused]] LocalAPIC local_apic(get_local_apic_base_addr());
    [[maybe_unused]] IOAPIC io_apic(full_madt->io_apic.physical_address);

    // then load the rest of the singleton classes.
    Terminal terminal(frame_info->framebuffer_width, frame_info->framebuffer_height);
    EventQueue events;


    // remap IRQs in APIC
    io_apic.remap_IRQ(2, 32); // PIT moved to pin2 on APIC. 0 is taken for something else
    io_apic.remap_IRQ(1, 33); // Keyboard
    io_apic.remap_IRQ(8, 40); // RTC
    io_apic.remap_IRQ(14, 46); // IDE primary
    io_apic.remap_IRQ(15, 47); // IDE primary

    configure_pit(2000);
    // local_apic.configure_timer(1024);
    // todo: configure apic timer.
    // Configure interrupt tables and enable interrupts.
    IDT idt;

    LOG("Singletons loaded.");

#if ENABLE_SERIAL_LOGGING
    register_file_handle(0, Serial::get_file()); // stdin
    register_file_handle(1, Serial::get_file()); // stdout
    register_file_handle(2, Serial::get_file()); // stderr
    FILE* com = fopen("/dev/com1", "w");
    fprintf(com, "%s\n", "This should print to com0 via fprintf");
    printf("This should print to com0 via printf\n");
#elif ENABLE_TERMINAL_LOGGING
    // TODO: handle terminal file wrapper also.
    register_file_handle(0, nullptr); // stdin
    register_file_handle(1, Terminal::get_stdout_file()); // stdout
    register_file_handle(2, Terminal::get_stderr_file()); // stderr
    printf("This should print out to terminal via printf\n");
    fprintf(stderr, "%s\n", "This should print error to screen via fprintf");
    fprintf(stdout, "%s\n", "This should print out to screen via fprintf");
#endif
    // TODO: Draw splash should programmatically draw using the logo from the middle as a texture.
    PCI_populate_list();
    [[maybe_unused]] auto PCI_IDE_controller = PCI_get_IDE_controller();
    if (PCI_IDE_controller->prog_if() == 0x80)
    {
        LOG("IDE controller supports bus mastering.");
    }
    else
    {
        LOG("IDE controller doesn't support bus mastering. Aborting.");
        return;
    }

    BM_controller_base_port = PCI_IDE_controller->bar(4);
    if (BM_controller_base_port & 0x1)
    {
        LOG("Using port access not memory mapped io for configuration");
        BM_controller_base_port -= 1; // remove the last bit which was set
    }
    else
    {
        LOG("base port is memory address. Not implemented.");
        return;
    }

    LOG("IDE base port raw: ", BM_controller_base_port);
    int n_drives = populate_drives_list(atapi_drives);

    if (n_drives == 0)
    {
        LOG("No drives found.");
        return;
    }
    else if (n_drives < 0)
    {
        LOG("Error initialising drives.");
    }
    // TODO: Possibly set up the ATAPIDrive inside the IDEStorageContainer to avoid the need for a IDE_notifiable class etc.
    auto secondary_bus_master = BusMasterController(BM_controller_base_port, atapi_drives[0].drive_info);

    [[maybe_unused]] auto CD_ROM = new IDEStorageContainer(&atapi_drives[0], PCI_IDE_controller, &secondary_bus_master);
    CD_ROM->mount();
    register_storage_device(CD_ROM);


    char filename[] = "doom1.wad";
    [[maybe_unused]] auto file = fopen(filename, "rb");
    char* dest[2048];
    fread(dest, 1, 2048, file);
    fclose(file);

    // ArtFile* doomfile = CD_ROM->find_file(filename);
    // if (doomfile)
    // {
    //     char doom_data[1024 * 64];
    //     doomfile->read(doom_data, 1024 * 64);
    //     char* ident = strndup(doom_data, 4);
    //     LOG("Doom file ident: ", ident);
    //     free(ident);
    // }


    /* TODO: in order to implement the read/seek/loadfile/whatever else,
     *I need to know how to handle loading in "pages" of data and reading
     * within those or create a massive buffer and copy the whole file to memory.
     */




    // todo: put the handle of this buffer and command calls in a function. This entire loop should probably in a different file.
    constexpr size_t cmd_buffer_size = 1024;
    char cmd_buffer[cmd_buffer_size] = {0};
    size_t cmd_buffer_idx = 0;
    LOG("LOADED OS. Entering event loop.");

#if !ENABLE_TERMINAL_LOGGING
    terminal.write("Loading done.\n");
#endif


    // Event handler loop.

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
                                if (cmd_buffer_idx > 0)
                                {
                                    cmd_buffer[--cmd_buffer_idx] = ' ';
                                }
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
                        case '\n':
                            {
                                terminal.write("\n");
                                process_cmd(cmd_buffer, cmd_buffer_idx);
                                memset(cmd_buffer, 0, cmd_buffer_size);
                                cmd_buffer_idx = 0;
                                terminal.refresh();
                                break;
                            }
                        default:
                            {
                                bool is_alpha = (key >= 97 && key <= 122);
                                if (keyboard_modifiers & 0b1000) // caps lock enabled
                                    if (is_alpha) // alphanumeric keys get shifted to caps
                                    {
                                        cmd_buffer[cmd_buffer_idx++] = shift_map[cin];
                                        terminal.write(shift_map[cin]);
                                        break;
                                    }
                                if ((keyboard_modifiers & 0b0001)) // shift is down or capslock is on
                                {
                                    cmd_buffer[cmd_buffer_idx++] = shift_map[cin];
                                    terminal.write(shift_map[cin]);
                                    break;
                                }
                                else
                                {
                                    cmd_buffer[cmd_buffer_idx++] = key;
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
        // else
        PIT_sleep_ms(1);
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
