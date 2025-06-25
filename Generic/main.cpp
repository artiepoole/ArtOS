// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

#include <float.h>
#include <../ArtOS_lib/kernel.h>
#include <paging.h>
#include <SMBIOS.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "CPPMemory.h"
#include "Serial.h"
#include "types.h"
#include "VideoGraphicsArray.h"
#include "IDT.h"
#include "PIC.h"
#include "Terminal.h"
#include "syscall.h"
#include "paging.h"
#include "icxxabi.h"
#include "LocalAPIC.h"
#include "IOAPIC.h"
#include "ACPI.h"
#include "PCIDevice.h"
#include "multiboot2.h"
#include "stdio.h"
#include "Files.h"
#include "ArtFile.h"
#include "RTC.h"
#include "PIT.h"
#include "EventQueue.h"
#include "IDEStorageContainer.h"
#include "ATA.h"
#include "BusMasterController.h"
#include "CPUID.h"
#include "logging.h"
#include "memory.h"
#include "Scheduler.h"
#include "GDT.h"
#include "Terminal.h"
#include "Serial.h"
#include "CPPMemory.h"
#if FORLAPTOP
#include "CPUID.h"
#include "SMBIOS.h"
#endif
#if SIMD
#include "simd_enable.h"
#endif

IDE_drive_info_t drive_list[4] = {};
uintptr_t BM_controller_base_port = 0;


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

    // Enable simd if available
#if SIMD
    simd_enable();
#endif
    // Must populate boot info in order to set up memory handling required to use malloc/new
    [[maybe_unused]] artos_boot_header* boot_info = multiboot2_populate(boot_info_addr);
    mmap_init(&boot_info->mmap);
    multiboot2_tag_framebuffer_common* frame_info = &boot_info->framebuffer_common;

    size_t framebuffer_size_b = frame_info->framebuffer_width * frame_info->framebuffer_height * frame_info->framebuffer_bpp / 8;
    paging_identity_map(frame_info->framebuffer_addr, framebuffer_size_b, true, false);
    art_memory_init();


    // Then load all the boot information into a usable format.
    LOG("Populated boot info.");


    // Load serial early for logging.
#if ENABLE_SERIAL_LOGGING
    // auto serial = Serial();
    auto& serial = get_serial();
    serial.link_file();
#endif

    // We want time-stamping to work asap.
    WRITE("Sun Jan  0 00:00:00 1900\tLoading singletons...\n");
    RTC rtc;

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
    auto local_apic = new LocalAPIC(get_local_apic_base_addr());
    auto io_apic = new IOAPIC(full_madt->io_apic.physical_address);
#if ENABLE_SERIAL_LOGGING
    serial.register_device();
#endif
    // then load the rest of the singleton classes.

    vga.drawSplash();
    vga.draw();
    auto bar = vga.createProgressBar(
        (frame_info->framebuffer_width - 540) / 2,
        47 * frame_info->framebuffer_height / 100,
        540,
        80,
        5,
        24);
    vga.incrementProgressBarChunk(bar);
    vga.incrementProgressBarChunk(bar);
    Terminal terminal(45 + 5, 3 * frame_info->framebuffer_height / 5 + 5, frame_info->framebuffer_width - 100, 2 * frame_info->framebuffer_height / 5 - 55);
    vga.incrementProgressBarChunk(bar);
    EventQueue kernel_events;
    vga.incrementProgressBarChunk(bar);


    // remap IRQs in APIC
    io_apic->remap_IRQ(2, 32); // PIT moved to pin2 on APIC. 0 is taken for something else
    io_apic->disable_IRQ(2);
    vga.incrementProgressBarChunk(bar);
    io_apic->remap_IRQ(1, 33); // Keyboard
    vga.incrementProgressBarChunk(bar);
    io_apic->remap_IRQ(8, 40); // RTC
    vga.incrementProgressBarChunk(bar);
    io_apic->remap_IRQ(14, 46); // IDE primary
    vga.incrementProgressBarChunk(bar);
    io_apic->remap_IRQ(15, 47); // IDE primary
    vga.incrementProgressBarChunk(bar);

    configure_pit(2000, io_apic, 2);

    vga.incrementProgressBarChunk(bar);

    // Create a new GDT and load it. Paging is used so this is just redundant
    GDT_init();
    // Configure interrupt tables and enable interrupts.
    IDT idt;
    vga.incrementProgressBarChunk(bar);

    CPUID_init(); // load CPUID values and try and get TSC rate otherwise get TSC rate from PIT calibration
    local_apic->configure_timer(DIVISOR_128); // use TSC rate to calibrate TSC->LAPIC timer ratio and calculate LAPIC timer rate at given divisor

    // TODO: In order to implement scheduling:
    // todo: Processes need a way to yield
    // todo: Processes need to have a way to start/be executed
    // todo: SLEEP should schedule things instead of waiting on RTC
    // todo: get_time (precise) should work out the system time using TSC or something, and get_date_time can have low precision like 1ms or 1s by reading from RTC.

    // TODO: implement the idea of a timer which can have a callback and a one shot duration etc.

    LOG("Singletons loaded.");

