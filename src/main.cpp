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
    auto CD_ROM = IDEStorageContainer(&atapi_drives[0], PCI_IDE_controller, &secondary_bus_master);

    // TODO: This should be moved to a mount function or something similar. The following populates a directory tree.
    // TODO: when constructing the tree, the filedata should contain starting offset IN BYTES
    constexpr size_t buf_size = sizeof(iso_primary_volume_descriptor_t);
    iso_primary_volume_descriptor_t volume_descriptor{};
    u32 data_start = 16*2048; // in LBA
    CD_ROM.read((&volume_descriptor), data_start, buf_size);
    // LOG("First data: ", buffer[0]);
    LOG("Volume Descriptor type: ", volume_descriptor.descriptor_type);
    LOG("Ident: ", volume_descriptor.identifier);
    if (strncmp(volume_descriptor.identifier, "CD001", 5) != 0) { LOG("Did not detect volume descriptor."); }
    LOG("Block size: ", volume_descriptor.logical_block_size_LE);
    LOG("Path table size: ", volume_descriptor.path_table_size_LE);

    // Load the path table
    char path_table_data[volume_descriptor.path_table_size_LE];
    CD_ROM.read(path_table_data, volume_descriptor.path_l_table_loc_lba*2048, volume_descriptor.path_table_size_LE);
    // Count entries
    size_t offset = 0;
    size_t n_dirs = 0;
    while (offset < volume_descriptor.path_table_size_LE)
    {
        iso_path_table_entry dir_entry = {};
        dir_entry.header = *reinterpret_cast<iso_path_table_entry_header*>(path_table_data + offset);
        offset += sizeof(iso_path_table_entry_header) + dir_entry.header.name_length;
        if (offset % 2) { offset += 1; }
        n_dirs++;
    }

    // populate entries
    // TODO: the first entry loads the root dir and so the directory names in lowercase capable format are stored here!
    iso_path_table_entry path_table[n_dirs];
    iso_path_table_entry* fs_dir_entry = nullptr;
    offset = 0;
    for (size_t entry = 0; entry < n_dirs; entry++)
    {
        path_table[entry].header = *reinterpret_cast<iso_path_table_entry_header*>(path_table_data + offset);
        offset += sizeof(iso_path_table_entry_header);
        char* dir_name = strndup(&path_table_data[offset], path_table[entry].header.name_length);
        // char* filename = static_cast<char*>(malloc(path_table[entry].header.name_length+1));

        // filename[path_table[entry].header.name_length] = '\0';
        path_table[entry].dir_name = dir_name;

        offset += path_table[entry].header.name_length;
        if (offset % 2) { offset += 1; }
        u16 id = path_table[entry].header.parent_dir_id;
        u32 loc = path_table[entry].header.extent_loc;
        LOG("Directory found. Name: ", dir_name, " Directory ID: ", id, " Directory LBA: ", loc);
        if (strncmp(dir_name, "FS", 2) == 0)
        {
            fs_dir_entry = &path_table[entry];
        }
    }
    // Check if FS was detected. Just a test.
    if (fs_dir_entry == nullptr) { LOG("User filesystem directory not found."); }

    // Load all file names and print em.
    for (size_t entry = 0; entry < n_dirs; entry++)
    {
        offset = 0;

        char first_sector_data[2048];
        CD_ROM.read(&first_sector_data, path_table[entry].header.extent_loc*2048, 2048);
        iso_directory_record_header root_dir_header = *reinterpret_cast<iso_directory_record_header*>(&first_sector_data[offset]);
        size_t full_size = root_dir_header.data_length_LE;
        if (full_size > 1024 * 64)
        {
            LOG("Cannot load full data without implementing larger physical region for DMA or handling partial transfers. Reading first 64k of data.");
            full_size = 1024 * 64;
        }
        char full_data[full_size];
        CD_ROM.read(&full_data, root_dir_header.extent_loc_LE*2048, full_size);

        while (offset < full_size)
        {
            size_t start_offset = offset;
            iso_directory_record_header file_header = *reinterpret_cast<iso_directory_record_header*>(&full_data[offset]);
            if (file_header.record_length == 0) // entries will not cross a sector boundary.
            {
                offset = offset + (2048 - (offset % 2048)); // moves to start of next sector
                if (offset >= full_size)
                {
                    break; // if out of limits, move to next dir.
                }
                // otherwise continue from next entry
                start_offset = offset;
                file_header = *reinterpret_cast<iso_directory_record_header*>(&full_data[offset]);
            }
            offset += sizeof(iso_directory_record_header); // skip header to name
            char* filename = strndup(&full_data[offset], file_header.file_name_length);
            if (strncmp(filename, "", file_header.file_name_length) == 0)
            {
                free(filename);
                filename = strdup(".");
            }
            if (strncmp(filename, "\1", file_header.file_name_length) == 0)
            {
                free(filename);
                filename = strdup("..");
            }
            offset += file_header.file_name_length;
            offset += offset % 2;
            size_t extension_pos = 0;
            while (extension_pos < start_offset + file_header.record_length)
            {
                auto [tag, len] = *reinterpret_cast<file_id_ext_header*>(&full_data[offset + extension_pos]);
                if (strncmp(tag, "NM", 2) == 0)
                {
                    free(filename);
                    filename = strndup(&full_data[offset + extension_pos + 5], len - 5);
                    break;
                }
                else if (len > 0)
                {
                    extension_pos += len;
                }
                else
                {
                    break;
                }
            }
            // Have to convert from packed to not. can't implicitly do so for some reason.
            u32 len = file_header.data_length_LE;
            u8 flags = file_header.flags;
            LOG("Directory: ", path_table[entry].dir_name, " name: ", filename, " data length: ", len, " flags raw: ", flags, " is directory: ", flags & 0x02);

            offset = start_offset + file_header.record_length;
            // offset += offset%2;
            // skip the rest of the data (PX extensions etc)
            //todo: parse extension info: replace file names with lowercase capable and longer strings
            free(filename);
            if (flags & 0x80) { LOG("Didn't read all extents for the previous file"); }
        }
    }

    /* TODO: in order to implement the read/seek/loadfile/whatever else,
     *I need to know how to handle loading in "pages" of data and reading
     * within those or create a massive buffer and copy the whole file to memory.
     */


#if ENABLE_SERIAL_LOGGING
    register_file_handle(0, "/dev/stdin", NULL, Serial::com_write);
    register_file_handle(1, "/dev/stdout", NULL, Serial::com_write);
    register_file_handle(2, "/dev/stderr", NULL, Serial::com_write);
    FILE* com = fopen("/dev/com1", "w");
    fprintf(com, "%s\n", "This should print to com0 via fprintf");
    printf("This should print to com0 via printf\n");
#elif ENABLE_TERMINAL_LOGGING
    register_file_handle(0, "/dev/stdin", NULL, NULL);
    register_file_handle(1, "/dev/stdout", NULL, Terminal::user_write);
    register_file_handle(2, "/dev/stderr", NULL, Terminal::user_err);
    printf("This should print to terminal via printf\n");
#endif


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
