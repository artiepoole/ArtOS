//
// Created by artypoole on 30/07/24.
//

#ifndef PCI_H
#define PCI_H

#include "types.h"

// PCI device info in configuration space information populated.
// https://wiki.osdev.org/PCI
struct PCI_Address_t
{
    union
    {
        struct
        {
            u8 offset : 8;
            u8 function : 3;
            u8 slot : 5;
            u8 bus : 8;
            u8 reserved : 7;
            u8 enabled : 1;
        } components;

        u32 address;
    };
};

struct PCI_header_t
{
    union
    {
        struct
        {
            u16 vendor_id, device_id;
        };
        u32 reg0;
    };
    union
    {
        struct
        {
            u16 command, status;
        };
        u32 reg1;
    };
    union
    {
        struct
        {
            u8 revision_id, prog_IF, subclass, class_code;
        };
        u32 reg2;
    };
    union
    {
        struct
        {
            u8 cache_line_size,latency_timer, header_type, BIST;
        };
        u32 reg3;
    };
    u32 reg4, reg5, reg6, reg7, reg8, reg9, regA, regB, regC, regD, regE;
    union
    {
        struct
        {
            u8 interrupt_line, interrupt_pin;
            u16 half_reg_F;
        };
        u32 regF;
    };

};


class PCIDevice
{
public:
    PCIDevice(u8 bus, u8 slot, u8 func);
    // 0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |           vendor ID           |           device ID           |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |            command            |             status            |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |  revision ID  |    prog IF    |    subclass   |     class     |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |cache line size| latency timer |   header type |      bist     |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    PCI_Address_t address{};
    u16 vendor;
    // Generic header values
    u16 vendor_id() const;
    u16 device_id() const;
    u16 command() const;
    u16 status() const;
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

};

void PCI_list();

#include "types.h"

#endif //PCI_H
