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

    u16 vendor_id();
    u16 device();
    u8 sub_class();
    u8 main_class();
    u8 header_type();
    u8 intr_line();
    u32 bar(size_t n);
    u16 subsystem();
    void log_format();

private:
    u8 config_readb(u8 offset);
    u16 config_readw(u8 offset);
    u32 config_readl(u8 offset);

};

void PCI_list();

#include "types.h"

#endif //PCI_H
