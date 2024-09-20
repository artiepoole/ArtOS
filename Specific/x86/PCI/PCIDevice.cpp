//
// Created by artypoole on 30/07/24.
//

#include "PCIDevice.h"

#include "logging.h"

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

size_t pci_device_count = 0;
PCIDevice device_list[32];

PCIDevice::PCIDevice()
{
    vendor = 0xFFFF;
}

PCIDevice::PCIDevice(const u8 bus, const u8 slot, const u8 func)
{
    address = PCI_Address_t{0, func, slot, bus, 0, 1};
    header = populate_values();
    vendor = vendor_id();
}

PCI_Address_t PCIDevice::get_address()
{
    return address;
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
    if ((header_type() == 1 && n > 1) || (header_type() == 2))
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
    return 0;
}

u8 PCIDevice::min_grant() const
{
    // todo: check if this is a good error value
    if (header_type() != 0) return 0xFF;
    return header.regF >> 16 & 0xFF;
}

u8 PCIDevice::max_latency() const
{
    // todo: check if this is a good error value
    if (header_type() != 0) return 0xFF;
    return header.regF >> 24 & 0xFF;
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

    LOG("class: ", dev_types[class_code()], " subclass: ", sub_class(), " vendor id: ", vendor_id());
}

u16 PCIDevice::get_command()
{
    header.reg1 = config_read_register(0x04);
    return header.command;
}

u16 PCIDevice::set_command_bit(u8 bit, bool value)
{
    if (value) header.command |= 0x1 << bit;
    else header.command &= (~0x1) << bit;

    header.reg1 = config_write_register(0x4, header.reg1);

    return header.command;
}


u16 PCIDevice::get_status()
{
    header.reg1 = config_read_register(0x04);
    return header.status;
}

u16 PCIDevice::set_status_bit(u8 bit, bool value)
{
    if (value) header.status |= 0x1 << bit;
    else header.status &= (~0x1) << bit;
    header.reg1 = config_write_register(0x04, header.reg1);
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

u32 PCIDevice::config_read_register(const u8 offset) const
{
    const PCI_Address_t offset_address = {offset, address.components.function, address.components.slot, address.components.bus, 0, 1};
    outd(PCI_CONFIG_ADDRESS, offset_address.address);
    // const u32 tmp = (ind(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF;
    return ind(PCI_CONFIG_DATA);
}

u32 PCIDevice::config_write_register(const u8 offset, const u32 data) const
{
    const PCI_Address_t offset_address = {
        offset,
        address.components.function,
        address.components.slot,
        address.components.bus,
        0,
        1
    };

    // set the data
    outd(PCI_CONFIG_ADDRESS, offset_address.address);
    outd(PCI_CONFIG_DATA, data);
    // read it back
    outd(PCI_CONFIG_ADDRESS, offset_address.address);
    return ind(PCI_CONFIG_DATA);
}


void PCI_populate_list()
{
    for (u8 slot = 0; slot < 31; slot++)
    {
        for (u8 func = 0; func < 7; func++)
        {
            if (auto dev = PCIDevice(0, slot, func); dev.vendor_id() != 0xFFFF)
            {
                device_list[pci_device_count++] = dev;
                dev.log_format();
            }
        }
    }
}

PCIDevice* PCI_get_IDE_controller()
{
    if (pci_device_count == 0) PCI_populate_list();
    for (size_t i = 0; i < pci_device_count; i++)
    {
        if (device_list[i].class_code() == 1 && device_list[i].sub_class() == 1) return &device_list[i];
    }
    LOG("No IDE controller detected.");
    return nullptr;
}
