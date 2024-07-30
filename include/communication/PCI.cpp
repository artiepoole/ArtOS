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

char dev_types[][32] = {
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
    vendor = vendor_id();
}

u16 PCIDevice::vendor_id()
{
    return config_readw(0x0);
}

u16 PCIDevice::device()
{
    return config_readw(0x2);
}

u8 PCIDevice::sub_class()
{
    return config_readb(0xA);
}


u8 PCIDevice::main_class()
{
    u8 ret = config_readb(0xB);
    // auto& log = Serial::get();
    // log.log("ret", static_cast<u16>(ret));
    return ret;
}

u8 PCIDevice::header_type()
{
    return config_readb(0xE);
}

u8 PCIDevice::intr_line()
{
    return config_readb(0x3C);
}

u32 PCIDevice::bar(size_t n)
{
    return config_readl(0x10 + 4 * n);
}

u16 PCIDevice::subsystem()
// only for header_type == 0
{
    return config_readw(0x2e);
}

void PCIDevice::log_format()
{
    auto& log = Serial::get();
    log.log("PCI device. Bus: ", static_cast<u16>(address.components.bus), " slot: ", static_cast<u16>(address.components.slot), " function: ", static_cast<u16>(address.components.function));

    log.log("class: ", dev_types[main_class()], " subclass: ", static_cast<u16>(sub_class()), " vendor id: ", vendor_id());
}

u8 PCIDevice::config_readb(const u8 offset)
{
    const PCI_Address_t offset_address = {static_cast<u8>(offset), address.components.function, address.components.slot, address.components.bus, 0, 1};
    outd(PCI_CONFIG_ADDRESS, offset_address.address);

    return static_cast<u8>(ind(PCI_CONFIG_DATA));
}

u16 PCIDevice::config_readw(const u8 offset)
{
    const PCI_Address_t offset_address = {static_cast<u8>(offset), address.components.function, address.components.slot, address.components.bus, 0, 1};
    outd(PCI_CONFIG_ADDRESS, offset_address.address);

    return static_cast<u16>(ind(PCI_CONFIG_DATA));
}

u32 PCIDevice::config_readl(const u8 offset)
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
