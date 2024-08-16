//
// Created by artypoole on 30/07/24.
//

#include "PCI.h"

#include "Serial.h"

#include "ports.h"
#include "stdlib.h"
using namespace std;


// enum dev_class
// {
//     Unclassified,
//     MassStorage,
//     Network,
//     Display,
//     Multimedia,
//     Memory,
//     Bridge,
//     SimpleCommController
// };


char dev_types[255][32] = {
    "Unclassified",
    "Mass Storage",
    "Network",
    "Display",
    "Multimedia",
    "Memory",
    "Bridge",
    "SimpleCommController",
    "Base System Peripheral",
    "Input Device Controller",
    "Docking Station",
    "Processor",
    "Serial Controller",
    "Wireless Controller",
    "Intelligent Controller",
    "Signal Processing Controller",
    "Processing Accelerator",
    "Non-Essential Instrumentation",
    "0x3F (Reserved)", "0x15", "0x16", "0x17", "0x18", "0x19", "0x1A", "0x1B", "0x1C", "0x1D", "0x1E", "0x20", "0x21", "0x22", "0x23", "0x24",
    "0x25", "0x26", "0x27", "0x28", "0x29", "0x2A", "0x2B", "0x2C", "0x2D", "0x2E", "0x30", "0x31", "0x32", "0x33", "0x34", "0x35", "0x36",
    "0x37", "0x38", "0x39", "0x3A", "0x3B", "0x3C", "0x3D", "0x3E", "Co-Processor", "0xFE (Reserved)", "0x42", "0x43", "0x44", "0x45", "0x46", "0x47", "0x48", "0x49", "0x50", "0x51", "0x52", "0x53", "0x54", "0x55", "0x56", "0x57", "0x58", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "Unassigned"
};


PCIDevice::PCIDevice(const u8 bus, const u8 slot, const u8 func)
{
    address = PCI_Address_t{0, func, slot, bus, 0, 1};
    header = populate_values();
    vendor = vendor_id();
}

PCI_header_t PCIDevice::populate_values() const
{
    PCI_header_t local_header{};
    local_header.reg0 = config_read_register(0x0);
    if (local_header.vendor_id != 0xFFFF) // If the device does not exist, returns 0xFFFF
    {
        local_header.reg1 = config_read_register(0x4);
        local_header.reg2 = config_read_register(0x8);
        local_header.reg3 = config_read_register(0xC);
        local_header.reg4 = config_read_register(0x10);
        local_header.reg5 = config_read_register(0x14);
        local_header.reg6 = config_read_register(0x18);
        local_header.reg7 = config_read_register(0x1C);
        local_header.reg8 = config_read_register(0x20);
        local_header.reg9 = config_read_register(0x24);
        local_header.regA = config_read_register(0x28);
        local_header.regB = config_read_register(0x2C);
        local_header.regC = config_read_register(0x30);
        local_header.regD = config_read_register(0x34);
        local_header.regE = config_read_register(0x38);
        local_header.regF = config_read_register(0x3C);
    }
    if (header_type() == 2)
    {
        local_header.reg10 = config_read_register(0x40);
        local_header.reg11 = config_read_register(0x44);
    }
    return local_header;
}

u16 PCIDevice::vendor_id() const
{
    return header.vendor_id;
}

u16 PCIDevice::device_id() const
{
    return header.device_id;
}

u8 PCIDevice::sub_class() const
{
    return header.subclass;
}


u8 PCIDevice::class_code() const
{
    return header.class_code;
}

u8 PCIDevice::header_type() const
{
    return header.header_type;
}


u32 PCIDevice::bar(u8 n) const
{
    if ((header_type() == 1 and n > 1) or (header_type() == 2))
    {
        LOG("Tried to access BAR outside of valid range for this header type. There are 6 BARs for type_0, 2 for type_1 and 0 for type_2.");
        return 0;
    }
    switch (n)
    {
    case 0:
        return header.reg4;
    case 1:
        return header.reg5;
    case 2:
        return header.reg6;
    case 3:
        return header.reg7;
    case 4:
        return header.reg8;
    case 5:
        return header.reg9;
    default:
        LOG("Tried to access BAR outside of valid range. ");
        return 0;
    }
}

u16 PCIDevice::subsystem_id() const
// only for header_type == 0
{
    if (header_type() == 0)
    {
        return static_cast<u16>(header.regB >> 16);
    }
    else return 0;
}

