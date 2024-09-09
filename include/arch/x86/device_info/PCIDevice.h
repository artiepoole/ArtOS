//
// Created by artypoole on 30/07/24.
//

#ifndef PCIDEVICE_H
#define PCIDEVICE_H

#include "types.h"
#include "PCI.h"

// PCI device info in configuration space information populated.
// https://wiki.osdev.org/PCI


class PCIDevice
{
public:
    PCIDevice();
    PCIDevice(u8 bus, u8 slot, u8 func);


    PCI_Address_t address{};

    u16 vendor;
    // Generic header values
    u16 vendor_id() const;
    u16 device_id() const;
    u16 get_command();
    u16 set_command_bit(u8 bit, bool value);
    u16 get_status();
    u16 set_status_bit(u8 bit, bool value);
    u8 rev_id() const;
    u8 prog_if() const;
    u8 sub_class() const;
    u8 class_code() const;
    u8 cache_line_size() const;
    u8 latency_timer() const;
    u8 header_type() const;
    u8 bist() const;
    u32 bar(u8 n) const;
    u8 capablilites() const;
    u8 interrupt_line() const;
    u8 interrupt_pin() const;
    u32 expansion_RBA() const;

    // Header type 0 (device) only.
    u32 card_cis() const;
    u16 subsystem_vendor_id() const;
    u16 subsystem_id() const;


    // Header type 1 (PCI-PCI bridge)
    //
    // Primary Bus Number
    // secondary Bus Number
    // Subordinate Bus Number
    // Secondary Latency Timer
    // I/O Base
    // I/O Limit
    // Secondary Status
    // Memory Base
    // Memory Limit
    // Prefetchable Memory Base
    // Prefetchable Memory Limit
    // Prefetchable Base Upper 32 Bits
    // Prefetchable Limit Upper 32 Bits
    // I/O Base Upper 16 Bits
    // I/O Limit Upper 16 Bits
    // Bridge Control

    // Header type 2 (PCI-to-CardBus bridge)
    //
    // CardBus Socket/ExCa base address
    // Offset of capabilities list
    // Secondary status
    // CardBus latency timer
    // PCI bus number
    // CardBus bus number
    // Memory Base Address N
    // Memory Limit N
    // I/O Base Address N
    // I/O Limit N
    // 16-bit PC Card legacy mode base address


    u8 min_grant() const;
    u8 max_latency() const;


    void log_format();

private:
    PCI_header_t header{};
    PCI_header_t populate_values() const;
    u8 config_readb(u8 offset);
    u16 config_readw(u8 offset);
    u32 config_read_register(u8 offset) const;
    u32 config_write_register(u8 offset, u32 data) const;
};

void PCI_populate_list();
PCIDevice* PCI_get_IDE_controller();

#include "types.h"

#endif //PCI_H
