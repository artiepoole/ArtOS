//
// Created by artypoole on 16/08/24.
//

#ifndef ACPI_H
#define ACPI_H

#include "types.h"


struct ACPIGenericAddressStructure
{
    u8 AddressSpace;
    u8 BitWidth;
    u8 BitOffset;
    u8 AccessSize;
    u64 Address;
};

struct ACPISDTHeader
{
    char Signature[4];
    u32 Length;
    u8 Revision;
    u8 Checksum;
    char OEMID[6];
    char OEMTableID[8];
    u32 OEMRevision;
    u32 CreatorID;
    u32 CreatorRevision;
};

struct FADT
{
    ACPISDTHeader h;
    u32 FirmwareCtrl;
    u32 Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    u8 Reserved;

    u8 PreferredPowerManagementProfile;
    u16 SCI_Interrupt;
    u32 SMI_CommandPort;
    u8 AcpiEnable;
    u8 AcpiDisable;
    u8 S4BIOS_REQ;
    u8 PSTATE_Control;
    u32 PM1aEventBlock;
    u32 PM1bEventBlock;
    u32 PM1aControlBlock;
    u32 PM1bControlBlock;
    u32 PM2ControlBlock;
    u32 PMTimerBlock;
    u32 GPE0Block;
    u32 GPE1Block;
    u8 PM1EventLength;
    u8 PM1ControlLength;
    u8 PM2ControlLength;
    u8 PMTimerLength;
    u8 GPE0Length;
    u8 GPE1Length;
    u8 GPE1Base;
    u8 CStateControl;
    u16 WorstC2Latency;
    u16 WorstC3Latency;
    u16 FlushSize;
    u16 FlushStride;
    u8 DutyOffset;
    u8 DutyWidth;
    u8 DayAlarm;
    u8 MonthAlarm;
    u8 Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    u16 BootArchitectureFlags;

    u8 Reserved2;
    u32 Flags;

    // 12 byte structure; see below for details
    ACPIGenericAddressStructure ResetReg;

    u8 ResetValue;
    u8 Reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    u64 X_FirmwareControl;
    u64 X_Dsdt;

    ACPIGenericAddressStructure X_PM1aEventBlock;
    ACPIGenericAddressStructure X_PM1bEventBlock;
    ACPIGenericAddressStructure X_PM1aControlBlock;
    ACPIGenericAddressStructure X_PM1bControlBlock;
    ACPIGenericAddressStructure X_PM2ControlBlock;
    ACPIGenericAddressStructure X_PMTimerBlock;
    ACPIGenericAddressStructure X_GPE0Block;
    ACPIGenericAddressStructure X_GPE1Block;
};


struct MADT
{
    ACPISDTHeader h;
    u32 apicAddress;
    u32 flags;
};

struct RSDT
{
    ACPISDTHeader h;
    FADT* facp;
    MADT* madt;
};

struct apic_madt_entry_header
{
    u8 type;
    u8 length;
};


//tpye 0
struct local_apic_entry
{
    apic_madt_entry_header h;
    u8 apic_processor_id;
    u8 apic_id;
    u32 flags;
};

// type 1
struct io_apic_entry
{
    apic_madt_entry_header h;
    u8 io_apic_id;
    u8 reserved;
    u32 physical_address;
    u32 global_system_interrupt_base;
};

//type 2
struct io_apic_interrupt_source_override_entry
{
    apic_madt_entry_header h;
    u8 bus;
    u8 irq_source;
    u32 global_system_interrupt;
    u16 flags;
};

// type 3
// non maskable interrupt : NMI
struct io_apic_NMI_source_entry
{
    apic_madt_entry_header h;
    u8 nmi_source;
    u8 reserved;
    u16 flags;
    u32 global_system_interrupt;
};

//type 4
// NMI = non maskable interrupt
struct local_apic_NMI_entry
{
    apic_madt_entry_header h;
    u8 acpi_proessor_id;
    u16 flags;
    u8 lint;
};

struct local_apic_source_address_override_entry
{
    apic_madt_entry_header h;
    u16 reserved;
    u64 local_apic_address;
};

struct full_madt
{
    MADT *madt;
    u64 pad1;
    u32 pad2;
    local_apic_entry local_apic;
    io_apic_entry io_apic;
    io_apic_interrupt_source_override_entry iso[5];
};


void populate_madt(u32 madt_location);

#endif //ACPI_H