u32 PCIDevice::expansion_RBA() const
{
    switch (header_type())
    {
    case 0:
        return header.regC;
    case 1:
        return header.regE;
    default: return 0;
    }
}

u8 PCIDevice::prog_if() const
{
    return header.prog_IF;
}

void PCIDevice::log_format()
{
    LOG("PCI device. Bus: ", static_cast<u16>(address.components.bus), " slot: ", static_cast<u16>(address.components.slot), " function: ", static_cast<u16>(address.components.function));

    LOG("class: ", dev_types[class_code()], " subclass: ", static_cast<u16>(sub_class()), " vendor id: ", vendor_id());
}

u16 PCIDevice::command() const
{
    return header.command;
}

u16 PCIDevice::status() const
{
    return header.status;
}

u8 PCIDevice::rev_id() const
{
    return header.revision_id;
}

u8 PCIDevice::cache_line_size() const
{
    return header.cache_line_size;
}

u8 PCIDevice::latency_timer() const
{
    return header.latency_timer;
}

u8 PCIDevice::bist() const
{
    return header.BIST;
}

u8 PCIDevice::capablilites() const
{
    switch (header_type())
    {
    case 0:
    case 1:
        return static_cast<u8>(header.regD);
    default:
        LOG("Invalid field 'capabilities' for header type: ", header_type());
        return 0;
    }
}

u8 PCIDevice::interrupt_line() const
{
    return header.interrupt_line;
}

u8 PCIDevice::interrupt_pin() const
{
    return header.interrupt_pin;
}

// Header type 0 (device) only.
u32 PCIDevice::card_cis() const
{
    if (header_type() != 0)
    {
        LOG("Invalid field 'card_cis' for header type: ", header_type());
        return 0;
    }
    return header.regA;
}

u16 PCIDevice::subsystem_vendor_id() const
{
    switch (header_type())
    {
    case 0:
        return static_cast<u16>(header.regB);
    case 2:
        return header.subsystem_vendor_id; // only populated specifically if header type is 2
    default:
        LOG("Invalid field 'subsystem vendor id' for header type: ", header_type());
        return 0;
    }
}

// u8 PCIDevice::config_readb(const u8 offset)
// {
//     const PCI_Address_t offset_address = {static_cast<u8>(offset), address.components.function, address.components.slot, address.components.bus, 0, 1};
//     outd(PCI_CONFIG_ADDRESS, offset_address.address);
//
//     return static_cast<u8>(ind(PCI_CONFIG_DATA));
// }
//
// u16 PCIDevice::config_readw(const u8 offset)
// {
//     const PCI_Address_t offset_address = {static_cast<u8>(offset), address.components.function, address.components.slot, address.components.bus, 0, 1};
//     outd(PCI_CONFIG_ADDRESS, offset_address.address);
//
//     return static_cast<u16>(ind(PCI_CONFIG_DATA));
// }

u32 PCIDevice::config_read_register(const u8 offset) const
{
    const PCI_Address_t offset_address = {static_cast<u8>(offset), address.components.function, address.components.slot, address.components.bus, 0, 1};
    outd(PCI_CONFIG_ADDRESS, offset_address.address);
    // const u32 tmp = (ind(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF;
    return ind(PCI_CONFIG_DATA);
}


void PCI_list()
{
    for (u8 slot = 0; slot < 31; slot++)
    {
        {
            for (u8 func = 0; func < 7; func++)
            {
                if (auto dev = PCIDevice(0, slot, func); dev.vendor_id() != 0xffff)
                {
                    dev.log_format();
                }
            }
        }
    }
}

// template<typename T>
// T PCI_read_config(T return_type, u8 offset)
// {
//
//     const PCI_Address_t address = {offset, func, slot, bus, 0, 1};
//     outd(PCI_CONFIG_ADDRESS, )
//     switch( size_of(return_type))
//     {
//     case 1:
//         {
//
//         }
//     }
// }

// pub inline fn config_read(self: PciDevice, comptime size: type, comptime offset: u8) size {
//     // ask for access before reading config
//     x86.outd(PCI_CONFIG_ADDRESS, @bitCast(self.address(offset)));
//     switch (size) {
//         // read the correct size
//         u8 => return x86.inb(PCI_CONFIG_DATA),
//         u16 => return x86.inw(PCI_CONFIG_DATA),
//         u32 => return x86.ind(PCI_CONFIG_DATA),
//         else => @compileError("pci only support reading up to 32 bits"),
//     }
// }