#if ENABLE_TERMINAL_LOGGING
    // TODO: handle terminal file wrapper also.
    register_file_handle(0, nullptr); // stdin
    vga.incrementProgressBarChunk(bar);
    register_file_handle(1, get_terminal().get_stdout_file()); // stdout
    vga.incrementProgressBarChunk(bar);
    register_file_handle(2, get_terminal().get_stderr_file()); // stderr
    vga.incrementProgressBarChunk(bar);
    printf("This should print out to terminal via printf\n");
    fprintf(stderr, "This should print error to screen via fprintf\n");
    fprintf(stdout, "This should print out to screen via fprintf\n");
    vga.incrementProgressBarChunk(bar);
#elif ENABLE_SERIAL_LOGGING
    // TODO: register_file_handle does not instantiate an fstream properly. Only fopen does.
    // This means that these should not be registered directly and instead should use filenames.
    // These filenames are already in place.
    register_file_handle(0, get_serial().get_file()); // stdin
    vga.incrementProgressBarChunk(bar);
    register_file_handle(1, get_serial().get_file()); // stdout
    vga.incrementProgressBarChunk(bar);
    register_file_handle(2, get_serial().get_file()); // stderr
    vga.incrementProgressBarChunk(bar);
    int com = art_open("/dev/com1\0", 0);
    art_write(com, "This should print to com0 via art_write.\0", 42);
    vga.incrementProgressBarChunk(bar);
    printf("This should print to com0 via printf\n");
#endif
    PCI_populate_list();
    vga.incrementProgressBarChunk(bar);
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
    vga.incrementProgressBarChunk(bar);
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
    vga.incrementProgressBarChunk(bar);
    LOG("IDE base port raw: ", BM_controller_base_port);
    vga.incrementProgressBarChunk(bar);
    if (int n_drives = populate_drives_list(drive_list); n_drives == 0)
    {
        LOG("No drives found.");
        return;
    }
    else if (n_drives < 0)
    {
        LOG("Error initialising drives.");
    }
    vga.incrementProgressBarChunk(bar);
    int cd_idx = 0;
    for (; cd_idx < 4; cd_idx++)
    {
        if (drive_list[cd_idx].packet_device)
        {
            break;
        }
    }
    vga.incrementProgressBarChunk(bar);
    // TODO: Possibly set up the ATAPIDrive inside the IDEStorageContainer to avoid the need for a IDE_notifiable class etc.
    // TODO: Initialisation of the controller should only happen if the device is DM capable i.e. this should be moved to the
    //          IDEStorageContainer constructor.
    char dev_name[] = "/dev/cdrom0";
    auto secondary_bus_master = BusMasterController(BM_controller_base_port, &drive_list[cd_idx]);
    vga.incrementProgressBarChunk(bar);
    auto CD_ROM = new IDEStorageContainer(drive_list[cd_idx], PCI_IDE_controller, &secondary_bus_master, dev_name);
    vga.incrementProgressBarChunk(bar);
    CD_ROM->mount();
    vga.incrementProgressBarChunk(bar);


    // vga.draw();
    terminal.setRegion(0, 0, frame_info->framebuffer_width, frame_info->framebuffer_height);
    LOG("LOADED OS. Entering event loop.");

    [[maybe_unused]] auto scheduler = new Scheduler(local_apic, &kernel_events);

    int shell_file = art_open("b.art", 0);
    if (shell_file < 1) { LOG("CRITICAL: Failed to open b.art"); }

    art_exec(shell_file);
    yield();
    art_close(shell_file);

    WRITE("ERROR: Left main loop.");
    asm("hlt");

    // todo: Create string handling to concatenate strings and print them more easily
    // todo: restructure code to have the graphics stuff handled in a different file with only printf handled in
    // main.cpp
}
