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
            u32 offset : 8;
            u32 function : 3;
            u32 slot : 5;
            u32 bus : 8;
            u32 reserved : 7;
            u32 enabled : 1;
        } components;

        u32 address;
    };
};

struct PCI_header_t
{
    // Generic to all header types:
    //
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
    union
    {
        struct
        {
            u16 vendor_id;
            u16 device_id;
        };

        u32 reg0;
    };

    union
    {
        struct
        {
            u16 command;
            u16 status;
        };

        u32 reg1;
    };

    union
    {
        struct
        {
            u8 revision_id;
            u8 prog_IF;
            u8 subclass;
            u8 class_code;
        };

        u32 reg2;
    };

    union
    {
        struct
        {
            u8 cache_line_size;
            u8 latency_timer;
            u8 header_type;
            u8 BIST;
        };

        u32 reg3;
    };

    // If header type 0:
    u32 reg4; // BAR 0
    u32 reg5; // BAR 1
    u32 reg6; // BAR 2
    u32 reg7; // BAR 3
    u32 reg8; // BAR 4
    u32 reg9; // BAR 5
    u32 regA;
    u32 regB;
    u32 regC;
    u32 regD;
    u32 regE;

    union
    {
        struct
        {
            u8 interrupt_line;
            u8 interrupt_pin;
            u16 half_reg_F;
        };

        u32 regF;
    };

    // Only valid for device 2
    union
    {
        struct
        {
            u16 subsystem_device_id;
            u16 subsystem_vendor_id;
        };

        u32 reg10;
    };

    union
    {
        u32 legacy_mode_address;
        u32 reg11;
    };
};

#endif //PCI_H
