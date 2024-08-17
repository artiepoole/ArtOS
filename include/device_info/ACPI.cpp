//
// Created by artypoole on 16/08/24.
//

#include "ACPI.h"
#include "Serial.h"

full_madt_t madt ={};

/*
 *  Must start at the original MADT entry point with signature APIC as this is the only place that describes the total length of the APIC entries.
 */
full_madt_t* populate_madt(const u32 madt_location)
{
    madt.madt_stub = reinterpret_cast<MADT_stub*>(madt_location);
    const auto start_address = reinterpret_cast<u8*>(madt_location);
    u8* current_address = start_address + 0x2C; // first tag is at this offset
    size_t iso_index = 0;
    const u32 size = reinterpret_cast<MADT_stub*>(madt_location)->h.Length;
    while (current_address < start_address + size)
    {
        const auto tag = reinterpret_cast<apic_madt_entry_header*>(current_address);
        switch (tag->type)
        {
        case 0:
            madt.local_apic = *reinterpret_cast<local_apic_entry*>(current_address);
            LOG("Found local APIC with ID: ", static_cast<u16>(madt.local_apic.apic_id));
            break;
        case 1:
            madt.io_apic = *reinterpret_cast<io_apic_entry*>(current_address);
            LOG("Found IO APIC with ID: ", static_cast<u16>(madt.io_apic.io_apic_id));
            break;
        case 2:
            madt.iso[iso_index] = *reinterpret_cast<io_apic_interrupt_source_override_entry*>(current_address);
            LOG("Found IO APIC ISO with source: ", static_cast<u16>(madt.iso[iso_index].irq_source));
            iso_index ++;
            break;
        case 3:
            LOG("io_apic_NMI_source_entry");
            break;
        case 4:
            LOG("local_apic_NMI_entry");
            break;
        case 5:
            LOG("local_apic_source_address_override_entry");
            break;
        default:
            LOG("Unknown apic entry type: ", static_cast<u16>(tag->type));
        }
        current_address += tag->length;
    }

    // madt = reinterpret_cast<full_madt*>(reinterpret_cast<u8*>(madt_location));
    return &madt;
    LOG("MADT popualted");
}